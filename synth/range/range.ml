open Core_kernel;;
open Range_definition;;
open Random;;
open Spec_definition;;
open Utils;;

exception RangeError of string

let rec value_from_range_item item =
	match item with
	| RangeInteger(i) -> RInt(i)
	| RangeFloat(f) -> RFloat(f)
	| RangeBool(b) -> RBool(b)
    | RangeArray(typ, a) ->
            RArray(typ, List.map a value_from_range_item)

let random_value_from_range_range_in_range r =
	match r with
	| RangeItem(i) -> value_from_range_item i
	| RangeRange(small, large) ->
			let vsmall = value_from_range_item small in
			let vlarge = value_from_range_item large in
			match vsmall, vlarge with
			| RInt(low), RInt(high) ->
					let v = Random.int (high - low) in
					RInt(v + low)
			| RFloat(low), RFloat(high) ->
					let v = Random.float (high -. low) in
					RFloat(v +. low)
			| RBool(low), RBool(high) ->
					if (not low) && (high) then
						let v = (Random.int 1) = 1 in
						RBool(v)
					else
						(* If the aren't different, then they'll
						be the same.  *)
						RBool(low)
            | RArray(_, low), RArray(_, high) ->
                    (* Is there a better way to do this? *)
                    raise (RangeError "Invalid range range with arrays")
			| _, _ ->
					raise (RangeError "Expected a typechecked range before actualy executing it. ")

(* We do a super shitty algorithm for doing this
here, where we just pick random elements from
the set.  Should 100% be more intelligent than this.*)
let random_value_in_range range =
	match range with
	| RangeSet(items) ->
			let () = if (Array.length items) = 0 then
				Printf.printf "Empty items passed!\n"
			else
				()
			in
			let n = Random.int (Array.length items) in
			(* Pick a random item and get the thing from that.  *)
			random_value_from_range_range_in_range (Array.get items n);;

let range_size_add x y = match x, y with
    | Infinite, _ -> Infinite
    | _, Infinite -> Infinite
    | Finite(x), Finite(y) -> Finite(x + y)

let range_item_size ritem =
    match ritem with
    | RangeItem(i) -> Finite(1)
    | RangeRange(start, finish) ->
            (
            match start, finish with
            | RangeInteger(s), RangeInteger(f) -> Finite(f - s)
            | RangeFloat(s), RangeFloat(f) ->
                    (* Can we do better than this? *)
                    (* Perhaps by adding to the range size type? *)
                    Infinite
			| RangeBool(s), RangeBool(f) ->
					if (Bool.compare s f) = 0 then
						Finite(1)
					else
						Finite(2)
            | RangeArray(typ, low), RangeArray(typ2, high) ->
                    raise (RangeError "Invlaid range range with arrays")
			| _, _ ->
					raise (RangeError ("Type Error"))
            )


(* How many items in this set? *)
let range_size r =
    match r with
    | RangeSet(items) ->
            let item_lengths = Array.map items range_item_size in
            Array.fold item_lengths ~f:range_size_add ~init:(Finite(0))

let rec range_value_to_item i = match i with
    | RInt(fint) -> RangeInteger(fint)
    | RFloat(ffloat) -> RangeFloat(ffloat)
	| RBool(fbool) -> RangeBool(fbool)
	| RArray(typ, farr) -> RangeArray(typ, List.map farr range_value_to_item)

let range_values_range_range rr =
    match rr with
    | RangeItem(i) -> [value_from_range_item i]
    | RangeRange(f, t) ->
            match f, t with
            | RangeInteger(fint), RangeInteger(toint) ->
                    List.map (int_range fint toint) (fun ival ->
                        RInt(ival))
            | RangeFloat(ffloat), RangeFloat(tofloat) ->
                    raise (RangeError "Can't do value set of floating range (i.e. of infinite window)")
			| RangeBool(sbool), RangeBool(tbool) ->
					if (Bool.compare sbool tbool) = 0 then
						[RBool(sbool)]
					else
						[RBool(sbool); RBool(tbool)]
			| RangeArray(typ, sarr), RangeArray(ttyp, tarr) ->
					raise (RangeError "can't have rangesrange of arrays")
            | _, _ ->
                    raise (RangeError "Type error")

