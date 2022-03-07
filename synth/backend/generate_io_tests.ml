open Core_kernel;;
open Spec_definition;;
open Spec_utils;;
open Value_utils;;
open Options;;
open Yojson;;
open Synthtype_topology;;
open Utils;;
open Range;;
open Program;;
open Range_checker_synth;;
open Builtin_types;;

exception TypeException of string
exception GenerationFailure

let rec generate_file_numbers upto =
	if upto = 0 then []
	else if upto = 1 then
		["1"]
	else
		(string_of_int upto) :: (generate_file_numbers (upto - 1))

let generate_int_within_range rangemap namestring =
    match Hashtbl.find rangemap namestring with
    | None -> (* It's normal to have no rangemap matches if there
                were no rangevars specified.  *)
            Random.int (1000)
    | Some(range) ->
			(* let () = Printf.printf "Looking at name %s\n" namestring in
			let () = Printf.printf "Has rangemap %s\n" (range_set_to_string range) in
			let () = Printf.printf "Rangeset is emapy is %b\n" (empty_range_set range) in *)
            match random_value_in_range range with
            | RInt(v) -> v
            | _ -> raise (TypeException "Unexpected non-int result to int query")

let generate_uint_within_range rangemap namestring =
	match Hashtbl.find rangemap namestring with
	| None ->
			Random.int (1000)
	| Some(range) ->
			match random_value_in_range range with
			| RInt(v) -> v
			| _ -> raise (TypeException "Unexepcted non-int result to int query")

let generate_bool_within_range rangemap namestring =
	match Hashtbl.find rangemap namestring with
	| None -> (Random.int (1)) = 1
	| Some(range) ->
			match random_value_in_range range  with
			| RBool(v) -> v
			| RInt(v) -> (v <> 0)
			| _ -> raise (TypeException "Unexpected non-bool result")

let generate_float_within_range rangemap namestring =
    match Hashtbl.find rangemap namestring with
    | None -> (* Ditto above *)
            Random.float 100.0
    | Some(range) ->
            match random_value_in_range range with
            | RFloat(v) -> v
            | _ -> raise (TypeException "Unexepced non-float result to float query")

(* This one assumes that the array has been specified -- the implementation
for generating a random array is below.  *)
let generate_array_from_range rangemap namestring =
	let range = Hashtbl.find_exn rangemap namestring in
	match random_value_in_range range with
	| RArray(_, v) as rarr -> range_value_to_synth_value rarr
	| _ -> raise (TypeException "Unexepected non-array result to the array query")


