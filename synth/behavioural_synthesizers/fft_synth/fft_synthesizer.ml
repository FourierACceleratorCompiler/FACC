open Core_kernel;;
open Fft_synthesizer_definition;;
open Fft_synthesizer_gen;;
open Spec_definition;;
open Spec_utils;;
open Value_utils;;
open Program;;
open Options;;

exception FFTSynth of string

(* This uses a few macros etc. to make code generation
simpler.  Those are defined in these include files.
(relative to the libs folder, which is included at
build time on the command line).  *)
let fft_synth_includes options = match options.target with
	| CXX ->  [
		"#include <clib/fft_synth/lib.h>"
	]

(* This is a behavioural synthesizer designed to help 'fit' FFT
functions to user code.  It's designed to do common things that
FFT functions forget/choose different behaviour for, like synthesizing
normalization/denormalization and bit-reversal.  *)
(* Because it has such a limited scope, it doesn't have
to have a particularly sane/scalable design --- I expect that
similar synth tools will be useful for the general problem of
accelerator utilization.  The key, here (as everywhere) is to
avoid overfitting a particular problem with compositionality ---
it has to be obvious to compiler developers what additional tools
the synthesizer needs to address the problem correctly. *)

(* This 100% needs some heuristic filters --- the blowup is big
and it is generating a huge number of programs when really
there are only a few sensible choices.  *)
(* IMO such an heuristic filter should be pretty easy. *)

(* Here, compositoinality is in the form of functions. *)
(* We also provide a number of sketches, designed
   for the various situations we are likely to encounter
   in FFT situations.  *)

(* This synthesizer is a bit crap at time because it doesn't exactly
capture the semantics of the underlying C.  Expect that to be
an issue eventually.  *)
(* let fs_sketches = *)
(*     [FSSeq( *)
(*         [ *)
(*         FSConditional(FSArrayOp(FSNormalize, FSArrayVariableHole), *)
(*         FSGreaterThan(FSIntVariableHole, FSConstant(Int64V(0)))); *)
(*         FSConditional(FSArrayOp(FSNormalize, FSArrayVariableHole), *)
(*         FSGreaterThan(FSIntVariableHole, FSConstant(Int64V(0)))); *)
(*         ] *)
(*     )] *)
let fs_sketches = 
    let uncond_array_op = 
        (* Unconditional operation on an array. *)
        FSArrayOp(FSArrayOpHole, FSArrayVariableHole) in
    let cond_array_op =
        FSConditional(FSArrayOp(FSArrayOpHole, FSArrayVariableHole), FSConditionalHole) in
    [
    (* Empty program --- tbh not really sure we need this here,
    but it makes testing a bit easier for now. *)
    FSSeq([]);
    (* Conditional operation on an array. *)
    cond_array_op;
    uncond_array_op;
    (* Some functions return as pairs of arrays
       rather than single arryas, so also
       look at pairs of functions.  *)
    FSConditional(FSSeq([
        uncond_array_op;
        uncond_array_op
    ]), FSConditionalHole);
    (* Unconditional also.  *)
    (* TODO -- could make this more efficient
    with some joint synthesis techniques to tie
    the two array ops together.  *)
    FSSeq([
        uncond_array_op;
        uncond_array_op;
    ])
    (* TODO --- More as required?  Perhaps that is general enough? *)
]

let fs_conditional_sketches = [
    FSGreaterThan(FSIntVariableHole, FSIntConstantHole);
    FSLessThan(FSIntVariableHole, FSIntConstantHole);
    FSGreaterThan(FSFloatVariableHole, FSFloatConstantHole);
    FSLessThan(FSFloatVariableHole, FSFloatConstantHole);
    (* PowerOfTwo(VariableHole)*)
]

let fs_array_operator = [
    FSBitReversal;
    FSNormalize;
	FSHalfNormalize;
    FSDenormalize;
	FSHalfDenormalize;
]

(* Typical ways that FFT/IFFT are specified. *)
let fs_int_constants = [
    Int64V(-1); Int64V(0); Int64V(1)
]

let fs_float_constants = [
    Float64V(-1.0); Float64V(0.0); Float64V(1.0)
]

let rec to_string_structure structure =
    match structure with
    | FSConditional(act, condition) ->
            "Cond(" ^ (to_string_cond condition) ^ ": " ^ (to_string_structure act) ^ ")"
    | FSArrayOp(oper, var) ->
            "ArrayOp(" ^ (to_string_op oper) ^ ": " ^ (to_string_var var) ^ ")"
    | FSSeq(elems) ->
            String.concat ~sep:";" (List.map elems to_string_structure)
    | FSStructureHole -> "StructureHole"
