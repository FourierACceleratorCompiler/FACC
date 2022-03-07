open Core_kernel;;
open Yojson;;
open Yojson.Basic.Util;;
open Run_definition;;
open Options;;
open Utils;;
open Spec_definition;;
open Float_compare;;
open Json_utils;;
open Program;;
open Program_utils;;
open Range_checker;;

(* Largely, we assume taht j1 and j2 have the same members
this will sometimes crash and sometimes spuriously go true
if they do not.  *)
let rec compare_jsons options fcomp j1 j2 =
	(* let () = Printf.printf "JSON j1 is %s \n%!" (Yojson.Basic.pretty_to_string j1) in
	let () = Printf.printf "JSON j2 is %s \n%!" (Yojson.Basic.pretty_to_string j2) in *)
    let j1_members = keys j1 in
	let result = List.for_all j1_members (fun mem ->
		compare_json_elts options fcomp (j1 |> member mem) (j2 |> member mem)
	) in
	(* let () = Printf.printf "Exiting comparison\n%!" in *)
	result

and compare_json_elts options fcomp e1 e2 =
	let result = match (e1, e2) with
	  | `Assoc(njson_pairs1), `Assoc(njson_pairs2) ->
			  let sorted_p1 = List.sort njson_pairs1 (fun (s1, j1) -> fun (s2, j2) ->
				  String.compare s1 s2
			  ) in
			  let sorted_p2 = List.sort njson_pairs2 (fun (s1, j1) -> fun (s2, j2) ->
				  String.compare s1 s2
			  ) in (
			  match List.zip sorted_p1 sorted_p2 with
			  | Ok(ls) ->
					  (* let () = Printf.printf "Looping in match%!\n" in *)
					  let r = List.for_all ls (fun ((name1, json1), (name2, json2)) ->
						  ((String.compare name1 name2) = 0) && (compare_json_elts options fcomp json1 json2)
					  ) in
					  r
			  | Unequal_lengths -> false
			  )
	  | `Bool(b1), `Bool(b2) ->
			  (Bool.compare b1 b2) = 0
	  | `Float(f1), `Float(f2) ->
              fcomp#compare f1 f2
	  | `Int(i1), `Int(i2) ->
			  i1 = i2
	  | `List(l1), `List(l2) ->
			  (* let () = Printf.printf "Looping in list\n%!" in *)
			  ((List.length l1) = (List.length l2)) &&
			  List.for_all (List.zip_exn l1 l2) (fun (i1, i2) ->
				  compare_json_elts options fcomp i1 i2
			  )
	(* I mean, this could be true, but not sure why it would appear.  *)
	  | `Null, `Null -> assert false
	  | `String(s1), `String(s2) ->
			  (String.compare s1 s2) = 0
	  | `Null, other ->
			  let () = Printf.printf "Comparing %s and Null!" (Yojson.Basic.pretty_to_string other) in
			  assert false
	  | _, `Null -> assert false
	  | _ -> false
	in
	if result then
		result
	else
		let () = if options.debug_comparison then
			Printf.printf "Comparison between %s and %s returned false!\n"
                (Yojson.Basic.pretty_to_string e1)
                (Yojson.Basic.pretty_to_string e2)
		else () in
		result

