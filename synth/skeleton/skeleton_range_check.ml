open Core_kernel;;
open Options;;
open Spec_definition;;
open Spec_utils;;
open Skeleton_definition;;
open Skeleton_utils;;
open Builtin_conversion_functions;;
open Range;;
open Range_definition;;
open Range_checker_synth;;
open Gir_utils;;

exception UnimplementedException
exception RangeCheckException of string

type range_direction =
	| RangeForward
	| RangeBackward

let right_shift_value i =
	match i with
	| RangeInteger(i) -> RangeInteger(1 lsr i)
	| _ -> raise (RangeCheckException "Can't shift non int!")

let right_shift_item i =
	match i with
	| RangeItem(i) -> RangeItem(right_shift_value i)
	| RangeRange(f, t) ->
			RangeRange(right_shift_value f,
						right_shift_value t)

let right_shift_all r =
	match r with
	| RangeSet(rset) ->
			RangeSet(Array.map rset right_shift_item)

let log_value i =
	match i with
	| RangeInteger(i) -> RangeInteger(Utils.log_2 i)
	| _ -> raise (RangeCheckException "Cna't log non-int!") (* I mean, we could... *)

let log_item i =
	match i with
	| RangeItem(i) -> RangeItem(log_value i)
	| RangeRange(f, t) ->
			RangeRange(log_value f, log_value t)

let log_all r = 
	match r with
	| RangeSet(rset) ->
			RangeSet(Array.map rset log_item)

let divalways i mby =
	match i with
	| RangeInteger(x) ->
			RangeInteger(x / mby)
	| _ -> raise (RangeCheckException "Unsuppored div convtype")

let range_item_divide_always i mby =
	match i with
	| RangeRange(f, t) ->
			RangeRange(divalways f mby, divalways t mby)
	| RangeItem(i) ->
			RangeItem(divalways i mby)

let range_item_divide i mby =
	let div i =
		match i with
		| RangeInteger(x) ->
				if (x % mby) = 0 then
					Some(RangeInteger(x / mby))
				else
					None
		| _ -> raise (RangeCheckException "unsupported multiply-by conversion type")
	in
	match i with
	| RangeRange(f, t) ->
			(* should probably actually properly do this.  *)
			Some(RangeRange(divalways f mby, divalways t mby))
	| RangeItem(i) ->
			Option.map (div i) (fun i -> RangeItem(i))

let range_item_multiply i mby =
	let range_value_mul i =
		match i with
		| RangeInteger(x) ->
				RangeInteger(x * mby)
		| _ -> assert false
	in
	match i with
	| RangeRange(f, t) ->
			let res = RangeRange(range_value_mul f, range_value_mul t) in
			let () = Printf.printf "New range is %s\n" (range_range_to_string res) in
			res
	| RangeItem(i) ->
			RangeItem(range_value_mul i)

