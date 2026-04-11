flex prolang.l
bison -d Syn.y
gcc lex.yy.c Syn.tab.c ts.c quad.c -lfl -ly -o prolang.exe
.\prolang.exe <testCode.txt