open Core_kernel;;
open Spec_definition;;
open Spec_utils;;
open Skeleton_definition;;
open Skeleton_utils;;
open Utils;;
open Options;;
open Builtin_conversion_functions;;

exception SkeletonFilter of string

let filter_dimvar_set dms = 
    (* We don't need to have the same dimension with
    the same source var for multiple targets.  *)
	(* Same thing with the to varaibles :) *)
    let flookup = Hashtbl.create (module String) in
	let tlookup = Hashtbl.create (module String) in
	let tbllookup_set = fun (tbl, n) ->
		let str = (name_reference_to_string n) in
		let already_mapped = Hashtbl.find tbl str in
		let () = Hashtbl.set tbl str true in
		match already_mapped with
			| Some(n) -> false
			| None -> true
	in
    (* let () = Printf.printf "Length of input dms is %d\n" (List.length dms) in *)
    let filtered = List.filter dms (fun dm ->
        match dm with
        | DimvarOneDimension(VarMatch(f, t, mode)) -> (
            let result = (tbllookup_set (flookup, f)) && (tbllookup_set (tlookup, t)) in
            (* let () = Printf.printf "Inspecting variable %s, with results %b\n" (dimvar_mapping_to_string dm) (result) in *)
            result
		)
        (* TODO -- do we also need to do some filtering here? *)
		| DimvarOneDimension(ConstantMatch(f)) ->
                (* let () = Printf.printf "Looking at a constant, %s\n" (dimvar_mapping_to_string dm) in *)
                true
    )
    in
    let unique_dimvar_set = Utils.remove_duplicates dimvar_equal filtered in
    unique_dimvar_set

(* No variables assigned from more than once.  *)
let no_multiple_cloning_check (skel: skeleton_type_binding) =
    let assigned_from = Hashtbl.create (module String) in
    let () = ignore(
        List.map skel.bindings (fun bind ->
            List.map bind.fromvars_index_nesting (fun vbind ->
                match vbind with
                | AssignConstant(c) -> ()
                | AssignVariable(indnest) ->
                    (* Get the indnesst string *)
                    let indnest_string = name_reference_to_string (StructName(indnest)) in
                    let existing_count = Hashtbl.find assigned_from indnest_string in
                    let newvar = match existing_count with
                    | None -> 1
                    | Some(x) -> x + 1
                    in
                    let _ = Hashtbl.set assigned_from indnest_string newvar in
                    ()
            )
        )
    ) in
    (* make sure that all the variables are used only once.
    Could update this to allow variables being used in
    multidefs to be used more than once, but that is a task
    for anohter day.  *)
    List.for_all (Hashtbl.data assigned_from) (fun v -> v = 1)

let rec build_whole_arnm_internal arnms =
	match arnms with
	| [] -> []
	| [x] -> [[x]]
	| x :: xs ->
			let sub_build = build_whole_arnm_internal xs in
			[x] :: (prepend_all x sub_build)

let is_in_api_list var varlist =
    let checkvar v =
        (* OK, so this is a super stupid way of doing this, but
           it is dirty and quick.  The problem is the granularity of
           the API liveout list, which is currently low (whole-variable).  *)
        (String.equal var v) || (String.is_prefix ~prefix:(var ^ ".") v)
    in
    List.exists varlist checkvar

(* Compute equivalence classes between variables.  For example, we might
   do:
       api_len = input_len
       ...
       output_len = api_len
    in which case, api_len, output_len and input_len are an equivalence
    class.

    Note that there are some edge cases to deal with.  Most notably,
    we can only keep track of things that are not "live out" from the API
    --- of course, we should be careful with that concept, e.g. if the API
    takes a scratch workspace.

    Anyway, right now, we don't handle APIs with things like scratch
    spaces that are written to, but are arguably not liveout. *)
