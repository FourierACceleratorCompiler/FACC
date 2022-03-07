open Core_kernel;;
open Range_definition;;

exception ParseException of string

(* We need to have recursive types to store the name mappings,
   since that's what the type inputs that we take can be.  *)
type name_reference =
	| AnonymousName
	| Name of string
	(* To represent class names and member variables.  This
	   DOES NOT mean the list of all members of a class, but
	   rather the list of member names you have to traverse
	   to get to the member. *)
	| StructName of name_reference list

(* This is the way that the type stream is passed in.  *)
type synth_type =
	| Bool
	| Int16 | Int32 | Int64
	| UInt16 | UInt32 | UInt64
	| Float16 | Float32 | Float64
	| Pointer of synth_type
	| Array of synth_type * dimension_type
	| Unit
	| Struct of string
	| Fun of (synth_type list) * synth_type
and dimension_type =
	(* For each array variable, this keeps the
	   /prospective/ dimensions that it could have. *)
	(* This should be assigned in the assign_dimensions pass##
	or can be specified by the .  *)
    | EmptyDimension
	| Dimension of dimension_value
and dimension_value =
	| DimVariable of name_reference * dimension_relation
	| DimConstant of int
and dimension_relation =
	| DimEqualityRelation
	| DimPo2Relation
	| DimDivByRelation of int

type synth_value =
	| BoolV of bool
    | Int16V of int
    | Int32V of int
    | Int64V of int
	| UInt16V of int
	| UInt32V of int
	| UInt64V of int
    | Float16V of float
    | Float32V of float
    | Float64V of float
    | UnitV
	| PointerV of synth_value
    | ArrayV of synth_value list
    | StructV of string * (string, synth_value) Hashtbl.t
    | FunV of string

type classtype = {
	members: string list;
	functions: string list;
    typemap: (string, synth_type) Hashtbl.t;
    io_typemap: ((string, synth_type) Hashtbl.t);
}

type structtype = {
	members: string list;
    typemap: (string, synth_type) Hashtbl.t;
    io_typemap: ((string, synth_type) Hashtbl.t);
}

type structure_metadata =
	| ClassMetadata of classtype
	| StructMetadata of structtype

type typemap = {
	variable_map: (string, synth_type) Hashtbl.t;
	classmap: (string, structure_metadata) Hashtbl.t;
	alignment_map: (string, int) Hashtbl.t;
	original_typemap: typemap option
}

type iospec = {
	funname: string;
	livein: string list;
	liveout: string list;
	(* This should be an element of liveout, or empty.  *)
	(* This of course does not reflex 'real' C --- we don't
	try to do that here --- the compiler framework can
	easily change this to a traditional return variable
	and it means we don't have to deal with return types
	in the JSON formats.  We do perhaps need a pointer
	type to implement this correctly, but that is a future
	challenge.  *)
    returnvar: string list;
    funargs: string list;
	execcmd: string;
	required_includes: string list;
	(* Note that value profiles don't have to specify every
	value --- the backend generator can do the rest.  *)
	value_profiles: ((string, synth_value) Hashtbl.t) list;
	rangemap: (string, range_set) Hashtbl.t;
	validmap: (string, range_set) Hashtbl.t;
	(* Which consts appear in the user code -- indexed
	by type --- used as a guide to generate consts.  *)
	constmap: (string, synth_value list) Hashtbl.t;
	compiler_flags: string list;
}

type apispec = {
    livein: string list;
    liveout: string list;
	funname: string;
	funargs: string list;
	required_includes: string list;
    compiler_flags: string list;
	validmap: (string, range_set) Hashtbl.t;
}
