open Core_kernel;;
open Cmdliner;;
open Generate_code;;
open Parse_iospec;;
open Parse_api;;
open Parse_classmap;;
open Spec_definition;;
open Options;;
open Gir;;
open Program;;

let () = Printexc.record_backtrace true;;

exception JSONGenException of string

type mode =
	| Wrapper
	| ValueProfiler

let get_mode mname =
	match mname with
	| "Wrapper" -> Wrapper
	| "ValueProfiler" -> ValueProfiler
	| _ -> raise (JSONGenException ("Unknown mode" ^ mname ^ " use Wrapper or ValueProfiler"))

(* This is a simple program that generates the JSON
wrappers needed for argument processing for a given
interface.

The idea is to pass in the IOSpec that it needs to handle,
this this produces some code that goes around that as a wrapper.
*)
let main modename iospec_file output_file =
    (* Yes, this is a terrible hack I am 100% going to regret
    because I have no clear understanding why it needs both.  *)
	let mode = get_mode modename in
	let options = { default_options with generate_timing_code = true } in
    let iospec, iotypemap, classspec = load_iospec options iospec_file in
	let empty_alignmenttbl = Hashtbl.create (module String) in
	let rec typemap = {
		variable_map = iotypemap;
		classmap = classspec;
		alignment_map = empty_alignmenttbl;
		(* Some of the udnerlyign functions assume
		the existance of this thing.  *)
		original_typemap = Some(typemap);
	} in
	let liveins, liveouts =
		match mode with
		| Wrapper -> iospec.livein, iospec.liveout
		(* Of course, this is not really trying to setup any 'working' code ---
		the reason we need the value profiler is because we can't directly generate
		the inputs because we can't draw them from a random input distribution.
		But we do want the value profiler to dump all the livein variables out,
		so that we can use them as inputs later.  So this intends to do just that.
		*)
		| ValueProfiler -> iospec.livein, iospec.livein
	in
	let emptymaptbl = Hashtbl.create (module String) in
    (* Most of these values aren't used since we don't gen
    the whole program, just the main.  *)
    let base_program = {
		funargs = iospec.funargs;
        livein = liveins;
        gir = EmptyGIR;
        liveout = liveouts;
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
	let returntype, _ = cxx_type_from_returnvar iotypemap iospec.returnvar in
	(* Generate the JSON wrapper: *)
	let main_helper_funcs, post_accel_helpers, main_func = cxx_main_function options false returntype base_program in
	let code = otherimports ^ "\n" ^ main_helper_funcs ^ "\n" ^ main_func in
	let () = assert (Filename.check_suffix output_file ".cpp") in
	Out_channel.write_all output_file ~data:code

(* TODO -- use the flag processing also.  *)
let mode =
	let doc = "Mode (wrapper|value_profiler)" in
	Arg.(required & pos 0 (some string) None & info [] ~docv:"Mode" ~doc)

let iospec =
	let doc = "IO Spec for the Function" in
	Arg.(required & pos 1 (some string) None & info [] ~docv:"IOSpec" ~doc)

let outfile =
	let doc = "Output file for the function wrap" in
	Arg.(required & pos 2 (some string) None & info [] ~docv:"Outfile" ~doc)

let info =
	let doc = "Generate JSON wrappers" in
	Term.info "JSONGen" ~doc

let args_t = Term.(const main $ mode $ iospec $ outfile)

let () = Term.exit @@ Term.eval (args_t, info)
