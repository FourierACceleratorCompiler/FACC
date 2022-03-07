(* This file contains the parser for the type information. *)
open Core_kernel;;
open Yojson;;
open Yojson.Basic.Util;;
open Spec_definition;;
open Parse_type;;
open Synthesize;;
open Parse_typemap;;

let load_individual_type options json_definition =
	let isstruct = json_definition |> member "type" |> to_string in
	let symbols = List.map (json_definition |> member "symbols" |> to_list) (fun j -> j |> to_string) in
	let functions =
		if (String.compare isstruct "class") = 0 then
			List.map (json_definition |> member "functions" |> to_list) to_string
			(* No functions in a struct.  *)
		else []
	in
	let typemap_members = symbols @ functions in
	let typemap = load_typemap options json_definition typemap_members in
	if (String.compare isstruct "class") = 0 then
		ClassMetadata({members=symbols; functions=functions; typemap=typemap; io_typemap=typemap})
	else
		StructMetadata({members=symbols; typemap=typemap; io_typemap=typemap})

let load_classmap_from_json options json =
	let tbl = Hashtbl.create (module String) in
	(* Get the names of all the defined types.  *)
	let typedefs = json |> keys in
	(* Get the types from these definitions.  *)
	let typepairs = List.map typedefs (fun name -> (name, load_individual_type options (json |> member name))) in
	ignore(List.map typepairs (fun (name, value) -> Hashtbl.add tbl name value));
	tbl;;

let load_classmap options filename =
	let json = Yojson.Basic.from_file filename in
	load_classmap_from_json options json
