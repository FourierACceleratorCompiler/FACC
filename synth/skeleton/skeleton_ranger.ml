open Core_kernel;;
open Spec_definition;;
open Spec_utils;;
open Skeleton_definition;;
open Skeleton_utils;;
open Builtin_conversion_functions;;
open Range;;
open Range_definition;;
open Range_checker_synth;;
open Options;;
open Utils;;
open Skeleton_range_check;;

exception SkeletonRangerException of string

let getIdentityConversionFunction options r1 r2 =
	let overlap_size = range_size (range_set_intersection r1 r2) in
	let overlap_fraction = range_size_divide (range_size r1) overlap_size in
	if (range_size_less_than overlap_fraction options.range_size_difference_factor) then
		identityConversionFunction r1 r2
	else
		(* let () = Printf.printf "For ranges %s and %s, did not get the identity conversion" (range_set_to_string r1) (range_set_to_string r2) in *)
		(* If there isn't enough direct overlap of the two range sets, then
			we probably shouldn't do this assignment as an identity assignment. *)
		[]

(* Tranform a range by some conversion function (forward) *)
let transform_range convf range =
    let apply_conv i = match i with
    | RangeInteger(i) ->
            (
            match convf with
            | IdentityConversion -> RangeInteger(i)
            | PowerOfTwoConversion -> RangeInteger(Utils.power_of_two i)
            | DivideByConversion(n) -> RangeInteger(i / n)
            | Map(_, _, vlist) ->
                    let (_, res) = List.find_exn vlist ~f:(fun (f, t) ->
                        if range_value_eq (range_value_to_item f) (RangeInteger(i)) then
                            true
                        else
                            false
                    ) in
                    range_value_to_item res
            )
    | _ -> assert false (* IMO conversions are only generated
    for ints right now.  *)
    (* Tha'ts not strictly true I think --- might also
    be FP, for e.g. maps, but I can't remember. *)
    in
    let transform_range_itms itm = match itm with
    | RangeItem(i) -> RangeItem(apply_conv i)
    | RangeRange(f, t) ->
            (* todo --- might have to flip f/t for some
             things e.g. divide by -1.   THose
             aren't generated right now. *)
            RangeRange(apply_conv f, apply_conv t)
    in
    match range with
    | RangeSet(itms) ->
            RangeSet(Array.map itms transform_range_itms)


(* There is a lot more we could do here, e.g.
   making ranges with 0 overlap unlikely etc.  *)
(* Generate a conversion from r1 to r2.  *)
(* We really only do a small subset of this right now ---
handling the general case is obviously exponential,
within an already exponential problem.. *)
let range_conversion options r1 r2 =
    let r1_size = range_size r1 in
    let r2_size = range_size r2 in
	let multiply_options =
		if is_integer_range (range_type r1) && is_integer_range (range_type r2) && is_even_range r2 then
			(* TODO --- there must be a better way to do this? *)
			(* E.g. using the information about length variables *)
			[DivideByConversion(2)]
		else
			[]
	in
	let po2_options =
		if is_integer_range (range_type r1) && is_integer_range (range_type r2) then
            (* r2 is the assignment target, so we are looking
            for the range of that to be bigger.  *)
			let r1_max = range_max r1 in
			let r2_max = range_max r2 in
            if ((range_compare r1_max (RangeInteger(64))) < 0) && ((range_compare r2_max (RangeInteger(64))) > 0) then
                [PowerOfTwoConversion]
            else
                []
        else
            []
    in
	let range_size_comp = range_size_compare r1_size r2_size in
	let perm_options = if range_size_comp = 0 then
        if (range_size_compare r1_size rangeConversionSizeLimit) = -1 then
            permutationConversionOptions r1 r2
        else
            getIdentityConversionFunction options r1 r2
    else
        getIdentityConversionFunction options r1 r2
	in multiply_options @ po2_options @ perm_options

(* Various things are implausible, like the fromrange
being lots and lots and the to range being nearly empty.
I suspect a lot more could be done here.  *)
(* There is a lot of nuance we want to avoid here, e.g.
if the user has picked doubles, and the accelerator is
floats, we probably still want to try it.  *)
let range_compat_check options conversion from_range to_range =
    let from_size = range_size from_range in
    let to_size = range_size to_range in
    (* Want to check if fom_size >> to_size (if so then fail) *)
    (* So, compute fom_size / to_size, and see if it's greater
       than a threshold.  *)
    let range_factor = range_size_divide (from_size) (to_size) in
    (* If this is less than the range factor, then keep it.  *)
    let to_smaller = (range_size_less_than range_factor
        (options.range_size_difference_factor)) in
    let () = if options.debug_skeleton_range_filter then
        let () = Printf.printf "Considering values with fromsize %s and tosize %s, range factor is %s\n" (range_size_to_string from_size) (range_size_to_string to_size) (range_size_to_string range_factor) in
        let () = Printf.printf "Threshold is %s, decision is therefore %b\n" (range_size_to_string options.range_size_difference_factor) (to_smaller) in
        ()
    else ()
    in
	let has_intersection = range_set_has_intersection (transform_range conversion from_range) to_range in
	(* If the sets are small enough, we'll try combinatorial mapping, so
	   we don't need to worry too much about overlap.  *)
	let small_set_from = (range_size_compare from_size rangeConversionSizeLimit) = -1 in
	let small_set_to = (range_size_compare to_size rangeConversionSizeLimit) = -1 in
	let small_sets = small_set_from && small_set_to in
    to_smaller && (has_intersection || small_sets)

let range_from_fromvars rangemap fromvars =
    match fromvars with
    | [] -> (* no fromvars -- means that this dead-in, so doesnt
			   need to be assigned to. *)
			None
    | [x] ->
            (
            match x with
            | AssignVariable(v) ->
                    let fromvar_name = index_nesting_to_string v in
                    let ranges = Hashtbl.find rangemap fromvar_name in
                    ranges
            | AssignConstant(c) ->
                    range_from_synth_value c
            )
    (* Although the type conceptually supports this, I'm not 100% sure
    that multiple fromvars is really a sane thing to do.  It will
    lead to a lot of blowup if done naively.   Anyway, does warrant
    examination, because I can imagine some very cool usecases
    just need to be careful with hat associated blowup.  Hopefully
    the ranger can do well at reducing it here.  *)
    (* Anywa, I think the rest of this should just work if we go
    down this path. *)
    | x :: xs ->
            raise (SkeletonRangerException "Multiple fromvars aren't yet supported --- will need models of the combination functions used.")

let range_from_tovar rangemap tovar =
    Hashtbl.find rangemap (index_nesting_to_string tovar)

let check_binds options from_rangemap to_rangemap (bind: flat_single_variable_binding) =
    (* Currently only support the range detection from a single
       var at this time.  I think that that is the right way
       to go about this --- more vars, and you obviously need
       a combining function call --- now that could (perhaps should?)
       be explored here too, perhaps as a preprocessing pass.
       *)
    let fromvar_range = range_from_fromvars from_rangemap bind.fromvars_index_nesting in
    let tovar_range = range_from_tovar to_rangemap bind.tovar_index_nesting in
    match fromvar_range, tovar_range with
    (* can't check if range is non-existent. *)
    | None, _ -> [bind]
    | _, None -> [bind]
    | Some(frange), Some(trange) ->
            let () = if options.debug_skeleton_range_filter then
                let () = Printf.printf "Doing range compat check for %s and %s\n" (index_nesting_to_string bind.tovar_index_nesting) (String.concat (List.map bind.fromvars_index_nesting assignment_type_to_string)) in
                ()
            else ()
            in
            let conv_fs = range_conversion options frange trange in
            List.filter_map conv_fs (fun conv_f ->
                let compatible = range_compat_check options conv_f frange trange in
                let () = if options.debug_skeleton_range_filter then
                    let () = Printf.printf "Under conversion %s compatible: %b\n" (conversion_function_to_string conv_f) (compatible) in
                    () else ()
                in
                if compatible then
                    Some({
                        fromvars_index_nesting = bind.fromvars_index_nesting;
                        tovar_index_nesting = bind.tovar_index_nesting;
                        valid_dimensions = bind.valid_dimensions;
                        conversion_function = conv_f
                    })
                else
                    None
            )

(* This is a pass aimed at 'range-ifying' bindings -- it excludes
   bindings with vastly different valid range bindings, and
   it introduces range conversions between common finite
   set alternatives, e.g. set(-1, 1) and set(0, 1).

   I think this is where we could support power-of-two
   conversions in future, e.g. between set(2, 4, 8, 16...)
   and set(1, 2, 3, 4, 5....)

   This either returns Some(binds) (with ranges)
   or None, if at least one of the binds seemed so range-incompatible
   that it didn't need to happen.
   *)
let rangecheck_binds options vbinds from_rangemap to_rangemap =
    let individual_bind_checks =
        List.map vbinds (check_binds options from_rangemap to_rangemap) in
    let binds = List.fold individual_bind_checks ~f:(fun binds ->
            fun bindcheck -> match bindcheck, binds with
				| [], _ -> None
				| _, None -> None
                | b, Some(bs) -> Some(b :: bs)
    ) ~init:(Some([])) in
	let binds = match binds with
	| Some(bind) -> bind
	| None -> [] in
	let binds = cross_product binds in
	List.map binds (fun bind -> {
			flat_bindings = bind
		})

(* Note that this can  increase /or/ decrese the number of skeletons
   to check.  *)
let rangecheck_skeletons options (skeletons: flat_skeleton_binding list) from_rangemap to_rangemap =
	List.concat (List.map skeletons (fun skeleton ->
		rangecheck_binds options skeleton.flat_bindings from_rangemap to_rangemap
	))