let execute_conversion_on_range direction conversion inp_ranges =
    match conversion with
    | IdentityConversion ->
            (* Identity is 1 to 1, so must be one assigning var *)
            let () = assert ((List.length inp_ranges) = 1) in
            List.hd_exn inp_ranges
	| PowerOfTwoConversion ->
			let () = assert ((List.length inp_ranges) = 1) in
			let res = List.hd_exn inp_ranges in
			(
			match direction with
			| RangeForward ->
					let shifted = right_shift_all res in
					shifted
			| RangeBackward ->
					let shifted = log_all res in
					shifted
			)
	| DivideByConversion(mby) ->
			let () = assert ((List.length inp_ranges) = 1) in
			let res = List.hd_exn inp_ranges in
			(
			match direction with
			| RangeForward ->
					(
					match res with
					| RangeSet(items) ->
							RangeSet(Array.filter_map items (fun i ->
								range_item_divide i mby
							))
					)
			| RangeBackward ->
					(
					match res with
					| RangeSet(items) ->
							RangeSet(Array.map items (fun item ->
								range_item_multiply item mby
							))
					)
			)
    | Map(fromt, tot, mappairs) ->
            (* Maps are 1 to 1, so must be one assigning var *)
            let () = assert ((List.length inp_ranges) = 1) in
            let inp_range = List.hd_exn inp_ranges in
            (* Sometimes this is calculated in reverse: in that case,
               we actually want to consider the inverse mapping.  *)
            let reversed_mappairs =
                match direction with
                | RangeForward ->
                        mappairs
                | RangeBackward ->
                        List.map mappairs (fun (e1, e2) -> (e2, e1))
            in
            (* Create a new range that is the same, but
            has all values that overlap the mapped
            values changed.  *)
            (* Really should unify the range types
               and the synthvalue types into one type.  *)
            match inp_range with
            | RangeSet(range_items) ->
                    let range_items_list = Array.to_list range_items in
                    let result_list = List.concat (List.map range_items_list (fun item ->
                        match item with
                        | RangeItem(i) ->
                                (* See if this is in the map.  *)
                                let res = List.find reversed_mappairs (fun (f, t) ->
                                    if range_value_eq (range_value_to_item f) i then
                                        true
                                    else
                                        false
                                ) in
                                let new_item = match res with
                                (* If we found the value, use that. *)
                                | Some((_, new_value)) -> RangeItem(range_value_to_item new_value)
                                (* Otherwise, we didn't use the value.  *)
                                | None -> RangeItem(i)
                                in
                                [new_item]
                        | RangeRange(lower, higher) as range ->
                                (* We are going to split
                                this into a series of ranges
                            and individual values.  e.g.
                            if the map is from 0 to 1 and
                            the range is -5 to 5, then we
                            want to generate -5 to -1,
                            1 and 1 to 5.  *)
                                (* We need to compress ranges after this anyway.  *)
                                let overlapping = List.filter reversed_mappairs (fun (f, t) ->
                                    range_value_in range (range_value_to_item f)
                                )
                                in
                                (* Compute the new values.  *)
                                let new_values = List.map overlapping (fun (f, t) ->
                                    RangeItem(range_value_to_item t)
                                ) in
                                (* TODO -- remove the old values --- this overapproximates
                                (which is safe, but unessecary) as is. *)
                                range :: new_values
                    )
                    ) in
                    RangeSet(Array.of_list result_list)

(* This supports either forward or backward range analysis --- you need a forward
analysis to generate the range-checking code, and you need a backward analysis to
generate range restrictions for input-value generation.  *)
let transform_rangemap_by options forward_range unassigned_map map bindings =
    let result_tbl = Hashtbl.create (module String) in
    let () = ignore(List.map bindings.flat_bindings (fun flat_binding ->
        let inputs_count = List.length flat_binding.fromvars_index_nesting in
        let () = if options.debug_range_check then
            let () = Printf.printf "Creating range check for var '%s'\n" (index_nesting_to_string flat_binding.tovar_index_nesting) in
			let () = Printf.printf "Input bindmap has variables '%s'\n" (String.concat ~sep:"', '" (Hashtbl.keys map)) in
			let () = Printf.printf "Input is in the hash map: %b\n" (Hashtbl.mem map (index_nesting_to_string flat_binding.tovar_index_nesting)) in
            let () = Printf.printf "Has %d inputs\n" (inputs_count) in
            ()
        else () in
        if inputs_count = 0 then
            (* If there are no fromvars, this is something that
            is not livein, so won't have any range requirements
            attached to it anyway.  *)
            (* We could assert that here to be honest  *)
            ()
        else
            (* First, get the ranges for each of the input
               variables for this binding.  *)
            let ranges = List.map flat_binding.fromvars_index_nesting (fun fvar ->
                match fvar with
                | AssignConstant(c) ->
                    (* This value set will have a constant value --- note
                    that not all types are currently supported by the
                    range thing, so this will give none in some cases.  *)
                    let value_set: range_set option = range_from_synth_value c in
                    value_set
                | AssignVariable(fvar_nest) ->
                    let valuesets: range_set option =
						match forward_range with
						| RangeForward -> Hashtbl.find map (index_nesting_to_string fvar_nest)
						| RangeBackward -> Hashtbl.find map (index_nesting_to_string flat_binding.tovar_index_nesting)
					in
                    valuesets
            ) in
            (* Convert the input ranges to output values if they
            exist.  *)
            if List.for_all ranges (Option.is_some) then
                let ranges = List.filter_map ranges Utils.id in
                let result_range =
					execute_conversion_on_range forward_range flat_binding.conversion_function ranges
				in
				let () = if (empty_range_set result_range) then
                    let () = Printf.printf "Generated an empty result range. \n" in
                    let () = Printf.printf "Input range was %s\n" (String.concat (List.map ranges range_set_to_string))  in
                    let () = Printf.printf "Output range is %s (size %s)\n" (range_set_to_string result_range) (range_size_to_string (range_size result_range)) in
					let () = Printf.printf "Conversion function was %s\n" (flat_single_variable_binding_to_string flat_binding) in
                    assert false
                else
                    ()
                in
                let () =
					match forward_range with
					| RangeForward ->
							Hashtbl.set result_tbl (index_nesting_to_string flat_binding.tovar_index_nesting) result_range
					| RangeBackward ->
							(* Not currently supporting many-to-one assignments here --- unclear range effects.  *)
							match flat_binding.fromvars_index_nesting with
								| [] -> () (* This is something that ust has to be defined, not assigned to.  *)
								| [(AssignConstant(_))] -> () (* No range reduction to specify on a constant.  *)
								| [(AssignVariable(fvar_nest))] ->
										Hashtbl.set result_tbl (index_nesting_to_string fvar_nest) result_range
								| x :: xs ->
										raise (RangeCheckException "Many to one assignments not implemented")
                in
                ()
            else
				let () = if options.debug_range_check then
					Printf.printf "No range found for variable %s\n" (index_nesting_to_string flat_binding.tovar_index_nesting)
				else () in
                (* If any of the inputs to this var have undefined
                   rangemaps, then we can't do anything here.  *)
            ()
    )) in
	(* Anything that doesn't have any bindings should still have it's value
		restrictions propagated from the user code.  *)
	let umapkeys = Hashtbl.keys unassigned_map in
	let () =
		ignore (List.map umapkeys (fun rkey ->
		if Hashtbl.mem result_tbl rkey then
			(* This key already had a binding that we already entered.  *)
			()
		else
			(* This is a variable without a mapping, so it doesn't have
			any validmap entry that can be applied.  Just propagate the
			rangemap.  Of course, it is a very valid question
			of why this matters if it doesn't have a binding: then
				answer is that we use the inputs to compute the correct
			answer form the user code, and skipping the application
			of the rangemap means we provide unrealistic
			inputs to the user code (i.e. it's just going to crash).
			*)
			let res = Hashtbl.add result_tbl rkey (Hashtbl.find_exn unassigned_map rkey) in
			match res with
			| `Ok -> ()
			| `Duplicate -> assert false
	)) in
    result_tbl

let generate_range_check_skeleton options typemap iospec apispec pre_binding =
    (* First, we need to generate what the real input/valid
    ranges are /before/ translation through the binding code. *)
    let transformed_io_rangemap = transform_rangemap_by options RangeBackward iospec.rangemap apispec.validmap pre_binding in
    (* Then, use these to call the range gen.  This generates
    some GIR conditions that are going to be used later, not
    any skeleton code --- perhaps they should generate
    some skeleton stuff instead?  Not sure there's any
    benefit to doing that, but it would perhaps
    be cleaner.  *)
    let result = generate_range_check options typemap iospec.livein transformed_io_rangemap iospec.rangemap iospec.validmap in
    let () = if options.dump_range_check then
        let () = Printf.printf "Generated range check (%s)\n" (match result with
        | None -> "None"
        | Some(cond) -> conditional_to_string cond
        ) in
        ()
    else ()
    in
    result

let check_valid_map rangemap validmap iomap =
	let keys = Hashtbl.keys iomap in
	let _ = List.map keys (fun key ->
		let range = Hashtbl.find_exn iomap key in
		if (empty_range_set range) then
			let () = Printf.printf "Error generating for key %s\n" (key) in
            let () = Printf.printf "Range for variable was %s\n" (match (Hashtbl.find rangemap key) with
            | None -> "None"
            | Some(r) -> range_set_to_string r
            ) in
            let () = Printf.printf "Validity range for variable was %s\n" (range_set_to_string (Hashtbl.find_exn validmap key)) in
			raise (RangeCheckException "Error! A mapping with a zero-sized domain has been generated!  That should have been filtered earlier")
		else
			()
	) in
	()

let generate_range_checks_skeleton options typemap iospec apispec pre_bindings =
    List.map pre_bindings (generate_range_check_skeleton options typemap iospec apispec)

let generate_input_ranges_skeleton options rangemap validmap binding =
	let () =
		if options.debug_input_map_generation then
			let () = Printf.printf "Starting to generate input range restrictions...\n" in
			let () = Printf.printf "Have rangemap keys %s and validmap keys %s\n"
				(String.concat ~sep:"," (Hashtbl.keys rangemap)) (String.concat ~sep:", " (Hashtbl.keys validmap))
			in
			()
		else ()
	in
	let transformed_io_validmap = transform_rangemap_by options RangeBackward rangemap validmap binding in
	let () =
		if options.debug_input_map_generation then
			let () = Printf.printf "Have transformed io validmap %s\n" (range_map_to_string transformed_io_validmap) in
			() else ()
	in
	(* Compute the intersection of the valid map and the
	rangemap, which is what the inputs should actually
	be generated from.  *)
	let keys = Hashtbl.keys transformed_io_validmap in
	let inputmap = Hashtbl.create (module String) in
	let _ = List.map keys (fun key ->
		let transformed_range = Hashtbl.find_exn transformed_io_validmap key in
		let user_range = Hashtbl.find rangemap key in
		let input_range = match user_range with
		| None -> transformed_range
		| Some(r) -> range_set_intersection r transformed_range
		in
		let _ = Hashtbl.set inputmap key input_range in
		()
	) in
	let () = check_valid_map rangemap transformed_io_validmap inputmap in
	let () =
		if options.debug_input_map_generation then
			let () = Printf.printf "For pre skeleton %s\n" (flat_skeleton_type_binding_to_string binding) in
			let () = Printf.printf "Generated input map %s\n" (range_map_to_string inputmap) in
			()
		else ()
	in
	inputmap

let generate_post_check_ranges options rangemap validmap binding =
	let transformed_rangemap = transform_rangemap_by options RangeForward rangemap validmap binding in
	(* Now, do the intersection of the transformed rangemap
		and the validmap.  *)
	let keys = Hashtbl.keys validmap in
	let reachable_validmap = Hashtbl.create (module String) in
	let _ = List.map keys (fun key ->
		let validrange = Hashtbl.find_exn validmap key in
		let inputrange = Hashtbl.find transformed_rangemap key in
		let resultset = match inputrange with
			| Some(s) -> range_set_intersection validrange s
			| None -> validrange
		in
		let _ = Hashtbl.set reachable_validmap key resultset in
		()
	) in
	reachable_validmap

(* Given some pre-mapping, and some api value restrictions, generate
the range of inputs that we should be testing with.  *)
let generate_input_ranges options rangemap validmap pre_bindings =
	List.map pre_bindings (generate_input_ranges_skeleton options rangemap validmap)

(* Given some pre-mapping, api valud restrictions, generate
the set of values that each variable can take /after/ the
valid-check is complete.  *)
let generate_post_check_ranges options rangemap validmap pre_bindings =
	List.map pre_bindings (generate_post_check_ranges options rangemap validmap)