and to_string_cond cond =
    match cond with
    | FSGreaterThan(v1, v2) ->
            (to_string_var v1) ^ " > " ^ (to_string_var v2)
    | FSLessThan(v1, v2) ->
            (to_string_var v1) ^ " < " ^ (to_string_var v2)
    | FSPowerOfTwo(v) ->
            "PowerOfTwo(" ^ (to_string_var v) ^ ")"
    | FSConditionalHole ->
            "ConditionalHole"
and to_string_var var =
    match var with
    | FSVariable(v) -> (name_reference_to_string v)
    | FSConstant(v) -> (synth_value_to_string v)
    | FSScalarVariableHole -> "ScalarHole"
    | FSArrayVariableHole -> "ArrayHole"
    | FSIntVariableHole -> "IntHole"
    | FSFloatVariableHole -> "FloatHole"
    | FSIntConstantHole -> "IntConstHole"
    | FSFloatConstantHole -> "FloatConstHole"
and to_string_op op = match op with
    | FSBitReversal -> "BitReversal"
    | FSNormalize -> "Normalize"
	| FSHalfNormalize -> "HalfNormalize"
    | FSDenormalize -> "Denormalize"
	| FSHalfDenormalize -> "HalfDenormalize"
    | FSArrayOpHole -> "ArrayOperatorHole"

let rec fs_fill_holes filler structure =
    match structure with
    | FSConditional(act, condition) ->
            let act_options = (fs_fill_holes filler act) in
            let cond_options = (fill_conditional_holes filler condition) in
            List.map (List.cartesian_product act_options cond_options) (fun (a, c) ->
                FSConditional(a, c)
            )
    | FSArrayOp(oper, var) ->
            let oper_options = fill_array_op_hole filler oper in
            let var_options = fill_variable filler var in
            List.map (List.cartesian_product oper_options var_options) (fun (o, v) ->
                FSArrayOp(o, v)
            )
    | FSSeq(elems) ->
			(
            match elems with
            | [] ->
                    (* Preserve empty seqs. *)
					[FSSeq([])]
            | elems ->
                let elem_opts = List.map elems (fs_fill_holes filler) in
                List.map (Utils.cross_product elem_opts)
                (fun opt -> FSSeq(opt))
			)
    | FSStructureHole -> raise (FFTSynth "DOn't support filling structural holes")

and fill_conditional_holes filler structure =
    match structure with
    | FSGreaterThan(v, const) ->
            let v_opts = fill_variable filler v in
            let c_opts = fill_variable filler const in
            List.map (List.cartesian_product v_opts c_opts) (fun (v, c) ->
                FSGreaterThan(v, c)
            )
    | FSLessThan(v, const) ->
            let v_opts = fill_variable filler v in
            let c_opts = fill_variable filler const in
            List.map (List.cartesian_product v_opts c_opts) (fun (v, c) ->
                FSLessThan(v, c)
            )
    | FSPowerOfTwo(v) ->
            let v_opts = fill_variable filler v in
            List.map v_opts (fun v ->
                FSPowerOfTwo(v)
            )
    | FSConditionalHole ->
            List.concat (List.map fs_conditional_sketches (fill_conditional_holes filler))
and fill_array_op_hole filler v =
    match v with
    | FSArrayOpHole ->
            List.concat (List.map fs_array_operator (fill_array_op_hole filler))
    | other -> [other]

and fill_variable filler v = 
    filler v

let hole_options options bool_variables array_variables int_variables float_variables variable_type =
	(* let () = Printf.printf "Bool variables are: %s\n" (name_reference_list_to_string bool_variables) in
	let () = Printf.printf "Array variables are: %s\n" (name_reference_list_to_string array_variables) in
	let () = Printf.printf "Int variables are: %s\n" (name_reference_list_to_string int_variables) in
	let () = Printf.printf "Float variables are: %s\n" (name_reference_list_to_string float_variables) in *)
    let result = match variable_type with
    | FSIntConstantHole -> List.map fs_int_constants (fun x -> FSConstant(x))
    | FSFloatConstantHole -> List.map fs_float_constants (fun x -> FSConstant(x))
    | FSArrayVariableHole -> List.map array_variables (fun x -> FSVariable(x))
    | FSIntVariableHole -> List.map int_variables (fun x -> FSVariable(x))
    | FSFloatVariableHole -> List.map float_variables (fun x -> FSVariable(x))
    | FSScalarVariableHole -> List.map (int_variables @ float_variables) (fun x -> FSVariable(x))
    | FSVariable(x) -> [variable_type]
    | FSConstant(x) -> [variable_type] in
    let () = if options.debug_fft_synthesizer then
        let () = Printf.printf "Request was for %s\n" (to_string_var variable_type) in
        let () = Printf.printf "Size of result is %d\n" (List.length result) in
        ()
    else () in
    result

