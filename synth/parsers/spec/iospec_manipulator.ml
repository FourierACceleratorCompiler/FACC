open Core_kernel;;
open Spec_definition;;
open Spec_utils;;
open Run_definition;;
open Options;;
open Json_utils;;
open Program;;
open Program_utils;;

exception SparsityException of string

let generate_results_for (opts: options) typemap (iospec: iospec) inp_files =
	(* Perhaps this should be parallelized? *)
	let progexec = iospec.execcmd in
	let results = Utils.parmap opts (fun infile ->
        let outfile = infile ^ "_result.json" in
		let timeout = (string_of_int opts.execution_timeout) in
        let runcmd = "timeout " ^ timeout ^ " " ^ progexec ^ " " ^ infile ^ " " ^ outfile in
		let () = if opts.debug_iospec_manipulator then
			let () = Printf.printf "Runcmd is %s\n%!" (runcmd) in
			()
		else () in
        (* TODO -- Need to have a timeout here.  *)
        let res = Sys.command runcmd in
		let () = if opts.debug_iospec_manipulator then
			let () = Printf.printf "Finished with result: %d\n" (res) in
			() else ()
		in
        if res <> 0 then
            RunFailure
        else
			if outfile_has_errors opts typemap outfile then
				RunFailure
			else
				RunSuccess(outfile)
	) inp_files
	in
	let success_count = List.count results (fun r -> match r with
		| RunFailure -> false
		| RunSuccess(_) -> true
	) in
	results, success_count

let compute_default_results opts iospec (inp_files: (program * string list) list) =
	let results = List.map inp_files (fun (program, inp_file_set) ->
		let results, succ_count = generate_results_for opts (get_io_typemap program) iospec inp_file_set in
		let () = if succ_count = 0 then
				let () = Printf.printf "Can't find any working inputs to user code: likely too sparse (try more inputs?)" in
                ()
		else ()
		in
		results, succ_count
	) in
	let _ = if List.for_all results (fun (_, c) -> c = 0) then
		raise (SparsityException "Error: FACC was able to find 0 working inputs to the user code: likely too sparse (try profiling the code and providing likely inputs).")
	else
		()
	in
	List.map results (fun (v, _) -> v)
