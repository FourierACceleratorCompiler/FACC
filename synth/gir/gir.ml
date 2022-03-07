open Core_kernel;;
open Spec_definition;;

(* NOTE: In GIR everything must be assigned to at most once.
This can be in a loop, etc. but must be once to allow
for topology computations.  *)
(* UNTIL: everything is put in the program type -- at that point
it is no longer SSA (since we need the underlying
C to not be SSA).  *)

type gir_name =
	Name of string

type function_ref = FunctionRef of gir_name
(* Needs a list name and the variable that we are considering. *)
type expression =
	| VariableReference of variable_reference
	| FunctionCall of function_ref * varlist
    (* Lookup variable_reference in the list of pairs of synthvalues.  *)
    | GIRMap of gir_name * (synth_value * synth_value) list
and variable_reference =
	| Variable of gir_name
	| MemberReference of variable_reference * gir_name
	(* Variable * index variable *)
	| IndexReference of variable_reference * expression
	| Constant of synth_value
	| Cast of variable_reference * synth_type
and rvalue =
	| Expression of expression

and lvalue =
    | LVariable of variable_reference

and varlist =
    | VariableList of variable_reference list
and conditional =
	| Compare of variable_reference * variable_reference * binary_comparitor
	| Check of variable_reference * unary_comparator
	| CondOr of conditional * conditional
	| CondAnd of conditional * conditional
and binary_comparitor =
	| GreaterThan
    | GreaterThanOrEqual
	| LessThan
    | LessThanOrEqual
	| Equal
    | FloatEqual
and unary_comparator =
	| PowerOfTwo

and gir =
	(* Keep track of the name to define, and whether
	it escapes the function e.g. is a return value.  *)
	| Definition of gir_name * bool * (synth_type option)
	| IfCond of conditional * gir * gir
	| Sequence of gir list
	(* This can eitehr assign lists to lists, of variables to
	   variables.  *)
	| Assignment of lvalue * rvalue
	(* Body, induction variable name, loop max value *)
    | LoopOver of gir * gir_name * expression
	| Expression of expression
    (* Function definition, has a name, a list of args, and a body.  *)
    | FunctionDef of gir_name * (gir_name list) * gir * ((string, synth_type) Hashtbl.t)
    (* Depending on the allocation mode, this may or may
    not expand to anything.  *)
	| Free of variable_reference
    | Return of expression
	| EmptyGIR
	(* (why?) Todo --- add a lambda here *)
