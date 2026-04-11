%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ts.h"
#include "quad.h"

int  yylex(void);
int  yyerror(char *msg);

extern int nb_ligne;
extern int nb_col;
TypeVar type_courant; /* Type courant des variables when we have multiple declarations   */
%}

%union {
    int    ival;
    float  fval;
    char*  sval;
}

%token <sval> IDF
%token <sval> STRING
%token <ival> NUM_INT
%token <fval> NUM_FLOAT

%token BeginProject EndProject Setup Run
%token DEFINE_MC CONST_MC
%token INTEGER_MC FLOAT_MC
%token IF_MC THEN_MC ELSE_MC ENDIF_MC
%token LOOP_MC WHILE_MC ENDLOOP_MC
%token FOR_MC IN_MC TO_MC ENDFOR_MC
%token INPUT_MC OUT_MC

%token AND OR NON

%token OP_ASSIGN
%token OP_INIT
%token OP_EQ
%token OP_NE
%token OP_LT
%token OP_GT
%token OP_LE
%token OP_GE
%token OP_ADD
%token OP_SUB
%token OP_MUL
%token OP_DIV

%token SEP_SEMICOLON
%token SEP_COLON
%token SEP_COMMA
%token SEP_PIPE
%token SEP_LBRACKET
%token SEP_RBRACKET
%token SEP_LBRACE
%token SEP_RBRACE
%token SEP_LPAREN
%token SEP_RPAREN

%left AND OR
%right NON
%left OP_EQ OP_NE OP_LT OP_GT OP_LE OP_GE
%left OP_ADD OP_SUB
%left OP_MUL OP_DIV

%type <sval> expression condition
%type <sval> valeur


%start programme

%%

programme
    : BeginProject IDF SEP_SEMICOLON
      Setup SEP_COLON
      partie_declaration
      Run SEP_COLON
      SEP_LBRACE liste_instructions SEP_RBRACE
      EndProject SEP_SEMICOLON
        {
            printf("\nAnalyse syntaxique et semantique correcte.\n");
            ts_afficher();
            afficher_quads();
            YYACCEPT;
        }
    ;

partie_declaration
    : liste_declarations
    |  
    ;

liste_declarations
    : declaration liste_declarations
    | declaration
    ;

declaration
    : decl_variable
    | decl_tableau
    | decl_constante
    ;

decl_variable
    : DEFINE_MC liste_idf SEP_COLON type SEP_SEMICOLON
        { printf("Declaration de variable(s) correcte.\n"); }
    | DEFINE_MC liste_idf SEP_COLON type OP_INIT valeur SEP_SEMICOLON
        { printf("Declaration de variable(s) avec initialisation correcte.\n"); }
    ;

decl_tableau
    : DEFINE_MC IDF SEP_COLON SEP_LBRACKET type SEP_SEMICOLON NUM_INT SEP_RBRACKET SEP_SEMICOLON
        {
            if ($7 <= 0) { /* taille inf ou egale a 0*/
                printf("ERREUR semantique: taille de tableau invalide pour '%s', ligne %d, col %d\n",
                       $2, nb_ligne, nb_col);
            } else {
                ts_inserer_tableau($2, type_courant, $7);
            }
            printf("Declaration de tableau correcte.\n");
        }
    ;

decl_constante
    : CONST_MC IDF SEP_COLON type OP_INIT valeur SEP_SEMICOLON
        {
            ts_inserer_constante($2, type_courant);
            ts_marquer_init($2);/* Marquer la constante comme initialisee */
            generer_quad(":=", $sval6, "", $2);
            printf("Declaration de constante correcte.\n");
        }
    ;

liste_idf
    : IDF
        { ts_inserer_variable($1, type_courant); }
    | IDF SEP_PIPE liste_idf
        { ts_inserer_variable($1, type_courant); }
    ;

type
    : INTEGER_MC   { type_courant = TYPE_INTEGER; } /*we affect to use it later in cases ike int z|b|c; same type*/
    | FLOAT_MC     { type_courant = TYPE_FLOAT;   }
    ;

