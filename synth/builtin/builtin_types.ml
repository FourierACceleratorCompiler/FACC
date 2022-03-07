open Core_kernel;;
open Spec_definition;;

exception BuiltinTypeError of string

let two_float32_type = Struct("facc_2xf32_t")
let two_float32_type_hashtable =
	let tbl = Hashtbl.create (module String) in
	let _ = Hashtbl.add tbl "f32_1" Float32 in
	let _ = Hashtbl.add tbl "f32_2" Float32 in
	tbl
let two_float32_type_metadata = StructMetadata({
	members = ["f32_1"; "f32_2"];
	typemap = two_float32_type_hashtable;
	io_typemap = two_float32_type_hashtable;
})

let two_float64_type = Struct("facc_2xf64_t")
let two_float64_hashtable =
	let tbl = Hashtbl.create (module String) in
	let _ = Hashtbl.add tbl "f64_1" Float64 in
	let _ = Hashtbl.add tbl "f64_2" Float64 in
	tbl
let two_float64_type_metadata = StructMetadata({
	members = ["f64_1"; "f64_2"];
	typemap = two_float64_hashtable;
	io_typemap = two_float64_hashtable;
})

let builtin_structs =
	["facc_2xf32_t"; "facc_2xf64_t"]

let is_builtin_struct n =
	List.mem builtin_structs n Utils.string_equal

let builtin_struct_from_name name =
	match name with
	| "facc_2xf32_t" -> two_float32_type_metadata
	| "facc_2xf64_t" -> two_float64_type_metadata
	| _ -> raise (BuiltinTypeError ("No such type " ^ name))

(* What is the size modifier for an underlying array of some length.   *)
let get_size_modifier_for typ =
	match typ with
	| Struct("facc_2xf32_t") -> 2
	| Struct("facc_2xf64_t") -> 2
	| other -> 1

let is_infered_type typ =
	match typ with
	| Struct("facc_2xf32_t") -> true
	| Struct("facc_2xf64_t") -> true
	| _ -> false
