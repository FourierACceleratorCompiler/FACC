open Cmdliner
open Gir
open Parse_compile_settings
open Parse_iospec
open Parse_classmap
open Parse_api
open Synthesize
open Options
open Spec_utils

let () = Printexc.record_backtrace true;;
let _ = Random.init 0

let main options iospec_file api_file  =
	let () = Printf.printf "Loading specifications...\n" in
    let iospec, iotypemap, ioclassmap = load_iospec options iospec_file in
    let api, apitypemap, apiclassmap, apialignment = load_target_api options api_file in
	let classspec = merge_maps ioclassmap apiclassmap in
	let () = Printf.printf "Synthesizing...\n" in
    let _ = run_synthesis options classspec iotypemap iospec apitypemap apialignment api in
	()

let optswrapper compile_settings_file iospec_file api_file dump_skeletons
        debug_generate_skeletons dump_assigned_dimensions debug_assign_dimensions
		debug_load debug_generate_gir dump_generate_gir
		debug_generate_program dump_generate_program
		print_synth_program_nums target execution_folder
		compiler_cmd debug_build_code debug_gir_topology_sort
		debug_generate_code number_of_tests debug_generate_io_tests
        debug_synth_topology debug_iospec_manipulator
		skip_build dump_test_results debug_test debug_skeleton_flatten
		stop_before_build only_test debug_gir_reduce debug_comparison
		all_tests post_synthesis_tool debug_post_synthesis
        dump_behavioural_synth debug_fft_synthesizer compiler_flags
		debug_range_check dump_range_check pre_accel_dump_function
		param_constant_generation_threshold debug_skeleton_constant_gen
		debug_skeleton_verify
		debug_skeleton_multiple_lengths_filter range_size_diff_factor
        debug_skeleton_range_filter debug_skeleton_filter
		execution_timeout skip_test mse_threshold
        debug_input_map_generation generate_timing_code
		array_length_threshold no_parmap
        debug_gir_generate_define_statements debug_infer_structs
		debug_expand_typemaps dump_typemaps debug_generate_malloc
		debug_skeleton_deduplicate =
    (* First make the options object, then call the normal main function.  *)
	let compile_settings = load_compile_settings compile_settings_file in
    let target_type = backend_target_from_string target in
    let options = {
		compile_settings = compile_settings;

        target = target_type;
		execution_folder = execution_folder;
		compiler_cmd = (match compiler_cmd with
		| Some(cmd) -> cmd
		| None -> get_compiler_cmd target_type
        );
		compiler_flags = (match compiler_flags with
        | Some(flags) -> String.split_on_char ';' flags
		| None -> get_compiler_flags target_type
		);
        post_synthesizer = get_behavioural_syn post_synthesis_tool;
		pre_accel_dump_function = pre_accel_dump_function;
		execution_timeout = execution_timeout;
        generate_timing_code = generate_timing_code;

		param_constant_generation_threshold = param_constant_generation_threshold;
        range_size_difference_factor = range_factor_from_option range_size_diff_factor;
		mse_threshold = mse_threshold;
        array_length_threshold = array_length_threshold;

        number_of_tests = number_of_tests;
		all_tests = all_tests;

		skip_build = skip_build;
		stop_before_build = stop_before_build;
		skip_test = skip_test;
		use_parmap = not no_parmap;
		
		only_test = only_test;

		dump_typemaps = dump_typemaps;
		dump_assigned_dimensions = dump_assigned_dimensions;
        dump_skeletons = dump_skeletons;
        dump_range_check = dump_range_check;
		dump_generate_gir = dump_generate_gir;
		dump_generate_program = dump_generate_program;
		dump_test_results = dump_test_results;
        dump_behavioural_synth = dump_behavioural_synth;

		debug_load = debug_load;
		debug_generate_skeletons = debug_generate_skeletons;
        debug_assign_dimensions = debug_assign_dimensions;
		debug_infer_structs = debug_infer_structs;
		debug_expand_typemaps = debug_expand_typemaps;
		debug_generate_gir = debug_generate_gir;
		debug_generate_program = debug_generate_program;
		debug_generate_malloc = debug_generate_malloc;
		debug_generate_code = debug_generate_code;
		debug_build_code = debug_build_code;
        debug_generate_io_tests = debug_generate_io_tests;
		debug_iospec_manipulator = debug_iospec_manipulator;
		debug_test = debug_test;
		debug_comparison = debug_comparison;

		debug_gir_topology_sort = debug_gir_topology_sort;
		debug_gir_reduce = debug_gir_reduce;
		debug_gir_generate_define_statements = debug_gir_generate_define_statements;

        debug_synth_topology = debug_synth_topology;
		
		debug_skeleton_flatten = debug_skeleton_flatten;
        debug_skeleton_constant_gen = debug_skeleton_constant_gen;
		debug_skeleton_deduplicate = debug_skeleton_deduplicate;
		debug_skeleton_verify = debug_skeleton_verify;
		debug_skeleton_multiple_lengths_filter = debug_skeleton_multiple_lengths_filter;
        debug_skeleton_range_filter = debug_skeleton_range_filter;
		debug_skeleton_filter = debug_skeleton_filter;
        debug_input_map_generation = debug_input_map_generation;

		debug_range_check = debug_range_check;

		debug_post_synthesis = debug_post_synthesis;
        debug_fft_synthesizer = debug_fft_synthesizer;

		print_synthesizer_numbers = print_synth_program_nums;
    }
    in
	let () =
		if Option.is_some options.only_test && options.skip_build then
			let () = Printf.printf "WARNING: Set --only-test and --skip-build may lead to inconsistent results\n" in
			Printf.printf "(--only-test renumbers executables)\n"
		else ()
	in
    main options iospec_file api_file