valeur
    : NUM_INT
    | NUM_FLOAT
    | SEP_LPAREN OP_ADD NUM_INT SEP_RPAREN
    | SEP_LPAREN OP_SUB NUM_INT SEP_RPAREN
    | SEP_LPAREN OP_ADD NUM_FLOAT SEP_RPAREN
    | SEP_LPAREN OP_SUB NUM_FLOAT SEP_RPAREN
    ;

liste_instructions
    : instruction liste_instructions
    |  
    ;

instruction
    : instruction_affectation
    | instruction_condition
    | instruction_loop_while
    | instruction_for
    | instruction_input
    | instruction_output
    ;

instruction_affectation
    : IDF OP_ASSIGN expression SEP_SEMICOLON
        {
            if (!ts_est_declare($1)) {
                printf("ERREUR semantique: variable '%s' non declaree, ligne %d, col %d\n",
                       $1, nb_ligne, nb_col);
            } else if (ts_est_constante($1)) {
                printf("ERREUR semantique: modification de la constante '%s', ligne %d, col %d\n",
                       $1, nb_ligne, nb_col);
            } else {
                generer_quad(":=", $3, "", $1);
                ts_marquer_init($1);
            }
            printf("Affectation correcte.\n");
        }
    | IDF SEP_LBRACKET expression SEP_RBRACKET OP_ASSIGN expression SEP_SEMICOLON
        {
            if (!ts_est_declare($1)) {
                printf("ERREUR semantique: tableau '%s' non declare, ligne %d, col %d\n",
                       $1, nb_ligne, nb_col);
            } else {
                char dest[MAX_ARG]; /*taille max dun argument dans un quadruplet*/
                sprintf(dest, "%s[%s]", $1, $3); /* T[1]*/
                generer_quad(":=", $6, "", dest); /*we affect result in T[1]*/
            }
            printf("Affectation tableau correcte.\n");
        }
    ;

instruction_condition
    : IF_MC SEP_LPAREN condition SEP_RPAREN THEN_MC SEP_COLON
      SEP_LBRACE
        {
            /* BZ: si condition fausse, sauter */
            generer_quad("BZ", $3, "", "");
        }
      liste_instructions SEP_RBRACE
      ELSE_MC
        {
            /* BR: sauter le bloc else */
            generer_quad("BR", "", "", "");
            /* Mettre a jour le BZ pour pointer ici */
            char idx_str[20];
            sprintf(idx_str, "%d", qc);
            maj_quad(qc - 1 - 1, idx_str);  /* le BZ est 2 avant */
        }
      SEP_LBRACE liste_instructions SEP_RBRACE
      ENDIF_MC SEP_SEMICOLON
        {
            /* Mettre a jour le BR pour pointer apres le else */
            char idx_str[20];
            sprintf(idx_str, "%d", qc); /*index de la prochaine instruction, donc apres le else*/
            /* Trouver le BR genere avant le else */
            int br_idx = -1; /*-1 signifie non trouve*/
            for (int i = qc - 1; i >= 0; i--) { /*qc pointe vers case vide lzm -1*/
                if (strcmp(quads[i].op, "BR") == 0 && strlen(quads[i].result) == 0) {
                    br_idx = i;
                    break;
                }
            }
            if (br_idx >= 0) maj_quad(br_idx, idx_str);/*metre a jour quad de Bravec index fin*/
            printf("Instruction if-else correcte.\n");
        }
    | IF_MC SEP_LPAREN condition SEP_RPAREN THEN_MC SEP_COLON
      SEP_LBRACE
        {
            generer_quad("BZ", $3, "", "");
        }
      liste_instructions SEP_RBRACE
      ENDIF_MC SEP_SEMICOLON
        {
            char idx_str[20];
            sprintf(idx_str, "%d", qc);
            /* Mettre a jour le BZ */
            for (int i = qc - 1; i >= 0; i--) {
                if (strcmp(quads[i].op, "BZ") == 0 && strlen(quads[i].result) == 0) {
                    maj_quad(i, idx_str);
                    break;
                }
            }
            printf("Instruction if correcte.\n");
        }
    ;

