(* Parse a range value.  *)
open Core_kernel;;
open Options;;
open Range_definition;;
open Spec_definition;;
open Spec_utils;;
open Yojson.Basic.Util;;

exception TypeCheckException of string

let rec typecheck_item typ item =
	match item with
	| RangeInteger(_) -> is_integer_type typ
	| RangeFloat(_) -> is_float_type typ
	| RangeBool(_) -> is_bool_type typ
	| RangeArray(t, arr) ->
			match typ with
			(* TODO --- could also do a check against dimension here.  *)
			| Array(stype, dims) ->
					List.for_all arr (typecheck_item stype)
			| _ -> false

let typecheck_range_range typ r =
	match r with
	| RangeItem(i) -> typecheck_item typ i
	| RangeRange(f, t) ->
			(typecheck_item typ f) &&
			(typecheck_item typ t)
let typecheck t ast =
	match ast with
	| RangeSet(itms) ->
			Array.for_all (Array.map itms (typecheck_range_range t)) (fun x -> x)

let rec desugar_item i =
    match i with
    | SugaredRangeInteger(i) -> RangeInteger(i)
    | SugaredRangeFloat(f) -> RangeFloat(f)
    | SugaredRangeBool(b) -> RangeBool(b)
    | SugaredRangeArray(t, a) -> RangeArray(t, List.map a desugar_item)
and desugar_range_range rr = match rr with
    | SugaredRangeRange(f, t) -> [RangeRange(desugar_item f, desugar_item t)]
    | SugaredRangeItem(i) -> [RangeItem(desugar_item i)]
    | SugaredRangeFunction(f) ->
            match f with
            | SugaredRangePowerOfTwo(from, upto) ->
					(* TODO --- perhaps this should be a two-argument thing?  e.g. from and to? *)
					(* It's very plausible that an FFT wouldn't support powers from 1 *)
					(* Note that since ocaml only has 63-bit signed ints, we only really support
					up to 2^62 before things get weird. *)
                    List.map (Utils.between from upto) (fun x -> RangeItem(RangeInteger(1 lsl x)))
and desugar r = match r with
    | SugaredRangeSet(srange) ->
			let range_items = Array.concat (Array.to_list (Array.map srange (fun x -> (Array.of_list (desugar_range_range x))))) in
			let () = assert ((Array.length range_items) > 0) in
            RangeSet(range_items)

(* We'd really like to handle more complex things, like
functions, e.g. every odd number in this.  *)
let parse_range options typ range_string =
	let lexbuf = Lexing.from_string range_string in
	let ast = desugar (Rangeparse.t Rangelex.read lexbuf) in
    let res = typecheck typ ast in
    let () =
        if options.debug_load then
            let () = Printf.printf "Type is %s and range is %s\n" (synth_type_to_string typ) (range_string) in
            ()
        else ()
    in
    if res then
        ast
    else
		let () = Printf.printf "Type is %s and range is %s\n" (synth_type_to_string typ) (range_string) in
        raise (TypeCheckException "Range for variable doesn't typecheck")


let load_rangetable options classmap typemap range_field =
    let range_tbl = Hashtbl.create (module String) in
    let () = match range_field with
    | `Null -> ()
    | range_json ->
            let ranged_vars = keys range_json in
            let _ = List.map ranged_vars (fun k ->
            let typof = type_of typemap classmap k in
            let r = Hashtbl.add range_tbl k (parse_range options typof (range_field |> member k |> to_string)) in
            let () = match r with
            | `Ok -> ()
            | `Duplicate -> raise (TypeCheckException "Duplicate range def!")
            in
            ()
	) in
	() in
    range_tbl