(* Given a range with a finite number of items, generate
a set of values from it.  *)
let range_values rset = match rset with
    | RangeSet(items) ->
            let sub_items = Array.to_list (Array.map items range_values_range_range) in
            (* Warning: this is not uniquified, so needs
            the input ranges to be disjoint.  *)
            List.concat sub_items

(*  Implemented in what I think is the standard OCaml
way of -1 => LT, 0 => EQ, 1 => GT *)
let range_size_compare r1_size r2_size =
    match r1_size, r2_size with
    | Infinite, Infinite -> 0
    | Infinite, _ -> 1
    | _, Infinite -> -1
    | Finite(r1), Finite(r2) ->
            Int.compare r1 r2

let rec range_type_value i = match i with
	| RangeInteger(_) -> RangeIntegerType
	| RangeFloat(_) -> RangeFloatType
	| RangeBool(_) -> RangeBoolType
	| RangeArray(typ, sub) -> RangeArrayType(typ)

let rec sugared_range_type_value i = match i with
	| SugaredRangeInteger(_) -> RangeIntegerType
    | SugaredRangeFloat(_) -> RangeFloatType
    | SugaredRangeBool(_) -> RangeBoolType
    | SugaredRangeArray(t, s) -> RangeArrayType(t)

and range_type_item i = match i with
	| RangeItem(i) -> range_type_value i
	| RangeRange(f, _) -> range_type_value f

and range_type r = match r with
	| RangeSet(items) ->
			range_type_item (Array.get items 0)

let rec range_compare v1 v2 =
	match v1, v2 with
	| RangeInteger(i1), RangeInteger(i2) ->
			Int.compare i1 i2
	| RangeFloat(f1), RangeFloat(f2) ->
			Float.compare f1 f2
	| RangeBool(b1), RangeBool(b2) ->
			Bool.compare b1 b2
	| RangeArray(t, a1), RangeArray(t2, a2) ->
			(* Use a python-like compare --- first element matters most.  *)
			(* Longer arrays are always later though.  *)
            (
			match List.zip a1 a2 with
			| Ok(l) ->
					let cmpranges = List.map l (fun (e1, e2) -> range_compare e1 e2) in
					let result = List.fold cmpranges ~init:0 ~f:(fun eq -> (fun cmpvalue ->
						if eq = 0 then
							cmpvalue
						else
							eq
					)) in
					result
			| Unequal_lengths ->
					if (List.length a1) < (List.length a2) then
						-1
					else
						1
            )
	| _, _ -> raise (RangeError "Type error")

let is_integer_range t =
	match t with
	| RangeIntegerType -> true
	| _ -> false

let range_item_max im =
	match im with
	| RangeItem(i) -> i
	| RangeRange(f, t) -> t

let range_max r = match r with
	| RangeSet(items) ->
			let max_of_each = Array.map items range_item_max in
			let max_overall = Array.fold max_of_each ~init:(Array.get max_of_each 0) ~f:(fun acc -> fun r ->
				if (range_compare acc r) < 0 then
					r
				else
					acc
			) in
			max_overall

let range_item_min im =
	match im with
	| RangeItem(i) -> i
	| RangeRange(f, t) -> f

let range_min r = match r with
	| RangeSet(items) ->
			let min_of_each = Array.map items range_item_min in
			let min_overall = Array.fold min_of_each ~init:(Array.get min_of_each 0) ~f:(fun acc -> fun r ->
				if (range_compare acc r) > 0 then
					r
				else
					acc
			) in
			min_overall

let range_value_set_sort vset =
    (* Perhaps it would be better to keep these sets as arrays? *)
    List.sort vset (fun a -> fun b ->
        match a, b with
        | RInt(a), RInt(b) -> Int.compare a b
        | RFloat(a), RFloat(b) -> Float.compare a b
		| RBool(a), RBool(b) -> Bool.compare a b
        | _ -> raise (RangeError "Type error")
    )

