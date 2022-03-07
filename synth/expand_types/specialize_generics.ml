open Core_kernel;;
open Spec_definition;;
open Spec_utils;;
open Builtin_types;;
open Options;;

(* This pass aims to take input fields
that look like void* and specialize
them into actual tpes.  Obviously, that's a challenging
task. 

The key heuristic we use here is the target-specialization
heuristic, the idea that we try to infer types
that exist in the API we are trying to compile to. *)

(* TODO --- perhaps some types don't make sense to cast to? *)
let filter_target_types tps =
    tps

let rebuild_typemaps tps =
    List.map tps (fun tpset ->
        let tbl = Hashtbl.create (module String) in
        let () = ignore(List.map tpset (fun (v, t) ->
            Hashtbl.add tbl v t
        )) in
        tbl
    )

(* This isn't used right now, but in a language
with more sane generics, imagine it'd look a bit like this.  *)
let rec generate_pointer_types_generic inference_types typ = 
    match typ with
    | Pointer(st) ->
            List.map (generate_pointer_types inference_types typ) (fun inf_type -> Pointer(inf_type))
    | Generic -> inference_types
    | other -> [other]

(* Suppose this should be "C".  Which only supports generics
within a pointer, and so after you cast you still end up
with a pointer, except arrays, since those are pointers.
But those are only arrays not pointers at the first
level of pointerness,  unless it's a multidimensional array.

One things for sure: it's a shit-show.
*)
let rec generate_pointer_types_cxx inference_types typ =
    match typ with
    | Pointer(Generic) ->
            List.map inference_types (fun inftype ->
                match inftype with
                (* So, infering arrays don't have to be pointers,
                but everything else should be.  *)
                | Array(sty, d) ->
                        Array(sty, d)
                | Pointer(other) ->
                        Pointer(other)
                | other ->
                        (* In C, everything that is not a pointer/array
                        should be wrapped.  *)
                        Pointer(other)
            )
    | Poitner(other) ->
            let subinference = generate_pointer_types_cxx inference_types other in
            List.map subinference (fun s -> Pointer(s))
    | Generic -> raise (SpecializationException "Unsupported unwrapped generic in C")
    | other -> [other]

let generate_pointer_types options inference_types typ =
    match options.target with
    | CXX -> generate_pointer_types_cxx inference_types typ

let specialize_generics options typemap (apispec: apispec) =
	let target_types = filter_target_types (get_types_used_in apispec.variables typemap) in

    let newpairs = List.map (Hashtbl.members typemap.variable_map)  (fun variable ->
        let vtyp = Hashtbl.find_exn typemap.variable_map variable in
        let suggested_types = generate_pointer_types options target_types vtyp in
        List.map suggested_types (fun t -> (variable, suggested_types))
	) in

    let variable_type_sets = Utils.cross_product newpairs in
    rebuild_typemaps typemap variable_type_sets
