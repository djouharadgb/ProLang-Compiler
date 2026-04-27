%{
    #include <stdio.h>
    #include <string.h>
    #include "ts.h"
    #include "quad.h"
    #include "Syn.tab.h"
    int nb_ligne = 1;
    int nb_col   = 1;
    #define RED   "\033[1;31m"
    #define RESET "\033[0m"
%}

letter      [A-Za-z]
digit       [0-9]
int_num     (0|[1-9][0-9]*)
float_num   (0|[1-9][0-9]*)"."[0-9]+
float_bad     (0|[1-9][0-9]*)"."
strings     \"[^\"]*\"
IDF         {letter}[a-z0-9_]*
espace      [ \t]+
saut_ligne  \n

%x COMMENT

%%

"//*"                  { BEGIN(COMMENT); } /* entrer dans l'etat COMMENT */
<COMMENT>"*//"         { BEGIN(INITIAL); } /* sortir de l'etat COMMENT */
<COMMENT>{saut_ligne}  { nb_ligne++; nb_col = 1; }
<COMMENT>.             { /* ignorer le contenu du commentaire */ }

"%%"[^\n]*            { nb_col += yyleng; } /* commentaire mono-ligne */

"BeginProject"      { nb_col += yyleng; ts_inserer_mc("BeginProject", "BeginProject"); return BeginProject; }
"EndProject"        { nb_col += yyleng; ts_inserer_mc("EndProject",   "EndProject");   return EndProject;   }
"Setup"             { nb_col += yyleng; ts_inserer_mc("Setup",        "Setup");        return Setup;        }
"Run"               { nb_col += yyleng; ts_inserer_mc("Run",          "Run");          return Run;          }
"define"            { nb_col += yyleng; ts_inserer_mc("define",       "DEFINE_MC");    return DEFINE_MC;    }
"const"             { nb_col += yyleng; ts_inserer_mc("const",        "CONST_MC");     return CONST_MC;     }
"integer"           { nb_col += yyleng; ts_inserer_mc("integer",      "INTEGER_MC");   return INTEGER_MC;   }
"float"             { nb_col += yyleng; ts_inserer_mc("float",        "FLOAT_MC");     return FLOAT_MC;     }
"if"                { nb_col += yyleng; ts_inserer_mc("if",           "IF_MC");        return IF_MC;        }
"then"              { nb_col += yyleng; ts_inserer_mc("then",         "THEN_MC");      return THEN_MC;      }
"else"              { nb_col += yyleng; ts_inserer_mc("else",         "ELSE_MC");      return ELSE_MC;      }
"endIf"             { nb_col += yyleng; ts_inserer_mc("endIf",        "ENDIF_MC");     return ENDIF_MC;     }
"loop"              { nb_col += yyleng; ts_inserer_mc("loop",         "LOOP_MC");      return LOOP_MC;      }
"while"             { nb_col += yyleng; ts_inserer_mc("while",        "WHILE_MC");     return WHILE_MC;     }
"endloop"           { nb_col += yyleng; ts_inserer_mc("endloop",      "ENDLOOP_MC");   return ENDLOOP_MC;   }
"for"               { nb_col += yyleng; ts_inserer_mc("for",          "FOR_MC");       return FOR_MC;       }
"input"             { nb_col += yyleng; ts_inserer_mc("input",        "INPUT_MC");     return INPUT_MC;     }
"to"                { nb_col += yyleng; ts_inserer_mc("to",           "TO_MC");        return TO_MC;        }
"endfor"            { nb_col += yyleng; ts_inserer_mc("endfor",       "ENDFOR_MC");    return ENDFOR_MC;    }
"in"                { nb_col += yyleng; ts_inserer_mc("in",           "IN_MC");        return IN_MC;        }
"out"               { nb_col += yyleng; ts_inserer_mc("out",          "OUT_MC");       return OUT_MC;       }
"AND"               { nb_col += yyleng; ts_inserer_mc("AND",          "AND");          return AND;          }
"OR"                { nb_col += yyleng; ts_inserer_mc("OR",           "OR");           return OR;           }
"NON"               { nb_col += yyleng; ts_inserer_mc("NON",          "NON");          return NON;          }

_[A-Za-z0-9_]*      {
               nb_col += yyleng;
               printf(RED "ERREUR lexicale: identificateur mal forme '%s' "
                   "(ne doit pas commencer par '_'), ligne %d, col %d" RESET "\n",
                   yytext, nb_ligne, nb_col - yyleng);
              }

