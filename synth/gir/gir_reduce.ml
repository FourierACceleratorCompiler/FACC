open Core_kernel;;
open Options;;
open Gir;;
open Gir_utils;;

(* GIR reduce is a simple GIR pass that removes trivial artefacts
from other passes, e.g. nested sequences, deletes EmptyGIR etc. *)

(* Note that we can assume that the C compiler will do a lot
   of optimization so we don't have to do those here for the
   sake of the C compiler --- we need to simplify things so
   that future GIR passes can have less work to do.  *)
let rec reduce_rvalue (options: options) (rval: rvalue): rvalue =
    match rval with
    | Expression(expr) ->
            Expression(reduce_expression options expr)
and reduce_expression (options: options) expr: expression =
    let () = if options.debug_gir_reduce then
        Printf.printf "Before reduce expression: %s\n" (expression_to_string expr)
    else () in
    let result = match expr with
    | VariableReference(_) -> expr
	(* Remove calls to the identity function.  *)
    | FunctionCall(FunctionRef(Name("identity")), VariableList([v])) ->
                    VariableReference(v)
    | FunctionCall(_, _) -> expr
    | GIRMap(_, _) -> expr
    in
    let () = if options.debug_gir_reduce then
        Printf.printf "after reduce expression: %s\n" (expression_to_string result)
    else () in
    result

let rec reduce_gir (options: options) gir: gir =
    let () = if options.debug_gir_reduce then
        Printf.printf "Before reduce: %s\n" (gir_to_string gir)
    else () in
    let result = match gir with
	| Sequence(subitems) ->
			let flattened = List.concat (List.map subitems (fun subitem ->
                let reduced_subitem = reduce_gir options subitem in
				match reduced_subitem with
				| Sequence(subseqs) ->
						let sub_exed = List.map subseqs (reduce_gir options) in
						(* Now, remove the subseqs. *)
						let unsequed = List.concat (
							List.map sub_exed (fun f ->
								match f with
								| Sequence(xs) -> xs
								| x -> [x]
							)
						) in
						unsequed
				| x -> [x]
			)
			)
			in
			let filtered = List.filter flattened (fun sub ->
				match sub with
				| EmptyGIR -> false
				| _ -> true
			)
			in
			if (List.is_empty filtered) then
				EmptyGIR
			else
				Sequence(filtered)
	| LoopOver(gir, ind, max) ->
			LoopOver((reduce_gir options gir), ind, (reduce_expression options max))
	| FunctionDef(name, args, body, typmap) ->
			FunctionDef(name, args, (reduce_gir options body), typmap)
    | Expression(expr) ->
            Expression(reduce_expression options expr)
    | Assignment(ton, fromn) ->
            Assignment(ton, reduce_rvalue options fromn)
    | Definition(_, _, _) -> gir
	| IfCond(c, iftrue, iffalse) ->
			(* Could/should reduce the cond here --- don't think
			   we generate loads of redundant stuff though. *)
			IfCond(c, reduce_gir options iftrue, reduce_gir options iffalse)
	| Free(v) ->
			gir
    | Return(rval) ->
			Return(reduce_expression options rval)
    | EmptyGIR -> EmptyGIR
    in
    let () = if options.debug_gir_reduce then
        Printf.printf "After reduce: %s\n" (gir_to_string result)
    else () in
    result

(* Check for nested Sequence nodes. *)
let rec reduce_gir_check gir =
	match gir with
	| Sequence(subseq) ->
			let _ = List.map subseq (
				fun x ->
					match x with
					| Sequence(_) -> assert (false)
					| _ -> ()
			) in ()
	| _ -> ()
