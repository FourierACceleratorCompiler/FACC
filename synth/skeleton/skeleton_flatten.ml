open Core_kernel;;
open Spec_definition;;
open Spec_utils;;
open Skeleton_definition;;
open Skeleton_utils;;
open Skeleton_filter;;
open Options;;
open Utils;;
open Builtin_conversion_functions;;

let flatten_binding (svar_binding: single_variable_binding_option_group) =
    (* let () = Printf.printf "Length of bindings is %d\n" (List.length svar_binding.valid_dimensions_set) in *)
	if List.length svar_binding.valid_dimensions_set > 0 then
		let reduced_dimvar_set = List.map svar_binding.valid_dimensions_set filter_dimvar_set in
		let combinations = cross_product reduced_dimvar_set in
		List.map combinations (fun dim ->
		(* let () = Printf.printf "Building for dimensions %s" (dimvar_mapping_list_to_string dim) in*)
		{
			fromvars_index_nesting = svar_binding.fromvars_index_nesting;
			tovar_index_nesting = svar_binding.tovar_index_nesting;
			valid_dimensions = dim;
			conversion_function = IdentityConversion
		}
		)
	else
		(* Not everything /has/ to have a vlid dimension --
			and we still want to keep those.  *)
		[{
			fromvars_index_nesting = svar_binding.fromvars_index_nesting;
			tovar_index_nesting = svar_binding.tovar_index_nesting;
			valid_dimensions = [];
			conversion_function = IdentityConversion
		}]

(* The skeleton pass generates more than one 'possible binding' per list
   item.   This pass flattens that out into a single list.  *)
let flatten_skeleton (opts: options) (skels: skeleton_type_binding list): flat_skeleton_binding list =
	let () = if opts.debug_skeleton_flatten then
		Printf.printf "Input length is %d\n" (List.length skels)
	else () in
	let vbinds = List.concat (List.map skels (
			fun skel ->
				let () = if opts.debug_skeleton_flatten then
					let () = Printf.printf "Starting new skeleton!\n" in
					let () = Printf.printf "Scanning length of %d\n" (List.length skel.bindings) in
					let () = Printf.printf "Skeleton is %s\n" (skeleton_type_binding_to_string skel) in
                    let () = Printf.printf "Number of input variables %d\n" (List.length skel.bindings) in
                    let () = Printf.printf "Number of elements per variable is: %s\n"
                                (String.concat ~sep:", " (List.map skel.bindings (fun b -> Int.to_string (List.length (b.fromvars_index_nesting))))) in
					()
				else () in
                (* Each elt in the skel bindings is a list of possible dimvars *)
				let binding_options = List.map skel.bindings flatten_binding in
                (* Product them all together so each is a sinlge list of options.  *)
				let bindings = cross_product binding_options in
                let () = if opts.debug_skeleton_flatten then
                    let () = Printf.printf "Post flattening are %s\n" (
                        flat_single_variable_binding_list_list_to_string bindings
                    ) in
                    let () = Printf.printf "Number of variables is %d\n" (List.length bindings) in
                    () else () in
				bindings
		)
	) in
	let result = List.map vbinds (fun bind ->
	{
		flat_bindings = bind
	}
    ) in
	let () = if opts.debug_skeleton_flatten then
		Printf.printf "Produced results of length %d \n " (List.length result)
	else () in
	(* There must be more things after we expand them. *)
    (* Failure of this assertion implies that there is something odd going on
    with the dimension variables.  *)
	let () = assert ((List.length result) >= (List.length skels)) in
	let result = List.filter result length_variable_compatability in
	result
