open Core_kernel;;

(* These are really allocation modes for C/C++.  They might
have applications to other languages.  *)
type allocation_mode =
	| StackAllocationMode
    | HeapAllocationMode
    | StaticAllocationMode

type compile_settings = {
    allocation_mode: allocation_mode;
}

let default_compile_settings = {
	allocation_mode = StackAllocationMode;
}
