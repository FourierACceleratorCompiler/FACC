open Core_kernel;;
open Spec_definition;;
open Expand_typemaps;;
open Generate_gir;;
open Generate_programs;;
open Generate_code;;
open Build_code;;
open Generate_io_tests;;
open Skeleton;;
open Skeleton_utils;;
open Iospec_manipulator;;
open Executable_test;;
open Post_synthesis;;
open Options;;
open Code_checker;;

exception TypeException of string

(* TODO --- Find a better way to deal with bounds.  *)
let maxarraylength = 100;;
let floatmax = 100.0;;
let intmax = 1000;;

(* This is a debug flag to make it easier to peer into
   what the post-synthesis passes are doing without
   being overloaded.  Also to make repeat synthesis
   from the same inputs much faster.  *)
let reduce_programs (opts:options) programs =
    let filtered_programs = match opts.only_test with
        | None -> programs
        | Some(filter) ->
                match List.nth programs filter with
				| None -> raise (OptionsException ("No program with number " ^ (string_of_int filter) ^ " found"))
				| Some(prog) -> [prog]
    in
    filtered_programs

let run_synthesis (opts:options) (classmap: (string, structure_metadata) Hashtbl.t) (iospec_typemap) (iospec: iospec) (apispec_typemap) (apispec_alignment) (api: apispec) =
	(* Assign possible dimension equalities between vector types.  *)
	(* This updates the type ref tables in place, so no reassigns needed.  *)
	let () = if opts.print_synthesizer_numbers then
		Printf.printf "Starting synthesis!%!\n"
	else () in
	let unified_type_maps = generate_unified_typemaps opts classmap iospec iospec_typemap api apispec_typemap apispec_alignment in
	let () = if opts.print_synthesizer_numbers then
		let () = Printf.printf "Generated the dimensions%!\n" in
		let () = Printf.printf "Have %d possible annotated typemaps\n%!" (List.length unified_type_maps) in
		()
	else () in
    (* Generate the possible skeletons to consider *)
    let skeleton_pairs = generate_all_skeleton_pairs opts unified_type_maps iospec api in
	let () = if opts.dump_skeletons then
		Printf.printf "%s%s\n" "Skeletons are%! " (flat_skeleton_pairs_and_ranges_to_string skeleton_pairs)
	else
		() in
    let () = if opts.print_synthesizer_numbers then
        Printf.printf "Number of skeletons generated is %d%!\n" (List.length skeleton_pairs)
    else () in
	(* Do some lenvar expansion to avoid incompatible lenvar
	   at the next stages? *)
	(* Should also do some assignment merging here, e.g. if
		we have structs that are exactly the same type.  *)
	(* Do some internal simulation on the pairs? *)
	(* Generate the actual conversion functions between the code pairs *)
	let conversion_functions = generate_gir opts iospec api skeleton_pairs in
    let () = if opts.print_synthesizer_numbers then
        Printf.printf "Number of conversion pairs generated is %d%!\n" (List.length conversion_functions)
    else () in
	(* Generate program from the pre/post convsersion function pairs. *)
	let programs = generate_programs opts iospec api conversion_functions in
    let () = if opts.print_synthesizer_numbers then
        Printf.printf "Number of programs from these pairs is %d%!\n" (List.length programs)
    else () in
    let reduced_programs = reduce_programs opts programs in
	(* Do some opts? *)
    (* Do some filtering pre-generation? *)
	(* Generate some code.  *)
	(* START ROUND 1 Of Tests *)
    (* True means dump intermediates  --- needed for later synthesis rounds.  *)
	let generated_code = generate_code opts api iospec true reduced_programs in
	let () = if opts.print_synthesizer_numbers then
		Printf.printf "Number of codes generated is %d%!\n" (List.length generated_code)
	else () in
	if opts.stop_before_build then
		()
	else (
	(* Build the code *)
	let code_files = build_code opts iospec api generated_code in
	let () = if opts.print_synthesizer_numbers then
		let () = Printf.printf "Number of codes built is %d\n" (List.length code_files) in
		Printf.printf "Generating tests for the %d programs\n" (List.length reduced_programs)
	else () in
	(* Generate some I/O tests.  *)
	let io_tests = generate_io_tests opts iospec reduced_programs in
	let () = if opts.print_synthesizer_numbers then
		Printf.printf "Number of IO tests generated is %d%!\n" (List.length (match io_tests with | (_, t) :: _ -> t | _ -> assert false))
	else () in
	(* Generate the 'correct' responses for the IO tests *)
	let real_response_files = compute_default_results opts iospec io_tests in
	let () = if opts.print_synthesizer_numbers then
		Printf.printf "Real responses generated\n"
	else () in
	(* Try the code until we find one that works.  *)
	let working_codes = find_working_code opts code_files io_tests real_response_files in
	(* END Round 1 of Tests *)

    (* Run post-synthesis *)
	let () = if opts.print_synthesizer_numbers then
		Printf.printf "Starting post synthesis (%d programs)\n%!" (List.length working_codes)
	else ()
	in
    let post_synthesis_programs = run_post_synthesis opts iospec api reduced_programs working_codes in
    (* TODO --- regenerate the code and output the working ones
        in an output file!. *)
    let working_programs = List.filter_map post_synthesis_programs (fun (p, passing) -> if passing then Some(p) else None) in
	(* Do some opts? *)
    (* Do not dump intermediates in the final result! *)
	let () = Printf.printf "===============================================\n" in
	let working_programs_code = generate_code opts api iospec false working_programs in
    let () = print_working_code opts api working_programs_code in
	let () = print_working_code_warnings opts working_programs_code in
    let () = Printf.printf "Done!\n" in
    ()
	)
;;
