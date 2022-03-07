open Core_kernel;;
open Options;;
open Spec_definition;;
open Range_definition;;
open Range;;
open Gir;;
open Gir_utils;;
open Spec_utils;;

exception RangeSynthError of string

(* There are three ways to implement this:
	- positive check (i.e. do the inputs lie in range
	of the expected inputs)
	- positive difference (do the inputs lie in range
	of the expected inputs, but excluding checks where
	they can't be due to the valid range of user code)
	- negative difference (are the inputs outside
	the range of expected inputs assuming they
	are withing the range of user code.)

	We implement the positive check right now, but
	other options may be more efficient for different
	ranges.
	*)

let item_between i (lower, higher) =
	match i, lower, higher with
	| RangeInteger(i), RangeInteger(lower), RangeInteger(higher) ->
			(i >= lower) && (i <= higher)
	| RangeFloat(i), RangeFloat(lower), RangeFloat(higher) ->
			((Float.compare i lower) > 0) && ((Float.compare i higher) < 0)
	| RangeArray(ti, i), RangeArray(tl, l), RangeArray(th, h) ->
			raise (RangeSynthError "Unuspported array range")
	| _, _, _ -> raise (RangeSynthError "Type error")

let check_value_in v set =
	match v with
	| RangeItem(i) ->
			Array.find_map set (fun set_item ->
				match set_item with
				| RangeRange(lower, higher) ->
						if range_value_in (RangeRange(lower, higher)) i then
							Some(RangeItem(i))
						else
							None
				| RangeItem(other_item) ->
						if range_value_eq i other_item then
							Some(RangeItem(i))
						else
							None
			)
	| RangeRange(lower, higher) ->
			Array.find_map set (fun set_item ->
				match set_item with
				| RangeRange(otherlower, otherhigher) ->
						let overlapping_range =
							range_overlap (lower, higher) (otherlower, otherhigher) in
						overlapping_range
				| RangeItem(other_item) ->
						if range_value_in (RangeRange(lower, higher)) other_item then
							Some(RangeItem(other_item))
						else
							None
			)

(* There is a 100% chance that this could
   be implemented more efficiently.  *)
let range_intersection r1 r2 =
	Array.filter_map r1 (fun range_item ->
		let new_item = check_value_in range_item r2 in
		new_item
	)

let range_set_intersection r1 r2 =
    match r1, r2 with
    | RangeSet(r1), RangeSet(r2) ->
            RangeSet(range_intersection r1 r2)

let range_set_has_intersection r1 r2 =
	not (empty_range_set (range_set_intersection r1 r2))

let positive_check_for vname var_type range_set =
	let conds = Array.map range_set (fun range_elem ->
		match range_elem with
		| RangeRange(lower, higher) ->
				let lowerv = range_item_to_synth_value lower in
				let higherv = range_item_to_synth_value higher in
				CondAnd(
					Compare(vname, Constant(lowerv), GreaterThanOrEqual),
					Compare(vname, Constant(higherv), LessThanOrEqual)
				)
		| RangeItem(RangeInteger(i)) ->
				let ivalue = Constant(range_item_to_synth_value (RangeInteger(i))) in
                if is_integer_type var_type then
                    Compare(vname, ivalue, Equal)
                else if is_bool_type var_type then
                    (* Need to cast: we can generate this for
                    e.g. bools which have a significant degree
                    of correspondance with ints.  *)
                    (* Not sure what is should be cast to? *)
                    Compare(Cast(vname, Int16), ivalue, Equal)
                else
                    raise (RangeSynthError "Unsupported type to compare to ints")
		| RangeItem(RangeFloat(i)) ->
				let ivalue = Constant(range_item_to_synth_value (RangeFloat(i))) in
				Compare(vname, ivalue, FloatEqual)
		| RangeItem(RangeBool(b)) ->
				let ivalue = Constant(range_item_to_synth_value (RangeBool(b))) in
				Compare(vname, ivalue, Equal)
		| RangeItem(RangeArray(_, _)) ->
				raise (RangeSynthError "Unsupported equality checks for arrays")
	) in
	Array.fold conds ~init:None ~f:(fun acc -> (fun cond ->
		match acc with
		| None -> Some(cond)
		| Some(other_cond) ->
				Some(CondOr(cond, other_cond))
	)
	)

(* TODO --- ideally, we should replace all the calls to positive_check_for with
a call to a function that takes the positive vs negative tradeoff
	into account.  *)
let difference_check_no_valid vname var_type accel_valid user_range =
	let inter = range_intersection accel_valid user_range in
	positive_check_for vname var_type inter

let difference_check vname var_type accel_valid user_range user_valid =
	let inputs_to_check = range_intersection accel_valid user_range in
	(* Make sure to redirect the invalid user inputs inputs 
	to the user code. *)
	let inputs_with_diversions = range_intersection inputs_to_check user_valid in
	positive_check_for vname var_type inputs_with_diversions

let intersection_check vname var_type accel_valid user_valid =
	let inter = range_intersection accel_valid user_valid in
	positive_check_for vname var_type inter

let check vname var_type accel_valid =
	positive_check_for vname var_type accel_valid

(* The idea is that for each element in, we check if it's in
	the out range for that variable.  *)
let generate_check_for options var var_type accel_valid_analysis input_range_analysis input_valid_analysis =
	match accel_valid_analysis, input_range_analysis, input_valid_analysis with
	| Some(RangeSet(accel_valid)), Some(RangeSet(input_range)), None ->
			(* Do a difference check without the
			valid range analysis.  *)
			difference_check_no_valid var var_type accel_valid input_range
	| Some(RangeSet(accel_valid)), Some(RangeSet(input_range)), Some(RangeSet(valid_range)) ->
			(* Do a difference check with the valid range analysis
			for the user code. *)
			difference_check var var_type accel_valid input_range valid_range
	| Some(RangeSet(accel_valid)), None, Some(RangeSet(valid_range)) ->
			(* We should just take the intersection of the
			two sets --- i.e. not use the accelerator if it would
			work but the user code would fail.  *)
			intersection_check var var_type accel_valid valid_range
	| Some(RangeSet(accel_valid)), None, None ->
			(* just do a straight check.  *)
			check var var_type accel_valid
	| None, Some(RangeSet(input_range)), Some(RangeSet(valid_range)) ->
			(* TODO _-- we should try to back off to the user
			code when the input range and teh valid range
			don't line up so that error messages/states
			are preserved.  *)
			None
	| None, None, Some(_) ->
			None
	| None, Some(_), None ->
			None
	| None, None, None ->
			(* If none are set, we can't do the analysis.  *)
			None

(* Array types are currently not supported
	for range gen because they require a complicated
	equality check --- the space of arrays is also
	extremely sparse, so there is some question about
	the potential usefulness of such equality checks.  *)
let supports rty =
	match rty with
	| Some(RangeIntegerType) -> true
	| Some(RangeBoolType) -> true
	| Some(RangeFloatType) -> true
	| Some(RangeArrayType(_)) -> false
	| None -> true

let is_supported_type s1 s2 s3 =
	let s1_type = Option.map s1 range_type in
	let s2_type = Option.map s2 range_type in
	let s3_type = Option.map s3 range_type in
	
	let s1_supported = supports s1_type in
	let s2_supported = supports s2_type in
	let s3_supported = supports s3_type in

	s1_supported && s2_supported && s3_supported

(* This should be able to generate range checks either
   on the accelerator variables or on the user code
   variables.  *)
let generate_range_check options typemap vars accel_valid input_range input_valid =
	let wrapped_vars = List.map vars (fun var -> Variable(Name(var))) in
	let range_checks: conditional list = List.filter_map wrapped_vars (fun var ->
		let var_string = (variable_reference_to_string var) in
		let var_type = type_of typemap.variable_map typemap.classmap (variable_reference_to_string var) in
		let accel_valid_for_var = Hashtbl.find accel_valid var_string in
		let input_range_for_var = Hashtbl.find input_range var_string in
		let input_valid_for_var = Hashtbl.find input_valid var_string in
        let () = if options.debug_range_check then
            let () = Printf.printf "Starting rage gen for var %s\n" (var_string) in
            ()
        else () in
        let res =
			if is_supported_type accel_valid_for_var input_range_for_var input_valid_for_var then
				generate_check_for options var var_type accel_valid_for_var input_range_for_var input_valid_for_var
			else
				(* E.g. arrays are not currently supported for range
				generation here -- only for guiding the random
				input generator/skeleton binder.
				Just need to support array equality and this should
				be fine. *)
				None
		in
        let () = if options.debug_range_check then
            let () = Printf.printf "Finished range check for var %s, has result %s\n" (var_string) (match res with | None -> "None" | Some(r) -> (conditional_to_string r)) in
            ()
        else () in
        res
	) in
	List.fold range_checks ~init:None ~f:(fun acc -> fun rcheck ->
		match acc with
		| None -> Some(rcheck)
		| Some(other_check) ->
				Some(
					CondAnd(
						rcheck,
						other_check
					)
				)
	)
