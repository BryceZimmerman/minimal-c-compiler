GRAMMAR = parser.y

CFLAGS = -I. -funsigned-char -g -DYYDEBUG 	
YFLAGS = -v -d

mcc: y.tab.o lex.yy.o tree.o driver.o errors.o symtable.o cgen.o
	gcc $(CFLAGS) -o mcc driver.o y.tab.o lex.yy.o tree.o errors.o symtable.o cgen.o -ll

y.tab.o: y.tab.c y.tab.h 
	gcc $(CFLAGS) -c y.tab.c 

y.tab.c: $(GRAMMAR)
	yacc $(YFLAGS) $(GRAMMAR)

lex.yy.o: lex.yy.c y.tab.h 
	gcc $(CFLAGS) -c lex.yy.c

lex.yy.c: scanner.l
	lex scanner.l

tree.o: tree.c tree.h
	gcc $(CFLAGS) -c tree.c

errors.o: errors.c errors.h
	gcc $(CFLAGS) -c errors.c

symtable.o: symtable.c symtable.h
	gcc $(CFLAGS) -c symtable.c

cgen.o: cgen.c cgen.h
	gcc $(CFLAGS) -c cgen.c

driver.o: driver.c
	gcc  $(CFLAGS) -c driver.c

clean:
	rm -f y.tab.* y.output lex.yy.* *.o *~ mcc     



