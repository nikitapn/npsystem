%{
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include "lang.tab.hh"

#define YY_DECL int yylex(yy::parser::semantic_type* yylval, yy::parser::location_type* loc)

using tok = yy::parser::token;

#ifdef _MSC_VER
	#pragma warning(disable: 4267)
  #define YY_NO_UNISTD_H 1
  #define fileno _fileno
  int isatty(int fd) { fd; return 0; }
#endif
%}

%option noyywrap
												
%%

"\n" 		{ loc->lines(); }
"\r\n" 	{ loc->lines(); }
"\t" 		{ loc->columns(2); }
" "  		{ loc->columns(); }

"TRUE" { 
		loc->step(); 
		loc->columns(4); 
		yylval->val_discrete = true; 
		return tok::NUMBER_DISCRETE; 
	}
"FALSE" { 
		loc->step(); 
		loc->columns(5); 
		yylval->val_discrete = false; 
		return tok::NUMBER_DISCRETE; 
	}

"<="     	{ loc->step(); loc->columns(2); return tok::LE; }
">="     	{ loc->step(); loc->columns(2); return tok::GE; }
":="     	{ loc->step(); loc->columns(2); return tok::ASSIGNMENT; }
"IF"     	{ loc->step(); loc->columns(2); return tok::IF; }
"THEN"   	{ loc->step(); loc->columns(4); return tok::THEN; }
"ELSIF"  	{ loc->step(); loc->columns(5); return tok::ELSIF; }
"ELSE"  	{ loc->step(); loc->columns(4); return tok::ELSE; }
"END_IF" 	{ loc->step(); loc->columns(6); return tok::END_IF; }
"PROGRAM"    	{ loc->step(); loc->columns(7); return tok::PROGRAM; }
"END_PROGRAM" { loc->step(); loc->columns(11); return tok::END_PROGRAM; }
"FUNCTION"    	{ loc->step(); loc->columns(8); return tok::FUNCTION; }
"END_FUNCTION" { loc->step(); loc->columns(12); return tok::END_FUNCTION; }
"FUNCTION_BLOCK"    	{ loc->step(); loc->columns(14); return tok::FUNCTION_BLOCK; }
"END_FUNCTION_BLOCK" { loc->step(); loc->columns(18); return tok::END_FUNCTION_BLOCK; }
"VAR"    	{ loc->step(); loc->columns(3); return tok::VAR; }
"VAR_GLOBAL" { loc->step(); loc->columns(10); return tok::VAR_GLOBAL; }
"END_VAR" { loc->step(); loc->columns(7); return tok::END_VAR; }
"//".* 		{ loc->lines(); }
"BOOL" 		{ loc->step(); loc->columns(4); return tok::NPT_BOOL; }
"I8" 			{ loc->step(); loc->columns(2); return tok::NPT_I8; }
"U8"  		{ loc->step(); loc->columns(2); return tok::NPT_U8; }
"I16" 		{ loc->step(); loc->columns(3); return tok::NPT_I16; }
"U16" 		{ loc->step(); loc->columns(3); return tok::NPT_U16; } 
"I32" 		{ loc->step(); loc->columns(3); return tok::NPT_I32; } 
"U32" 		{ loc->step(); loc->columns(3); return tok::NPT_U32; }
"F32" 		{ loc->step(); loc->columns(3); return tok::NPT_F32; }


[0-9]+ { 
		loc->step(); 
		loc->columns(static_cast<int>(yyleng)); 
		yylval->val_str = { yytext, yyleng }; 
		return tok::NUMBER_INTEGER; 
	}

[0-9]*[.][0-9]+  { 
		loc->step(); 
		loc->columns(static_cast<int>(yyleng)); 
		yylval->val_float = static_cast<float>(std::atof(yytext)); 
		return tok::NUMBER_FLOAT; 
	}

[\/](.+)[\/](.+)[\/](.+)[\.][A-Z]+ { 
	// to do: correct lenght for utf-8 string
		loc->step();
		loc->columns(static_cast<int>(yyleng)); 
		yylval->val_str = { yytext, yyleng }; 
		return tok::EXTERNAL_IDENTIFIER; 
	}

[_a-zA-Z]+[0-9]? { 
		loc->step();
		loc->columns(static_cast<int>(yyleng)); 
		yylval->val_str = { yytext, yyleng }; 
		return tok::IDENTIFIER; 
	}

[\(\)\+\-\*\/\>\<\,\;\:]	{ loc->step(); loc->columns(); return *yytext; }

.	{ 
		loc->step();
		loc->columns(static_cast<int>(yyleng)); 
		std::cerr << "unexpected character: " << yytext << '\n' << *loc << std::endl;	
		exit(-1); 
	}
%%

namespace yy {
void set_buffer(char* buffer, size_t size) {
  yy_scan_buffer(buffer, size);
}
}