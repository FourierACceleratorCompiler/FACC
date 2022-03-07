open Core_kernel;;
open Generate_code;;
open Spec_definition;;
open Options;;
open Alcotest;;

let test_malloc_results () =
	let testtmap = {
		variable_map = Hashtbl.create (module String);
		classmap = Hashtbl.create (module String);
		alignment_map = Hashtbl.create (module String);
		original_typemap = None
	}
	in
	let vars = (Array(Array(Float32, Dimension(DimConstant(10))), Dimension(DimConstant(10)))) in
    (* TODO --- obviouysly this mallocs way too much space --- obviously this is also not the only string
    that would do this shit.  *)
    Alcotest.(check (string)) "same string" "float** testvar = (float**) facc_malloc (0, sizeof(float)*10*10);
for (int i1 = 0; i1++; i1 < 10) {
float* testvar_sub_element = (float*) facc_malloc (0, sizeof(float)*10);;
testvar[i1] = testvar_sub_element;
}" (cxx_definition_synth_type_to_string default_options testtmap None true vars vars "" "testvar")

let main () =
	[
		"malloc-test",
		[
			test_case "malloc_it" `Quick test_malloc_results;
		];
	]
