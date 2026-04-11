%{
    #include <stdio.h>
    #include <string.h>
    #include "ts.h"
    #include "quad.h"
    #include "Syn.tab.h"

    int nb_ligne = 1;
    int nb_col   = 1;
%}

letter      [A-Za-z]
digit       [0-9]
int_num     (0|[1-9][0-9]*)
float_num   (0|[1-9][0-9]*)"."[0-9]+
strings     \"[^\"]*\"
IDF         {letter}([a-z0-9]|"_"[a-z0-9])*
espace      [ \t]+
saut_ligne  \n

%x COMMENT

%%

"//*"                  { BEGIN(COMMENT); }
<COMMENT>"*//"         { BEGIN(INITIAL); }
<COMMENT>{saut_ligne}  { nb_ligne++; nb_col = 1; }
<COMMENT>.             { /* ignorer le contenu du commentaire */ }

"%%"[^\n]*            { nb_col += yyleng; /* commentaire mono-ligne */ }

"BeginProject"      { nb_col += yyleng; return BeginProject; }
"EndProject"        { nb_col += yyleng; return EndProject;   }
"Setup"             { nb_col += yyleng; return Setup;        }
"Run"               { nb_col += yyleng; return Run;          }
"define"            { nb_col += yyleng; return DEFINE_MC;    }
"const"             { nb_col += yyleng; return CONST_MC;     }
"integer"           { nb_col += yyleng; return INTEGER_MC;   }
"float"             { nb_col += yyleng; return FLOAT_MC;     }
"if"                { nb_col += yyleng; return IF_MC;        }
"then"              { nb_col += yyleng; return THEN_MC;      }
"else"              { nb_col += yyleng; return ELSE_MC;      }
"endIf"             { nb_col += yyleng; return ENDIF_MC;     }
"loop"              { nb_col += yyleng; return LOOP_MC;      }
"while"             { nb_col += yyleng; return WHILE_MC;     }
"endloop"           { nb_col += yyleng; return ENDLOOP_MC;   }
"for"               { nb_col += yyleng; return FOR_MC;       }
"input"             { nb_col += yyleng; return INPUT_MC;     }
"to"                { nb_col += yyleng; return TO_MC;        }
"endfor"            { nb_col += yyleng; return ENDFOR_MC;    }
"in"                { nb_col += yyleng; return IN_MC;        }
"out"               { nb_col += yyleng; return OUT_MC;       }
"AND"               { nb_col += yyleng; return AND;          }
"OR"                { nb_col += yyleng; return OR;           }
"NON"               { nb_col += yyleng; return NON;          }

{IDF}               {
                        nb_col += yyleng;
                        if (yyleng > 14) {
                            printf("ERREUR lexicale: identificateur '%s' depasse 14 caracteres, ligne %d, col %d\n",
                                   yytext, nb_ligne, nb_col - yyleng);
                        } else {
                            yylval.sval = strdup(yytext);
                            return IDF;
                        }
                    }
{float_num}         {
                        nb_col += yyleng;
                        yylval.fval = atof(yytext);
                        return NUM_FLOAT;
                    }
{int_num}           {
                        nb_col += yyleng;
                        yylval.ival = atoi(yytext);
                        if (yylval.ival < -32768 || yylval.ival > 32767) {
                            printf("ERREUR lexicale: entier '%s' hors limites [-32768, 32767], ligne %d, col %d\n",
                                   yytext, nb_ligne, nb_col - yyleng);
                        }
                        return NUM_INT;
                    }
{strings}           {
                        nb_col += yyleng;
                        yylval.sval = strdup(yytext);
                        return STRING;
                    }

"=="                { nb_col += yyleng; return OP_EQ;     }
"!="                { nb_col += yyleng; return OP_NE;     }
">="                { nb_col += yyleng; return OP_GE;     }
"<="                { nb_col += yyleng; return OP_LE;     }
"<-"                { nb_col += yyleng; return OP_ASSIGN;  }
">"                 { nb_col += yyleng; return OP_GT;     }
"<"                 { nb_col += yyleng; return OP_LT;     }

"="                 { nb_col += yyleng; return OP_INIT;   }
"+"                 { nb_col += yyleng; return OP_ADD;    }
"-"                 { nb_col += yyleng; return OP_SUB;    }
"*"                 { nb_col += yyleng; return OP_MUL;    }
"/"                 { nb_col += yyleng; return OP_DIV;    }

";"                 { nb_col += yyleng; return SEP_SEMICOLON; }
":"                 { nb_col += yyleng; return SEP_COLON;     }
","                 { nb_col += yyleng; return SEP_COMMA;     }
"|"                 { nb_col += yyleng; return SEP_PIPE;      }
"["                 { nb_col += yyleng; return SEP_LBRACKET;  }
"]"                 { nb_col += yyleng; return SEP_RBRACKET;  }
"{"                 { nb_col += yyleng; return SEP_LBRACE;    }
"}"                 { nb_col += yyleng; return SEP_RBRACE;    }
"("                 { nb_col += yyleng; return SEP_LPAREN;    }
")"                 { nb_col += yyleng; return SEP_RPAREN;    }

{espace}            { nb_col += yyleng; }
{saut_ligne}        { nb_ligne++; nb_col = 1; }

.                   {
                        printf("ERREUR lexicale: caractere inconnu '%s', ligne %d, col %d\n",
                               yytext, nb_ligne, nb_col);
                        nb_col += yyleng;
                    }

%%
