open Core_kernel;;
open Range_definition;;
open Range;;
open Spec_definition;;
open Gir_eval;;
open Program;;

exception RangeCheckerException of string

(* This checks whether these inputs satisfy the
	range_check condition specified in program.

	Note that the user of this function should be
	careful, as range checks are typically applied
	/after/ argument mapping, and so values
	may have changed/not line up.

	It is intended to be used with the outputs
	from the pre_acc_call results.
	*)
let inputs_in_range (program: program) inputs =
	match program.range_checker with
	| None ->
			(* No range checker, so
			the inputs are in range :) *)
			true
	| Some(rprog) ->
		match eval_conditional rprog.condition inputs with
        | BoolV(v) -> v
        | _ -> raise (RangeCheckerException "Conditional produced non bool result!")
