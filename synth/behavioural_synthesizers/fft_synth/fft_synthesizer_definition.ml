open Core_kernel;;
open Spec_definition;;

type fs_structure =
    | FSConditional of fs_structure * fs_condition
    | FSArrayOp of fs_array_operator * fs_variable
    | FSSeq of fs_structure list
    | FSStructureHole

and fs_condition =
    | FSGreaterThan of fs_variable * fs_variable
    | FSLessThan of fs_variable * fs_variable
    | FSPowerOfTwo of fs_variable
    | FSConditionalHole

and fs_variable =
    | FSScalarVariableHole
    | FSArrayVariableHole
    | FSIntVariableHole
    | FSFloatVariableHole
    | FSIntConstantHole
    | FSFloatConstantHole
    | FSVariable of name_reference
    | FSConstant of synth_value

and fs_array_operator =
    | FSBitReversal
    | FSNormalize
	| FSHalfNormalize
    | FSDenormalize
	| FSHalfDenormalize
    | FSArrayOpHole
