open Core_kernel;;
open Skeleton_definition;;
open Skeleton_utils;;
open Spec_definition;;
open Spec_utils;;
open Range;;
open Range_definition;;
open Options;;

let join_option_lists optlists =
	let filtered = List.concat (List.filter_map optlists (Utils.id)) in
	match filtered with
	| [] -> None
	| xs -> Some(xs)

let get_constants_from_constmap (constmap: (string, synth_value list) Hashtbl.t) vname styp =
	Hashtbl.find constmap vname
	(* match styp with *)
	(* | SInt(nr) -> *)
	(* 		let i16 = Hashtbl.find constmap (synth_type_to_string Int16) in *)
	(* 		let i32 = Hashtbl.find constmap (synth_type_to_string Int32) in *)
	(* 		let i64 = Hashtbl.find constmap (synth_type_to_string Int64) in *)
	(* 		let ui16 = Hashtbl.find constmap (synth_type_to_string UInt16) in *)
	(* 		let ui32 = Hashtbl.find constmap (synth_type_to_string UInt32) in *)
	(* 		let ui64 = Hashtbl.find constmap (synth_type_to_string UInt64) in *)
	(* 		join_option_lists [i16; i32; i64; ui16; ui32; ui64] *)
	(* | SBool(nr) -> *)
	(* 		Hashtbl.find constmap (synth_type_to_string Bool) *)
	(* | SFloat(nr) -> *)
	(* 		let f16 = Hashtbl.find constmap (synth_type_to_string Float16) in *)
	(* 		let f32 = Hashtbl.find constmap (synth_type_to_string Float32) in *)
	(* 		let f64 = Hashtbl.find constmap (synth_type_to_string Float64) in *)
	(* 		join_option_lists [f16; f32; f64] *)

let rec dimconsts_from_types typ =
	match typ with
	| SType(_) -> []
	| STypes(typs) ->
			List.concat (
				List.map typs dimconsts_from_types
			)
	| SArray(_, subtyps, lenvar) ->
			let subconsts = dimconsts_from_types subtyps in
			(* Consts used to define this array.  *)
			let this_consts = match lenvar with
			| EmptyDimension -> []
			| Dimension(dim) ->
					match dim with
						| DimVariable(_, _) -> []
						| DimConstant(c) -> [Int64V(c)]
			in
			subconsts @ this_consts

let get_length_constants styp input_styps: synth_value list option =
	match styp with
	| SInt(nr) -> Some(
		List.concat (List.map input_styps dimconsts_from_types)
	)
	(* Non-int types don't get suggestions from the
		input array sizes!*)
	| SBool(nr) -> None
	| SFloat(nr) -> None

let generate_plausible_constants_from_range options vrange: synth_value list option =
	match vrange with
	| None -> None
	| Some(vrange) ->
		let vrange_size = range_size vrange in
		match vrange_size with
		| Infinite -> None (* No way can we generate that! *)
		| Finite(n) ->
				(* Arbitrary param --- recommended to be fairly small,
				as increasing it seems unlikely to have much positive
				effect on constant gen.  *)
				if n < options.param_constant_generation_threshold then
					let rvalues = range_values vrange in
					Some(
						List.map rvalues (fun rval ->
							range_value_to_synth_value rval
						)
					)
				else
					None

let dim_type_to_simple_stype stype =
    match stype with
    | SType(t) -> Some(t)
    | STypes(ts) -> None
    | SArray(_, _, _) -> None

let opt_length_of l =
    match l with
    | None -> 0
    | Some(l) -> List.length l

(* Generate a list of constants to pass in as plausible
assignments.  *)
let generate_plausible_constants_map options supplied_constmap validmap input_stypes stypes =
	let constmap = Hashtbl.create (module String) in
    let () = if options.debug_skeleton_constant_gen then
        Printf.printf "Starting gen constants\n"
    else () in
    let () = ignore(List.map stypes (fun stype ->
        (* We only handle constant generation for the base
        cases right now --- we could 100% handle generaiton
        of more complex types e.g. structs/arrays etc,
        but not handled here. *)
        let simple_stype = dim_type_to_simple_stype stype in
        match simple_stype with
        | Some(stype) ->
            let vname = name_reference_to_string (name_refs_from_skeleton stype) in
            let () = if options.debug_skeleton_constant_gen then
                Printf.printf "Generating constants for variable %s\n" (vname)
            else () in
            (* So there are a few ways we get the constants that might be
               pluasible for each variable.  First,
               we can look at any constants that are suggested
               in the input json files --- these should correspond
               to e.g. constants used in user code.
               *)
            let consts: synth_value list option = get_constants_from_constmap supplied_constmap vname stype in
            (* Second, we can look at any array length parameters
            that are specified as constants.  *)
            (* Note of course, that this only returns something
            if there is an integer input.  *)
            let lparams: synth_value list option  = get_length_constants stype input_stypes in
            (* Third, if a variable has a partiuclarly
            small valid range, we can suggest constant
            parameters that correspond to each value
            within that range.  *)
            let range_consts: synth_value list option = generate_plausible_constants_from_range options (Hashtbl.find validmap vname) in

            let () = if options.debug_skeleton_constant_gen then
                let len_consts: int = opt_length_of consts in
                let len_lparams: int = opt_length_of lparams in
                let len_range_consts: int = opt_length_of range_consts in
                let () = Printf.printf "Length of variuos sub-arrays is provided consts: %d, length parameters: %d, valid range constants:%d\n" (len_consts) (len_lparams) (len_range_consts) in
                ()
            else ()
            in
            let const_options = join_option_lists [consts; lparams; range_consts] in
            let restricted_const_options =
                match range_consts, const_options with
                | Some(rconst), Some(copts) ->
                        (* If we have the range restrictions set, then only consider consts
                        in the valid range for this variable.  *)
                        (* Some(List.filter copts (fun opt ->
                            (List.exists rconst (fun r -> synth_value_equal r opt)
                        ))
                        ) *)
						const_options
                | _, _ -> const_options
            in
            (
            match restricted_const_options with
            | Some(copt) ->
                    let deduplicated = Utils.remove_duplicates (synth_value_equal) copt in
                    let () =
                        if options.debug_skeleton_constant_gen then
                            let () = Printf.printf "Number of options in total for var %s is %d\n" (vname) (List.length deduplicated) in
                            let () = Printf.printf "Options are: %s\n" (synth_value_list_to_string deduplicated) in
                            ()
                        else () in
                    let _ = Hashtbl.set constmap vname deduplicated in
                    ()
            | None ->
                    ()
            )
        | None -> ()

		)
	)
    in
    constmap