let evaluate_variable v = 
    v

let compare_fs (a: synth_value) b = match (int_from_value a, int_from_value b) with
    | Some(a), Some(b) -> Int.compare a b
    | _, _ -> (* At least one is none, try a float compare instead.  *)
            match (float_from_value a, float_from_value b) with
            | Some(a), Some(b) -> Float.compare a b
            (* No other cases are currently handled.  *)
            | _, _ -> raise (FFTSynth ("Type error, can't compare " ^ (synth_value_to_string a) ^ " and " ^ (synth_value_to_string b)))

let rec generic_normalize op arr: synth_value list =
    let length = float_of_int (List.length arr) in
	(* let () = Printf.printf "Input length of array is %d\n" (List.length arr) in *)
    List.map arr (fun e1 ->
        let result: synth_value = match e1 with
		| StructV(sname, subeles) ->
				let newtbl = Hashtbl.create (module String) in
				let _ = List.map (Hashtbl.keys subeles) (fun key ->
                    let value = Hashtbl.find_exn subeles key in
					match value with
					| Float16V(f) -> Hashtbl.set newtbl key (Float16V(op f length))
					| Float32V(f) -> Hashtbl.set newtbl key (Float16V(op f length))
					| Float64V(f) -> Hashtbl.set newtbl key (Float16V(op f length))
					| ArrayV(subarr) -> Hashtbl.set newtbl key (ArrayV(generic_normalize op subarr))
					(* TODO  -- support nested classes.  *)
					| _ -> raise (FFTSynth "Unexpected non-float")
				) in
				StructV(sname, newtbl)
		| ArrayV(subarray) ->
				ArrayV(generic_normalize op subarray)
		| Float16V(f) ->
                let r: synth_value = Float16V(op f length) in
                r
		| Float32V(f) ->
				Float32V(op f length)
		| Float64V(f) ->
				Float64V(op f length)
		| _ -> raise (FFTSynth "Unepxected non-float")
        in
        result
		)

let normalize arr =
	generic_normalize (fun f -> fun length -> f /. length) arr

let half_normalize arr =
	generic_normalize (fun f -> fun length -> f /. (length /. 2.0)) arr

let denormalize arr =
	generic_normalize (fun f -> fun length -> f *. length) arr

let half_denormalize arr =
	generic_normalize (fun f -> fun length -> f *. (length /. 2.0)) arr

