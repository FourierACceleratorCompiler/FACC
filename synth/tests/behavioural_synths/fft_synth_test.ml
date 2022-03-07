open Spec_definition;;
open Core_kernel;;
open Alcotest;;
open Fft_synthesizer;;
open Fft_synthesizer_definition;;
open Spec_utils;;
open Options;;
open Float_compare;;

exception TestFail of string

(* Bit reverse *)
let test_bit_reverse () =
	let x = 5 in
	let y = bit_reverse x 16 in
	(* 10 is 1010, so should be 0101,
	   which is 5 *)
	Alcotest.(check (int)) "same int" 10 y

(* bit reversal *)
let test_bit_reversal () =
    let x = [0; 1; 2; 3; 4; 5; 6; 7] in
    let y = bit_reversal x in
	Alcotest.(check (list int)) "same lists" [0; 4; 2; 6; 1; 5; 3; 7] y

let test_sim () =
	let inputs = Hashtbl.create (module String) in
	let _ = Hashtbl.add inputs "v" (ArrayV([Float32V(2.0); Float32V(2.0)])) in
	let program =
		FSArrayOp(FSNormalize, FSVariable(Name("v")))
	in
	let () = runner program inputs in
	let flist = match Hashtbl.find_exn inputs "v" with
	| ArrayV([Float32V(x); Float32V(y)]) -> [x; y]
	| v ->
            let () = Printf.printf "Output was %s, expected two elt array." (synth_value_to_string v) in
            raise (TestFail "")
	in
	Alcotest.(check @@ float 0.1) "1.0 is 1.0" 1.0 (List.hd_exn flist)

let test_compare () =
	let inputs = Hashtbl.create (module String) in
	let _ = Hashtbl.add inputs "v" (ArrayV([Float32V(1.05); Float32V(1.05)])) in
	let fcomp = ((new fp_comp_mse default_options.mse_threshold)) in
	let res = Generic_sketch_synth.compare default_options (fcomp :> fp_comp) inputs inputs in
	let () = Alcotest.(check (bool)) "same bool" true res in
	let () = Printf.printf "MSE is %f\n" (fcomp#mse ()) in
	()


let main () = 
	[
		"bit-reversal",
		[
			test_case "Po2 test" `Quick test_bit_reversal
		];
		"bit-reverse",
		[
			test_case "4-bit-symmertric" `Quick test_bit_reverse
		];
		"execution-test",
		[
			test_case "test-sim" `Quick test_sim
		];
		"test-compare",
		[
			test_case "test-compare" `Quick test_compare
		]
	]
