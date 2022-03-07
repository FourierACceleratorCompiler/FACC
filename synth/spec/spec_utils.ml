open Core_kernel;;
open Spec_definition;;
open Range_definition;;

exception SpecException of string

let rec name_reference_to_string nref =
	match nref with
	| AnonymousName -> "Annon"
	| Name(s) -> s
	| StructName(ns) -> (String.concat ~sep:"." (List.map ns name_reference_to_string))

let name_reference_list_to_string nrefs =
	String.concat ~sep:", " (List.map nrefs name_reference_to_string)

let name_reference_option_list_to_string nrefs =
	String.concat ~sep:", " (List.map nrefs (fun n ->
		match n with
		| None -> "None"
		| Some(n) -> name_reference_to_string n))

let name_reference_concat n1 n2 =
	match (n1, n2) with
	| Name(x), Name(y) -> StructName([n1; n2])
	| StructName(xs), Name(y) -> StructName(xs @ [n2])
	| Name(x), StructName(ys) -> StructName(n1 :: ys)
	| StructName(xs), StructName(ys) ->
			StructName(xs @ ys)
	| _, AnonymousName -> n1
	| AnonymousName, _ -> n2

let rec name_reference_list_concat ns =
    match ns with
    | [] -> AnonymousName
    | x :: xs ->
            let rest = name_reference_list_concat xs in
            name_reference_concat x rest

let rec name_reference_canonicalize nms =
	match nms with
	| Name(x) -> Name(x)
	| StructName(xs) ->
			let nms = List.concat (
				List.map xs (fun xs ->
					match xs with
					| Name(x) -> [Name(x)]
					| AnonymousName -> []
					| StructName(xs) ->
							match name_reference_canonicalize (StructName(xs)) with
							| Name(x) -> [Name(x)]
							| AnonymousName -> []
							| StructName(xs) -> xs
				)
			) in
			StructName(nms)
	| AnonymousName -> AnonymousName

let name_reference_from_string ns =
    let result =
        if String.equal ns "" then
            AnonymousName
        else
            let chrs = (String.split_on_chars ~on:['.'] ns) |> (List.filter ~f:(fun x -> not (String.equal x ""))) in
            StructName(List.map chrs (fun c -> Name(c)))
    in
    (* let () = Printf.printf "From input name %s, computed name reference %s\n" (ns) (name_reference_to_string result) in *)
    result

let rec name_reference_equal n1 n2 =
    match (n1, n2) with
	| AnonymousName, AnonymousName -> true
    | Name(x), Name(y) -> (String.compare x y) = 0
    | StructName(xns), StructName(yns) -> (
            match List.zip xns yns with
            | Ok(nms) -> List.for_all nms (fun (a, b) -> name_reference_equal a b)
            | Unequal_lengths -> false
	)
	(* Assume no funny-business with
	oddly nested structnames *)
	| _, _ -> false

(* compare two lists of variables.  Return true if any two are equal.  *)
let name_reference_any_equal ns1 ns2 =
    List.exists ns1 (fun n1 ->
        List.exists ns2 (fun n2 ->
            name_reference_equal n1 n2
        )
    )

let rec name_reference_top_level_name n =
	match n with
	| AnonymousName -> raise (SpecException "No name for empty name ref")
	| Name(x) -> Name(x)
	| StructName(x :: xs) -> x
	| StructName([]) -> raise (SpecException "Invalid struct name")

let name_reference_list_concat ns =
	List.fold ~init:AnonymousName ~f:name_reference_concat ns

let dim_relation_to_string dimrel =
    match dimrel with
    | DimEqualityRelation -> " (=) "
    | DimPo2Relation -> " (^2) "
	| DimDivByRelation(x) -> " (/" ^ (string_of_int x) ^ ")"

let dimension_value_to_string (dim: dimension_value) =
	match dim with
	| DimConstant(i) -> (string_of_int i)
	| DimVariable(n, rel) -> (name_reference_to_string n) ^ (dim_relation_to_string rel)

let dimension_value_list_to_string dimlist =
    String.concat ~sep:", " (List.map dimlist dimension_value_to_string)

let dim_relation_equal r1 r2 = match r1, r2 with
	| DimEqualityRelation, DimEqualityRelation -> true
	| DimPo2Relation, DimPo2Relation -> true
	| DimDivByRelation(x), DimDivByRelation(y) ->
			x = y
	| _, _ -> false
	
