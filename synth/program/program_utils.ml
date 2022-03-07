open Core_kernel;;
open Gir;;
open Gir_reduce;;
open Program;;
open Spec_definition;;
open Spec_utils;;

exception ProgramException of string

(* Basically this says "does the user code have different
types and if so, then generate the cast" *)
let generate_cast_reference (program: program) v =
	match program.typemap.original_typemap with
	| Some(t) ->
			let typ = Hashtbl.find_exn t.variable_map v in
			let generated_typ = Hashtbl.find_exn program.typemap.variable_map v in
			if synth_type_equal typ generated_typ then
				Variable(Name(v))
			else
				Cast(Variable(Name(v)), typ)
	| None ->
			(* No casting to be done --- there is no
			infered typemap to use.  *)
			Variable(Name(v))

(* Insert the range reduction wrapper if it exists *)
let insert_conditional_call options gir (program: program) =
    match program.range_checker with
    | Some(rcheck) ->
			let raw_call =
						FunctionCall(
							FunctionRef(
								Name(program.user_funname)
							),
							VariableList(List.map program.funargs (generate_cast_reference program))
						)
			in
			(* Not all code needs return parameters *)
			let user_code_call =
				if (List.length program.returnvar) > 0 then
					Return(raw_call)
				else
					Expression(raw_call)
			in

			(* This is the call to the accelerator --- so wrap it in an if statement with a return.  *)
			IfCond(rcheck.condition,
				(* If the condition passes, then proceed as normal. *)
				gir,
				(* If false, then call to the user
				code.  *)
				(* The frees will exist in the rest of
				the code already, so we just need to put
				them in here.  *)
				Sequence([user_code_call])
			)
    | None -> gir

(* This inserts a call into a function to dump the vlaues of the variables
   that are live in to the function.  *)
(* It should be done before the range insertion, because it needs
	to find the function call and insert the dump call so that
	it is called /every/ time.  *)
(* Various backend passes rely on this, and I'm imagining that any
pre-behavioural synthesis will also rely on this. *)
(* It should really be a bit of a better pass --- it's not a full
analysis despite the fact that would be easy to write.
It's more of a 'special cases that the mid end generates' kind
of thing.  *)
let rec insert_around_call callname program gir pre_addition post_addition =
	let result = match gir with
	| Sequence(elems) ->
			Sequence(List.concat(
				List.map elems (fun elem ->
					match elem with
						| Assignment(lvalue, Expression(FunctionCall(FunctionRef(Name(n)), args))) as fcall ->
							if (Utils.string_equal n program.api_funname) then
								[pre_addition; fcall; post_addition]
							else
								[fcall]
						| (Expression(FunctionCall(FunctionRef(Name(n)), args))) as fcall ->
							if (String.compare n program.api_funname) = 0 then
								(* This is the right call --- insert right
								before. *)
								[pre_addition; fcall; post_addition]
							else
								[fcall]
						| IfCond(cond, ift, iff) ->
								(* OK -- so this should really
								be a proper implementation because
								this is a complete disaster.
								Anyway...  Just ignore the hard
								cases that the internals don't currently
								generate (e.g. the cond) *)
								let new_ift = match ift with
								| Sequence(_) -> insert_around_call callname program ift pre_addition post_addition
								| other -> insert_around_call callname program (Sequence([ift])) pre_addition post_addition
								in
								let new_iff = match iff with
								| Sequence(_) -> insert_around_call callname program iff pre_addition post_addition
								| other -> insert_around_call callname program (Sequence([iff])) pre_addition post_addition
								in
								[IfCond(cond, new_ift, new_iff)]
						| other -> [other]
			)))
	| _ -> raise (ProgramException "Expected outer structure to be a sequence!")
	in
	result

let insert_dump_intermediates_call apispec callname gir program =
	let addition = Expression(FunctionCall(FunctionRef(Name(callname)),
									VariableList(
										List.map apispec.livein (fun n ->
											Variable(Name(n))
											)
										)
									))
	in
	insert_around_call callname program gir addition EmptyGIR

let insert_wrapper_timing_code callname gir program =
	let pre_add = Expression(FunctionCall(FunctionRef(Name("StartAcceleratorTimer")), VariableList([]))) in
	let post_add = Expression(FunctionCall(FunctionRef(Name("StopAcceleratorTimer")), VariableList([]))) in
	insert_around_call callname program gir pre_add post_add

let generate_single_gir_body_from options apispec dump_intermediates program =
    (* Merge all the components into a single GIR representation for
       the function body.  *)
	(* WARNING: We don't deal with stuff like returns here right
	now --- those are expected to be dealt with by the appropriate
	backend, due to differences in how different languages handle
	return values (e.g. supporting tuples, vs supporting
	primitivs vs supporting all objects.  *)
	(* Really not 100% sure about what limitations this
	entails. *)
    let post_behavioural_addition = reduce_gir options (Sequence([
		program.gir;
		(match program.post_behavioural with
			| Some(p) -> p.program
			| None -> EmptyGIR
		)
	])) in
	(* If we require the intermediate output, e.g. after the array
	output assignments, then this flag will insert a call to the
	pre accel dump function right before the call to the accelerator.  *)
	let intermediate_dump_addition =
		if dump_intermediates then
			insert_dump_intermediates_call apispec options.pre_accel_dump_function post_behavioural_addition program
		else
			post_behavioural_addition
	in
	let wrapper_timing_addition =
		if options.generate_timing_code then
			insert_wrapper_timing_code apispec intermediate_dump_addition program
		else
			intermediate_dump_addition
	in
	let range_checked = insert_conditional_call options wrapper_timing_addition program in
	
	range_checked

let generate_includes_list_from program =
	match program.post_behavioural with
	| Some(p) -> p.includes
	| None -> []

let get_io_typemap (program: program) =
	match program.typemap.original_typemap with
	| Some(t) -> t
	| None -> program.typemap