(* TODO --- Could do with making this a bit more deterministic. *)
let rec generate_inputs_for options rangemap values_so_far name_string infered_type t structure_ordering =
	let () = if options.debug_generate_io_tests then
		let () = Printf.printf "Generating value for %s...\n" (name_string) in
		()
	else ()
	in
    match t with
    (* TODO -- Support negative values.  *)
	| Bool -> BoolV(generate_bool_within_range rangemap (name_string))
    | Int16 -> Int16V(generate_int_within_range rangemap (name_string))
    | Int32 -> Int32V(generate_int_within_range rangemap (name_string))
    | Int64 -> Int64V(generate_int_within_range rangemap (name_string))
	| UInt16 -> UInt16V(generate_uint_within_range rangemap (name_string))
	| UInt32 -> UInt32V(generate_uint_within_range rangemap (name_string))
	| UInt64 -> UInt64V(generate_uint_within_range rangemap (name_string))
    | Float16 -> Float16V(generate_float_within_range rangemap (name_string))
    | Float32 -> Float32V(generate_float_within_range rangemap (name_string))
    | Float64 -> Float64V(generate_float_within_range rangemap (name_string))
    | Fun(_, _) -> raise (TypeException "Can't generate types for a fun")
    | Unit -> UnitV
	| Pointer(stype) ->
			let infered_stype = match infered_type with
			| Pointer(sty) -> sty
			(* NOt that this case can't be easily handled, but this is a bit
			intertwined with the structure inference tool.  Just needs to understand
			what actually was infered, since it'd be a pain in the ass to do this
			in a general case I expect.  *)
			| other -> raise (TypeException "Infered pointer type over non-pointer")
			in
			PointerV(generate_inputs_for options rangemap values_so_far name_string infered_stype stype structure_ordering)
    (* TODO --- Probably need to
       make a distinction between square and non
       square arrays.  *)
    | Array(subtype, dimvar) ->
			(* Not sure whether this should use the context name at the moment --- I think it should  *)
			if Hashtbl.mem rangemap (name_string) then
				(* If the array has specified values
				we should use those --- often occurs with things
				that have precomputed constant tables, e.g. twiddle factors.  *)
				generate_array_from_range rangemap (name_string)
			else
				let (infered_subtype, infered_dim) = match infered_type with
				| Array(sty, dim) -> sty, dim
				(* As above with pointers: we can definitely handle this, but
				as of now the structure inferencer doesn't actually do this,
				and so it's difficult to understand what subtype to use without
				concrete usecases.  *)
				| other -> raise (TypeException "Infered an array on a non-array original type!")
				in
				(* If the name wasn't specified, then we should
				generate an array.  *)
				let infered_dimvar_size = match infered_dim with
				| Dimension(dms) -> dms
				| EmptyDimension ->
						raise (TypeException "Can't have empty dimensions!")
				in
				(* If this throws, there's an issue with the topo sorting below --- we
				   expect that the dimvars will have been assigned.  *)
				let base_arrlen =
						match infered_dimvar_size with
						| DimVariable(dimvar_name, relation) ->
								(* Referneces to the variable must be made from the context of this instance of the class (i.e. no escaping refs).  *)
								let wrapper = get_value values_so_far dimvar_name in
								let arrlen = match wrapper with
								| Int16V(v) -> v
								| Int32V(v) -> v
								| Int64V(v) -> v
								| UInt16V(v) -> v
								| UInt32V(v) -> v
								| UInt64V(v) -> v
								| _ ->
										(* probably we could handle this --- just need to have a think
										about what it means. *)
										raise (TypeException "Unexpected list dimension type (non-int) ")
								in
								let () = if options.debug_generate_io_tests then
									Printf.printf "Getting dimension %s, with original array length %d\n" (name_reference_to_string dimvar_name) (arrlen)
								else () in
								let result_length = (
								match relation with
								| DimEqualityRelation -> arrlen
								| DimPo2Relation ->
										Utils.power_of_two arrlen
								| DimDivByRelation(x) ->
										arrlen / x
                                ) in
								let () = if options.debug_generate_io_tests then
									Printf.printf "Got length after modification of %d\n" (result_length)
								else ()
								in
								result_length
						| DimConstant(c) -> c
				in
				let arrlen_modifier = get_size_modifier_for infered_subtype in
                let arrlen = arrlen_modifier * base_arrlen in
				let () = if options.debug_generate_io_tests then
					Printf.printf "Generating array length %d (base %d, size modifier %d)\n" (arrlen) base_arrlen arrlen_modifier
				else () in
				if arrlen < options.array_length_threshold then
					ArrayV(List.map (List.range 0 arrlen) (fun _ -> generate_inputs_for options rangemap values_so_far name_string infered_subtype subtype structure_ordering))
				else
					(* Don't want to try and generate arrays that are too big, because
					it just makes synthesis take forever, espc with
					slower computers.  *)
                    (* let () = Printf.printf "Arrlen is %d, maxlen is %d" (arrlen) (options.array_length_threshold) in *)
					raise (GenerationFailure)
    | Struct(structname) ->
			(* To generate the sub-typemap, use the toposorted fields for that partiuclar class.  *)
			(* let () = Printf.printf "Looking at %s\n" (structname) in *)
			let members, tmap, infered_tmap = Hashtbl.find_exn structure_ordering structname in
            (* Generate a value for each type in the metadata.  *)
            let valuetbl = Hashtbl.create (module String) in
            let _ = List.map members (fun member ->
				let resv = generate_inputs_for options rangemap valuetbl member (Hashtbl.find_exn infered_tmap.variable_map member) (Hashtbl.find_exn tmap.variable_map member) structure_ordering in
				Hashtbl.add valuetbl member resv
			) in
            StructV(structname, valuetbl)