(* Note that equivalence classes may not be complete (see API spec liveout
part), but they will be correct.  *)
let compute_equivalence_classes options api pre_skel post_skel =
    let equivalence_class_map = Hashtbl.create (module String) in
    let _ = List.map (pre_skel.flat_bindings @ post_skel.flat_bindings) (fun b ->
        let conversion_function = b.conversion_function in
        let fromvar = match b.fromvars_index_nesting with
        | [] -> []
        (* Note we are assuming that assignments are from single variables only.  *)
        | [fromv] -> (match fromv with
                | AssignConstant(_) -> []
                | AssignVariable(v) -> [name_reference_to_string (name_reference_list_concat v)]
        )
        | _ -> raise  (SkeletonFilter "Unexpected multiple var ")
        in
        let tovar = name_reference_to_string (name_reference_list_concat b.tovar_index_nesting) in
        (* Build the actual hashmap.  *)
        (* Note that we only cover identity conversions, although
        equivalence could exist in some other cases (? would it be useful to find it though?) *)
        let build_equivalence_class =
            if (is_in_api_list tovar api.liveout) || (List.exists fromvar (fun f -> is_in_api_list f api.liveout)) then
                (* If either variable is in the liveout, just don't build the equivalence
                    class --- liveout means that the variable could be changed by the function.
                    It also means this equivalence class isn't complete, but we aim
                    for a correct equivalence class instead. *)
                false
            else
                true
        in
        if build_equivalence_class && (is_identity_conversion conversion_function) then
            let _ =
                List.map fromvar (fun f ->
                    let result = tovar:: (match Hashtbl.find equivalence_class_map f with
                    | None -> []
                    | Some(xs) -> xs
                    )
                    in
                    let _ = Hashtbl.set equivalence_class_map f result in
                    ()
                )
            in
            (* This is meant to compute equivalence classes --- but not clear what this means if
                you have more than one variable.  *)
            let () = assert ((List.length fromvar) <= 1) in
            let tos = match Hashtbl.find equivalence_class_map tovar with
            | None -> fromvar
            | Some(xs) -> fromvar @ xs
            in
            let _ = Hashtbl.set equivalence_class_map tovar tos in
            ()
        else ()
    ) in
    equivalence_class_map

let build_whole_arnm arnms =
	let arnms = build_whole_arnm_internal arnms in
	List.map arnms (fun anm -> StructName(anm))

(* Check that we don't use multiple length variables into the same
   array.  I think that this should probably be made more stringent,
   e.g. to prefer using fewer length variables over more lenght
   variables, and could definitely do with some variable name help.
   *)
let length_variable_compatability (skel: flat_skeleton_binding) =
	let lenvars_for = Hashtbl.create (module String) in
	(* let () = Printf.printf "Staritng new interation\n" in*)
	List.for_all skel.flat_bindings (fun bind ->
		let built_up_arnms =
			build_whole_arnm bind.tovar_index_nesting in
		List.for_all (truncate_zip built_up_arnms bind.valid_dimensions) (fun (arnm, dimvar) ->
			let arnm_so_far_str = (name_reference_to_string arnm) in
			(* let () = Printf.printf "Arnm is %s\n" arnm_so_far_str in *)
			let v_used = Hashtbl.find lenvars_for arnm_so_far_str in
			(* let () = Printf.printf "VUsed is %s\n" (dimvar_mapping_to_string dimvar) in *)
			let _ = Hashtbl.set lenvars_for arnm_so_far_str dimvar in
			match v_used with
			| None ->
					true
			| Some(other) ->
					dimvar_equal dimvar other
		)
	)