let dimension_value_equal d1 d2 =
	match d1, d2 with
	| DimConstant(c1), DimConstant(c2) -> c1 = c2
	| DimVariable(v1, r1), DimVariable(v2, r2) -> (name_reference_equal v1 v2) && (dim_relation_equal r1 r2)
	| _, _ -> false

let dimension_constant_less_than d1 d2 =
	match d1, d2 with
	| Dimension(DimConstant(c1)), Dimension(DimConstant(c2)) ->
			c1 < c2
	| _, _ -> raise (SpecException "not allowed dim constant < comparison with non-constnats")

let rec dimension_type_to_string dim =
    match dim with
    | EmptyDimension -> "No dimensions set!"
    | Dimension(nrefs) ->
            (dimension_value_to_string nrefs)

let empty_dimension dim = match dim with
	| EmptyDimension -> true
	| _ -> false

let dimension_type_list_to_string dims =
	String.concat ~sep:"DIM: " (
		List.map dims dimension_type_to_string
	)

let dimension_type_equal d1 d2 = match d1, d2 with
    | EmptyDimension, EmptyDimension -> true
    | Dimension(a), Dimension(b) ->
		dimension_value_equal a b
	| _ -> false

let is_constant_dimension d =
	match d with
	| DimConstant(c) -> true
	| DimVariable(v, _) -> false
	
let is_constant_dimension_variable d =
	match d with
	| EmptyDimension -> false
	| Dimension(d) ->
			is_constant_dimension d

let constant_dimension_size d =
	match d with
	| DimConstant(c) -> Some(c)
	| DimVariable(_, _) -> None

let rec synth_type_to_string t =
    match t with
	| Bool -> "bool"
    | Int16 -> "int16"
    | Int32 -> "int32"
    | Int64 -> "int64"
	| UInt16 -> "uint16"
	| UInt32 -> "uint32"
	| UInt64 -> "uint64"
    | Float16 -> "float16"
    | Float32 -> "float32"
    | Float64 -> "float64"
	| Pointer(tp) -> "pointer(" ^ (synth_type_to_string tp) ^ ")"
    | Array(x, dims) -> "array(" ^ (synth_type_to_string x) ^ ": with dims " ^
        (dimension_type_to_string dims) ^ ")"
    | Struct(name) -> name
    | Fun(from, fto) -> (synth_type_list_to_string from) ^ "->" ^ (synth_type_to_string fto)
	| Unit -> "unit"

and synth_type_list_to_string stlist =
	String.concat ~sep:", " (List.map stlist synth_type_to_string)

let rec synth_type_equal s1 s2 =
	match s1, s2 with
	| Bool, Bool -> true
	| Int16, Int16 -> true
	| Int32, Int32 -> true
	| Int64, Int64 -> true
	| UInt16, UInt16 -> true
	| UInt32, UInt32 -> true
	| UInt64, UInt64 -> true
	| Float16, Float16 -> true
	| Float32, Float32 -> true
	| Float64, Float64 -> true
	| Pointer(tp1), Pointer(tp2) ->
			(synth_type_equal tp1 tp2)
	| Array(x, dms), Array(x2, dms2) ->
			(synth_type_equal x x2) &&
			(dimension_type_equal dms dms2)
	| Struct(nm), Struct(nm2) ->
			(String.compare nm nm2) = 0
	| Unit, Unit -> true
	| Fun(f, t), Fun(f2, t2) ->
			let from_types_equal =
				match List.zip f f2 with
				| Ok(l) -> List.for_all l (fun (a, b) -> synth_type_equal a b)
				| Unequal_lengths -> false
			in
			(from_types_equal) &&
			(synth_type_equal t t2)
	| _ -> false

let rec synth_value_to_string value =
    match value with
	| BoolV(v) -> string_of_bool v
    | Int16V(v) -> string_of_int v
    | Int32V(v) -> string_of_int v
    | Int64V(v) -> string_of_int v
	| UInt16V(v) -> string_of_int v
	| UInt32V(v) -> string_of_int v
	| UInt64V(v) -> string_of_int v
    | Float16V(v) -> string_of_float v
    | Float32V(v) -> string_of_float v
    | Float64V(v) -> string_of_float v
    | UnitV -> "Unit"
	| PointerV(v) ->
			"pointer(" ^ (synth_value_to_string v) ^ ")"
    | ArrayV(vs) ->
            "[" ^ (String.concat ~sep:", " (List.map vs synth_value_to_string)) ^ "]"
    | StructV(name, _) ->
            name
    | FunV(name) ->
            name

