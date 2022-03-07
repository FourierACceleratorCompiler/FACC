(* This file parses information about APIs *)
open Core_kernel;;
open Yojson;;
open Yojson.Basic.Util;;
open Spec_definition;;
open Spec_utils;;
open Parse_classmap;;
open Parse_type;;
open Parse_range;;
open Parse_const;;
open Options;;
open Json_utils;;

exception IOSpecError of string

let isnull x =
	match x with
	| `Null -> true
	| _ -> false

let extract_typemap typemap vars =
	let tbl: (string, synth_type) Hashtbl.t = (Hashtbl.create (module String)) in
	(* Exctract the type names and their values from the JSON. *)
	let typepairs = List.map vars (fun var ->
		if isnull (Yojson.Basic.Util.member var typemap) then
			raise (IOSpecError ("Not found type for variable " ^ var ^ "\n"))
		else
            (var, parse_type(typemap |> member var |> to_string))
	) in
	(* Add these types to the hash map.  *)
	ignore(List.map typepairs (fun (var, t)  -> Hashtbl.add tbl var t));
	tbl;;

let load_value_profiles vprofile =
	load_value_map_from (vprofile |> to_string)

let check_has_not json values =
	ignore(List.map values (fun m ->
		match (json |> member m) with
		| `Null -> ()
		| _ -> raise (IOSpecError ("IOSpec has useless value " ^ m))
	))

let load_iospec options filename =
	let json = Yojson.Basic.from_file filename in
	let classmap = load_classmap_from_json options (json |> member "classmap") in
	let livein = List.map (json |> member "livein" |> to_list) (fun j -> j |> to_string) in
	let liveout = List.map (json |> member "liveout" |> to_list) (fun j -> j |> to_string) in
	let retvars = List.map (match json |> member "returnvarname" with
    | `Null -> []
    | other -> other |> to_list
    ) to_string in
	let execcmd = json |> member "execcmd" |> to_string in
	let typemap = extract_typemap (json |> member "typemap") (livein @ liveout @ retvars) in
    let funname = json |> member "funname" |> to_string in
	let funargs = List.map (json |> member "funargs" |> to_list) (fun j -> j |> to_string) in
	let compiler_flags = match json |> member "compiler_flags" with
    | `Null -> []
    | other -> List.map (other |> to_list) to_string in
	let required_includes = List.map (json |> member "required_includes" |> to_list) to_string in
	let value_profiles = List.map (match json |> member "value_profiles" with
	| `Null -> []
	| other -> other |> to_list
	) load_value_profiles in
	let range_tbl = load_rangetable options classmap typemap (json |> member "range") in
	let valid_tbl = load_rangetable options classmap typemap (json |> member "valid") in
	let const_tbl = load_consttable options typemap (json |> member "consts") in
	(* Check for a few common errors: TODO -- we should realy do a more conclusive check here.  *)
	let _ = check_has_not json ["rangemap"; "validmap"] in
	let iospec: iospec = {
		livein=livein;
		liveout=liveout;
		execcmd=execcmd;
		funname=funname;
		funargs=funargs;
		compiler_flags=compiler_flags;
		required_includes=required_includes;
		rangemap = range_tbl;
        value_profiles = value_profiles;
		validmap = valid_tbl;
		constmap = const_tbl;
		returnvar=retvars;
	} in
	let () =
		if options.debug_load then
			Printf.printf "Loaded iospec is %s" (iospec_to_string iospec)
		else () in
	iospec, typemap, classmap
