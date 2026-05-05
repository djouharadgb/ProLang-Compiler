flex prolang.l
bison -d Syn.y
gcc lex.yy.c Syn.tab.c ts.c quad.c opt.c asm8086.c -lfl -ly -o prolang.exe
.\prolang.exe <erreur.txt