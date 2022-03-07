(* Parse one of the types.  *)
open Core_kernel;;
open Yojson;;
open Yojson.Basic.Util;;
open Spec_definition;;

let parse_type type_string = 
	let lexbuf = Lexing.from_string type_string in
	let ast = Typeparse.t Typelex.read lexbuf in
    ast
