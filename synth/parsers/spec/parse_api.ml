open Core_kernel;;
open Yojson;;
open Yojson.Basic.Util;;
open Spec_definition;;
open Parse_type;;
open Parse_typemap;;
open Parse_range;;
open Parse_classmap;;

exception APIException of string

(* TODO -- would like to support different postfixes e.g. B/b/k etc. *)
let load_alignment_map json =
	let table = Hashtbl.create (module String) in
	match json with
	| `Assoc(_) ->
		let _ = List.map (json |> keys) (fun key ->
			let _ = Hashtbl.add table key (json |> member key |> to_int) in
			()
		) in
		table
	| `Null ->
			(* We're allowed to have no alignment map *)
			table
	| other ->
			raise (APIException "Unexpected alignment type: expexted json map")

let load_target_api options filename =
	let json = Yojson.Basic.from_file filename in
	let classmap = load_classmap_from_json options (json |> member "classmap") in
	let livein = List.map (json |> member "livein" |> to_list) to_string in
	let liveout = List.map (json |> member "liveout" |> to_list) to_string in
	let typemap = load_typemap options json (livein @ liveout) in
	let funname = json |> member "functionname" |> to_string in
	let funargs = List.map (json |> member "functionargs" |> to_list) to_string in
	let alignment_requiremts = load_alignment_map (json |> member "alignment") in
	let valid_tbl = load_rangetable options classmap typemap (json |> member "valid") in
    (* Compiler flags not required --- empty list of flags if so.
    To be honest, this is a bit of a hack, we'd really like the entire
    thing to be backend independent.  However, it is sane for the calling
    compiler to tell us e.g. what we need to link in to build around
    the API.  (e.g. I've used this to link with FFTW). 
    Required_includes has the same problem FWIW.  *)
	let compiler_flags = match json |> member "compiler_flags" with
    | `Null -> []
    | other -> List.map (other |> to_list) to_string in
	let required_includes = List.map (json |> member "required_includes" |> to_list) to_string in
	let apispec: apispec = {
		livein = livein;
		liveout=liveout;
		funname = funname;
		funargs = funargs;
		required_includes = required_includes;
        compiler_flags = compiler_flags;
		validmap = valid_tbl;
	} in
	apispec, typemap, classmap, alignment_requiremts;;