let compare_outputs options f1 f2 =
    (* Open both in Yojson and parse. *)
	(* let () = Printf.printf "Comparing files %s and %s%!\n" (f1) (f2) in *)
    let f1_json = Yojson.Basic.from_file f1 in
    let f2_json = Yojson.Basic.from_file f2 in
	let fcomp = ((new fp_comp_mse options.mse_threshold) :> fp_comp) in
	let compare_result = compare_jsons options fcomp f1_json f2_json in
	let result = compare_result && (fcomp#result options) in
	(* let () = Printf.printf "Comparison finished\n%!" in *)
	result

let check_if_code_works (options:options) (program: program) execname test_no generated_io_tests correct_answer_files =
	(* TODO --- perhaps a parmap here?  Need to make sure the output files don't overlap if so. *)
    (* This might also end up being limited by disk performance.  Perhaps using
    a ramdisk would help? *)
	let tests_and_results = List.zip_exn generated_io_tests correct_answer_files in
	 (* Need to keep the output files distict for
	further analysis.  *)
	(* We could do something like 'for_all', but we don't
	really want to run every test for every executable ---
	most are going to fail immediately.  *)
	let ()  = if options.debug_test then
		Printf.printf "Starting tests for executable %s\n" execname
	else () in
	let res = List.map tests_and_results (fun (testin, testout) ->
            (
            (* Get an output name for this test.  *)
            let experiment_outname = testin ^ "_outtmp_" ^ (string_of_int test_no) ^ ".json" in
            (* Also get the output name for the intermediate
            (pre accelerator call) variable values.  *)
            let pre_accel_variables_outname = testin ^ "_outtmp_pre_accel_" ^ (string_of_int test_no) ^ ".json" in
            (* Run the program on this test input.  *)
            (* TODO --- maybe we should time this out?  Less
            clear whether we need that here than we did with
            the user code (where we also don't timeout) *)
            let timeout = string_of_int options.execution_timeout in
            let cmd = "timeout " ^ timeout ^ " " ^ execname ^ " " ^ testin ^ " " ^ experiment_outname ^ " " ^ pre_accel_variables_outname in
            let () = if options.debug_test then
                Printf.printf "Running test command %s%!" cmd
            else () in
            let result =
                if options.skip_test then
                    if Sys.file_exists experiment_outname then
						0
                    else
                        1
                else
                    Sys.command cmd in
			let () = if options.debug_test then
				Printf.printf "Done\n%!"
			else ()
			in
			let output_checked_result = 
				if result = 0 then
				(* Check if there is any UB in this.  Note that
				this may have to be language-depedent.  *)
				if outfile_has_errors options (get_io_typemap program) experiment_outname then
					let () = if options.debug_test then
						let () = Printf.printf "File %s seems to have errors: marking as failed.  " experiment_outname in
						()
					else ()
					in
					(* Failure code if the JSON does't checkout. *)
					1
				else
					0
				else
					(* Already failed --- just propagate that.  *)
					result
			in
            let same_res = match testout with
            | RunFailure -> 
                (* We could be a bit smarter than this.  Anyway,
                I'm hoping not to deal with too many failures,
                they're more of an edge case(? famous last words). *)
				(* Current aim is to work in a C style (since that is what
				we are targetting) where a run failure (segfault) is
				UB, so we can do anything we want -- including running
				an accelerator that doesn't crash as expected :*)
				(* This doesn't do any good in showing 'correctness' though,
				it's more of a vacuous property.   *)
				{
					input=testin;
					true_output=None;
					measured_output=None;
					passed=true;
					vacuous=true;
				}
            | RunSuccess(outf) ->
					let () = assert (not (outfile_has_errors options (get_io_typemap program) outf)) in
                    if output_checked_result = 0 then
						let () = assert (not (outfile_has_errors options (get_io_typemap program) experiment_outname)) in
						let inp_values = load_value_map_from testin in
						let inp_passes = inputs_in_range program inp_values in
                        {
                            input=testin;
                            true_output=Some(outf);
                            measured_output=Some((pre_accel_variables_outname, experiment_outname));
							passed=(compare_outputs options experiment_outname outf);
							(* if the input didn't pass the range check,
							we just called the origincal code, so this
							was vacuous.  *)
							vacuous = not inp_passes;
                        }
                    else
                        (* Run of accelerator failed --- this probably
                        shouldn't have happened.  *)
                        let () = Printf.printf "Warning: Accelerator failed on input (input file %s): accelerator bounds should be specified for better performance. \n" (testin) in
                        (* Say this was a non-match.  *)
                        {
                            input=testin;
                            true_output=Some(outf);
                            measured_output=None;
							passed=false;
							vacuous=false;
                        }
            in
            let () =
                if options.dump_test_results then
                    let () = Printf.printf "Executbale %s and test %s had result %b\n%!" (execname) (same_res.input) (same_res.passed) in
                    ()
                else ()
            in
            same_res
            )
		) in
	(* Glue together the results.  *)
	let total_count = List.length res in
	let passed_count = List.count res (fun (result) -> result.passed) in
	let passed = (total_count = passed_count) in
    (* OK: so this is a terrible hack, but when we just can't line up with the usercode
    sparsity, everything will 'pass' by default, and we'd like to avoid that.  I think there
    is a better place to do this, but I'm not 100% sure where.  IMO it might need it's own pass
    post-test generation where we go through and are like "this is clearly buggy, this jus tmissed
    due to bad luck" etc. *)
    (* Anyway, this makes sure that there is at least one non-vacuous testcase *)
    let lucky_pass = List.for_all res (fun res -> res.vacuous) in
	let valid_passes = List.count res (fun res -> not res.vacuous) in
	let () = Printf.printf "For executable %s, passed cound is %d of %d tests (%d are vacuous: luck pass is %b) \n%!" (execname) (passed_count) (total_count) (total_count - valid_passes) (lucky_pass) in
	res, (passed && (not lucky_pass))

let find_working_code (options:options) (generated_executables: (program * string) list) generated_io_tests correct_answer_files =
	let () = if options.debug_test then
		let () = Printf.printf "Number of tests is %d\n" (List.length generated_executables) in
		() else () in
	let groups = List.zip_exn generated_executables (List.zip_exn generated_io_tests correct_answer_files) in
	let test_no = ref 0 in
	let result = List.map groups (fun ((program, executable), ((_, inps), outps)) ->
		test_no := !test_no + 1;
		check_if_code_works options program executable !test_no inps outps
	) in
	let () = if options.debug_test then
		Printf.printf "Done executing tests\n"
	else ()
	in
	result

let print_working_code options (apispec: apispec) working_list =
    let output_dir = options.execution_folder ^ "/output" in
	let extension = Build_code.get_extension options in
    let result = Sys.command ("mkdir -p " ^ output_dir) in
    let () = assert (result = 0) in
    let numbers = Build_code.generate_file_numbers (List.length working_list) in
    let fnames = List.map (List.zip_exn working_list numbers) (fun ((prog, code), number) ->
        let filename = output_dir ^ "/option_" ^ number ^ extension in
        let () = Out_channel.write_all filename ~data:code in
        filename
    ) in
	let () = Printf.printf "Working tests are in the source files for executables %s\n"
	(String.concat ~sep:"," fnames) in
	let () = Printf.printf "There were %d working in total\n" (List.length fnames) in
	let all_flags = options.compiler_flags @ apispec.compiler_flags in
	let () = Printf.printf "Required compiler flags to build are: %s\n" (String.concat ~sep:" " all_flags) in
	()
