open Core_kernel;;
open Cmdliner;;
open Options;;
open Spec_definition;;
open Spec_utils;;
open Parse_iospec;;
open Parse_classmap;;
open Generate_io_tests;;
open Executable_test;;
open Program;;
open Iospec_manipulator;;

exception InjectivityFailure of string

let () = Printexc.record_backtrace true;;

let batch_size = 100;;

let main specname num =
    (
    let options = { default_options with
        generate_timing_code = true;
        execution_folder = "injectivity_comparison_temps";
        number_of_tests = batch_size;
    } in
    let iospec, iotypemap, classspec = load_iospec options specname in
    let empty_alignmenttbl = Hashtbl.create (module String) in
    let rec typemap = {
        variable_map = iotypemap;
        classmap = classspec;
        alignment_map = empty_alignmenttbl;
        original_typemap = Some(typemap)
    } in
    let dummy_program: program = {
        funargs = iospec.funargs;
        livein = iospec.livein;
        gir = EmptyGIR;
        liveout = iospec.liveout;
        typemap = typemap;
        range_checker = None;
        post_behavioural = None;
        returnvar = iospec.returnvar;
        user_funname = "TODO";
        generated_funname = "TODO";
        api_funname = "TODO";
        fundefs = [];
        inputmap = iospec.rangemap;
        original_pairs = None;
		allocated_variables = [];
    } in
    (* Keep a hashtable of the generated results. *)
    let number_tested = ref 0 in
	let duplicate_count = ref 0 in
    (* Could make this int and have more false collisions for less memory footprint. *)
    let tbl = Hashtbl.create (module Int) in
    let outtbl = Hashtbl.create (module Int) in
    (* Generate the IO tests in batches to avoid too much overhead.  *)
    let () =
    while !number_tested < num do
		(* Seed RNG repeatedly since this method uses a lot
		of randomness.  *)
		let () =
		if !number_tested % 100000 = 0 then
			let () = Printf.printf "Seedng %d\n" (!number_tested) in
			Random.init !number_tested
		else ()
		in
        (* Generate a new batch: *)
        let test_batch =
            match generate_io_tests options iospec [dummy_program] with
            | [(_,tests)] -> tests
            | _ -> raise (InjectivityFailure "Expected only a single result!")
        in
        (* Get rid of duplicate inputs, doing this with an MD5 hash.  *)
        let results, success_count = generate_results_for options typemap iospec test_batch in
		(* We could not do this, but also this is already going to be slow --- wasting time
		on invalid inputs is, well, a waste of time.  *)
		let () = assert (success_count > 0) in
        (* Run failures not allowed here, since we are looking for failure-free duplication.  *)
        (* Duplication check dubiously done with a md5sum command. *)
        ignore(List.map (List.zip_exn results test_batch) (fun (result, test) ->
			let resoutfile = match result with
            | RunFailure -> ""
            | RunSuccess(outfile) ->
					let inp_md5 = Md5.hash (Md5.digest_file_blocking test) in
					let out_md5 = Md5.hash (Md5.digest_file_blocking outfile) in
                match Hashtbl.find tbl inp_md5 with
                | Some(t) ->
                        (* Skip -- this md5 has already been tested.  *)
						outfile
                | None ->
                        let () = ignore(Hashtbl.add tbl inp_md5 true) in
                        let () = match Hashtbl.add outtbl out_md5 true with
                        | `Ok -> ()
                        | `Duplicate ->
                                duplicate_count := !duplicate_count + 1
                        in
                        let () = number_tested := !number_tested + 1 in
						outfile
			in
			let _ = Sys.command ("rm " ^ resoutfile ^ " " ^ test) in
			()
        )
        )
    done in
    let () = Printf.printf "Tested %d unique inputs, found %d duplicates\n" (!number_tested) (!duplicate_count) in
    ()
    )

let iospec =
	let doc = "IO Spec for the Function" in
	Arg.(required & pos 0 (some string) None & info [] ~docv:"IOSpec" ~doc)

let number =
    let doc = "Number of Examples" in
    Arg.(required & pos 1 (some int) None & info [] ~docv:"Number" ~doc)

let info =
	let doc = "Compute the epsilon-injectivity of a function" in
	Term.info "EpsilonInjectivity" ~doc

let args_t = Term.(const main $ iospec $ number)

let () = Term.exit @@ Term.eval (args_t, info)
