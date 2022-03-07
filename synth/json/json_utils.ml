open Core_kernel;;
open Spec_definition;;
open Spec_utils;;
open Yojson;;
open Yojson.Basic.Util;;

exception JSONException of string
exception JSONCheckException of string
exception JSONCheckFailed

let rec load_json j =
	let mempairs = List.map j (fun (name, elt) ->
		(name, load_json_elt (elt))
	) in
    let result = Hashtbl.create (module String) in
    let _ = List.map mempairs (fun (name, value) ->
        Hashtbl.add result name value
    ) in
    result

and load_json_elt (e: Yojson.Basic.t) =
	(* let () = Printf.printf "JSON is %s\n" (Yojson.Basic.show e) in *)
	match e with
	| `Assoc(d) ->
			StructV("Unknown", load_json d)
	| `Bool(b) ->
			BoolV(b)
	| `Float(f1) ->
			(* arbitrarily choose this I suppose --- note that
			 if this changes, there are a bunch of other synth
			 things that have to change. *)
			Float64V(f1)
	| `Int(i) ->
			Int32V(i)
	| `List(l) ->
			ArrayV(List.map l load_json_elt)
	| `Null ->
			raise (JSONException "Loading Null JSON")
	| `String(s) ->
			raise (JSONException "Unexpected type: string")

let load_value_map_from file =
	(* let () = Printf.printf "Loading from file %s\n" (file) in *)
	let json = Yojson.Basic.from_file file in
	let j_members = keys json in
    let json_elts = List.map j_members (fun mem ->
        (mem, json |> member mem)
    ) in
	load_json json_elts


let rec check_elts_for_uninitialized_reads typemap elts =
	List.map elts (fun (name, typ, json) ->
		check_elt_for_uninitialized_read typemap name typ json 
	)
and check_elt_for_uninitialized_read typemap name typ json =
	match json, typ with
	| `Assoc(d), Struct(n) ->
			let subtmap = get_class_typemap (Hashtbl.find_exn typemap.classmap n) in
			(* TODO -- we could also check that all memebrs
			are present.  *)
			let subelts = List.map d (fun (subname, subjson) ->
				(subname, (Hashtbl.find_exn subtmap subname), subjson)
			) in
			let new_typemap = {typemap with variable_map = subtmap } in
			ignore(check_elts_for_uninitialized_reads new_typemap subelts)
	| `Bool(b), Bool -> ()
	| `Float(f), Float16 -> ()
	| `Float(f), Float32 -> ()
	| `Float(f), Float64 -> ()
	| `Int(i), Int16 -> ()
	| `Int(i), Int32 -> ()
	| `Int(i), Int64 -> ()
	| `Int(i), UInt16 -> ()
	| `Int(i), UInt32 -> ()
	| `Int(i), UInt64 -> ()
	| `List(l), Array(t, EmptyDimension) -> raise (JSONCheckException "Unexpected empty dim")
	| `List(l), Array(t, Dimension(d)) ->
			(* Check the length first. *)
			let () =
				match d with
				| DimConstant(c) ->
						if c = (List.length l) then
							()
						else
							raise JSONCheckFailed
				| DimVariable(d, _) ->
						(* TODO --- we could do a more thorough check here... *)
						()
			in
			ignore(List.map l (check_elt_for_uninitialized_read typemap name t))
	| `Null, Float16 -> raise JSONCheckFailed
	| `Null, Float32 -> raise JSONCheckFailed
	| `Null, Float64 -> raise JSONCheckFailed
	(* Pointers are invisible in the JSON file *)
	| v, Pointer(sty) -> check_elt_for_uninitialized_read typemap name sty v
	(* We could just thrwo the JSON check fialed in these cases
	I expect -- just trying to avoid the weird silent failures that
	this thing is going to create... *)
	| `Null, _ -> raise (JSONCheckException ("Unexpected Null json (" ^ name ^ ")with type " ^ (synth_type_to_string typ)))
	| `String(s), _ -> raise (JSONCheckException ("Unexpected String json with type " ^ (synth_type_to_string typ)))
	| _, _ -> raise (JSONCheckException ("Unexpected value for type " ^ (synth_type_to_string typ)))

let check_for_uninitialized_reads options typemap outfile =
	let json = Yojson.Basic.from_file outfile in
	let json_elts = List.map (keys json) (fun mem ->
		(mem, Hashtbl.find_exn typemap.variable_map mem, json |> member mem)
	) in
	let result = try (ignore(check_elts_for_uninitialized_reads typemap json_elts); false)
	with JSONCheckFailed -> true in
	result
(* OK, so this aims to find failures where the executable exited
	correctly.  There are a couple ways we can try to pickup
	failures, and ultimately, they'll depend on the target language.

	In C, we use the address sanitizer to catch out-of-bound accesses,
	but this doesn't catch uninitialized reads, which are a symptom
	of incorrect length assumptions.  Those aren't a problem
	in themselves, since they'd be caught by any correctness
	checking, but they can cause crashes in the JSON loader
	because Nulls can appear where they are not expected
	(e.g. in FP paramters).
	*)
let outfile_has_errors options typemap outfile =
	check_for_uninitialized_reads options typemap outfile
