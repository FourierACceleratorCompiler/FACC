{
    open Rangeparse
}

let white = [' ']+
(* TODO --- would like to support chars etc. also. *)
let real = '-'?['0'-'9']+'.'['0'-'9']*
let integer = '-'?['0'-'9']+
let nat = ['0'-'9']+
let boolean = "true"|"false"

rule read =
    parse
        | white {read lexbuf}
        | "range" {RANGE}
        | "set" {SET}
		| "power_of_two" { POWER_OF_TWO }
		| "[" {L_SQ_BRACKET}
		| "]" {R_SQ_BRACKET}
        | "(" {LPAREN}
        | ")" {RPAREN}
        | "," {COMMA}
        | real as r { REAL (float_of_string r) }
		| nat as i { NAT(int_of_string i) }
        | integer as i { INTEGER (int_of_string i) }
		| boolean as b { BOOLEAN (bool_of_string b) }
        | eof { EOF }