let bit_reverse n maxbits =
    (* not sure (a) what this should do for non power of two
       or (b) if it's right for non power of two.  *)
    (* Also should be clear that I'm not 100% sure it matches
    the C implmentatin we use --- potential source of
    hard to find bugs IMO.  *)
    let result = ref 0 in
    let nref = ref n in
    let xref = ref maxbits in
    (* let () = Printf.printf "N is %d\n" (!nref) in *)
    while (!xref) <> 1 do
        (* let () = Printf.printf "XRef %d" (!xref) in *)
        result := (!result) lsl 1;
        result := (!result) lor ((!nref) land 1);
        nref := (!nref) lsr 1;
        (* let () = Printf.printf "Result so far is %d\n" (!result) in *)

        xref := (!xref) lsr 1;
    done;
    !result

let bit_reversal arr =
    let array_version = Array.of_list arr in
    let array_length = Array.length array_version in
	let () = for i = 0 to array_length - 1 do
        let reversed = bit_reverse i array_length in
        if reversed < i then
            let tmp = Array.get array_version i in
            let () = Array.set array_version i (Array.get array_version reversed) in
            let () = Array.set array_version reversed tmp in
            ()
        else
            ()
    done in
    Array.to_list array_version

let rec runner program (inputs: (string, synth_value) Hashtbl.t) =
    match program with
    | FSConditional(act, condition) -> (
            match eval_condition condition inputs with
            | true -> runner act inputs
            | false -> ()
    )
    | FSSeq(elems) ->
            (* These things mutate the inputs.  *)
            ignore(List.map elems (fun e ->
                runner e inputs
            ))
    | FSArrayOp(action_name, FSVariable(on)) ->
            let arr = match array_from_value (get_value inputs on) with
            | Some(arr) -> arr
            | None -> raise (FFTSynth "TYpe error")
            in
            (
            match action_name with
            | FSBitReversal ->
                    let reversed = bit_reversal arr in
                    set_value inputs on (ArrayV(reversed))
            | FSNormalize ->
					let new_value = normalize arr in
                    set_value inputs on (ArrayV(new_value))
			| FSHalfNormalize ->
					let new_value = half_normalize arr in
					set_value inputs on (ArrayV(new_value))
            | FSDenormalize ->
					let new_value = denormalize arr in
                    set_value inputs on (ArrayV(new_value))
			| FSHalfDenormalize ->
					let new_value = half_denormalize arr in
					set_value inputs on (ArrayV(new_value))
            | FSArrayOpHole ->
                    raise (FFTSynth "Can't execute algs with holes")
            )
    | FSArrayOp(action_name, _) ->
            raise (FFTSynth "Unsupported action on non-variable")
    | FSStructureHole ->
            raise (FFTSynth "Can't emulate a structural hole!")

and eval_condition cond inputs =
    match cond with
    | FSGreaterThan(vref1, vref2) ->
            let v1 = eval_variable vref1 inputs in
            let v2 = eval_variable vref2 inputs in
            (compare_fs v1 v2) = 1
    | FSLessThan(vref1, vref2) ->
            let v1 = eval_variable vref1 inputs in
            let v2 = eval_variable vref2 inputs in
            (compare_fs v1 v2) = -1
    | FSPowerOfTwo(v) ->
            (* Unimplemented right now *)
            raise (FFTSynth "Unimplemented")
    | FSConditionalHole ->
            raise (FFTSynth "Can't evaluate a hole!")

and eval_variable v (inputs: (string, synth_value) Hashtbl.t): synth_value =
	match v with
    | FSConstant(v) -> v
    | FSVariable(n) ->
        let v = get_value inputs n in
        v
    | _ -> raise (FFTSynth "Can't eval a hole")

let assert_not_empty (bres, ares, ires, fres) =
	let rec assert_list_not_empty l =
		let r = List.for_all l (fun l ->
			match l with
			| AnonymousName -> false
			| Name("") -> false
			| Name(_) -> true
			| StructName(ns) ->
					(assert_list_not_empty ns)
		)
		in
		let () = if r then () else Printf.printf "Found empty name in list %s\n" (name_reference_list_to_string l) in
		let () = assert r in
		r
	in
		assert (assert_list_not_empty bres);
		assert (assert_list_not_empty ares);
		assert (assert_list_not_empty ires);
		assert (assert_list_not_empty fres)

(* Generate the assignment options for each class of variable.  *)
(* This is really just a shitty heuristic, and it's just crap code
too.  Want to think of a better way of doing this.  *)
let rec split_variables classmap typemap variables =
	let rec add_variable_to_list (b, a, i, f, s) t v =
		match t with
		| Bool -> (v :: b, a, i, f, s)
		| Int16 -> (b, a, v :: i, f, s)
		| Int32 -> (b, a, v :: i, f, s)
		| Int64 -> (b, a, v :: i, f, s)
		| UInt16 -> (b, a, v :: i, f, s)
		| UInt32 -> (b, a, v :: i, f, s)
		| UInt64 -> (b, a, v :: i, f, s)
		| Float16 -> (b, a, i, v :: f, s)
		| Float32 -> (b, a, i, v :: f, s)
		| Float64 -> (b, a, i, v :: f, s)
		| Struct(nm) -> (b, a, i, f, (v, nm) :: s)
		| Array(subty, _) ->
				(b, v :: a, i, f, s)
		| Pointer(subty) ->
				add_variable_to_list (b, a, i, f, s) subty v
		| Unit -> raise (FFTSynth "Unit not supproted")
		| Fun(_, _) -> raise (FFTSynth "Higher order functions not supported")
	in
    (* Get the type of each variable.  *)
    let types = List.map variables (fun v -> Hashtbl.find_exn typemap (name_reference_to_string v)) in
	let b_vars, arr_vars, i_vars, f_vars, s_vars = List.fold (List.zip_exn types variables) ~init:([], [], [], [], [])
        ~f:(fun (b, a, i, f, s) -> fun (t, v) ->
			add_variable_to_list (b, a, i, f, s) t v
        ) in
    let struct_name_types = List.map s_vars (fun (varname, structname) ->
		get_struct_variables classmap varname structname
    ) in
    (* Probably could be done in a more scalable manner.  Anyway... *)
	let (bres, ares, ires, fres) = List.fold struct_name_types ~init:(b_vars, arr_vars, i_vars, f_vars) ~f:(fun (b, a, i, f) ->
            fun (b2, a2, i2, f2) ->
                (b @ b2, a @ a2, i @ i2, f @ f2)
    ) in
	let () = assert_not_empty (bres, ares, ires, fres) in
	(bres, ares, ires, fres)

and get_struct_variables classmap varname structname =
        let struct_metadata = Hashtbl.find_exn classmap structname in
        let structtypemap = get_class_typemap struct_metadata in
        let structmembers = List.map (get_class_members struct_metadata) (fun mem -> Name(mem)) in
        let (sb, sarr, si, sf) = split_variables classmap structtypemap structmembers in

        (* We need to prepend the structname to everything here.  *)
        let prepend_sname = name_reference_concat varname in
        (List.map sb prepend_sname,
		 List.map sarr prepend_sname,
         List.map si prepend_sname,
         List.map sf prepend_sname
        )

let rec get_operations_in prog =
	match prog with
	| FSConditional(s, cond) ->
			get_operations_in s
	| FSArrayOp(operator, v) ->
			[operator]
	| FSSeq(slist) ->
			List.concat(
			List.map slist get_operations_in
			)
	| FSStructureHole -> []

(* TODO *)
let get_variables_in p = []

let array_operator_equal x y =
	match x, y with
	| FSNormalize, FSNormalize -> true
	| FSDenormalize, FSDenormalize -> true
	| FSHalfNormalize, FSHalfNormalize -> true
	| FSHalfDenormalize, FSHalfDenormalize -> true
	| FSBitReversal, FSBitReversal -> true
	| FSArrayOpHole, FSArrayOpHole -> true
	| _, _ -> false

let is_likely_valid_program prog =
	let ops = get_operations_in prog in
	let _ = get_variables_in prog in
	
	(* which operations are never going to be
	useful in the same sketch.  *)
	let invalid_op_pairs = [
		[FSNormalize; FSDenormalize];
		[FSNormalize; FSHalfNormalize];
		[FSNormalize; FSHalfDenormalize];
		[FSHalfNormalize; FSHalfDenormalize];
		[FSHalfNormalize; FSDenormalize];
		[FSDenormalize; FSHalfDenormalize];
	] in
	List.for_all invalid_op_pairs (fun inv_ops ->
		List.exists inv_ops (fun op -> not(List.mem ops op array_operator_equal))
	)

class fft_synth_manipulator hole_opts =
    object
        inherit [fs_structure] Generic_sketch_synth.synth_manipulator as super
        method to_string (fs: fs_structure) =
            to_string_structure fs

        method fill_holes structs =
			let all_options = 
				List.concat (
					List.map structs (fs_fill_holes hole_opts)
				)
			in
			(* let () = Printf.printf "Pre filtering number of opts is %d\n" (List.length all_options) in *)
			let filtered = List.filter all_options is_likely_valid_program in
			(* let () = Printf.printf "Post filtering number of opts is %d\n" (List.length filtered) in *)
			filtered
        method runner prog state =
            runner prog state
    end

(* Now, run a generic sketch-based synthesis from these sketches. *)
let fft_synth options typemap variables (gir_program: program) iopairs: post_behavioural_program option =
    let bool_variables, array_variables, int_variables, float_variables = split_variables typemap.classmap typemap.variable_map variables in
    let hole_opts = hole_options options bool_variables array_variables int_variables float_variables in
    let fft_manip = ((new fft_synth_manipulator hole_opts) :> (fs_structure Generic_sketch_synth.synth_manipulator)) in
    let prog_opts = Generic_sketch_synth.generate_options options fft_manip fs_sketches in
	let valid_programs = Generic_sketch_synth.eval options fft_manip prog_opts iopairs in
	match valid_programs with
	| [] -> None
	| x :: xs ->
            let prog, typemap = generate_gir_for options typemap x in
			Some({
				includes = (fft_synth_includes options);
                program = prog;
                typemap = typemap
			})
