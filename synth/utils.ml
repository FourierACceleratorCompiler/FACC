open Core_kernel;;
open Options;;

exception UtilException of string

let id x = x

let rec cross_product ls =
	match ls with
	| [] -> []
	| [x] ->
			(* let () = Printf.printf "Length of x is %d" (List.length x) in *)
			List.map x (fun value -> [value])
	| (x :: xs) ->
			let subcross = cross_product xs in
			List.concat(
			List.map subcross (fun subprod ->
				List.map x (fun value -> 
					value :: subprod
				)
			)
			)


let uniq_cons eql x xs = if List.mem xs x eql then xs else x :: xs

let remove_duplicates eql xs = List.fold_right xs ~f:(uniq_cons eql) ~init:[]

let set_difference eql l1 l2 = List.filter l1 (fun x -> not (List.mem l2 x eql))

let hash_table_from_list s l =
    let tbl = Hashtbl.create s in
    let () = ignore(List.map l (fun (i, t) ->
        Hashtbl.add tbl i t
    )) in
    tbl

let rec truncate_zip l1 l2 =
	match l1, l2 with
	| [], _ -> []
	| _, [] -> []
	| x :: xs, y :: ys ->
			(x, y) :: (truncate_zip xs ys)

let rec extend_zip l1 l2 =
	match l1, l2 with
    | y :: [], x :: [] -> [(y, x)]
	| [], x :: xs -> raise (UtilException "Invalid Unequal lengths")
	| y :: ys, [] -> raise (UtilException "Invalid Unequal lengths")
	| [], [] -> []
	| x :: xs, y1 :: y2 :: ys ->
			(x, y1) :: (extend_zip xs (y2 :: ys))
	| x :: xs, y :: [] ->
			(x, y) :: (extend_zip xs [y])

let prepend_all x xs =
    List.map xs (fun l -> x :: l)

let max_of_int_list xs =
	List.fold xs ~f:(fun x -> fun y -> if x > y then x else y) ~init:(-1000000000)
	(* Crappy hack to regret: *)

let int_range low high =
    let rec int_range_internal low high =
        if low = high then
            []
        else
            low :: (int_range_internal (low + 1) high)
    in
    if low > high then
        []
    else
        int_range_internal low high

(* Map through f while f does not return None.  *)
let rec map_while ls f =
	match ls with
	| [] -> []
	| l :: ls ->
			match f(l) with
			| item, true -> item :: (map_while ls f)
			| item, false -> item :: []

let string_equal x y =
	(String.compare x y) = 0

let float_equal f1 f2 =
	  (* Aim for error no bigger than a 10th of f2, should
		100% make this configurable.  *)
	  let thresh = (Float.abs (f2)) /. 10.0 in
	  ((Float.compare f1 (f2 +. thresh)) = -1) &&
	  ((Float.compare f1 (f2 -. thresh) = 1))

let rec between x y =
	if x = y then
		[]
	else
		x :: (between (x + 1) y)

let log_2 i =
	let iref = ref i in
	let l2_value = ref 0 in
	while (!iref > 0) do
		iref := (!iref) lsr 1;
		l2_value := !l2_value + 1
	done;
	!l2_value - 1

let double_map l f =
	List.map l (fun e ->
		List.map e f
	)

(* Parmap is hard to trace errors through.  *)
let parmap options f l =
	if options.use_parmap then
		Parmap.parmap f (Parmap.L l)
	else
		List.map l f

let rec unzip3 ls =
	match ls with
	| [] -> [], [], []
	| (a, b, c) :: ls ->
			let xs, ys, zs = unzip3 ls in
			(a :: xs, b :: ys, c :: zs)

let power_of_two i = 1 lsl i

let strings_any_equal s1s s2s =
	List.exists s1s (fun s1 ->
		List.exists s2s (fun s2 ->
			string_equal s1 s2
		)
	)