(* Deal with the commandline arguments. *)
(* Required positional args *)
let compile_settings =
	let doc = "Compile Settings" in
	Arg.(required & pos 0 (some string) None & info [] ~docv:"CompileSettings" ~doc)

let iospec =
	let doc = "IO Specification for the Function" in
	Arg.(required & pos 1 (some string) None & info [] ~docv:"IOSpec" ~doc)

let apispec =
    let doc = "IO Specification for the target API" in
    Arg.(required & pos 2 (some string) None & info [] ~docv:"APISpec" ~doc)

(* Configuration flags *)
let target =
	let doc = "Target generation language (default C++). Must be one of [C++] right now." in
	Arg.(value & opt (some string) None & info ["target"] ~docv:"Target" ~doc)
let execution_folder =
	let doc = "Folder in which to build/run all the potential programs" in
	Arg.(value & opt string "synthethizer_temps" & info ["execution-folder"] ~docv:"ExecFolder" ~doc)
let compiler_cmd =
	let doc = "Build compiler command for internal IO analysis (default g++)" in
	Arg.(value & opt (some string) None & info ["compiler-command"] ~docv:"CompilerCommand" ~doc)
let compiler_flags =
    let doc = "Any flags to pass onwards to the compiler, semi-colon separated (';')
(warning: requires defaults --- see options.ml for the defaults)" in
    Arg.(value & opt (some string) None & info ["compiler-flags"] ~docv:"CompilerFlags" ~doc)
let post_synthesis_tool =
	let doc = "Which synthesizer to use for post-synthesis (options are FFT or None now" in
    Arg.(value & opt string "FFT" & info ["post-synthesizer"] ~docv:"PostSynthesizer" ~doc)
let pre_accel_dump_function =
	let doc = "What to call the internal function for pre accelerator dumping (may have to be specified to avoid name clashes)" in
	Arg.(value & opt string "pre_accel_dump_function" & info ["pre-accel-dump-funcation-name"] ~docv:"PreAccelDumpName" ~doc)
let execution_timeout =
    let doc = "Timeout for running the user code (seconds). " in
    Arg.(value & opt int 5 & info["execution-timeout"] ~docv:"ExecutionTimeout" ~doc)
let generate_timing_code =
	let doc = "Generate wrappers to enable timing code" in
	Arg.(value & flag & info ["generate-timing-code"] ~docv:"GenerateTimingCode" ~doc)

