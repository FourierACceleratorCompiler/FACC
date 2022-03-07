open Core_kernel;;
open Options;;

let print_working_code_warnings options progrfiles =
    ignore(List.map progrfiles (fun (prog, code) ->
        (* Check the code for facc_malloc.  If that's used,
        then we have to replace every free /in the program
        in which this is implemented/ with facc_free. *)
        let malloc_search = Str.regexp_string "facc_malloc" in
        let has_match =
			try ignore(Str.string_match malloc_search code 0); true
			with _ -> false
		in
        if has_match then
            Printf.printf "WARNING: At least one option uses facc_malloc: all calls to free that can be reached by this pointer must be replaced with facc_free\n"
        else ()
    ))
