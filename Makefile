
all: bison flex gcc
	@echo "Done."

bison: src/parser.y
	bison src/parser.y 

flex: src/scanner.l
	flex src/scanner.l

gcc: scanner.c parser.c src/tables.c src/types.c src/ast.c src/code.c
	gcc -w -o trabalho scanner.c parser.c src/tables.c src/types.c src/ast.c src/code.c

tm: src/tm.c src/tables.c src/types.c
	gcc -w -o tmsim src/tm.c src/tables.c src/types.c

clean:
	@rm -f *.o *.output scanner.c parser.h parser.c trabalho *.dot *.pdf tmsim