(* Generation paramter flags *)
let param_constant_generation_threshold =
	let doc = "How large does the valid range of a parameter have to be before we will not try constants on it.  " in
	Arg.(value & opt int 4 & info["constant-generation-threshold"] ~docv:"ConstantGenerationThreshold" ~doc)
let range_size_difference_factor =
	let doc = "What fraction of inputs does the accelerator have to support to be useful? (int, so really 1 / support fraction) -- for out-of-context this should be really high.  For in-context, it should be quite low.  None indicates infinite range size differences supported (recommneded for out-of-context)" in
	Arg.(value & opt (some int) (Some(3)) & info["range-size-difference-factor"] ~docv:"InputSupportFraction" ~doc)
let mse_threshold =
	let doc = "What is the mean-sequared error difference that is allowable between functions? (normalized by n log n)" in
	Arg.(value & opt (float) 0.01 & info ["mean-squared-error"] ~docv:"MSE" ~doc)
let array_length_threshold =
    let doc = "What is the size of array that we can generate?" in
    Arg.(value & opt (int) 10000 & info["array-length-threshold"] ~docv:"ArrayLengthThreshold" ~doc)

(* Testing configuration flags.  *)
let number_of_tests =
    let doc = "Max number of tests to generate.  " in
    Arg.(value & opt int 30 & info ["number-of-tests"] ~docv:"NumberOfTests" ~doc)
let only_test =
	let doc = "Only test the testcase with the number specified here
	(usually to see the output of that test)" in
	Arg.(value & opt (some int) None & info ["only-test"] ~docv:"OnlyTest" ~doc)
let all_tests =
	let doc = "Run all tests, don't stop after failure" in
	Arg.(value & flag & info ["run-all-tests"] ~docv:"TestAll" ~doc)

(* Generic debug flags *)
let print_synth_option_numbers =
	let doc = "Print number of options the synthesizer has at each stage" in
	Arg.(value & flag & info ["print-synthesizer-numbers"] ~docv:"PrintSkeletonNumbers" ~doc)
let no_parmap =
	let doc = "Don't use Parmap" in
	Arg.(value & flag & info ["no-parmap"] ~docv:"NoParmap" ~doc)

(* Debug flags to be used with care. *)
let skip_build =
	let doc = "Only run testing and not generation. Debug only. " in
	Arg.(value & flag & info ["skip-build"] ~docv:"TestOnly" ~doc)
let stop_before_build =
	let doc = "Stop before building any candidate programs.  " in
	Arg.(value & flag & info ["stop-before-build"] ~docv:"StopBeforeBuild" ~doc)
let skip_test =
	let doc = "Only run post-synth and not testing. Debug only. " in
	Arg.(value & flag & info ["skip-test"] ~docv:"SkipTest" ~doc)

(* Print IR flags *)
let dump_skeletons =
    let doc = "Dump skeletons" in
    Arg.(value & flag & info ["dump-skeletons"] ~docv:"DumpSkeletons" ~doc)
let dump_typemaps =
	let doc = "Dump typemaps" in
	Arg.(value & flag & info ["dump-typemaps"] ~docv:"DumpTypemaps" ~doc)
let dump_range_check =
	let doc = "Dump Range checks" in
	Arg.(value & flag & info ["dump-range-check"] ~docv:"DumpRangeCheck" ~doc)
let dump_assigned_dimensions =
	let doc = "Dump assigned dimension variables" in
	Arg.(value & flag & info ["dump-dimensions"] ~docv:"DumpDimensions" ~doc)
let dump_generate_gir =
	let doc = "Dump GIR after generation but before program generation" in
	Arg.(value & flag & info ["dump-generate-gir"] ~docv:"DumpGenerateGIR" ~doc)
let dump_generate_program =
	let doc = "Dump Generated program in gir form" in
	Arg.(value & flag & info ["dump-generate-program"] ~docv:"DumpGenerateProg" ~doc)
let dump_test_results =
	let doc = "Dump the results of testing " in
	Arg.(value & flag & info ["dump-test-results"] ~docv:"DumpTestResults" ~doc)
