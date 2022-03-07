open Core_kernel;;
open Spec_definition;;
open Spec_utils;;
open Utils;;
open Alcotest;;

let test_synth_val_eq () =
	Alcotest.(check (bool)) "same bool" true (synth_value_equal (Int64V(1024)) (Int64V(1024)))

let test_remove_duplicates () =
	Alcotest.(check (int)) "same int" 1 (List.length (remove_duplicates synth_value_equal [Int64V(1024); Int64V(1024)]))


let main () =
	[
		"synth-val-equal",
		[
			test_case "veq" `Quick test_synth_val_eq;
			test_case "vdup" `Quick test_remove_duplicates
		];
	]