let rec range_value_eq v1 v2 =
	match v1, v2 with
	| RangeBool(i), RangeBool(j) -> (Bool.compare i j) = 0
	| RangeInteger(i), RangeInteger(j) -> i = j
	| RangeFloat(i), RangeFloat(j) -> Utils.float_equal i j
	| RangeArray(t, suba), RangeArray(t1, suba2) ->
			(
			match List.zip suba suba2 with
			| Ok(l) -> List.for_all l (fun (e1, e2) -> range_value_eq e1 e2)
			| Unequal_lengths -> false
			)
	| RangeInteger(_), _ -> false
	| RangeFloat(_), _ -> false
	| RangeBool(_), _ -> false
	| RangeArray(_, _), _ -> false

let range_value_in r v =
	match r with
	| RangeItem(i) -> range_value_eq i v
	| RangeRange(f, t) ->
			match f, t, v with
			| RangeInteger(l), RangeInteger(h), RangeInteger(i) ->
					(i >= l) && (i <= h)
			| RangeFloat(l), RangeFloat(h), RangeFloat(i) ->
					((Float.compare l i) <= 0) && ((Float.compare h i) >= 0)
			| RangeBool(b1), RangeBool(b2), RangeBool(i) ->
					((Bool.compare i b1) = 0) || ((Bool.compare i b2) = 0)
			| RangeArray(_, _), RangeArray(_, _), RangeArray(_, _) ->
					raise (RangeError "Unsupported range range array operation")
			| _, _, _ -> raise (RangeError "Type error")

let range_overlap (lower, higher) (lower2, higher2) =
	let new_low = if (range_compare lower lower2) = -1 then
		(* Lower < lower2, so lower2 is the new base *)
		lower2
	else lower
	in
	let new_high = if (range_compare higher higher2) = 1 then
		(* higher > higher2, so higher 2 is the new high *)
		higher2
	else
		higher
	in
	if (range_compare new_low new_high) = 1 then
		(* New_low > new_high, so new range is empty.  *)
		None
	else
		Some(RangeRange(new_low, new_high))

let rec range_item_to_synth_value rvalue =
	match rvalue with
	(* TODO --- do we need to do something more sane with widths? *)
	| RangeInteger(i) -> Int64V(i)
	| RangeFloat(f) -> Float64V(f)
	| RangeBool(b) -> BoolV(b)
	| RangeArray(t, arr) ->
			ArrayV(List.map arr range_item_to_synth_value)

let rec range_value_to_synth_value rvalue =
    match rvalue with
    | RInt(v) -> Int32V(v)
    | RFloat(v) -> Float32V(v)
	| RBool(v) -> BoolV(v)
	| RArray(t, arr) ->
			ArrayV(List.map arr range_value_to_synth_value)

let rec synth_type_to_range_type stype =
	match stype with
	| Int16 -> RangeIntegerType
	| Int32 -> RangeIntegerType
	| Int64 -> RangeIntegerType
	| UInt16 -> RangeIntegerType
	| UInt32 -> RangeIntegerType
	| UInt64 -> RangeIntegerType
	| Float16 -> RangeFloatType
	| Float32 -> RangeFloatType
	| Float64 -> RangeFloatType
	| Bool -> RangeBoolType
	| Pointer(sty) -> synth_type_to_range_type sty
	| Array(sty, dim) ->
			RangeArrayType(synth_type_to_range_type sty)
	| _ -> raise (RangeError "Unsupported range type")

(* Since we don't currently support range types of functions,units/arrays etc.
   we return just an option here.  *)
