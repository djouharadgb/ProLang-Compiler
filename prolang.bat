flex prolang.l
bison -d Syn.y
gcc lex.yy.c Syn.tab.c ts.c quad.c opt.c -lfl -ly -o prolang.exe
.\prolang.exe <Testboucles.txt