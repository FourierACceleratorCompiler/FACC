open Core_kernel;;
open Spec_definition;;
open Spec_utils;;
open Binary_packer;;
open Alcotest;;

let test_unpack_struct () =
	let tmap = Hashtbl.create (module String) in
	let cmap = Hashtbl.create (module String) in
	let class_tmap = Hashtbl.create (module String) in
	(* Create the classmaps *)
	Hashtbl.add class_tmap "f64" Float64;
	Hashtbl.add class_tmap "i32" Int32;
	Hashtbl.add class_tmap "i32_2" Int32;
	Hashtbl.add class_tmap "i64" Int64;
	Hashtbl.add class_tmap "f32" Float32;
	Hashtbl.add tmap "example" (Struct("str"));
	Hashtbl.add cmap "str" StructMetadata({
		members = ["f64"; "i32"; "i32_2"; "i64"; "f32"];
		typemap = class_tmap;
		io_typemap = class_tmap
	});
	(* Create some values. *)

	let vmap = Hashtbl.create (module String) in
	Hashtbl.add vmap "f64" (Float64V(1.0))
	Hashtbl.add vmap "i32" (Int32V(1000))
	Hashtbl.add vmap "i32_2" (Int32V(32))
	Hashtbl.add vmap "i64" (Int64V(-11))
	Hashtbl.add vmap "f32" (Float32V(-0.1))

	Alcotest.(check (bool))

let main () =
	[
		"unpacking-test",
		[
			test_case "struct_unpack" `Quick test_unpack_struct
		]
	]