let synth_value_list_to_string values =
    String.concat ~sep:", " (List.map values synth_value_to_string)

let rec synth_value_to_type v =
	match v with
	| BoolV(_) -> Bool
	| Int16V(_) -> Int16
	| Int32V(_) -> Int32
	| Int64V(_) -> Int64
	| UInt16V(_) -> UInt16
	| UInt32V(_) -> UInt32
	| UInt64V(_) -> UInt64
	| Float16V(_) -> Float16
	| Float32V(_) -> Float32
	| Float64V(_) -> Float64
	| UnitV -> Unit
	| PointerV(v) ->
			Pointer(synth_value_to_type v)
	| ArrayV(vs) ->
			(* TODO --- support something better for
			empty arrays? *)
			let subtype = synth_value_to_type (List.hd_exn vs) in
			Array(subtype, Dimension(DimConstant(List.length vs)))
	| StructV(name, cnts) ->
			Struct(name)
	| FunV(v) ->
			(* Think we just need a typemap to do this one.  *)
			raise (SpecException "Unsupported")

let rec synth_value_equal c1 c2 =
	match c1, c2 with
	| BoolV(v1), BoolV(v2) -> (Bool.compare v1 v2) = 0
	(* Note that we could consider comparing
	different widths of synth type here.  *)
	| Int16V(v1), Int16V(v2) -> v1 = v2
	| Int32V(v1), Int32V(v2) -> v1 = v2
	| Int64V(v1), Int64V(v2) -> v1 = v2
	| UInt16V(v1), UInt16V(v2) -> v1 = v2
	| UInt32V(v1), UInt32V(v2) -> v1 = v2
	| UInt64V(v1), UInt64V(v2) -> v1 = v2
	| Float16V(v1), Float16V(v2) -> Utils.float_equal v1 v2
	| Float32V(v1), Float32V(v2) -> Utils.float_equal v1 v2
	| Float64V(v1), Float64V(v2) -> Utils.float_equal v1 v2
	| UnitV, UnitV -> true
	| PointerV(vp1), PointerV(vp2) ->
			synth_value_equal vp1 vp2
	| ArrayV(vs1), ArrayV(vs2) ->
			(
			match List.zip vs1 vs2 with
			| Ok(l) ->
					List.for_all l (fun (v1, v2) -> synth_value_equal v1 v2)
			| Unequal_lengths ->
					false
			)
	| StructV(name, tbl), StructV(name2, tbl2) ->
			if (String.compare name name2) = 0 then
				synth_table_equal tbl tbl2
			else
				false
	| FunV(n1), FunV(n2) ->
			(String.compare n1 n2) = 0
	| _, _ -> false

and synth_table_equal tbl1 tbl2 =
	let keys1 = List.sort (Hashtbl.keys tbl1) String.compare in
	let keys2 = List.sort (Hashtbl.keys tbl2) String.compare in
	match List.zip keys1 keys2 with
	| Ok(l) ->
			List.for_all l (fun (k1, k2) ->
				if (String.compare k1 k2) = 0 then
					synth_value_equal
						(Hashtbl.find_exn tbl1 k1)
						(Hashtbl.find_exn tbl2 k2)
				else
					false
			)
	| Unequal_lengths ->
			false

let rec synth_value_has_type v t =
	match v, t with
	| Int16V(_), Int16 -> true
	| Int32V(_), Int32 -> true
	| Int64V(_), Int64 -> true
	| UInt16V(_), UInt16 -> true
	| UInt32V(_), UInt32 -> true
	| UInt64V(_), UInt64 -> true
	| Float16V(_), Float16 -> true
	| Float32V(_), Float32 -> true
	| Float64V(_), Float64 -> true
	| BoolV(_), Bool -> true
	| UnitV, Unit -> true
	| PointerV(subv), Pointer(v) ->
			synth_value_has_type subv v
	| ArrayV(subvals), Array(sty, dim) ->
			let validdim =
				match dim with
				| EmptyDimension -> true (* no assigned dim, so valid length *)
				| Dimension(dimvalue) ->
					match constant_dimension_size dimvalue with
					| Some(d) ->
							(List.length subvals) = d
					| None -> true (* true since dim is variable
					length.  *)
            in
			List.for_all subvals (fun v ->
				synth_value_has_type v sty
			) && validdim
	| StructV(name, tbl), Struct(tname) ->
			(String.compare name tname) = 0
	| FunV(n1), Fun(f, t) ->
            (* Need somekinda typemap passed to do this one.  *)
            raise (SpecException "Functio ntypechecking not supported currently.  ")
	| _, _ -> false

