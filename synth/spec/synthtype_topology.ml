open Spec_definition;;
open Options;;
open Spec_utils;;
open Core_kernel;;

exception STopologyException of string

type synth_type_use_def = {
	name: name_reference;
	dependencies: name_reference list
}

let dep_to_string deps =
	"var (" ^ (name_reference_to_string deps.name) ^ ") with deps (" ^
	(name_reference_list_to_string deps.dependencies) ^ ")"

let dep_list_to_string dlist =
	String.concat ~sep:"\n" (
		List.map dlist dep_to_string
	)

let rec get_dependencies_for typemap typ =
	match typ with
	| Bool -> []
	| Int16 -> []
	| Int32 -> []
	| Int64 -> []
	| UInt16 -> []
	| UInt32 -> []
	| UInt64 -> []
	| Float16 -> []
	| Float32 -> []
	| Float64 -> []
	| Pointer(tp) -> get_dependencies_for typemap tp
	| Array(tp, dims) ->
			(* let () = Printf.printf "Dimvar is %s\n" (dimension_type_to_string dims) in *)
			let this_deps = match dims with
			| Dimension(x) -> (match x with
                (* We only consider non-interlooping structs here, although we could support
                more complex things with a more complex algorihtm.  *)
				| DimVariable(v, relation) -> [name_reference_top_level_name v]
				| DimConstant(_) -> []
			)
			| EmptyDimension -> raise (STopologyException "Unhandled")
			in
			this_deps @ (get_dependencies_for typemap tp)
	| Struct(sname) ->
            (* We don't do a sort of the individual field names.  External
             variables that might be used as references should be counted however.  *)
			(* let () = Printf.printf "Recursing for struct %s\n" (sname) in *)
			let metadata = Hashtbl.find_exn typemap.classmap sname in
			let subs = get_class_fields metadata in
			let stypedef = get_class_typemap metadata in
			let subtyps = List.map subs (Hashtbl.find_exn stypedef) in
            let subdefs = List.concat (
				List.map subtyps (get_dependencies_for typemap)
			) in
            (* Remove any types that are inherint to this type: they shouldn't be
            considered here.  *)
			(* let () = Printf.printf "For struct name %s\n" (sname) in
			let () = Printf.printf "Filtering out variables %s\n" (String.concat ~sep:", " subs) in
			let () = Printf.printf "Pre filtering is %s\n" (name_reference_list_to_string subdefs) in *)
			let result = List.filter subdefs (fun d -> not (List.mem subs (name_reference_to_string d) Utils.string_equal)) in
			(* let () = Printf.printf "Post filtering is %s\n" (name_reference_list_to_string result) in *)
			result
	| Unit ->
			[]
	| Fun(f, t) ->
			(* In reality, this could use a whole fuckload
			of variables --- all the ones in a closure.
			Not dealing with lambdas right now though :)
			(phew)
			*)
			[]

let compute_use_defs typemap names =
	List.map names (fun n ->
		let typ = Hashtbl.find_exn typemap.variable_map (name_reference_to_string n) in
		(* let () = Printf.printf "Getting dependencies for %s: %s\n" (name_reference_to_string n) (synth_type_to_string typ) in *)
		{
			name = n;
			dependencies = Utils.remove_duplicates name_reference_equal (get_dependencies_for typemap typ)
		}
	)

let split_deps deps =
		let still_has_deps = List.filter deps (fun v -> List.length v.dependencies <> 0) in
		let no_more_deps = List.filter deps (fun v -> List.length v.dependencies = 0) in
		still_has_deps, no_more_deps

let name_ref_def_check n1 n2 =
	not (name_reference_equal n1 n2)

let filter_deps name deps =
	(* TODO --- This should be made more complex to handle
	   fields in classes --- not needed for current targets
	   I think (look at the name_ref_def_check func).  *)
	{
		name = deps.name;
		dependencies = List.filter deps.dependencies (name_ref_def_check name)
	}

let rec synth_khan options vars s sorted =
	let () = if options.debug_synth_topology then
		let () = Printf.printf "==== Iteration ====\n" in
		let () = Printf.printf "Have a list of %s left\n" (dep_list_to_string vars) in
		let () = Printf.printf "Have a list of %s done\n" (dep_list_to_string sorted) in
		()
	else ()
	in
	match s with
	|  [] -> let () = if (List.length vars <> 0) then
		let () = Printf.printf "FAILED\n" in
		let () = Printf.printf "Had a list of %s left\n" (dep_list_to_string vars) in
		let () = Printf.printf "Had a list of %s done\n" (dep_list_to_string sorted) in
		assert false
	else () in
	sorted
	| n :: ss -> 
			let remaining_vars = List.map vars (filter_deps n.name) in
			let still_has_deps, no_more_deps = split_deps remaining_vars in
			synth_khan options still_has_deps (ss @ no_more_deps) (n :: sorted)

let synthtype_toposort options typemap snames =
	let () = if options.debug_synth_topology then
		let () = Printf.printf "Starting new synthtype topology sort\n" in
        let () = Printf.printf "Names are %s\n" (name_reference_list_to_string snames) in
		()
	else ()
	in
	let deps = compute_use_defs typemap snames in
	let rest, stack = split_deps deps in
	let topo_sorted = synth_khan options rest stack [] in
	List.rev (List.map topo_sorted (fun t -> t.name))
