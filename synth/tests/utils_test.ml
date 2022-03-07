open Alcotest;;
open Utils;;

let test_fp_equality () =
    let y = float_equal 1.2119601964950562 1.2119601964950562 in
	let () = Printf.printf "Bool test\n" in
	Alcotest.(check (bool)) "same bool" true y

let test_log2 () =
	let y = log_2 128 in
    Alcotest.(check (int)) "same int" 7 y

let test_po2 () =
	let y = power_of_two 8 in
	Alcotest.(check (int)) "same int" 256 y

let utils_tests =
	[
        "fp-equals",
        [
            test_case "FP Equality" `Quick test_fp_equality
        ];
		"log2-test",
		[
			test_case "Log2 Check" `Quick test_log2
		];
		"po2-test",
		[
			test_case "Po2 Check" `Quick test_po2
		]
    ]
