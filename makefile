run.exe: main.c ALEX/lexer.c UTILS/utils.c ASIN/parser.c AD/ad.c AT/at.c
	gcc -Wall -o run.exe main.c ALEX/lexer.c UTILS/utils.c ASIN/parser.c AD/ad.c AT/at.c
	
run: run.exe 
	./run.exe testat.c 