let synth_value_from_float t f =
	match t with
	| Float16 -> Float16V(f)
	| Float32 -> Float32V(f)
	| Float64 -> Float64V(f)
	| _ -> raise (SpecException "Unexpected non-float type")

let synth_value_from_int t i =
	let check_non_neg i =
		assert (i >= 0)
	in
	match t with
	| Int16 -> Int16V(i)
	| Int32 -> Int32V(i)
	| Int64 -> Int64V(i)
	| UInt16 -> check_non_neg i; UInt16V(i)
	| UInt32 -> check_non_neg i; UInt32V(i)
	| UInt64 -> check_non_neg i; UInt64V(i)
	| _ -> raise (SpecException "Unexpected non-int type")

let synth_value_from_bool t i =
	match t with
	| Bool -> BoolV(i)
	| _ -> raise (SpecException "Unexpected non-bool type")

let type_hash_table_to_string (type_hash: (string, synth_type) Hashtbl.t) =
	let keys = Hashtbl.keys type_hash in
    String.concat ~sep:", " (
	List.map
	(List.map keys (fun key -> (key, Hashtbl.find_exn type_hash key)))
		(fun (k, typ) -> k ^ ": " ^ (synth_type_to_string typ)))

let iospec_to_string (iospec: iospec) =
    "Livein: " ^ (String.concat ~sep:", " iospec.livein) ^
    "\nLiveout: " ^ (String.concat ~sep:", " iospec.liveout) ^
    "\nexeccmd: " ^ iospec.execcmd ^
	"\n"

let apispec_to_string (apispec: apispec) =
    "Livein: " ^ (String.concat ~sep:", " apispec.livein) ^
    "\nLiveout: " ^ (String.concat ~sep:", " apispec.liveout) ^
	"\n"

let get_class_typemap smeta =
	match smeta with
	| ClassMetadata(cls) -> cls.typemap
	| StructMetadata(str) -> str.typemap
let get_class_io_typemap smeta =
    match smeta with
    | ClassMetadata(cls) -> cls.io_typemap
    | StructMetadata(str) -> str.io_typemap
let get_class_members smeta =
	match smeta with
	| ClassMetadata(cls) -> cls.members @ cls.functions
	| StructMetadata(str) -> str.members
let get_class_fields smeta =
	match smeta with
	| ClassMetadata(cls) -> cls.members
	| StructMetadata(str) -> str.members
let is_class smeta =
	match smeta with
	| ClassMetadata(_) -> true
	| StructMetadata(_) -> false

let sort_members_by_type typemap members =
	(* This is like a shitty topology sort that isn't actually 
	a topology sort.  *)
	(* It puts the simple types first, then the more complex
	ones, and hopes you don't have any stupid shit around
	array length limits.  *)
	let member_priority_pairs = List.map members (fun mem ->
		(mem, match Hashtbl.find_exn typemap.variable_map mem with
		| Bool -> 0
		| Int16 -> 0
		| Int32 -> 0
		| Int64 -> 0
		| UInt16 -> 0
		| UInt32 -> 0
		| UInt64 -> 0
		| Float16 -> 0
		| Float32 -> 0
		| Float64 -> 0
		| Unit -> 0
		| Pointer(p) -> 1
		| Array(_, _) -> 2
		| Struct(s) -> 3
		| Fun(_, _) -> 4
		)
	) in
	let compare_func = fun (_, p) -> fun (_, p2) -> Int.compare p p2 in
	let sorted = List.sort member_priority_pairs compare_func in
	List.map sorted (fun (a, _) -> a)


let name_reference_list_equal n1 n2 =
	let zipped = List.zip n1 n2 in
	match zipped with
	| Ok(l) -> List.for_all l (fun (x, y) -> name_reference_equal x y)
	| Unequal_lengths -> false

let name_reference_list_list_equal n1 n2 = 
	let zipped = List.zip n1 n2 in
	match zipped with
	| Ok(l) -> List.for_all l (fun (x, y) -> name_reference_list_equal x y)
	| Unequal_lengths -> false

let name_reference_is_struct nr =
	match nr with
	| AnonymousName -> false
	| Name(_) -> false
	| StructName(_) -> true

