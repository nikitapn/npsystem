%language "c++"
 //%lex-param { type1 arg1 }
 //%defines
 //%define api.token.constructor
 //%define api.value.type variant
 //%define parse.assert
 %locations

%parse-param  {  npcompiler::ast::ParserContext& ctx_ } { npcompiler::ast::AstNode& root_ }

// needs bison >= 3.5
// %define parse.lac full 
%define parse.error verbose

%code requires // *.hh
{
#include "../number.hpp"
#include "../ast.hpp"
#include "../utils.hpp"

namespace yy {
void set_buffer(char* buf, size_t size);	
}

}

%code // *.cpp
{
	#include <iostream>
  #include <fstream>
  #include <string_view>
  
	extern int yylex(yy::parser::semantic_type* yylval, yy::parser::location_type* loc);

	using namespace npcompiler::ast;
	using namespace npcompiler;

	#define mkn(...) new AstNode(__VA_ARGS__)
	#define msv(x) std::string_view(x.ptr, x.len)
}


%token PROGRAM END_PROGRAM FUNCTION END_FUNCTION FUNCTION_BLOCK END_FUNCTION_BLOCK VAR VAR_GLOBAL END_VAR
%token IDENTIFIER EXTERNAL_IDENTIFIER
%token GE LE ASSIGNMENT IF THEN ELSIF ELSE END_IF
%token NUMBER_DISCRETE NUMBER_INTEGER NUMBER_FLOAT
%token NPT_I8 NPT_U8 NPT_I16 NPT_U16 NPT_I32 NPT_U32 NPT_F32 NPT_BOOL

%left ','
%left	'+' '-'
%left	'*' '/'
%left '('

%right ASSIGNMENT

%union {
	struct {	
		char* ptr;
		int len;
	} val_str;
	bool val_discrete;
	int val_int;
	float val_float;
	npcompiler::ast::AstNode* val_node;
}												

%type	<val_str>					NUMBER_INTEGER IDENTIFIER EXTERNAL_IDENTIFIER 
%type	<val_discrete>		NUMBER_DISCRETE
%type	<val_int>					var_type
%type	<val_float>				NUMBER_FLOAT
%type <val_node>				stmt stmt_list var_decl0 var_decl_seq var_decl_seq_global exp ident
												number assignment module module0 function function0 prog_decl var_decl_local var_decl_global if_stmt


%%
%start module;

module:
		module0 { root_.push($1); $$ = &root_; }
	| module module0 { root_.push($2); $$ = $1; }
;

module0:
		PROGRAM IDENTIFIER prog_decl stmt_list end_program { 
			$$ = $3;
			$3->push($4, mkn(AstType::Program_End)); 
			}
	|	FUNCTION IDENTIFIER function stmt_list end_function { $$ = $3, $3->push($4, mkn(AstType::Function_End)); }
	| FUNCTION_BLOCK IDENTIFIER function end_function_block { $$ = $3; }
;

end_program:
	END_PROGRAM { ctx_.symbols_local.clear(); }
;

end_function:
	END_FUNCTION { ctx_.symbols_local.clear(); }
;

end_function_block:
	END_FUNCTION_BLOCK { ctx_.symbols_local.clear(); }
;

function:
		function0 { $$ = mkn(AstType::Function, $1); }
	|	function function0 { $1->push($2); $$ = $1; }
;

function0:
		var_decl_local { $$ = $1; }
;

stmt_list:
		%empty { $$ = mkn(AstType::StmtList); }
	|	stmt_list stmt { $1->push($2); $$ = $1; }
;

stmt:
	 assignment { $$ = $1; }
	| if_stmt { $$ = $1; }
;

semicolon:
	error { ctx_.error = true; std::cerr << "missing semicolon after the statement.\n"; } | ';'
;

semicolon1:
	error { ctx_.error = true; std::cerr << "missing semicolon after the var declaration.\n"; } | ';'
;

ident:
	 IDENTIFIER {$$ = ctx_.ident_get(msv($1));}
	| EXTERNAL_IDENTIFIER {$$ = ctx_.ext_ident_get(msv($1));}
;

assignment:
	ident ASSIGNMENT exp semicolon { $$ = ctx_.create_assignment($1, $3); }
;

var_type: 
	 	NPT_BOOL 	{ $$ = npsys::nptype::NPT_BOOL; }
	|	NPT_U8	 		{ $$ = npsys::nptype::NPT_U8; }
	|	NPT_I8			{ $$ = npsys::nptype::NPT_I8; }
	|	NPT_U16 		{ $$ = npsys::nptype::NPT_U16; }
	|	NPT_I16 		{ $$ = npsys::nptype::NPT_I16; }
	|	NPT_U32 		{ $$ = npsys::nptype::NPT_U32; }
	|	NPT_I32 		{ $$ = npsys::nptype::NPT_I32; }
	| NPT_F32 	{ $$ = npsys::nptype::NPT_F32; }
;

prog_decl:
		%empty { $$ = mkn(AstType::Program); }
	| prog_decl var_decl_local { $$ = $1; $1->push($2); }
	| prog_decl var_decl_global { $$ = $1; $1->push($2); }
;

var_decl_local:
		VAR { ctx_.global = false; } var_decl_seq END_VAR { $$ = $3; }
;

var_decl_seq:
		%empty { $$ = mkn(AstType::LocalVarDeclSeq); }
	|	var_decl_seq var_decl0 { $1->push($2); $$ = $1; }
;

var_decl_global:
		VAR_GLOBAL { ctx_.global = true; } var_decl_seq_global END_VAR { $$ = $3; }
;

var_decl_seq_global:
		%empty { $$ = mkn(AstType::GlobalVarDeclSeq); }
	|	var_decl_seq_global var_decl0 { $1->push($2); $$ = $1; }
;

var_decl0: 
		IDENTIFIER ':' var_type semicolon1 { $$ = ctx_.ident_create(msv($1), $3); }
;

if_stmt:
		IF logical_exp THEN stmt_list else_clause { $$ = mkn(AstType::If); }
;

else_clause:
		ELSIF logical_exp THEN stmt_list else_clause
	|	ELSE stmt_list END_IF
	|	END_IF
;

number:
		NUMBER_DISCRETE 	{ std::cerr << "NUMBER_DISCRETE not impl\n"; }	
	|	NUMBER_INTEGER 		{ $$ = mkn(utils::parse_integer(msv($1))); }
	|	NUMBER_FLOAT 			{ $$ = mkn($1); }
;


logical_exp:
		exp
	| exp '<' exp
	| exp LE exp
	| exp '>' exp
	| exp GE
;


exp:
	  '(' exp ')' { $$ = $2; }
	| '-' exp 		{ $$ = mkn(AstType::Uminus, $2); }
	| '+' exp 		{ $$ = $2; }
	| exp '+' exp { $$ = ctx_.create_binary_op(AstType::Add, $1, $3); }
	| exp '-' exp { $$ = ctx_.create_binary_op(AstType::Add, $1, mkn(AstType::Uminus, $3)); }
	|	exp '*' exp { $$ = ctx_.create_binary_op(AstType::Mul, $1, $3); }
	|	exp '/' exp { $$ = ctx_.create_binary_op(AstType::Div, $1, $3); }
	|	number 			{ $$ = $1; }
	| ident 			{ $$ = $1; }
;

%%

namespace yy {
  // Report an error to the user.
  void parser::error (const location_type& loc, const std::string& msg) {
    std::cerr << '[' << loc << "] " << msg << '\n';
  }
}

