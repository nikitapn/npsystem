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
[0-9]+ { 
		loc->step(); 
		loc->columns(static_cast<int>(yyleng)); 
		yylval->val_int = std::atoi(yytext); 
		return tok::NUMBER_INTEGER; 
	} 
[0-9]*[.]?[0-9]+  { 
		loc->step(); 
		loc->columns(static_cast<int>(yyleng)); 
		yylval->val_float = static_cast<float>(std::atof(yytext)); 
		return tok::NUMBER_FLOAT; 
	}
[\(\)\+\-\*\/\>\<\,\;]	{ loc->step(); loc->columns(); return *yytext; }
"<="     	{ loc->step(); loc->columns(2); return tok::LE; }
">="     	{ loc->step(); loc->columns(2); return tok::GE; }
":="     	{ loc->step(); loc->columns(2); return tok::ASSIGNMENT; }
"IF"     	{ loc->step(); loc->columns(2); return tok::IF; }
"THEN"   	{ loc->step(); loc->columns(4); return tok::THEN; }
"ELSIF"  	{ loc->step(); loc->columns(5); return tok::ELSIF; }
"ELSE"  	{ loc->step(); loc->columns(4); return tok::ELSE; }
"END_IF" 	{ loc->step(); loc->columns(6); return tok::END_IF; }
"var"    	{ loc->step(); loc->columns(3); return tok::VAR; }
"//".* 		{ loc->lines(); }

[\/][A-Z_]+[0-9_]?[\/][A-Z_]+[A-Z0-9_]?[\.][A-Z]+ { 
		loc->step();
		loc->columns(static_cast<int>(yyleng)); 
		yylval->val_str = { yytext, yyleng }; 
		return tok::IDENTIFIER; 
	}
[\'][A-Z_]+[A-Z0-9_]?[\.][A-Z]+ { 
		loc->step();
		loc->columns(static_cast<int>(yyleng)); 
		yylval->val_str = { yytext, yyleng }; 
		return tok::IDENTIFIER; 
	}
[_a-zA-Z]+[0-9]? { 
		loc->step();
		loc->columns(static_cast<int>(yyleng)); 
		yylval->val_str = { yytext, yyleng }; 
		return tok::IDENTIFIER; 
	}
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