let name_reference_base_name nr =
	match nr with
	| AnonymousName -> raise (SpecException "Can't get base name of anon")
	| Name(_) -> nr
	| StructName(x :: xs) -> x
	| StructName([]) -> raise (SpecException "can't have empty strucname")

let rec type_of_name_reference (typemap: typemap) nr =
	match nr with
	| Name(n) -> Hashtbl.find_exn typemap.variable_map n
	| StructName([]) -> raise (SpecException "No type for empty struct name")
	| StructName([Name(x)]) ->
			Hashtbl.find_exn typemap.variable_map x
	(* Assume well formatted names, or this will crash *)
	| StructName(Name(x) :: xs) ->
			let rec compute_struct_name typ = match typ with
			| Struct(n) -> n
			(* Both arrays and pointers are 'invisible' wrt. a
			   struct reference PoV *)
			| Pointer(subtype) -> compute_struct_name subtype
			| Array(st, dim) -> compute_struct_name st
			| other -> raise (SpecException "Through-struct reference with non-struct type. ")
			in
			let sname = compute_struct_name (Hashtbl.find_exn typemap.variable_map x) in
			let updated_typemap = {
				typemap with
				variable_map = (get_class_typemap (Hashtbl.find_exn typemap.classmap sname))
			} in
			type_of_name_reference updated_typemap (StructName(xs))
	| StructName(_) -> raise (SpecException ("Ill-formatted" ^ (name_reference_to_string nr)))
	| AnonymousName -> raise (SpecException "no type for anon name")

let type_option_of_name_reference typemap nr =
	try (Some(type_of_name_reference typemap nr)) with SpecException(i) -> None

let rec name_reference_cannonicalize ns =
	let rec flatten x =
		match x with
				| AnonymousName -> []
				| StructName(ns) -> ns
				| Name(x) -> [Name(x)]
	in
	match ns with
	| AnonymousName -> AnonymousName
	| Name(x) -> Name(x)
	| StructName(Name(x) :: xs) ->
			StructName(Name(x) :: (List.concat ((List.map xs (fun x -> flatten (name_reference_cannonicalize x))))))
	| StructName(StructName(ns) :: xs) ->
			(
			match name_reference_cannonicalize (StructName(ns)) with
			| StructName(ns_rest) ->
					let tail = List.concat (
						List.map xs (fun x ->
							flatten (name_reference_cannonicalize x)
						)
					) in
					StructName(ns_rest @ (tail))
			| _ -> assert false (* Expect a struct name to come back as a struct name. *)
			)
	| StructName(AnonymousName :: xs) -> assert false (* ill formatted *)
	| StructName([]) -> AnonymousName


let type_of_name_reference_list typemap ns =
	type_of_name_reference typemap (name_reference_cannonicalize (StructName(ns)))

let is_float_type typ =
	match typ with
	| Float16 -> true
	| Float32 -> true
	| Float64 -> true
	| _ -> false

let is_bool_type typ =
	match typ with
	| Bool -> true
	| _ -> false

let is_integer_type typ =
	match typ with
	| Int16 -> true
	| Int32 -> true
	| Int64 -> true
	| UInt16 -> true
	| UInt32 -> true
	| UInt64 -> true
	| _ -> false

(* Given a typename of the format x.y.z, determine the type
   of z.  Note taht this is a type specification string,
   not a reference string, so above, it's perfectly
   fine for y to be an array.  *)
