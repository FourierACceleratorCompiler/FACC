%{
	open Spec_definition
%}

%token BOOL
%token INT16
%token INT32
%token INT64
%token UINT16
%token UINT32
%token UINT64
%token FLOAT16
%token FLOAT32
%token FLOAT64
%token ARRAY
%token POINTER
%token UNIT
%token LPAREN
%token RPAREN
%token ARROW
%token HASH
%token DOT
%token EOF
%token COMMA
%token <string> IDENT
%token <int> INTEGER

%left ARROW

%start  t
%type <Spec_definition.synth_type> t

%%

t:
 | tlist; ARROW; t {Fun($1, $3)}
 | tsub; EOF {$1};

tlist:
 | tsub; { [$1] };
 | tsub; COMMA; tlist { $1 :: $3 };

tident:
 | IDENT { Name($1) }
 | IDENT DOT tident {
     match $3 with
     | StructName(ns) -> StructName(Name($1) :: ns)
     | Name(n) -> StructName([Name($1); Name(n)])
     | AnonymousName -> Name($1)
 }

tsub:
 | BOOL { Bool }
 | INT16 { Int16 }
 | INT32 { Int32 }
 | INT64 { Int64 }
 | UINT16 { UInt16 }
 | UINT32 { UInt32 }
 | UINT64 { UInt64 }
 | FLOAT16 { Float16 }
 | FLOAT32 { Float32 }
 | FLOAT64 { Float64 }
 | UNIT { Unit }
 | ARRAY; LPAREN; tsub; RPAREN { Array($3, EmptyDimension) };
 | ARRAY; LPAREN; tsub; HASH; INTEGER; RPAREN {  Array($3, Dimension(DimConstant($5))) };
 | ARRAY; LPAREN; tsub; HASH; tident; RPAREN { Array($3, Dimension(DimVariable($5, DimEqualityRelation))) }
 | POINTER; LPAREN; tsub; RPAREN { Pointer($3) };
 | IDENT {Struct($1)}
