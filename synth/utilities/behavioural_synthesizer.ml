open Synthesizer_interface;;
open Json_utils;;
open Options;;

(* Enables direct access to the post-synthesizers.  *)

let main mode f1 f2 =
	let opts = default_options with { post_synthesizer = get_behavioural_syn mode } in
	(* Convert the I/O into value pairs.  *)
	let f1s = String.split_on_char ',' f1 in
	let f2s = String.split_on_char ',' f2 in
	let io_pairs = List.map (List.zip_exn f1s f2s) (fun (f1, f2) ->
		(load_value_map_from f1), (load_value_map_from f2)
	) in
	let first_pair_elem, _ = List.hd_exn io_pairs in
	let names = List.map (Hashtbl.keys first_pair_elem) in
	match opts.post_synthesizer with
	| NoSynthesizer -> Printf.printf "No synthesizer!"
	| InternalFFTSynth ->
			ignore(
				fft_synthesizer names io_pairs
			)

let f1 =
	let doc = "Input examples (comma separated)" in
	Arg.(required & pos 1 (some string) None & info [] ~docv:"Inps" ~doc)

let f2 =
	let doc = "Output examples (comma separated)" in
	Arg.(required & pos 2 (some string) None & info [] ~docv:"Outps" ~doc)

let mode =
	let doc = "Which behavioural synth to use (currently 'FFT' or 'None')" in
	Arg.(required & pos 0 (some string) None & info [] ~docv:"mode" ~doc)

let info =
	let doc = "Run one of the behavioural synthesizers on it's own." in
	Term.info "BehaviouralSyntRun"

let args_t = Term.(const main $ mode $ f1 $ f2)

let () = Term.exit @@ Term.eval (args_t, info)
