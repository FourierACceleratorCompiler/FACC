%{
    open Core_kernel
	open Range_definition
    open Range
%}

%token RANGE
%token COMMA
%token SET
%token LPAREN
%token RPAREN
%token L_SQ_BRACKET
%token R_SQ_BRACKET
%token <float> REAL
%token <int> INTEGER
%token <int> NAT
%token <bool> BOOLEAN
%token POWER_OF_TWO
%token EOF

%start t
%type <Range_definition.sugared_range_set> t

%%

t:
	| SET LPAREN set_contents RPAREN EOF { SugaredRangeSet(Array.of_list $3) }

set_contents:
	| set_item { [$1] }
	| set_item; COMMA; set_contents { $1 :: $3 }

set_item:
	| RANGE LPAREN item COMMA item RPAREN { SugaredRangeRange($3, $5) }
    | POWER_OF_TWO LPAREN NAT COMMA NAT RPAREN { SugaredRangeFunction(SugaredRangePowerOfTwo($3, $5)) }
	| item { SugaredRangeItem($1) }

item_list:
	| item; COMMA; item_list { $1 :: $3 }
	| item { [$1] }

item:
	| INTEGER { SugaredRangeInteger($1) }
	| NAT { SugaredRangeInteger($1) }
	| REAL { SugaredRangeFloat($1) }
	| BOOLEAN { SugaredRangeBool($1) }
	| L_SQ_BRACKET; item_list; R_SQ_BRACKET { SugaredRangeArray(sugared_range_type_value (List.hd_exn $2), $2) }