{IDF}               {
                        nb_col += yyleng;
                        if (yyleng > 14) {
                            printf(RED "ERREUR lexicale: identificateur '%s' depasse 14 caracteres, ligne %d, col %d" RESET "\n",
                                   yytext, nb_ligne, nb_col - yyleng);
                        } else if (yytext[yyleng - 1] == '_') {
                            printf(RED "ERREUR lexicale: identificateur '%s' ne peut pas "
                                   "se terminer par '_', ligne %d, col %d" RESET "\n",
                                   yytext, nb_ligne, nb_col - yyleng);
                        } else if (strstr(yytext, "__") != NULL) {
                            printf(RED "ERREUR lexicale: identificateur '%s' contient des "
                                   "tirets bas consecutifs '__', ligne %d, col %d" RESET "\n",
                                   yytext, nb_ligne, nb_col - yyleng);
                        } else {
                            yylval.sval = strdup(yytext); /* yylval stocke la valeur de l'identificateur */
                            return IDF;
                        }
                    }
{float_bad}  {
                nb_col += yyleng;
                printf(RED "ERREUR lexicale: constante reelle mal formee '%s' "
                       "(chiffre manquant apres '.'), ligne %d, col %d" RESET "\n",
                       yytext, nb_ligne, nb_col - yyleng);
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
                            printf(RED "ERREUR lexicale: entier '%s' hors limites [-32768, 32767], ligne %d, col %d" RESET "\n",
                                   yytext, nb_ligne, nb_col - yyleng);
                        }
                        return NUM_INT;
                    }
{strings}           {
                        nb_col += yyleng;
                        yylval.sval = strdup(yytext);
                        return STRING;
                    }

"=="                { nb_col += yyleng; ts_inserer_sep("==", "OP_EQ");    return OP_EQ;     }
"!="                { nb_col += yyleng; ts_inserer_sep("!=", "OP_NE");    return OP_NE;     }
">="                { nb_col += yyleng; ts_inserer_sep(">=", "OP_GE");    return OP_GE;     }
"<="                { nb_col += yyleng; ts_inserer_sep("<=", "OP_LE");    return OP_LE;     }
"<--"                { nb_col += yyleng; ts_inserer_sep("<--", "OP_ASSIGN"); return OP_ASSIGN;  }
">"                 { nb_col += yyleng; ts_inserer_sep(">",  "OP_GT");    return OP_GT;     }
"<"                 { nb_col += yyleng; ts_inserer_sep("<",  "OP_LT");    return OP_LT;     }

"="                 { nb_col += yyleng; ts_inserer_sep("=",  "OP_INIT");  return OP_INIT;   }
"+"                 { nb_col += yyleng; ts_inserer_sep("+",  "OP_ADD");   return OP_ADD;    }
"-"                 { nb_col += yyleng; ts_inserer_sep("-",  "OP_SUB");   return OP_SUB;    }
"*"                 { nb_col += yyleng; ts_inserer_sep("*",  "OP_MUL");   return OP_MUL;    }
"/"                 { nb_col += yyleng; ts_inserer_sep("/",  "OP_DIV");   return OP_DIV;    }

";"                 { nb_col += yyleng; ts_inserer_sep(";",  "SEP_SEMICOLON"); return SEP_SEMICOLON; }
":"                 { nb_col += yyleng; ts_inserer_sep(":",  "SEP_COLON");     return SEP_COLON;     }
","                 { nb_col += yyleng; ts_inserer_sep(",",  "SEP_COMMA");     return SEP_COMMA;     }
"|"                 { nb_col += yyleng; ts_inserer_sep("|",  "SEP_PIPE");      return SEP_PIPE;      }
"["                 { nb_col += yyleng; ts_inserer_sep("[",  "SEP_LBRACKET");  return SEP_LBRACKET;  }
"]"                 { nb_col += yyleng; ts_inserer_sep("]",  "SEP_RBRACKET");  return SEP_RBRACKET;  }
"{"                 { nb_col += yyleng; ts_inserer_sep("{",  "SEP_LBRACE");    return SEP_LBRACE;    }
"}"                 { nb_col += yyleng; ts_inserer_sep("}",  "SEP_RBRACE");    return SEP_RBRACE;    }
"("                 { nb_col += yyleng; ts_inserer_sep("(",  "SEP_LPAREN");    return SEP_LPAREN;    }
")"                 { nb_col += yyleng; ts_inserer_sep(")",  "SEP_RPAREN");    return SEP_RPAREN;    }

{espace}            { nb_col += yyleng; }
{saut_ligne}        { nb_ligne++; nb_col = 1; }

.                   {
                        printf(RED "ERREUR lexicale: caractere inconnu '%s', ligne %d, col %d" RESET "\n",
                               yytext, nb_ligne, nb_col);
                        nb_col += yyleng;
                    }

%%
