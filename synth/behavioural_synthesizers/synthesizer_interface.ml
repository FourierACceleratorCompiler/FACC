open Core_kernel;;
open Fft_synthesizer;;
open Json_utils;;
open Options;;
open Spec_definition;;
open Generic_sketch_synth;;
open Run_definition;;
open Program;;
open Range_checker;;

exception PostSynthesizerException of string

let rec is_configuration_type typ =
    match typ with
    | Bool -> true
    | Int16 -> true
    | Int32 -> true
    | Int64 -> true
    | UInt16 -> true
    | UInt32 -> true
    | UInt64 -> true
    (* I think this should be domain-specific, or
    perhaps range specific?  E.g. in the FFT domain,
    individual floats are highly likely to be configuration
    parameters. *)
    | Float16 -> true
    | Float32 -> true
    | Float64 -> true
    | Unit -> false
    | Array(_, _) -> false
    | Pointer(st) ->
            is_configuration_type st
    (* FIXME --- need to probably support passing
    this if it has configuration parameters within it. *)
    (* TODO --- ^^ that is probably not the best fix,
    what we should really do is traverse the struct, stop
    at any arrays, and just pass in refs to the
    individual components of the struct we thing are config
    parameters (e.g. array lengths).  *)
    | Struct(name) -> false
    | Fun(_, _) -> false

(* Get the parameters that seem likely to be configuration
   parameters, e.g. not arrays of things, that are
   values, and nothing liveout, since that'll also
   be a value --- just to be passed to the synthesizer
   to allow it to make decisions.  *)
(* TODO -- make this the APISpec, it will be easier
to get heuristics for probably.  It's IOspec due
to ease of access to parameters--- for the APISpec,
we need to make sure the parameters appear in the output
Json files.  *)
let configuration_parameters_for typemap (iospec: iospec) (apispec: apispec) =
	let deadout = Utils.set_difference (Utils.string_equal) iospec.livein iospec.liveout in
	let config_deadout = List.filter deadout (fun deadvar ->
		let typ = Hashtbl.find_exn typemap.variable_map deadvar in
        is_configuration_type typ
	) in
	config_deadout

(* The iofiles are pairs of files, where one represents the inputs
   and the other represents the outputs.  We need to get
   the configuration parameters from the right one.  *)
let post_synthesis_io_pairs options apispec iospec iofiles program configuration_parameters =
	let result = List.filter_map iofiles (fun (test_results: test_result) ->
		let () = if options.debug_post_synthesis then
			Printf.printf "Loading inputs from file %s\n" (test_results.input)
		else () in
		(* Load variable assignments from the outp, and the true outp. *)
		(* Load only the values in configuration paramters here: *)
		let inp_values = load_value_map_from test_results.input in
		match test_results.measured_output, test_results.true_output with
		| Some(pre_acc_call, measured_outps), Some(true_outps) ->
			let () = if options.debug_post_synthesis then
				let () = Printf.printf "Loading true outputs from file %s\n" (measured_outps) in
				Printf.printf "Loading gened outputs from file %s\n" (true_outps)
			else () in
			let outp_values = load_value_map_from measured_outps in
			let true_outp_values = load_value_map_from true_outps in
			(* Don't need this for now.  *)
			(* let pre_accel_call_values = load_value_map_from pre_acc_call in *)
			let _ = List.map configuration_parameters (fun v ->
				Hashtbl.add outp_values v (Hashtbl.find_exn inp_values v)
			) in
			(* ((inputs, configs), required results) *)
			(* Only use this for behavioural synthesis if it passes the range
			   checker.  *)
			(* This doesn't use the valid in range of the accelerator,
			but rather the range detection pass inserted by the
			range_check_synth pass.  *)
			if inputs_in_range program inp_values then
				Some({
					input=outp_values;
					output=true_outp_values
				})
			else
				(* Values that fail the range check
				won't be used for behavioural synthesis.  *)
				None
		| None, None ->
			(* If they both failed, we just don't need to
			do anything.  *)
			None
		| _, _ ->
			(* If only one failed, there is an issue with
				the valid range detector.  Can't solve that
				here, so just skip this test (This seems
				like it might be a bad idea...) *)
			None
	) in result

let synthesize_post (options: options) typemap (iospec: iospec) (apispec: apispec) (program: program) io_files =
	(* This synthesizes a program based on the IO files,
	   and the io/apispecs that tries to bridge
	   the gap between the finished outputs and
	   the outputs required by the function.

	   This should be available to multiple backend
	   synthesizers, in partiuclar I think that
	   Feser 2015 might work well.  Critically,
	   post-synthesis is a /scalable/ problem --- it
	   can be done independently of the code that
	   comes before.   Pre-syntheses is a slightly
	   more challenging problem, as the underlying
	   synthesizer won't /know/ what the IO pairs
	   are until after it can generate some output
	   for a given input.  *)

	(* For now, use the FFT synth, which knows domain-specific
	answers to likely incompatibility problems
	(scaling/inverse scaling, bit reversal). *)

	(* The inputs to the synthesizer are in pairs ---
	   they must cover both the /configuration settings/
	   and the I/O values --- some things only need
	   to happen in certain configurations, e.g.
	   scaling in FFT/IFFT.  *)
	(* We want to use the APIspec for configuration parameters
	because it is easier to get heuristic support for the
	API spec --- impossible to get good heuristics for the user
	code. *)
    (* We use the IOSpec since we have values for that
    --- need to modify the first round of IO analysis
    to get values for the APISpec inputs here.  *)
	(* TODO -- need to fix issue when there is a name clash
	between input code and real code.  *)
	let configuration_parameters = configuration_parameters_for typemap iospec apispec in
    let () = if options.debug_post_synthesis then
        Printf.printf "Configuration parameters detected is: %s\n" (
            String.concat ~sep:", " (configuration_parameters)
        )
    else () in
	(* The type of the function we want to synthesize is:
		(iospec.liveout * config_params) -> iospec.liveout *)
	(* Hash type ((synth_value list * synth_value list) * synth_value list) list *)
    let io_pairs = post_synthesis_io_pairs options apispec iospec io_files program configuration_parameters in
    (* create the unified typemaps.  *)
    (* TODO _-- REALLy need to handle name clashes --- perhaps
       by prefixing things? *)
    let names = List.map (configuration_parameters @ iospec.liveout) (fun n -> Name(n)) in
	(* The synthesizer NEEDS to result in a function definition
	of that type, and a way to convert that function into
	a string for each of the backends (or into GIR).
	Right now, only C++ is a valid
	backend, so that's all that's required.  *)
    let () = if options.debug_post_synthesis then
        Printf.printf "Starting post-synthesis\n"
    else () in
	match io_pairs with
	| [] -> (* So, this means that everything failed.  *)
			(* Usualy this is due to some incorrect length
			assumptions --- in any case, we can't have
			the post-synthesizer vacuously proving that
			there are many 'valid' programs.  *)
			None
	| io_pairs ->
		match options.post_synthesizer with
		| NoSynthesizer -> None
		| FFTSynth -> fft_synth options typemap names program io_pairs
