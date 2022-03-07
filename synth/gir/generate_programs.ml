open Core_kernel;;
open Spec_definition;;
open Spec_utils;;
open Gir;;
open Gir_utils;;
open Gir_clean;;
open Options;;
open Gir_topology;;
open Program;;

exception GenerateProgramException of string

let rec add_index_variables_to_typemap typemap gir =
    match gir with
	| Definition(nref, escapes, deftype) -> ()
	| Sequence(girs) -> ignore(List.map girs (add_index_variables_to_typemap typemap))
	(* This can eitehr assign lists to lists, of variables to
	   variables.  *)
	| Assignment(lval, rval) -> () (* Should not be possible
    to define a new index variable in an assignment.  *)
	(* Body, induction variable name, loop max value *)
    | LoopOver(body, indvar, maxvar) ->
            (
            match Hashtbl.add typemap (gir_name_to_string indvar) Int32 with
            | `Duplicate ->
					let () = Printf.printf "Variable %s detected twice!" (gir_name_to_string indvar) in
					raise (GenerateProgramException "Added same index var to the typemap twice!")
            | `Ok -> ()
            )
	| Expression(expr) -> ()
            (* Also not currently possible to have an index
            variable in an expression *)
	| EmptyGIR -> ()
    (* These have their own typemap. Could do this here
    I suppose, currently no reason to though.   *)
    | FunctionDef(_, _, _, _) -> ()
	| IfCond(_, iftrue, iffalse) ->
			let () = add_index_variables_to_typemap typemap iftrue in
			let () = add_index_variables_to_typemap typemap iffalse in
			()
	| Free(_) -> ()
    | Return(_) -> ()

let rec member x ys =
	match ys with
	| [] -> false
	| y :: ys -> ((String.compare x y) = 0) || (member x ys)

	(* Can't remmeber what this is actually called.
	But I want all a in x with a not in y.  *)
let rec disjoint_union x y =
	match x with
	| [] -> []
	| (x :: xs) ->
			if member x y then
				disjoint_union xs y
			else
				x :: (disjoint_union xs y)

let generate_program_for opts (apispec: apispec) (iospec: iospec) (girpair) =
	(* Generate the function call to the API.  *)
	let api_funcname = FunctionRef(Name(apispec.funname)) in
	let api_args = VariableList(List.map apispec.funargs (fun v -> Variable(Name(v)))) in
	let funcall = FunctionCall(api_funcname, api_args) in
	(* Ret type --- currently only support one since we are
	   targetting C.  More for functional/tuple languages
	   would not be hard.  *)
	(* TODO -- somehting like this for APIs with explicit
	return types.  *)
	(* let rettype = List.map iospec.returnvar (fun v -> LVariable(Variable(Name(v)))) in *)
	(* Build the typemap that we need.  Note we need
	   to (a) build the typemap, then (b) schedule then
		   rebuild the program again with the scheduled
		   program.  *)
	let rettype = [] in
    let body = match rettype with
	| [] ->
		(* This is a function call with no explicit
		   return arguments, so just splice the function call in.  *)
			Sequence([girpair.pre; Expression(funcall); girpair.post])
	| [resref] ->
			Sequence([girpair.pre;
				Assignment(resref, Expression(funcall));
				girpair.post])
	| _ ->
			raise (GenerateProgramException "Multi-argument returns not currently supported")
	in
	(* Run scheduling on pre and post so things are computed
	   before they are used.  *)
	let scheduled_pre = topological_program_sort opts girpair.typemap ~predefed:iospec.livein ~preassigned:iospec.livein girpair.pre in
	(* May need to fiddle with the preassigned /defined with function return values.  *)
	let scheduled_post = topological_program_sort opts girpair.typemap ~predefed:(iospec.liveout @ apispec.livein @ apispec.liveout @ (disjoint_union iospec.livein iospec.liveout) @ iospec.returnvar) ~preassigned:(apispec.livein @ apispec.liveout @ (disjoint_union iospec.livein iospec.liveout)) girpair.post in
	(* Finally rebuild the body.  *)
	let final_body = match rettype with
	| [] ->
		(* This is a function call with no explicit
		   return arguments, so just splice the function call in.  *)
		Sequence([scheduled_pre; Expression(funcall); scheduled_post])
	| [resref] ->
		Sequence([scheduled_pre;
			Assignment(resref, Expression(funcall));
			scheduled_post])
	| _ ->
			assert false
	in
	let () =
		if opts.dump_generate_program then
			Printf.printf "Final GIR is:\n %s\n" (gir_to_string final_body)
		else ()
	in
	(* Need to add the index variables to the typemap.  *)
	let () =
		add_index_variables_to_typemap girpair.typemap.variable_map body in
	(* Clean out unrequired fundefs *)
	let required_fundefs =
		remove_unused_fundefs final_body girpair.fundefs in
    {
		funargs = iospec.funargs;
        livein = iospec.livein;
        gir = final_body;
        liveout = iospec.liveout;
		range_checker = girpair.range_checker;
        post_behavioural = None;
        returnvar = iospec.returnvar;
        typemap = girpair.typemap;
		fundefs = required_fundefs;
		user_funname = iospec.funname;
		api_funname = apispec.funname;
		generated_funname = iospec.funname ^ "_accel";
		inputmap = girpair.inputmap;
        original_pairs = Some(girpair.original_pairs);
        allocated_variables = (Utils.set_difference Utils.string_equal iospec.returnvar iospec.funargs) @ apispec.funargs;
    }

(* Given a set of pre/post pairs, fill thsee out into whole programs
   that can be turned into C code.  *)
let generate_programs (opts: options) (iospec: iospec) (api: apispec)
	(conversion_functions: gir_pair list)  =
		let result_programs = List.map conversion_functions (generate_program_for opts api iospec) in
		let () = if opts.dump_generate_program then
			Printf.printf "Generated Programs are: %s\n\n" (
				program_list_to_string result_programs
			)
		else () in
		result_programs
