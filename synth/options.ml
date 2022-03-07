open Range_definition;;
open Compile_settings;;

exception OptionsException of string

type backend_target =
	| CXX

let backend_target_from_string str =
	match str with
	| Some("C++") -> CXX
	| None -> CXX
	| _ -> raise (OptionsException ("Unknown target "))

let get_compiler_cmd target =
    match target with
    | CXX -> "g++"

let get_compiler_flags target =
	match target with
	| CXX -> [
		(* Include the clibs.  *)
		"libs/clib/fft_synth_lib.co libs/clib/synthesizer.co -Ilibs -fsanitize=address"
	]

type behavioural_synthesizer =
	| FFTSynth
	| NoSynthesizer

let get_behavioural_syn name =
	match name with
	| "FFT" -> FFTSynth
	| "None" -> NoSynthesizer
	| _ -> raise (OptionsException ("Unexpect behavioural synth " ^ name))

let range_factor_from_option o =
    match o with
            | Some(n) -> Finite(n)
            | None -> Infinite;

type options = {
	compile_settings: compile_settings;

	(* Generic configuration *)
	target: backend_target; (* Language target *)
	execution_folder: string; (* Where to keep the executables for testing *)
    compiler_cmd: string;
	compiler_flags: string list;
	post_synthesizer: behavioural_synthesizer;
	pre_accel_dump_function: string;
    execution_timeout: int;
	generate_timing_code: bool;

	(* Generation Parameters.  *)
	param_constant_generation_threshold: int;
	range_size_difference_factor: range_size_t;
	mse_threshold: float; (* Similarity threshold for generation. *)
	array_length_threshold: int; (* How big can arrays be? *)

	(* Testing configuration *)
	number_of_tests: int;
	all_tests: bool; (* Keep running tests after failue.  *)
	(* Allow testing of a single generated test only.  *)
	only_test: int option;

	(* Speedup configs to enable partial runs during debugging. *)
	skip_build: bool;
	stop_before_build: bool;
	skip_test: bool;
	use_parmap: bool;

	(* IR Dumps *)
	dump_typemaps: bool;
	dump_assigned_dimensions: bool;
    dump_skeletons: bool;
	dump_range_check: bool;
	dump_generate_gir: bool;
	dump_generate_program: bool;
	dump_test_results: bool;
    dump_behavioural_synth: bool;

	(* Pass debug *)
	debug_load: bool;
	debug_assign_dimensions: bool;
	debug_infer_structs: bool;
	debug_expand_typemaps: bool;
	debug_generate_skeletons: bool;
	debug_generate_gir: bool;
    debug_generate_program: bool;
	debug_generate_malloc: bool;
    debug_generate_code: bool;
	debug_build_code: bool;
	debug_generate_io_tests: bool;
    debug_iospec_manipulator: bool;
	debug_test: bool;
	debug_comparison: bool;

	(* GIR passes debug.  *)
	debug_gir_topology_sort: bool;
    debug_gir_reduce: bool;
	debug_gir_generate_define_statements: bool;

	(* SType passes debug.  *)
	debug_synth_topology: bool;

	(* Skeleton passes debug.  *)
	debug_skeleton_flatten: bool;
    debug_skeleton_constant_gen: bool;
	debug_skeleton_deduplicate: bool;
	debug_skeleton_verify: bool;
	debug_skeleton_multiple_lengths_filter: bool;
    debug_skeleton_range_filter: bool;
	debug_skeleton_filter: bool;
	debug_input_map_generation: bool;

	(* Post Synthesis type debug.  *)
	debug_post_synthesis: bool;
    debug_fft_synthesizer: bool;

	(* Range check debug *)
	debug_range_check: bool;

	(* Generic debug *)
	print_synthesizer_numbers: bool;
}

(* this is a dirty hack --- fix the multiple frontends
   one opt parser issue is a better way to deal with this. *)
let default_options = {
	compile_settings = default_compile_settings;

	(* Generic configuration *)
	target = CXX;
	execution_folder = "synth_temps";
	compiler_cmd = "g++";
	compiler_flags = get_compiler_flags CXX;
	post_synthesizer = FFTSynth; (* Really don't want to keep it this way long term.  But while FFT is target, it makes sense.  *)
	(* Specified the name of the function to be used to dump
	intermediate results.  *)
	pre_accel_dump_function = "pre_accel_dump_function";
    execution_timeout = 5;
    generate_timing_code = false;

    param_constant_generation_threshold = 4;
	range_size_difference_factor = Finite(3);
    mse_threshold = 0.01;
	array_length_threshold = 10000;

	(* Testing configuration *)
	number_of_tests = 100;
	all_tests = false;
	(* Allow testing of a single test only.  *)
	only_test = None;

	(* Speedup configs to enable partial runs during debugging. *)
	skip_build = false;
	stop_before_build = false;
	skip_test = false;
	use_parmap = true;

	(* IR Dumps *)
	dump_typemaps = false;
	dump_assigned_dimensions = false;
	dump_skeletons = false;
	dump_range_check = false;
	dump_generate_gir = false;
	dump_generate_program = false;
	dump_test_results = false;
    dump_behavioural_synth = false;

	(* Pass debug *)
	debug_load = false;
	debug_generate_skeletons = false;
	debug_assign_dimensions = false;
	debug_infer_structs = false;
	debug_expand_typemaps = false;
	debug_generate_gir = false;
	debug_generate_program = false;
	debug_generate_malloc = false;
	debug_generate_code = false;
	debug_build_code = false;
	debug_generate_io_tests = false;
    debug_iospec_manipulator = false;
	debug_test = false;
	debug_comparison = false;

	(* GIR passes debug.  *)
	debug_gir_topology_sort = false;
	debug_gir_reduce = false;
    debug_gir_generate_define_statements = false;

	(* SType passes debug.  *)
	debug_synth_topology = false;

	(* Skeleton passes debug.  *)
	debug_skeleton_flatten = false;
    debug_skeleton_constant_gen = false;
	debug_skeleton_deduplicate = false;
	debug_skeleton_verify = false;
	debug_skeleton_multiple_lengths_filter = false;
    debug_skeleton_range_filter = false;
    debug_skeleton_filter = false;
	debug_input_map_generation = false;

    (* Post synthesis passes debug.  *)
    debug_post_synthesis = false;
    debug_fft_synthesizer = false;

	debug_range_check = false;

	(* Generic debug *)
	print_synthesizer_numbers = false;
}