let no_multiple_lengths options apispec tbl pre_binding_list post_binding_list =
	let () = if options.debug_skeleton_multiple_lengths_filter then
		let () = Printf.printf "Trying to check if  has multiple lengths: %s\n%s\n" (flat_skeleton_type_binding_to_string pre_binding_list) (flat_skeleton_type_binding_to_string post_binding_list) in
		()
	else ()
	in
    let equivalence_classes = compute_equivalence_classes options apispec pre_binding_list post_binding_list in
    let result = List.for_all (pre_binding_list.flat_bindings @ post_binding_list.flat_bindings) (fun fb ->
		let tname_so_far = ref [] in
		let fname_so_far = ref [] in
        let fromvars = match fb.fromvars_index_nesting with
        | [] -> (* This is just a def --- we still want to consider the tovar
                    but we need to zip right for the next section.  So just spoof it :) *)
        (* This is literally a teribly hack. *)
        (* Should not escape this method at all *)
            [AssignConstant(Int64V(0))]
		| other -> other
        in
		List.for_all fromvars (fun fromvar ->
			let fvar_contents = match fromvar with
			| AssignConstant(_) ->
					[None]
			| AssignVariable(v) ->
					List.map v (fun v -> Some(v))
			in
		List.for_all (truncate_zip (extend_zip fb.tovar_index_nesting fvar_contents) fb.valid_dimensions) (fun ((t, f), dimvar) ->
			let () = tname_so_far := t :: !tname_so_far in
			let has_tbinds = Hashtbl.find tbl (name_reference_list_to_string !tname_so_far) in
			let has_fbinds = match f with
			| Some(v) ->
					let () = fname_so_far := v :: !fname_so_far in
					let result = Hashtbl.find tbl (name_reference_list_to_string !fname_so_far) in
					(* Set the used dimensions for the fvar.  *)
					let _ = Hashtbl.set tbl (name_reference_list_to_string !fname_so_far) dimvar in
					result
			(* If we are assigning a constant, then we don't have to bother. *)
			| None -> None
			in
			let () = if options.debug_skeleton_multiple_lengths_filter then
				let () = Printf.printf "Considering fromvar %s and tovar %s\n" (name_reference_list_to_string !fname_so_far) (name_reference_list_to_string !tname_so_far) in
				let () = Printf.printf "Under dim %s\n" (dimvar_mapping_to_string dimvar) in
				()
			else ()
			in
			let tbinds_valid =
				match has_tbinds with
				| None ->
						true
				| Some(other) ->
						(* We could use a more loose sense of equality
						here, e.g. one that is closer to "could equal",
						but this seems more sensible *)
						let () = if options.debug_skeleton_multiple_lengths_filter then
							let () = Printf.printf "Comparing (tbinds) %s and %s\n" (dimvar_mapping_to_string dimvar) (dimvar_mapping_to_string other) in
							() else ()
						in
						dimvar_equal_commutative equivalence_classes dimvar other
			in
			let fbinds_valid =
				match has_fbinds with
				| None ->
						true
				| Some(other: dimvar_mapping) ->
						let () = if options.debug_skeleton_multiple_lengths_filter then
							let () = Printf.printf "Comparing %s and %s\n" (dimvar_mapping_to_string dimvar) (dimvar_mapping_to_string other) in
							() else ()
						in
                        (* Really not 100% sure why we need
                        the commutative equality here.  Anyway, the
                        skeleton pass seems to assign dimensions
                        in different directions depending on
                        whether this is pre or post, so this triggers
                        false-negatives here.  Ditto above.  *)
						dimvar_equal_commutative equivalence_classes dimvar other
			in
			(* Now, set the used dimensions for the tvar *)
			let _ = Hashtbl.set tbl (name_reference_list_to_string !tname_so_far) dimvar in
			tbinds_valid && fbinds_valid
		)
		)
	) in
	let () =
		if options.debug_skeleton_multiple_lengths_filter then
			let () = Printf.printf "Keep that skeleton bool is %b\n" (result)
			in ()
		else ()
    in
    result

let no_multiple_lengths_check options apispec skeleton =
	let lenvar_ass = Hashtbl.create (module String) in
	let result = no_multiple_lengths options apispec lenvar_ass skeleton.pre skeleton.post in
    let () = if options.debug_skeleton_multiple_lengths_filter then
        let () = Printf.printf "Result of multiple lengths check is %b\n" (result) in
        ()
    else ()
    in
    result

let dim_assign_equal dimlist dimvar =
    (* Don't currently support non-square multi-dimensional
    arrays, although we presumably could do.  *)
    name_reference_equal (name_reference_list_concat dimlist) dimvar

let dim_assign_any_equal fvars fvar =
    List.exists fvars (fun f ->
        String.equal f (name_reference_to_string fvar)
    )