let dump_behavioural_synth =
    let doc = "Dump behavioural synthesizer" in
    Arg.(value & flag & info ["dump-behavioural-synth"] ~docv:"DumpBehaviouralSynth" ~doc)

(* Debug GIR manipulation passes.  *)
let debug_gir_topology_sort =
	let doc = "Debug GIR topology passes" in
	Arg.(value & flag & info ["debug-gir-topology"] ~docv:"DebugGIRTopo" ~doc)
let debug_gir_reduce =
	let doc = "Debug the GIR reduction pass" in
	Arg.(value & flag & info ["debug-gir-reduce"] ~docv:"DebugGIRReduce" ~doc)
let debug_gir_generate_define_statements =
    let doc = "Debug GIR generate define statements" in
    Arg.(value & flag & info ["debug-gir-generate-define-statements"] ~docv:"DebugGIRDefines" ~doc)

(* debug stype passes.  *)
let debug_synth_topology =
    let doc = "Debug the synth topology sort pass" in
    Arg.(value & flag & info ["debug-synth-topology"] ~docv:"DebugSynthTopo" ~doc)
	
(* debug skeleton passes *)
let debug_skeleton_flatten =
	let doc = "Debug skeleton flatten pass" in
	Arg.(value & flag & info ["debug-skeleton-flatten"] ~docv:"DebugSkeletonFlatten" ~doc)
let debug_skeleton_constant_gen =
    let doc = "Debug the constant generation pass" in
    Arg.(value & flag & info ["debug-skeleton-constant-generation"] ~docv:"DebugSkeletonConstGen" ~doc)
let debug_skeleton_deduplicate =
	let doc = "Debug skeleton deduplicate" in
	Arg.(value & flag & info ["debug-skeleton-deduplicate"] ~docv:"DebugSkeletonDeduplicate" ~doc)
let debug_skeleton_verify =
	let doc = "Debug skeleton verify" in
	Arg.(value & flag & info ["debug-skeleton-verify"] ~docv:"DebugSkeletonVerify" ~doc)
let debug_skeleton_multiple_lengths_filter =
	let doc = "Debug multiple lengths removal pass" in
	Arg.(value & flag & info ["debug-multiple-length-filter"] ~docv:"DebugMultipleLengthFilter" ~doc)
let debug_skeleton_range_filter =
    let doc = "Debug the range filtering pass" in
    Arg.(value & flag & info ["debug-skeleton-range-filter"] ~docv:"DebugRangeFilter" ~doc)
let debug_skeleton_filter =
	let doc = "Debug the skeleton filter" in
	Arg.(value & flag & info ["debug-skeleton-filter"] ~docv:"DebugSkeletonFIlter" ~doc)
let debug_input_map_generation =
    let doc = "Debug skeleton input map generation" in
    Arg.(value & flag & info ["debug-input-map-generation"] ~docv:"DebugInputMapGeneration" ~doc)

(* Debug range passes *)
let debug_range_check =
	let doc = "Debug range check generaiton pass" in
	Arg.(value & flag & info ["debug-range-check"] ~docv:"DebugRangeCheck" ~doc)

(* Debug post synthesis *)
let debug_post_synthesis =
	let doc = "Debug post synthesis" in
	Arg.(value & flag & info ["debug-post-synthesis"] ~docv:"DebugPostSynthesis" ~doc)
let debug_fft_synthesizer =
    let doc = "Debug the behavioural synthesizer for FFTs" in
    Arg.(value & flag & info ["debug-fft-synth"] ~docv:"DebugFFTSynth" ~doc)

(* Debug pass internal flags *)
let debug_generate_gir =
	let doc = "Print debug information for generate_gir.ml" in
	Arg.(value & flag & info ["debug-generate-gir"] ~docv:"DebugGenGIR" ~doc)
let debug_generate_skeletons =
    let doc = "Print debug information for skeleton.ml" in
    Arg.(value & flag & info ["debug-skeletons"]  ~docv:"DebugSkeletons" ~doc)
let debug_assign_dimensions =
	let doc = "Print debug information for assign_dimensions.ml" in
	Arg.(value & flag & info ["debug-assign-dimensions"] ~docv:"DebugAssignDimensions" ~doc)