let type_of typmap classmap k =
    let rec typeloop currtypmap tps = match tps with
    (* Think this isn't possible anyway.  *)
    | [] -> raise (SpecException "Cannae have empty type")
    | x :: y :: ys ->
            (* Get the next typmap *)
            let current_typ = Hashtbl.find_exn currtypmap x in
            let class_name = match current_typ with
			| Pointer(Struct(sname)) -> sname (* Treat pointers as invisible. *)
			(* Note -- that should probably support nested pointers (wtf would anyone want that? *)
			| Struct(sname) -> sname
			| other -> raise (SpecException ("Cannae have a class of type " ^ (synth_type_to_string other)))
			in
            let this_classmap = get_class_typemap (Hashtbl.find_exn classmap class_name) in
            typeloop this_classmap (y :: ys)
    | x :: xs -> 
            (* Just the type of x. *)
            Hashtbl.find_exn currtypmap x
    in
    typeloop typmap (String.split_on_chars k ['.'])

(*  Functions to make handling synth values a bit more scalable. *)
let float_from_value v =
	match v with
	| Float16V(v) -> Some(v)
	| Float32V(v) -> Some(v)
	| Float64V(v) -> Some(v)
	| _ -> None

let int_from_value v =
	match v with
	| Int16V(v) -> Some(v)
	| Int32V(v) -> Some(v)
	| Int64V(v) -> Some(v)
	| UInt16V(v) -> Some(v)
	| UInt32V(v) -> Some(v)
	| UInt64V(v) -> Some(v)
	| BoolV(v) -> if v then Some(1) else Some(0)
	| _ -> None

let bool_from_value v =
	let value = int_from_value v in
	match value with
	| Some(value_num) ->
		if value_num = 0 then Some(true) else Some(false)
	| None -> None

let bool_from_value v =
	match v with
	| BoolV(v) -> Some(v)
	| _ -> None

let array_from_value v =
    match v with
    | ArrayV(vs) -> Some(vs)
    | _ -> None

let is_float_value v =
	match v with
	| Float16V(_) -> true
	| Float32V(_) -> true
	| Float64V(_) -> true
	| _ -> false

let is_int_value v =
	match v with
	| Int16V(_) -> true
	| Int32V(_) -> true
	| Int64V(_) -> true
	| UInt16V(_) -> true
	| UInt32V(_) -> true
	| UInt64V(_) -> true
	| _ -> false

let is_bool_value v =
	match v with
	| BoolV(_) -> true
	| _ -> false

let is_array_value v =
	match v with
	| ArrayV(_) -> true
	| _ -> false

(* cast a value v to a type of t if possible *)
(* This is a very minimalistic casting engine, and
it doesn't need to be a full one right now.  However,
it should be noted that any more general casting
engine is likely to end up being target language
specific.  *)
let synth_value_cast v t =
	if is_float_type t then
		(* Only supporting float -> float casts right now.  *)
		Option.map (float_from_value v) (synth_value_from_float t)
	else if is_integer_type t then
		Option.map (int_from_value v) (synth_value_from_int t)
	else if is_bool_type t then
		Option.map (bool_from_value v) (synth_value_from_bool t)
	else
		(* Currently not supporting any other casting for
			internal simulation.  Would be easy to add more to emulate
			e.g. C-style casting.  *)
		if synth_value_has_type v t then
			Some(v)
		else
			None

let table_clone t =
	let newtbl = Hashtbl.create (module String) in
	let keys = Hashtbl.keys t in
	let _ = List.map keys (fun key ->
		Hashtbl.add newtbl key (Hashtbl.find_exn t key)
	) in
	newtbl

(* Typemap utils.  *)
let clone_variablemap (t: (string, synth_type) Hashtbl.t) =
    table_clone t

let clone_classmap (t: (string, structure_metadata) Hashtbl.t) =
    table_clone t

let clone_valuemap (t: (string, synth_value) Hashtbl.t) =
	table_clone t

let clone_alignment_map (t: (string, int) Hashtbl.t) =
    table_clone t

let rec clone_typemap (t: typemap) =
    {
        variable_map = clone_variablemap t.variable_map;
        classmap = clone_classmap t.classmap;
        alignment_map = clone_alignment_map t.alignment_map;
        original_typemap =
            Option.map t.original_typemap clone_typemap
    }

let merge_maps (t1: (string, 'a) Hashtbl.t) (t2: (string, 'a) Hashtbl.t) =
	let newtbl = Hashtbl.create (module String) in
	let k1 = Hashtbl.keys t1 in
	let k2 = Hashtbl.keys t2 in

	let _ = List.map k1 (fun key ->
		Hashtbl.add newtbl key (Hashtbl.find_exn t1 key)
	) in
	let _ = List.map k2 (fun key ->
		let result = Hashtbl.add newtbl key (Hashtbl.find_exn t2 key) in
		match result with
		| `Ok -> ()
		| `Duplicate -> raise (SpecException "Can't merge tables with duplicate keys")
	) in
	newtbl

let clear_map (t: ('a, 'b) Hashtbl.t) =
	let keys = Hashtbl.keys t in
	let _ = List.map keys (fun key ->
		Hashtbl.remove t key
	) in
	()

let typemap_to_string typemap =
	(* TODO -- expand to the other components.  *)
	String.concat ~sep:"\n" (List.map (Hashtbl.keys typemap.variable_map) (fun key ->
		let typ = Hashtbl.find_exn typemap.variable_map key in
		key ^ ": " ^ (synth_type_to_string typ)
	)
	)
