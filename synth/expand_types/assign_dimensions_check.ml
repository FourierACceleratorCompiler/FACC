open Core_kernel;;
open Spec_definition;;
open Spec_utils;;

exception AssignDimensionsCheck of string

let rec typ_has_dimension n t =
	match t with
	| Array(st, dim) ->
			(
			match dim with
			| Dimension(_) -> typ_has_dimension n st
			| EmptyDimension ->
                    let () = Printf.printf "For variable %s, could find no valid dimensions!" n in
                    raise (AssignDimensionsCheck "Failed dim check")
			)
	| Pointer(st) ->
			typ_has_dimension n st
	| other ->
			(* No other types to check --- structs are checked elsewhere. *)
			()

let check_dimensions_in typemap =
	let keys = Hashtbl.keys typemap in
	let _ = List.map keys (fun key ->
		let typ = Hashtbl.find_exn typemap key in
		(* let () = Printf.printf "Checking %s: %s\n" (key) (synth_type_to_string typ) in *)
		typ_has_dimension key typ
	) in
	()

let check_dimensions_in_classmap cmap =
	let classes = Hashtbl.keys cmap in
	let _ = List.map classes (fun classname ->
		let tmap = get_class_typemap (Hashtbl.find_exn cmap classname) in
        check_dimensions_in tmap
	) in
	()

let assign_dimensions_check typemap =
	(* TODO -- could also check the io_typemap.  *)
	let () = check_dimensions_in typemap.variable_map in
	let () = check_dimensions_in_classmap typemap.classmap in
	()
