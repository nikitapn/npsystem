#calc: lex.yy.cc lang.tab.hh lang.tab.cc ast.hpp
#	g++ -std=c++17 $^ -o $(TARGET)

all: flex-bison

flex-bison: lexer parser

parser: src/generated/lang.tab.hh src/generated/lang.tab.cc

src/generated/lang.tab.hh generated/lang.tab.cc: src/lang.y
	bison -r all -t -Wempty-rule -d -v -b src/generated/lang $^

lexer: src/generated/lex.yy.cc

gdir:
	mkdir -p src/generated

src/generated/lex.yy.cc: src/lang.l | gdir
	flex -t $^ | sed s/"#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L"/"#if 1"/ > $@

clean:
	rm -f calc

veryclean: clean
	rm -f src/generated/*

.PHONY: lexer parser flex-bison clean veryclean