open Core_kernel;;
open Spec_definition;;
open Gir;;
open Range_definition;;
open Skeleton_definition;;

type post_behavioural_program = {
	(* Any includes required for the generated program.  *)
	includes: string list;
	typemap: typemap;
	program: gir
}

type range_program = {
	condition: conditional
}

(* This should be a list of live-in variables, the function
	and then the live out varaibles. *)
type program = {
	original_pairs: skeleton_pairs option;
	funargs: string list;
    livein: string list;
    gir: gir;
    liveout: string list;
	range_checker: range_program option;
	post_behavioural: post_behavioural_program option;
	typemap: typemap;
	inputmap: (string, range_set) Hashtbl.t;
	returnvar: string list;
	user_funname: string;
	generated_funname: string;
	api_funname: string;
	allocated_variables: string list;
	fundefs: gir list;
}

type gir_pair = {
	original_pairs: skeleton_pairs;
	pre: gir;
	post: gir;
	(* Which input ranges are good to test this? *)
	inputmap: (string, range_set) Hashtbl.t;
	(* What helper functions are required? *)
	fundefs: gir list;
	range_checker: range_program option;
	typemap: typemap;
}