let check_assignment_compatability options api_spec pre_skel post_skel dimensions =
	(* Build up a table of the conversion functions
	   used to assign between variables.  *)
	(* Iterate over all the dimensions used in this skeleton.  *)
	(* These are gathered together by another function. *)
	(* Check that there is an assignment matching the
	   dimension relation.  *)
    let equivalence_map = compute_equivalence_classes options api_spec pre_skel post_skel in
	List.for_all dimensions (fun dim ->
		let () =
			if options.debug_skeleton_multiple_lengths_filter then
			Printf.printf "Starting analysis of new dimension %s\n" (dimvar_mapping_to_string dim)
			else ()
		in
		(* Check that a suitable defining binding exists for each dimension.  *)
		List.exists (pre_skel.flat_bindings @ post_skel.flat_bindings) (fun bind ->
			let conversion_function = bind.conversion_function in
			let fromvars = match bind.fromvars_index_nesting with
			| [] -> []
			| [fromv] ->
					(match fromv with
					| AssignConstant(_) -> []
					| AssignVariable(v) -> v
					)
			(* To be honest, I'd just skip this here and go through
			to true.  I'm sure a betteer check dependending
			on the conversion function is pssible.  *)
			(* We could just replace this with an empty list ---
			doing this to give a hint when/if this multi variable
			assignment thing is fianlly supported.  *)
			| x :: xs -> raise (SkeletonFilter "Multiple from assignvars not currently supported")
			in
            let fromvar_name = (name_reference_to_string (name_reference_list_concat fromvars)) in
            let fromvars_equivalents =
                match Hashtbl.find equivalence_map fromvar_name with
                | None -> []
                | Some(fvars) -> fvars
            in
            let equiv_fromvars = fromvar_name :: fromvars_equivalents in
			let tovars = bind.tovar_index_nesting in
			(* This is just an heuristic check -- admittedly,
			later passes can crash if it fails, but in
			those could be (easily) fixed on their own. *)
			(* As a result, we aren't trying to handle anything complex
			here. *)

			(* Now, for that dimension assignment, check that
			it is compatible with this particular assignment.  *)
            let () = 
                if options.debug_skeleton_multiple_lengths_filter then
                    let () = Printf.printf "Looking at assignment %s (equiv %s) to %s with conversion %s and dimvar assumption %s\n"
                    (if (List.length fromvars) = 0 then "(Const)" else name_reference_list_to_string fromvars)
                    (if (List.length fromvars_equivalents) = 0 then "None" else (String.concat ~sep:", " fromvars_equivalents))
                    (name_reference_list_to_string tovars)
                    (conversion_function_to_string conversion_function)
                    (dimvar_mapping_to_string dim)
                    in ()
                else ()
            in
            let result = match dim with
            | DimvarOneDimension(VarMatch(tov, fromv, DimEqualityRelation)) ->
					(* In theory, the order of this doesn't matter,
					although the order should be normalized. *)
                    if dim_assign_equal tovars tov then
						(* conversion function must preserve the dimension relation.  *)
                        (* let () = Printf.printf "Tovars equal\n" in*)
						(dim_assign_any_equal equiv_fromvars fromv) &&
						(is_identity_conversion conversion_function)
                    else
                        (* This dimension has no overlap with
                        the assignment we are considering. *)
						false
			| DimvarOneDimension(VarMatch(tov, fromv, DimPo2Relation)) ->
					if dim_assign_equal tovars tov then
						(* as above.  *)
						(dim_assign_equal fromvars fromv) &&
						(is_po2_conversion conversion_function)
					else
						(* no overlap.  *)
						false
			| DimvarOneDimension(VarMatch(tov, fromv, DimDivByRelation(x))) ->
					if dim_assign_equal tovars tov then
						(* as above *)
						(dim_assign_equal fromvars fromv) &&
						(match conversion_function with
						| DivideByConversion(mby) -> x = mby
						| _ -> false)
					else
						(* no overlap.  *)
						false
            | DimvarOneDimension(ConstantMatch(_)) ->
					true
            in
            let () = 
                if options.debug_skeleton_multiple_lengths_filter then
                    let () = Printf.printf "Result was %b\n" result in
                    () else ()
            in
            result
        )
	)

let get_dimension_assignments skel =
	Utils.remove_duplicates (dimvar_equal_commutative (Hashtbl.create (module String))) (List.concat (List.map skel.flat_bindings (fun bind ->
                bind.valid_dimensions
            )
        )
    )
	
let length_assignment_check options api_spec skeleton =
	(* Check that dimvars have the same assignments as
	are actually going to be performed in the code.  *)
	let () = if options.debug_skeleton_multiple_lengths_filter then
		Printf.printf "Starting new length assignment check\n"
	else ()
	in
	let dimensions_list = (get_dimension_assignments skeleton.pre) @ (get_dimension_assignments skeleton.post) in
	let result = check_assignment_compatability options api_spec skeleton.pre skeleton.post dimensions_list in
	let () = if options.debug_skeleton_multiple_lengths_filter then
		Printf.printf "Keeping: %b\n" result
	else ()
	in
	result

(* Check a single skeleton.  *)
let skeleton_check skel =
	(* Don't assign to multiple interface variables
	from the same input variable --- that seems
	unlikely to happen in most contexts.  *)
	no_multiple_cloning_check skel

let skeleton_pair_check options api_spec p =
	(* Don't assign to/from a variable using different
	length parameters.  *)
	let multi_assign_check = no_multiple_lengths_check options api_spec p in
	let length_check = length_assignment_check options api_spec p in
	let result = multi_assign_check && length_check in
	let () = if options.debug_skeleton_multiple_lengths_filter then
		let () = Printf.printf "Pair Check - (multi assign: %b) (length_check: %b)" (multi_assign_check) (length_check) in
		() else () in
    result