instruction_loop_while
    : LOOP_MC WHILE_MC SEP_LPAREN
      condition SEP_RPAREN
        { /* in the case condition is false */
            generer_quad("BZ", $4, "", "");        }
      SEP_LBRACE liste_instructions SEP_RBRACE
      ENDLOOP_MC SEP_SEMICOLON
        {
            char debut_str[20];
            /* trouver le BZ le plus recent non patche */
            int bz_idx = -1;
            for (int i = qc - 1; i >= 0; i--) {
                if (strcmp(quads[i].op, "BZ") == 0 && strlen(quads[i].result) == 0) { /*we search for BZ*/
                    bz_idx = i; /*we found position of BZ index*/
                    break;
                }
            }
            /* Le debut est juste avant le BZ (la condition) */
            /* On genere BR vers debut_condition */
            if (bz_idx >= 0) { /*we found deb de la boucle*/
                sprintf(debut_str, "%d", bz_idx - 1 >= 0 ? bz_idx - 1 : 0); /*if its - then use 0 to avoid idce negative*/
                generer_quad("BR", "", "", debut_str); /*debut de la boucle, we generate BR*/
                char fin_str[20];
                sprintf(fin_str, "%d", qc);
                maj_quad(bz_idx, fin_str); /*we update BZ of position of Fin */
            }
            printf("Boucle while correcte.\n");
        }
    ;

instruction_for
    : FOR_MC IDF IN_MC expression TO_MC expression
        {
            if (!ts_est_declare($2)) {
                printf("ERREUR semantique: variable '%s' non declaree, ligne %d, col %d\n",
                       $2, nb_ligne, nb_col);
            }
            /* Initialiser i <- debut */
            generer_quad(":=", $4, "", $2);
            /* Condition: i <= fin */
            char *tmp = strdup(nouveau_temp());
            generer_quad("<=", $2, $6, tmp);
            generer_quad("BZ", tmp, "", "");
        }
      SEP_LBRACE liste_instructions SEP_RBRACE
      ENDFOR_MC SEP_SEMICOLON
        {
            /* Incrementer i <- i + 1 */
            char *tmp = strdup(nouveau_temp());
            generer_quad("+", $2, "1", tmp); /*incrementer avec 1*/
            generer_quad(":=", tmp, "", $2);/*affecter a IDF*/
            /* BR vers la condition */
            /* Trouver le BZ non patche */
            int bz_idx = -1;
            for (int i = qc - 1; i >= 0; i--) {
                if (strcmp(quads[i].op, "BZ") == 0 && strlen(quads[i].result) == 0) {
                    bz_idx = i;
                    break;
                }
            }
            if (bz_idx >= 0) {
                char cond_str[20];
                sprintf(cond_str, "%d", bz_idx - 1);
                generer_quad("BR", "", "", cond_str);
                char fin_str[20];
                sprintf(fin_str, "%d", qc);
                maj_quad(bz_idx, fin_str);
            }
            printf("Boucle for correcte.\n");
        }
    ;

instruction_input
    : INPUT_MC SEP_LPAREN IDF SEP_RPAREN SEP_SEMICOLON
        {
            if (!ts_est_declare($3)) {
                printf("ERREUR semantique: variable '%s' non declaree, ligne %d, col %d\n",
                       $3, nb_ligne, nb_col);
            }
            generer_quad("input", "", "", $3);
            ts_marquer_init($3);
            printf("Instruction input correcte.\n");
        }
    ;

instruction_output
    : OUT_MC SEP_LPAREN liste_out SEP_RPAREN SEP_SEMICOLON
        { printf("Instruction out correcte.\n"); }
    ;

liste_out
    : element_out
    | element_out SEP_COMMA liste_out
    ;

