open Core_kernel;;
open Cmdliner;;
open Generate_io_tests;;
open Parse_iospec;;
open Parse_api;;
open Parse_classmap;;
open Spec_definition;;
open Options;;
open Gir;;
open Program;;

let () = Printexc.record_backtrace true;;

let main iospec_file num_tests output_folder =
	let options = { default_options with
		number_of_tests = num_tests;
		execution_folder = output_folder;
		(* We could use parmap, but just generating a few IO examples it probably isn't
		   required.  It can obscure error messages (e.g. if there are ambiguities
		   in the iospec used.) *)
		use_parmap = false;
		debug_generate_io_tests = true;
	} in
	let iospec, iotypemap, classspec = load_iospec options iospec_file in
	let empty_alignmenttbl = Hashtbl.create (module String) in
	let emptymaptbl = Hashtbl.create (module String) in

	(* Construct the program from the input IO map.  We assume
	   that there is no ambiguity in the iospec.   That is, we
	   do not run type inference or length inference --- we expect
	   that the IO map has been anontated as such.  *)
	let rec typemap = {
		variable_map = iotypemap;
		classmap = classspec;
		alignment_map = empty_alignmenttbl;
		original_typemap = Some(typemap);
	} in
	let base_program = {
		funargs = iospec.funargs;
		livein = iospec.livein;
		liveout = iospec.liveout;
		gir = EmptyGIR;
		typemap = typemap;
		range_checker = None;
		post_behavioural = None;
		returnvar = iospec.returnvar;
		user_funname = "TODO";
		generated_funname = iospec.funname;
		api_funname = "TODO";
		fundefs = [];
		inputmap = emptymaptbl;
		original_pairs = None;
		allocated_variables = [];
	} in
	generate_io_tests options iospec [base_program]

let iospec =
	let doc = "IO Spec for function" in
	Arg.(required & pos 0 (some string) None & info [] ~docv:"IOSpec" ~doc)

let number =
	let doc = "Number of Examples" in
	Arg.(required & pos 1 (some int) None & info [] ~docv:"Number" ~doc)

let destfolder =
	let doc = "Distination folder" in
	Arg.(required & pos 2 (some string) None & info [] ~docv:"Destination" ~doc)

let info =
	let doc = "Generate IO Examples" in
	Term.info "IOGen" ~doc

let args_t = Term.(const main $ iospec $ number $ destfolder)
let () = Term.exit @@ Term.eval (args_t, info)
