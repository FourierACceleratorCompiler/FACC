open Core_kernel;;
open Parse_range;;
open Parse_type;;
open Yojson.Basic.Util;;
open Range;;

exception ConstTableException of string

let load_consttable options typemap json =
    let result = Hashtbl.create (module String) in
	let () = match json with
	| `Assoc(_) ->
		let keys = json |> keys in
		let _ = ignore(
			List.map keys (fun key ->
				let typ = Hashtbl.find_exn typemap key in
				let range = json |> member key |> to_string in
				let parsed_range = range_values (parse_range options typ range) in
				let synth_values = List.map parsed_range range_value_to_synth_value in
				Hashtbl.set result key synth_values
			)
		) in
		()
	(* Const table not specified.  *)
	| `Null -> ()
	| _ -> raise (ConstTableException "Unexpected JSON format for constable")
	in
	result