element_out
    : STRING     { generer_quad("out", $1, "", ""); }
    | IDF        {
                    if (!ts_est_declare($1)) {
                        printf("ERREUR semantique: variable '%s' non declaree, ligne %d, col %d\n",
                               $1, nb_ligne, nb_col);
                    }
                    generer_quad("out", $1, "", "");
                 }
    | NUM_INT    {
                    char s[20]; sprintf(s, "%d", $1);
                    generer_quad("out", s, "", "");
                 }
    | NUM_FLOAT  {
                    char s[20]; sprintf(s, "%f", $1);
                    generer_quad("out", s, "", "");
                 }
    ;


condition
    : expression OP_EQ expression
        { $$ = strdup(nouveau_temp()); generer_quad("==", $1, $3, $$); } /*we need to ;ut result in temp, $$ cant just affect to it like that*/
    | expression OP_NE expression
        { $$ = strdup(nouveau_temp()); generer_quad("!=", $1, $3, $$); }
    | expression OP_LT expression
        { $$ = strdup(nouveau_temp()); generer_quad("<",  $1, $3, $$); }
    | expression OP_GT expression
        { $$ = strdup(nouveau_temp()); generer_quad(">",  $1, $3, $$); }
    | expression OP_LE expression
        { $$ = strdup(nouveau_temp()); generer_quad("<=", $1, $3, $$); }
    | expression OP_GE expression
        { $$ = strdup(nouveau_temp()); generer_quad(">=", $1, $3, $$); }
    | SEP_LPAREN condition AND condition SEP_RPAREN
        { $$ = strdup(nouveau_temp()); generer_quad("AND", $2, $4, $$); }
    | SEP_LPAREN condition OR  condition SEP_RPAREN
        { $$ = strdup(nouveau_temp()); generer_quad("OR",  $2, $4, $$); }
    | NON SEP_LPAREN condition SEP_RPAREN
        { $$ = strdup(nouveau_temp()); generer_quad("NON", $3, "",  $$); }
    ;


expression
    : expression OP_ADD expression
        { $$ = strdup(nouveau_temp()); generer_quad("+", $1, $3, $$); }
    | expression OP_SUB expression
        { $$ = strdup(nouveau_temp()); generer_quad("-", $1, $3, $$); }
    | expression OP_MUL expression
        { $$ = strdup(nouveau_temp()); generer_quad("*", $1, $3, $$); }
    | expression OP_DIV expression
        {
            /* Verification division par zero */
            if (strcmp($3, "0") == 0 || strcmp($3, "0.0") == 0) {
                printf("ERREUR semantique: division par zero, ligne %d, col %d\n",
                       nb_ligne, nb_col);
            }
            $$ = strdup(nouveau_temp()); generer_quad("/", $1, $3, $$);
        }
    | SEP_LPAREN expression SEP_RPAREN
        { $$ = $2; }
    | SEP_LPAREN OP_ADD expression SEP_RPAREN
        { $$ = $3; }
    | SEP_LPAREN OP_SUB expression SEP_RPAREN
        { $$ = strdup(nouveau_temp()); generer_quad("NEG", $3, "", $$); }
    | IDF
        {
            if (!ts_est_declare($1)) {
                printf("ERREUR semantique: variable '%s' non declaree, ligne %d, col %d\n",
                       $1, nb_ligne, nb_col);
            }
            $$ = $1;
        }
    | IDF SEP_LBRACKET expression SEP_RBRACKET
        {
            if (!ts_est_declare($1)) {
                printf("ERREUR semantique: tableau '%s' non declare, ligne %d, col %d\n",
                       $1, nb_ligne, nb_col);
            }
            $$ = strdup(nouveau_temp());
            generer_quad("TAB", $1, $3, $$);
        }
    | NUM_INT
        {
            $$ = (char*)malloc(20);
            sprintf($$, "%d", $1);
        }
    | NUM_FLOAT
        {
            $$ = (char*)malloc(20);
            sprintf($$, "%f", $1);
        }
    ;

%%

int main()
{
    ts_initialiser();
    yyparse();
    ts_liberer();
    return 0;
}

int yywrap()
{
    return 1;
}

int yyerror(char *msg)
{
    printf("ERREUR syntaxique: %s, ligne %d, col %d\n", msg, nb_ligne, nb_col);
    return 1;
}
