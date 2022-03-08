%language "c++"
 //%lex-param { type1 arg1 }
 //%defines
 //%define api.token.constructor
 //%define api.value.type variant
 //%define parse.assert
 %locations

%parse-param  {  npcompiler::ast::ParserContext& ctx_ } { npcompiler::ast::AstNode& root_ }
												
%code requires // *.hh
{
#include "../ast.hpp"

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

	#define mkn(...) new AstNode(__VA_ARGS__)
	#define msv(x) std::string_view(x.ptr, x.len)
}


%token IDENTIFIER GE LE ASSIGNMENT IF THEN ELSIF ELSE END_IF VAR
%token NUMBER_DISCRETE NUMBER_INTEGER NUMBER_FLOAT

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

%type	<val_str>					IDENTIFIER
%type	<val_discrete>		NUMBER_DISCRETE
%type	<val_int>					NUMBER_INTEGER
%type	<val_float>				NUMBER_FLOAT
%type <val_node>				stmt stmt_list var_decl var_decl0 var_decl_seq if_stmt exp number assignment


%%
%start script;

script:
		stmt_list { root_.push($1); }
;

stmt_list:
		%empty { $$ = mkn(non_term, AstType::StmtList); }
	|	stmt_list stmt { $1->push($2); $$ = $1; }
;

stmt:
	  assignment { $$ = $1; }
	|	var_decl { $$ = $1; }
	| if_stmt { $$ = $1; }
	| ';' { $$ = nullptr; }
;

assignment:
	IDENTIFIER ASSIGNMENT exp ';' { /*$$ = mkn(non_term, AstType::Assignment, $1, $3);*/ }
;

var_decl0: 
		IDENTIFIER { 
			auto ident = Identifier0{IdentType::Variable, msv($1), false};
			ctx_.ident_put(static_cast<Identifier&>(ident)); 
			$$ =  mkn(std::move(ident)); 
		}
	|	IDENTIFIER ASSIGNMENT exp { 
			auto ident = Identifier0{IdentType::Variable, msv($1), false};
			ctx_.ident_put(static_cast<Identifier&>(ident)); 
			$$ = mkn(non_term, AstType::Assignment, mkn(std::move(ident)), $3); 
		}
;

var_decl_seq:
		var_decl0 { $$ = mkn(non_term, AstType::VarDeclSeq, $1); }
	|	var_decl_seq ',' var_decl0 { $1->push($3); $$ = $1; }
;

var_decl:
		VAR var_decl_seq ';' { $$ = $2; }
;

if_stmt:
		IF logical_exp THEN stmt_list else_clause { $$ = mkn(non_term, AstType::If); }
;

else_clause:
		ELSIF logical_exp THEN stmt_list else_clause
	|	ELSE stmt_list END_IF
	|	END_IF
;

number:
		NUMBER_DISCRETE { std::cerr << "NUMBER_DISCRETE not impl\n"; }	
	|	NUMBER_INTEGER { $$ = mkn($1); }
	|	NUMBER_FLOAT { $$ = mkn($1); }
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
	| '-' exp 		{ $$ = mkn(non_term, AstType::Uminus, $2); }
	| '+' exp 		{ $$ = $2; }
	| exp '+' exp { $$ = mkn(non_term, AstType::Add, $1, $3); }
	| exp '-' exp { $$ = mkn(non_term, AstType::Add, $1, mkn(non_term, AstType::Uminus, $3)); }
	|	exp '*' exp { $$ = mkn(non_term, AstType::Mul, $1, $3); }
	|	exp '/' exp { $$ = mkn(non_term, AstType::Div, $1, $3); }
	|	number { $$ = $1; }
	| IDENTIFIER { $$ = mkn(ctx_.ident_get(msv($1))); }
;

%%

namespace yy {
  // Report an error to the user.
  void parser::error (const location_type& loc, const std::string& msg) {
    std::cerr << '[' << loc << "] " << msg << '\n';
  }
}

