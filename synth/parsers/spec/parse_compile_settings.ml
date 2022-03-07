open Core_kernel;;
open Yojson;;
open Yojson.Basic.Util;;
open Compile_settings;;

exception ParseCompileException of string

let load_compile_settings filename =
	let json = Yojson.Basic.from_file filename in
	let allocation_mode = match json |> member "allocation_mode" |> to_string with
	| "Stack" -> StackAllocationMode
	| "Heap" -> HeapAllocationMode
	| "Static" -> StaticAllocationMode
	| _ -> raise (ParseCompileException "Expected a field 'allocation_mode' with one of Stack|Heap|Static for where the copy arguments are stored.")
	in
	let compile_settings: compile_settings = {
        allocation_mode = allocation_mode;
	} in
    compile_settings