let rec generate_io_values_worker options rangemap generated_vs vs structure_orderings infered_typemap io_typemap =
	match vs with
	| [] -> ()
	| x :: xs ->
			let name_string = name_reference_to_string x in
			let () = if Hashtbl.mem generated_vs name_string then
				let () = if options.debug_generate_io_tests then
					Printf.printf "Skipping variable (defined by value profile) %s\n" (name_string)
				else () in
				() (* Skip because this has already been generated by a value profile.  *)
			else
				let typx = Hashtbl.find_exn io_typemap.variable_map name_string in
				let infered_typx = Hashtbl.find_exn infered_typemap.variable_map name_string in
				let inputs = generate_inputs_for options rangemap generated_vs name_string infered_typx typx structure_orderings in
				let res = Hashtbl.add generated_vs (name_reference_to_string x) inputs in
				let () = assert (match res with | `Ok -> true | _ -> false) in
				()
			in
			(generate_io_values_worker options rangemap generated_vs xs structure_orderings infered_typemap io_typemap)

let create_mapping_from_value_profiles profiles = match profiles with
	(* If there are no value profiles, there is nothing to select from :) *)
	| [] -> Hashtbl.create (module String)
	| profiles ->
            (* Note that this is a shallow clone --- needs to be a deep clone
               to properly support the partial profiling required by some
               data structures.  *)
			clone_valuemap (List.nth_exn profiles (Random.int (List.length profiles)))

let rec generate_io_values options value_profiles num_tests rangemap livein structure_orderings infered_typemap io_typemap =
	match num_tests with
	| 0 -> []
	| n ->
        let inputs = ref None in
        let () = while (Option.is_none !inputs) do
            (* This can fail if it happens to generate arrays that
            are too long to test successfully.  *)
            let () = inputs := (try
                let mapping = create_mapping_from_value_profiles value_profiles in
                let () = (generate_io_values_worker options rangemap mapping livein structure_orderings infered_typemap io_typemap) in
                (* let () = Printf.printf "Generation success\n" in *)
                (Some(mapping))
            with GenerationFailure -> 
                (* let () = Printf.printf "Generation failure\n" in *)
                None
            )
            in ()
        done in
		(Option.value_exn !inputs) :: (generate_io_values options value_profiles (num_tests - 1) rangemap livein structure_orderings infered_typemap io_typemap)

let rec value_to_string value =
    let str_value = match value with
	| BoolV(v) -> string_of_bool v
    | Int16V(v) -> string_of_int v
    | Int32V(v) -> string_of_int v
    | Int64V(v) -> string_of_int v
	| UInt16V(v) -> string_of_int v
	| UInt32V(v) -> string_of_int v
	| UInt64V(v) -> string_of_int v
	(* JSON does not support formats of the form 1., so
	we need to append a 0 to make it valid :) *)
    | Float16V(v) -> (string_of_float v) ^ "0"
    | Float32V(v) -> (string_of_float v) ^ "0"
    | Float64V(v) -> (string_of_float v) ^ "0"
    (* Probably not right --- needs to mesh with
    however 'unit' is passed in from the JSON representation.
    Can avoid for now with C++ as main target.  *)
    | UnitV -> "()"
    | ArrayV(vals) ->
            let vals = List.map vals value_to_string in
            "[" ^ (String.concat ~sep:", " vals) ^ "]"
	| PointerV(v) ->
			(value_to_string v)
    | StructV(n, valuetbl) ->
            io_test_to_string (Hashtbl.keys valuetbl) valuetbl
    | FunV(_) ->
            raise (TypeException "Unhandled. ")
	in str_value

and io_test_to_string names values =
    let vals_list = List.map names (fun n ->
        let value = Hashtbl.find_exn values n in
        let value_str = value_to_string value in
        "\"" ^ n ^ "\": " ^ value_str
    ) in
    "{" ^ (String.concat ~sep:",\n" vals_list) ^ "}"

let write_io_tests (options: options) program_number names values =
    let target_folder = options.execution_folder ^ "/" ^ "io/" ^ program_number ^ "/" in
    (* Make the folder.  *)
    let res = Sys.command ("mkdir -p " ^ target_folder) in
    let () = assert (res = 0) in
    let nums = generate_file_numbers (List.length values) in
    (* Convert to json and write out.  *)
	List.map (List.zip_exn nums values) (fun (n, vals) ->
		let targ_file = (target_folder ^ "/" ^ n ^ ".json") in
		let json_str = io_test_to_string names vals in
		let () = Out_channel.write_all targ_file ~data:json_str in
		targ_file
	)

let wrap_nrefs nms =
    List.map nms (fun nm ->
        Name(nm)
    )

(* This pass assumes that classes don't have extra-class variable dependencies.
   In reality, we could support those at the top level topo-sort provided there
   aren't any loops etc.  *)
let generate_toposorted_classmap (options: options) input_typemap io_typemap =
	let names = Hashtbl.keys input_typemap.classmap in
	let result_hashmap = Hashtbl.create (module String) in
	let _ = List.map names (fun name ->
		let structdata = Hashtbl.find_exn input_typemap.classmap name in
		let typemap = get_class_io_typemap structdata in
		let names = List.map (get_class_fields structdata) (fun v -> Name(v)) in
		let full_typemap = {input_typemap with variable_map = typemap } in
		let toposorted_values = synthtype_toposort options full_typemap names in
		(* TODO -- I don't know why this isn't using the io_typemap as input. *)
		Hashtbl.add result_hashmap name ((List.map toposorted_values name_reference_to_string), full_typemap, full_typemap)
	) in
	result_hashmap

let generate_io_tests_for_program options (iospec: iospec) program_number (program: program) =
	let () =
		if options.debug_generate_io_tests then
			Printf.printf "Starting to generate IO tests\n"
		else () in
	(* generate the values for each input. *)
	let num_tests = options.number_of_tests in
    let livein_namerefs = wrap_nrefs iospec.livein in
	(* For generating IO examples (they need to run in the original
	program), we need to use the original program's typemap,
	not the infered typemap.  In practice of course, examples
	generated are entirely equivalent, but casting doesn't
	quite work the same in JSON.  *)
	let io_typemap = match program.typemap.original_typemap with
	| Some(t) -> t
	| None -> program.typemap
	in
	let toposorted_values = synthtype_toposort options io_typemap livein_namerefs in
	let structure_orderings = generate_toposorted_classmap options program.typemap io_typemap in
	let () =
		if options.debug_generate_io_tests then
			let () = Printf.printf "Topo sorted values are %s\n" (name_reference_list_to_string toposorted_values) in
			Printf.printf "Starting to generate values\n"
		else () in
	let values = generate_io_values options iospec.value_profiles num_tests program.inputmap toposorted_values structure_orderings program.typemap io_typemap in
	(* Now, convert those to YoJSON values to be written out.  *)
	let json_files = write_io_tests options program_number iospec.livein values in
	json_files

let generate_io_tests options (iospec: iospec) programs =
	let () = if options.debug_generate_io_tests then
		Printf.printf "Starting generating tests: Generating tests for %d programs (try using --no-parmp if you don't see any other debug information) \n" (List.length programs)
	else () in
	let numbers = generate_file_numbers (List.length programs) in
	Utils.parmap options (fun (number, program) ->
		(program, generate_io_tests_for_program options iospec number program))
        (List.zip_exn numbers programs)
