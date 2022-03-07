open Core_kernel;;
open Spec_definition;;
open Spec_utils;;

exception SynthValueException of string

let rec get_value inputs vname =
	match vname with
	| Name(n) -> Hashtbl.find_exn inputs n
	| StructName([n]) -> Hashtbl.find_exn inputs (name_reference_to_string n)
	| StructName(n :: ns) ->
			let rec handle_substruct substruct =
			(
					match substruct with
					| StructV(n, values) ->
							get_value values (StructName(ns))
					(* This is a bit of a stupid hack --- but need some way of extracting
					 things from arrays.  *)
					(* Or, at least, I think it's a stupic hack.  Might actually be the
					right place to do this... *)
					| ArrayV(values) ->
							ArrayV(List.map values (fun v -> match v with
							| StructV(n, values) -> get_value values (StructName(ns))
							| _ -> raise (SynthValueException ("Unexecptected non struct"))))
					| PointerV(vs) ->
							handle_substruct vs
					| _ ->
							raise (SynthValueException ("Unexpected non struct " ^ (synth_value_to_string substruct)))
			)
			in
			let substruct = Hashtbl.find_exn inputs (name_reference_to_string n) in
			handle_substruct substruct
	| _ -> raise (SynthValueException "Unexpected name")

let rec set_value inputs vname result =
	match vname with
	| Name(n) -> Hashtbl.set inputs n result
	| StructName([n]) ->
			Hashtbl.set inputs (name_reference_to_string n) result
	| StructName(n :: ns) ->
			(
			match Hashtbl.find_exn inputs (name_reference_to_string n) with
			| StructV(n, vmap) ->
					set_value vmap (StructName(ns)) result
			| ArrayV(values) ->
					ignore(
						List.map values (fun v ->
							match v with
							| StructV(n, vmap) ->
									set_value vmap (StructName(ns))
							| _ -> raise (SynthValueException "Unexpected type")
						)
					)
			| _ -> raise (SynthValueException "Unexpected type")
			)
	| _ -> raise (SynthValueException "Unexepected Name")
