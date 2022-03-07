type run_result =
	(* Filename of output JSON. *)
	| RunSuccess of string
	| RunFailure
	(* Perhaps need a timeout? *)

type test_result = {
	input: string;
	true_output: string option;
	(* Intermediate results * outputs *)
	measured_output: (string * string) option;
	passed: bool;
	vacuous: bool;
}