let debug_infer_structs =
	let doc = "Print debug information for infer_structs.ml" in
	Arg.(value & flag & info ["debug-infer-structs"] ~docv:"DebugInferStructs" ~doc)
let debug_expand_typemaps =
	let doc = "Print debug information for exapdn_typemaps.ml" in
	Arg.(value & flag & info ["debug-expand-typemaps"] ~docv:"DebugExpandTypemaps" ~doc)
let debug_load =
	let doc = "Debug the loading pass" in
	Arg.(value & flag & info ["debug-load"] ~docv:"DebugLoad" ~doc)
let debug_generate_program =
	let doc = "Debug the generate program pass" in
	Arg.(value & flag & info ["debug-generate-program"] ~docv:"DebugGenProgram" ~doc)
let debug_generate_malloc =
	let doc = "Debug malloc generation" in
	Arg.(value & flag & info ["debug-generate-malloc"] ~docv:"DebugGenMalloc" ~doc)
let debug_generate_code =
	let doc = "Debug the generate code pass" in
	Arg.(value & flag & info ["debug-generate-code"] ~docv:"DebugGenCode" ~doc)
let debug_build_code =
	let doc = "Debug code building pass" in
	Arg.(value & flag & info ["debug-build-code"] ~docv:"DebugBuildCode" ~doc)
let debug_generate_io_tests =
    let doc = "Debug generation of IO tests " in
    Arg.(value & flag & info ["debug-generate-io-tests"] ~docv:"DebugGenerateIO" ~doc)
let debug_iospec_manipulator =
	let doc = "Debug iospec manipulator (e.g. running the generated tests over the tested API)" in
	Arg.(value & flag & info ["debug-iospec-manipulator"] ~docv:"DebugIOSpecManip" ~doc)
let debug_test =
	let doc = "Debug the testing phase.  " in
	Arg.(value & flag & info ["debug-test"] ~docv:"DebugTest" ~doc)
let debug_comparison =
	let doc = "Debug comparison between true answer JSON files and accelerator produced JSON files." in
	Arg.(value & flag & info ["debug-comparison"] ~docv:"DebugComparison" ~doc)

(* Debug flags *)
let info =
	let doc = "Synthesize support for hardware accelerators" in
	Term.info "synth" ~doc

(* This command line parser is a shitshow, or I don't know how to use it.
   In any case, this has to stay in the right order.  *)
let args_t = Term.(const optswrapper $ compile_settings $ iospec $ apispec $ dump_skeletons $
    debug_generate_skeletons $ dump_assigned_dimensions $ debug_assign_dimensions $ debug_load $
	debug_generate_gir $ dump_generate_gir $ debug_generate_program $ dump_generate_program
	$ print_synth_option_numbers $ target $ execution_folder $ compiler_cmd $ debug_build_code $
	debug_gir_topology_sort $ debug_generate_code $ number_of_tests $ debug_generate_io_tests $
    debug_synth_topology $ debug_iospec_manipulator $ skip_build $ dump_test_results $ debug_test $
	debug_skeleton_flatten $ stop_before_build $ only_test $ debug_gir_reduce $ debug_comparison $
	all_tests $ post_synthesis_tool $ debug_post_synthesis
    $ dump_behavioural_synth $ debug_fft_synthesizer $ compiler_flags
	$ debug_range_check $ dump_range_check $ pre_accel_dump_function $
	param_constant_generation_threshold $ debug_skeleton_constant_gen $
	debug_skeleton_verify $
	debug_skeleton_multiple_lengths_filter $ range_size_difference_factor
    $ debug_skeleton_range_filter $ debug_skeleton_filter $ execution_timeout
	$ skip_test $ mse_threshold $ debug_input_map_generation $
    generate_timing_code $ array_length_threshold $ no_parmap $ debug_gir_generate_define_statements $
	debug_infer_structs $ debug_expand_typemaps $ dump_typemaps $ debug_generate_malloc
	$ debug_skeleton_deduplicate)
let () = Term.exit @@ Term.eval (args_t, info)
