open Alcotest

let () =
	run "Test"
	((Utils_test.utils_tests) @ (Spec_test.main ()) @ (Generate_code_test.main ()) @ (Fft_synth_test.main ()))
