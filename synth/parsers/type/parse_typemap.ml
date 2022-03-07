open Core_kernel;;
open Yojson;;
open Yojson.Basic.Util;;
open Spec_definition;;
open Parse_type;;
open Options;;

exception LoadTypemapException of string

let is_null_json j = match j with
	| `Null -> true
	| _ -> false

let load_typemap options json_definition typenames =
	let () = if options.debug_load then
		Printf.printf "Loading names %s into typemap\n" (String.concat ~sep:", " typenames)
	else ()
	in
	let typemap = Hashtbl.create (module String) in
	(* Get the types parsed *)
	let json_mapping = json_definition |> member "typemap" in
	let typemap_pairs = List.map typenames (fun name ->
		if is_null_json (Yojson.Basic.Util.member name json_mapping) then
			raise (LoadTypemapException ("Not found type for variable " ^ name ^ "\n"))
		else
			(name, json_mapping |> member name |> to_string |> parse_type)
	) in
	(* Put the parsed types in a hash table. *)
	ignore(List.map typemap_pairs (fun (name, typ) -> Hashtbl.add typemap name typ));
	typemap
