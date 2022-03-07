open Core_kernel;;
open Spec_definition;;
open Spec_utils;;
open Utils;;
open Options;;
open Float_compare;;

type io_pair = {
    input: (string, synth_value) Hashtbl.t;
    output: (string, synth_value) Hashtbl.t
}

(* This is the helper methods required by the generic
   synthesizer.  Should be implemented for new
   domain-specific synthesizers.
*)
class virtual ['a] synth_manipulator =
    object (self)
        method virtual fill_holes: 'a list -> 'a list
        method virtual to_string: 'a -> string
		method virtual runner: 'a -> (string, synth_value) Hashtbl.t -> unit
        method list_to_string ls =
            String.concat ~sep:"\n" (
                List.map ls self#to_string
            );
    end

let generate_options options sketch_utils sketches =
	let result = sketch_utils#fill_holes sketches in
	let () = if options.debug_post_synthesis then
		let () = Printf.printf "Running gen options\n" in
        let () = Printf.printf "Options are %s\n" (sketch_utils#list_to_string result) in
        Printf.printf "Number of options is %d%!\n" (List.length result)
    else () in
	result

let rec compare options fcomp v1 v2 =
	(* Assume v1_keys is the set we want to check, and
	  v1_keys \in v1_keys *)
	let v1_keys = Hashtbl.keys v1 in
	(* let () = Printf.printf "V1tbl keys are: %s\n" (String.concat (Hashtbl.keys v1)) in
	let () = Printf.printf "V2tbl keys are: %s\n" (String.concat (Hashtbl.keys v2)) in *)
	List.for_all v1_keys (fun mem ->
		let () = if options.debug_post_synthesis then
			(* This was a bit too noisy *)
			(* Printf.printf "Comparing for key %s\n" mem *)
			()
		else () in
		compare_elts options fcomp (Hashtbl.find_exn v1 mem) (Hashtbl.find_exn v2 mem)
	)

and compare_elts options fcomp v1 v2 =
	let result = match v1, v2 with
	(* TODO -- may need to make this more flexible ---
	FFTSynth has some crappiness around dealing with
	precise widths.  *)
	| BoolV(b1), BoolV(b2) -> (Bool.compare b1 b2) = 0
    | Int16V(e1), Int16V(e2) -> e1 = e2
    | Int32V(e1), Int32V(e2) -> e1 = e2
    | Int64V(e1), Int64V(e2) -> e1 = e2
	| UInt16V(e1), UInt16V(e2) -> e1 = e2
	| UInt32V(e1), UInt32V(e2) -> e1 = e2
	| UInt64V(e1), UInt64V(e2) -> e1 = e2
	(* TODO --- this is a bit of a hack -- I'm not sure
	   why there are different widths reaching this point. *)
    | Float16V(e1), e2 ->
			(
			match float_from_value e2 with
			| Some(f2) -> fcomp#compare e1 f2
			| None -> false
			)
    | Float32V(e1), e2 ->
			(
			match float_from_value e2 with
			| Some(f2) -> fcomp#compare e1 f2
			| None -> false
			)
    | Float64V(e1), e2 ->
			(
			match float_from_value e2 with
			| Some(f2) -> fcomp#compare e1 f2
			| None -> false
			)
    | UnitV, UnitV -> true
    | ArrayV(vs1), ArrayV(vs2) ->
			((List.length vs1) = (List.length vs2)) &&
			(List.for_all (List.zip_exn vs1 vs2) (fun (i1, i2) ->
				compare_elts options fcomp i1 i2
			))
	| PointerV(vs1), PointerV(vs2) ->
			compare_elts options fcomp vs1 vs2
    | StructV(n, vls), StructV(n2, vls2) ->
			((String.compare n n2) = 0) && (compare options fcomp vls vls2)
	| _, _ -> false
	in
	let () = if options.debug_post_synthesis then
		if not result then
			let () = Printf.printf "Comparison failed on structures %s and %s\n"
			(synth_value_to_string v1) (synth_value_to_string v2)
			in
			let () = Printf.printf "These have types %s and %s\n"
			(synth_type_to_string (synth_value_to_type v1))
			(synth_type_to_string (synth_value_to_type v2))
			in
			()

		else ()
	else () in
	result

let rec deep_copy tbl =
    let new_tbl = Hashtbl.create (module String) in
    let keys = Hashtbl.keys tbl in
    let () = ignore(List.map keys (fun key ->
        let add = match Hashtbl.find_exn tbl key with
        | StructV(n, vls) ->
                StructV(n, deep_copy vls)
        | v -> (* Nothing to deep copy.  *)
                v
        in
		(* let () = Printf.printf "Deep copying key %s with value %s" (key) (synth_value_to_string add) in *)
        Hashtbl.add new_tbl key add
    )) in
    new_tbl

let eval options fft_manip sketches ios =
	let run_results = List.map sketches (fun sketch ->
		(* We don't currently need all the results, so we
		   do a take while here instead.  *)
		let () = if options.debug_post_synthesis then
			Printf.printf "Starting analysis for program %s\n" (fft_manip#to_string sketch)
		else () in
		List.take_while ios (fun io ->
            (* TODO -- could support non-void returns? *)
            let this_state = deep_copy io.input in
			let () = fft_manip#runner sketch this_state in
			let fcomp = ((new fp_comp_mse options.mse_threshold) :> fp_comp) in
			(* Order is important --- not everything in
				'this_state' is live out *)
			(compare options fcomp io.output this_state) && (fcomp#result options)
		), sketch
	) in
	let result = List.filter_map run_results (fun (results, prog) ->
		let total = List.length ios in
		let passed = List.length results in
		let () = if options.debug_post_synthesis then
			let () = Printf.printf "Using program %s\n" (fft_manip#to_string prog) in
			Printf.printf "Passed %d out of %d options\n" (passed) (total)
		else () in
		let has_result = (passed = total) in
		if has_result then
			Some(prog)
		else
			None
	) in
	let () = Printf.printf "Number of working synthed programs is %d\n" (List.length result) in
	result