let rec item_from_synth_value svalue =
	let rvalue = match svalue with
		| Int16V(v) -> Some(RangeInteger(v))
		| Int32V(v) -> Some(RangeInteger(v))
		| Int64V(v) -> Some(RangeInteger(v))
		| UInt16V(v) -> Some(RangeInteger(v))
		| UInt32V(v) -> Some(RangeInteger(v))
		| UInt64V(v) -> Some(RangeInteger(v))
		| Float16V(v) -> Some(RangeFloat(v))
		| Float32V(v) -> Some(RangeFloat(v))
		| Float64V(v) -> Some(RangeFloat(v))
		| BoolV(v) -> Some(RangeBool(v))
		| PointerV(v) ->
				item_from_synth_value v
		| ArrayV(v) ->
                (
				match v with
				| [] -> (* Just generate something I suppose --- think
				that where this is used it doesn't matter too much.
				Also, empty arrays in C etc. is a bit strange.
				*)
						Some(RangeArray(RangeIntegerType, []))
				| xs ->
						let xs = List.map xs item_from_synth_value in
                        let xs: range_item list = List.map xs (fun f -> match f with | None -> raise (RangeError "Unexpected") | Some(f) -> f) in
						let typ = range_type_value (List.hd_exn xs) in
						Some(RangeArray(typ, xs))
                )
		| _ -> None
	in
    rvalue

let range_from_synth_value svalue =
    let rvalue = item_from_synth_value svalue in
	Option.map rvalue (fun rv -> RangeSet(
		Array.of_list [RangeItem(rv)]
	))

let range_size_less_than r1 r2 =
	match r1, r2 with
	| Infinite, _ -> false
	| _, Infinite -> true
	| Finite(x), Finite(y) -> x < y

let range_size_diff r1 r2 =
	match r1, r2 with
	(* This isn't really math, more 'it does what 
	I need it to do' *)
	| Infinite, Infinite -> Finite(0)
	| Finite(n), Infinite -> Finite(0)
	| Infinite, Finite(n) -> Infinite
	| Finite(n), Finite(m) -> Finite(n - m)

let range_size_divide n m =
	match n, m with
	| Infinite, Infinite -> Finite(1)
	| Infinite, Finite(n) -> Infinite
	| Finite(n), Infinite -> Finite(0)
	| Finite(n), Finite(0) -> Infinite
	| Finite(n), Finite(m) -> Finite(n / m)

let range_size_to_string n =
    match n with
    | Finite(x) -> string_of_int x
    | Infinite -> "Inf"

let rec range_item_to_string i =
    match i with
    | RangeInteger(i) -> (string_of_int i)
    | RangeFloat(f) -> (string_of_float f)
    | RangeBool(b) -> (string_of_bool b)
    | RangeArray(t, vs) ->
            "[" ^ (String.concat ~sep:", " (List.map vs range_item_to_string)) ^ "]"

let range_range_to_string rrange =
	match rrange with
	| RangeRange(f, t) ->
            (range_item_to_string f) ^ " to " ^ (range_item_to_string t)
    | RangeItem(i) ->
            (range_item_to_string i)

let range_set_to_string rset =
	match rset with
	| RangeSet(elems) ->
			"{" ^ (String.concat ~sep:", " (Array.to_list (Array.map elems range_range_to_string))) ^ "}"

let range_map_to_string rangemap =
	let keys = Hashtbl.keys rangemap in
	String.concat ~sep:"\n" (List.map keys (fun key ->
		let range = Hashtbl.find_exn rangemap key in
		"For key " ^ key ^ ": have range " ^ (range_set_to_string range) ^ "(size " ^ (range_size_to_string (range_size range)) ^ ")"
	))

let empty_range_set r =
	let rsize = range_size r in
	(range_size_compare rsize (Finite(0))) = 0

let range_item_to_int i =
	match i with
	| RangeInteger(i) -> i
	| _ -> raise (RangeError "Can't do non int to integer")

(*  These functions ask if this is an /entirely/ even
    range.  ie. would it be safe to divide it by two.
	*)
let is_even_item i = match i with
	| RangeInteger(x) -> (x % 2) = 0
	| _ -> false (* only consider integer types.  *)

let is_even_range_range r = match r with
	| RangeRange(f, t) ->
			false
	| RangeItem(i) ->
			is_even_item i

let is_even_range r =
	match r with
	| RangeSet(elems) ->
			Array.for_all elems is_even_range_range
