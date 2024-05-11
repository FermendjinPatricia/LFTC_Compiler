run.exe: main.c ALEX/lexer.c UTILS/utils.c ASIN/parser.c AD/ad.c
	gcc -Wall -o run.exe main.c ALEX/lexer.c UTILS/utils.c ASIN/parser.c AD/ad.c
	
run: run.exe 
	./run.exe testad.c 
