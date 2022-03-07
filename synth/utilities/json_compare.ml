open Executable_test;;
open Cmdliner;;
open Options;;

let main f1 f2 =
    let opts = {default_options with debug_comparison = true; } in
    let result = compare_outputs opts f1 f2 in
    Printf.printf "Result of comparison is %b\n" (result)

let f1 =
	let doc = "JSON file 1" in
	Arg.(required & pos 0 (some string) None & info [] ~docv:"JSON1" ~doc)
let f2 =
	let doc = "JSON file 2" in
	Arg.(required & pos 1 (some string) None & info [] ~docv:"JSON2" ~doc)

let info =
	let doc = "Compare JSON files" in
	Term.info "JSONGen" ~doc

let args_t = Term.(const main $ f1 $ f2)

let () = Term.exit @@ Term.eval (args_t, info)
