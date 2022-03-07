open Core_kernel;;
open Spec_definition;;
open Spec_utils;;
open Builtin_types;;
open Options;;

(* So this is a shit-ly done pass.  The concept is pretty expansive, but
it's going to need good heuristics to scale well.  It doesn't take into
account several cases that it probably should.

Anyway, that doesn't matter much, it should be easy to extend as appropriate,
but it is far from complete now.  *)

(* The main challenge of this pass is avoiding too much blowup.
	They key here is likely to end up to be using heuristics
	inferred from the user code -- there is plenty of work on inferring
	structs.    However, for now, we dont use that --- the all-or-nothing
	heuristic seems to be sufficient for most user code (that is,
	either all e.g. float* arrays are actually f32x2 arrays, or none
	are.  *)
let infer_structs_on_typemap options typemap variables =
    let pairs, used_types = List.unzip (List.map variables (fun variable ->
        let current_type = Hashtbl.find_exn typemap.variable_map variable in
        match current_type with
        | Array(t, d) ->
				(* TODO -- insert a real struct trheshold.  *)
                if (is_constant_dimension_variable d) && (dimension_constant_less_than d (Dimension(DimConstant(4)))) then
                    (* Allocate this as an n-struct, provided n < struct_threshold *)
                    (* If we do this, we reduce to a single pointer.  *)
                    (* TODO *)
                    [(variable, Array(t, d))], []
                else
                    (* FFT-specific heuristic: try making this a 2-tuple.  *)
					let newdims =
						match d with
						(* If we are infering a 2x struct on this variable, we'll need to reduce
						it by a factor of two.  *)
						(* Note that we don't use the newdim
						if the inference fails.  *)
						| Dimension(DimConstant(c)) -> [Dimension(DimConstant(c / 2)); Dimension(DimConstant(c))]
						| Dimension(DimVariable(v, DimEqualityRelation)) -> [
							(* We are infering structs below that
							affect the effective length of the array
							by a factor of two.  So,
							depending on how ingrained this infered
							type is in user code, it may or may
							not affect the struct by a factor of two. *)
							Dimension(DimVariable(v, DimEqualityRelation)); Dimension(DimVariable(v, DimDivByRelation(2)))
						]
						(* TODO -- completeness would suggest that we need the same
						expansion for Po2 lengths. *)
						| other -> [other]
					in
                    (
                    match t with
                    | Float16 ->
                            (* TODO *)
                            [(variable, Array(t, d))], []
                    | Float32 ->
							(List.map newdims (fun newdim -> (variable, Array(two_float32_type, newdim)))) @
                            [(variable, Array(t, d))], [two_float32_type]
                    | Float64 ->
							(List.map newdims (fun newdim -> (variable, Array(two_float64_type, newdim)))) @
                            [(variable, Array(t, d))], [two_float64_type]
                    | other ->
                            (* TODO -- support ints here? *)
                            [(variable, Array(t, d))], []
                    )
        | Struct(name) as other ->
                (* We don't handle structs here *)
                [(variable, other)], []
        | other ->
                [(variable, other)], []
    )
    ) in
    pairs, (List.concat used_types)

(* Create a new typemap from the old one, copying the variables
def'ed in copyvars and setting the variables specified in expandvars
as specified.  *)
let construct_typemap_from_assignments oldtmap copyvars expandvars =
    let newtbl = Hashtbl.create (module String) in
    let _ = List.map copyvars (fun cvar ->
        Hashtbl.add newtbl cvar (Hashtbl.find_exn oldtmap cvar)
    ) in
    let _ = List.map expandvars (fun (n, t) ->
        Hashtbl.add newtbl n t
    ) in
    newtbl

let expand_and_preserve original_typemap (variables: string list) vpairs =
    let preserve_vars = Utils.set_difference Utils.string_equal (Hashtbl.keys original_typemap) variables in
    let new_typemaps = Utils.cross_product vpairs in
    List.map new_typemaps (fun vassignments ->
        construct_typemap_from_assignments original_typemap preserve_vars vassignments
    )

let rec get_array_types_from_type t wasarray =
	match t with
	| Array(x, d) -> get_array_types_from_type x true
	| other ->
			if wasarray then
				Some(other)
			else
				None

let get_array_types tmap n =
	let value = Hashtbl.find_exn tmap n in
	get_array_types_from_type value false

(* TODO -- this should support classmaps too.  *)
let all_or_nothing_approximation variable_sets tmap =
	List.for_all variable_sets (fun variables ->
		let atypes = List.filter_map variables (get_array_types tmap) in
		let all_infered =
			List.for_all atypes (fun atype ->
				match atype with
				(* These are the types we use the domain-specific
				   type complex struct inference on.  *)
				| Float16 -> false
				| Float32 -> false
				| Float64 -> false
				| _ -> true
			)
		in
		(* TODO -- this might not work with more generic
		struct inference.  *)
		let all_uninfered =
			List.for_all atypes (fun atype ->
				match atype with
				| Struct(name) ->
						(* Check if this type is one
						of the infered structs.  *)
						(* This check may need to be changed
						up if we introduce another way of
						introducing builtin structs.  *)
						if is_builtin_struct name then
							false
						else
							true
				| _ -> true
			)
		in
		(* Each variable set can have different properties
		(expected use is that one variable set will correspond
		to the 'from' function, and one will correspond to 
		the 'to' function. *)
		all_infered || all_uninfered
	)

let apply_structure_heuristics variable_sets tmaps =
	List.filter tmaps (fun tmap ->
		(* Apply the all-or-nothing approxmation, i.e. the idea
		that we aren't just looking at a single isolated type,
		but rather a programmer who has made a representation
		decision.  *)
		(* TODO -- support classmaps *)
		all_or_nothing_approximation variable_sets tmap.variable_map
	)

(* TODO --- support classmaps *)
let infer_structure options typemap variables =
    let () = if options.debug_infer_structs then
        Printf.printf "Beggingin infer structs\n"
    else ()
    in
    (* Get the expansions of the formals.  *)
    let typemap_expansions, builtins = infer_structs_on_typemap options typemap variables in
    (* Insert the classmap generation e.g. here.  *)
    let new_variable_maps = expand_and_preserve typemap.variable_map variables typemap_expansions in
    (* Add any inferred types to the classmap.  *)
    let new_classmap = clone_classmap typemap.classmap in
    let _ = List.map builtins (fun builtin ->
        let name = synth_type_to_string builtin in
        match Hashtbl.find new_classmap name with
        | Some(_) -> () (* TODO -- should we do an assert here? *)
        | None ->
                let _ = Hashtbl.add new_classmap name (builtin_struct_from_name name) in
                ()
    ) in
	let result = List.map new_variable_maps (fun vmap ->
        {
            variable_map = vmap;
			classmap = new_classmap;
			(* The alignment map is unchanged, so we don't need
			to do anything about it.  *)
			alignment_map = typemap.alignment_map;
			original_typemap = Some(typemap);
        }
    ) in
	let variable_sets = [ variables ] in
    let filtered_results = apply_structure_heuristics variable_sets result in
	let () = if options.debug_infer_structs then
        let () = Printf.printf "Total number of structs before heuristitcs is %d\n" (List.length result) in
        let () = Printf.printf "Total number of resulting typemaps is %d\n" (List.length filtered_results) in
        ()
    else ()
    in
    filtered_results
