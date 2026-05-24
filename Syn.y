%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RED   "\033[1;31m"
#define RESET "\033[0m"
#include "ts.h"
#include "quad.h"
#include "opt.h"
#include "asm8086.h"

int  yylex(void);
int  yyerror(char *msg);

extern int nb_ligne;
extern int nb_col;
extern int semantic_errors;
TypeVar type_courant;

/* Pending IDF list */
#define MAX_PENDING_IDF 100
static char *pending_idfs[MAX_PENDING_IDF];
static int pending_count = 0;

/* Contexte des boucles while imbriquees */
#define MAX_WHILE_NEST 100
static int while_start_stack[MAX_WHILE_NEST];
static int while_bz_stack[MAX_WHILE_NEST];
static int while_top = -1;


#define MAX_FOR_NEST 100
static int for_cond_stack[MAX_FOR_NEST]; /* indice du quad condition */
static int for_bz_stack[MAX_FOR_NEST];   /* indice du quad BZ        */
static int for_top = -1;
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
%type <ival> sem_checkpoint

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
            if (semantic_errors == 0)
                printf("\nAnalyse syntaxique et semantique correcte.\n");
            else
                printf(RED "\n%d erreur(s) semantique(s) detectee(s) - analyse terminee avec erreurs." RESET "\n", semantic_errors);

            ts_afficher();
            printf("\n=== Code intermediaire AVANT optimisation ===\n");
            afficher_qdr();
            optimiser_quadruplets();
            printf("\n=== Code intermediaire APRES optimisation ===\n");
            afficher_qdr();
            if (semantic_errors == 0) {
                generer_asm(NULL);
            } else {
                printf("Generation assembleur ignoree (erreurs semantiques).\n");
            }

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
    | decl_constante
    | error SEP_SEMICOLON
        {
            
            int _i;
            for (_i = 0; _i < pending_count; _i++) free(pending_idfs[_i]);
            pending_count = 0;
            yyerrok;
        }
    ;

decl_variable
    : DEFINE_MC liste_idf SEP_COLON suite_definition
    ;

suite_definition
    : type SEP_SEMICOLON
        {
            int ok = 1;
            int _i;
            for (_i = 0; _i < pending_count; _i++) {
                if (!ts_inserer_variable(pending_idfs[_i], type_courant)) ok = 0;
                free(pending_idfs[_i]);
            }
            pending_count = 0;
            if (ok) printf("Declaration de variable(s) correcte.\n");
        }
    | type OP_INIT valeur SEP_SEMICOLON
        {
            int ok = 1;
            int _i;
            for (_i = 0; _i < pending_count; _i++) {
                if (!ts_inserer_variable(pending_idfs[_i], type_courant)) { ok = 0; free(pending_idfs[_i]); continue; }
                ts_marquer_init(pending_idfs[_i]);
                quadr(":=", $3, "", pending_idfs[_i]);
                ts_set_val(pending_idfs[_i], $3);
                free(pending_idfs[_i]);
            }
            pending_count = 0;
            if (ok) printf("Declaration de variable(s) avec initialisation correcte.\n");
        }
    | SEP_LBRACKET type SEP_SEMICOLON NUM_INT SEP_RBRACKET SEP_SEMICOLON
        {
            if (pending_count != 1) {
                printf(RED "ERREUR semantique: un tableau ne peut avoir qu'un seul nom, ligne %d, col %d" RESET "\n",
                       nb_ligne, nb_col);
                semantic_errors++;
            } else if ($4 <= 0) {
                printf(RED "ERREUR semantique: taille de tableau invalide pour '%s', ligne %d, col %d" RESET "\n",
                       pending_idfs[0], nb_ligne, nb_col);
            } else {
                ts_inserer_tableau(pending_idfs[0], type_courant, $4);
                printf("Declaration de tableau correcte.\n");
            }
            int _i;
            for (_i = 0; _i < pending_count; _i++) free(pending_idfs[_i]);
            pending_count = 0;
        }
    ;

decl_constante
    : CONST_MC IDF SEP_COLON type OP_INIT valeur SEP_SEMICOLON
        {
            if (ts_inserer_constante($2, type_courant)) {
                ts_marquer_init($2);
                quadr(":=", $6, "", $2);
                ts_set_val($2, $6);
                printf("Declaration de constante correcte.\n");
            }
        }
    ;

liste_idf
    : IDF
        {
            if (pending_count < MAX_PENDING_IDF)
                pending_idfs[pending_count++] = strdup($1);
        }
    | IDF SEP_PIPE liste_idf
        {
            if (pending_count < MAX_PENDING_IDF)
                pending_idfs[pending_count++] = strdup($1);
        }
    ;

type
    : INTEGER_MC   { type_courant = TYPE_INTEGER; }
    | FLOAT_MC     { type_courant = TYPE_FLOAT;   }
    ;

valeur
    : NUM_INT
        { $$ = (char*)malloc(20); sprintf($$, "%d", $1); }
    | NUM_FLOAT
        { $$ = (char*)malloc(20); sprintf($$, "%f", $1); }
    | SEP_LPAREN OP_ADD NUM_INT SEP_RPAREN
        { $$ = (char*)malloc(20); sprintf($$, "%d", $3); }
    | SEP_LPAREN OP_SUB NUM_INT SEP_RPAREN
        { $$ = (char*)malloc(20); sprintf($$, "%d", -$3); }
    | SEP_LPAREN OP_ADD NUM_FLOAT SEP_RPAREN
        { $$ = (char*)malloc(20); sprintf($$, "%f", $3); }
    | SEP_LPAREN OP_SUB NUM_FLOAT SEP_RPAREN
        { $$ = (char*)malloc(20); sprintf($$, "%f", -$3); }
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
    | error SEP_SEMICOLON
        { yyerrok; }
    ;

sem_checkpoint
    : /* vide */
        { $$ = semantic_errors; }
    ;

instruction_affectation
    : IDF OP_ASSIGN sem_checkpoint expression SEP_SEMICOLON
        {
            int expr_has_error = (semantic_errors > $3);

            if (!ts_est_declare($1)) {
                printf(RED "ERREUR semantique: variable '%s' non declaree, "
                       "ligne %d, col %d" RESET "\n", $1, nb_ligne, nb_col);
                semantic_errors++;
            } else if (ts_est_constante($1)) {
                printf(RED "ERREUR semantique: modification de la constante '%s', "
                       "ligne %d, col %d" RESET "\n", $1, nb_ligne, nb_col);
                semantic_errors++;
            } else if (expr_has_error) {
                /* expression invalide : pas d'affectation */
            } else {
                const char *type_dest = ts_get_type($1);
                const char *type_src  = ts_get_type($4);
                if (type_dest != NULL && type_src != NULL
                    && strcmp(type_dest, type_src) != 0) {
                    printf(RED "ERREUR semantique: incompatibilite de types pour "
                           "'%s' (%s) <-- expression (%s), ligne %d, col %d"
                           RESET "\n",
                           $1, type_dest, type_src, nb_ligne, nb_col);
                    semantic_errors++;
                } else {
                    quadr(":=", $4, "", $1);
                    ts_marquer_init($1);
                    printf("Affectation correcte.\n");
                }
            }
        }
    | IDF SEP_LBRACKET sem_checkpoint expression SEP_RBRACKET OP_ASSIGN expression SEP_SEMICOLON
        {
            int expr_has_error = (semantic_errors > $3);

            if (!ts_est_declare($1)) {
                printf(RED "ERREUR semantique: tableau '%s' non declare, "
                       "ligne %d, col %d" RESET "\n", $1, nb_ligne, nb_col);
                semantic_errors++;
            } else if (expr_has_error) {
                /* index ou valeur invalide */
            } else {
                int idx_val = atoi($4);
                int taille  = ts_get_taille($1);

                if (idx_val < 0) {
                    printf(RED "ERREUR semantique: index negatif (%d) pour le "
                           "tableau '%s', ligne %d, col %d" RESET "\n",
                           idx_val, $1, nb_ligne, nb_col);
                    semantic_errors++;
                } else if (taille > 0 && idx_val >= taille) {
                    printf(RED "ERREUR semantique: index (%d) hors limites pour "
                           "le tableau '%s' (taille %d), ligne %d, col %d"
                           RESET "\n",
                           idx_val, $1, taille, nb_ligne, nb_col);
                    semantic_errors++;
                } else {
                    char dest[100];
                    sprintf(dest, "%s[%s]", $1, $4);
                    quadr(":=", $7, "", dest);
                    printf("Affectation tableau correcte.\n");
                }
            }
        }
    ;

instruction_condition
    : IF_MC SEP_LPAREN condition SEP_RPAREN THEN_MC SEP_COLON
      SEP_LBRACE
        {
            quadr("BZ", $3, "", "");
        }
      liste_instructions SEP_RBRACE
      suite_if
    ;

suite_if
    : ELSE_MC
        {
            quadr("BR", "", "", "");
            /* patcher le BZ vers le debut du else */
            char idx_str[20];
            sprintf(idx_str, "%d", qc);
            int _i;
            for (_i = qc - 2; _i >= 0; _i--) {
                if (strcmp(quad[_i].oper, "BZ") == 0 && strlen(quad[_i].res) == 0) {
                    updateQuad(_i, 3, idx_str);
                    break;
                }
            }
        }
      SEP_LBRACE liste_instructions SEP_RBRACE
      ENDIF_MC SEP_SEMICOLON
        {
            /* patcher le BR vers apres le else */
            char idx_str[20];
            sprintf(idx_str, "%d", qc);
            int _i;
            for (_i = qc - 1; _i >= 0; _i--) {
                if (strcmp(quad[_i].oper, "BR") == 0 && strlen(quad[_i].res) == 0) {
                    updateQuad(_i, 3, idx_str);
                    break;
                }
            }
            printf("Instruction if-else correcte.\n");
        }
    | ENDIF_MC SEP_SEMICOLON
        {
            char idx_str[20];
            sprintf(idx_str, "%d", qc);
            int _i;
            for (_i = qc - 1; _i >= 0; _i--) {
                if (strcmp(quad[_i].oper, "BZ") == 0 && strlen(quad[_i].res) == 0) {
                    updateQuad(_i, 3, idx_str);
                    break;
                }
            }
            printf("Instruction if correcte.\n");
        }
    ;

instruction_loop_while
    : LOOP_MC WHILE_MC SEP_LPAREN
      {
            if (while_top < MAX_WHILE_NEST - 1) {
                while_top++;
                while_start_stack[while_top] = qc;
                while_bz_stack[while_top] = -1;
            }
      }
      condition SEP_RPAREN
        {
            quadr("BZ", $5, "", "");
            if (while_top >= 0)
                while_bz_stack[while_top] = qc - 1;
        }
      SEP_LBRACE liste_instructions SEP_RBRACE
      ENDLOOP_MC SEP_SEMICOLON
        {
            if (while_top >= 0) {
                char debut_str[20];
                sprintf(debut_str, "%d", while_start_stack[while_top]);
                quadr("BR", "", "", debut_str);

                char fin_str[20];
                sprintf(fin_str, "%d", qc);
                if (while_bz_stack[while_top] >= 0)
                    updateQuad(while_bz_stack[while_top], 3, fin_str);

                while_top--;
            }
            printf("Boucle while correcte.\n");
        }
    ;


instruction_for
    : FOR_MC IDF IN_MC expression TO_MC expression
        {
            if (!ts_est_declare($2)) {
                printf(RED "ERREUR semantique: variable '%s' non declaree, ligne %d, col %d" RESET "\n",
                       $2, nb_ligne, nb_col);
                semantic_errors++;
            }

            /* Initialiser : var <- debut */
            quadr(":=", $4, "", $2);
            ts_marquer_init($2);

            /* Empiler le contexte for AVANT d'emettre la condition */
            if (for_top < MAX_FOR_NEST - 1) {
                for_top++;
                for_cond_stack[for_top] = qc;   /* indice du prochain quad = debut condition */
                for_bz_stack[for_top]   = -1;
            }

            /* Condition : var <= fin */
            char *tmp = strdup(nouveau_temp());
            quadr("<=", $2, $6, tmp);

            /* BZ de sortie (non patche pour l'instant) */
            quadr("BZ", tmp, "", "");
            if (for_top >= 0)
                for_bz_stack[for_top] = qc - 1;  /* indice du BZ qu'on vient d'emettre */
        }
      SEP_LBRACE liste_instructions SEP_RBRACE
      ENDFOR_MC SEP_SEMICOLON
        {
            if (for_top >= 0) {
                /* Incrementer : var <- var + 1 */
                char *tmp = strdup(nouveau_temp());
                quadr("+", $2, "1", tmp);
                quadr(":=", tmp, "", $2);

                /* BR vers le debut REEL de la condition (enregistre sur la pile) */
                char cond_str[20];
                sprintf(cond_str, "%d", for_cond_stack[for_top]);
                quadr("BR", "", "", cond_str);

                /* Patcher le BZ de CE for vers apres le BR (sortie boucle) */
                char fin_str[20];
                sprintf(fin_str, "%d", qc);
                if (for_bz_stack[for_top] >= 0)
                    updateQuad(for_bz_stack[for_top], 3, fin_str);

                for_top--;
            }
            printf("Boucle for correcte.\n");
        }
    ;

instruction_input
    : INPUT_MC SEP_LPAREN IDF SEP_RPAREN SEP_SEMICOLON
        {
            if (!ts_est_declare($3)) {
                printf(RED "ERREUR semantique: variable '%s' non declaree, ligne %d, col %d" RESET "\n",
                       $3, nb_ligne, nb_col);
                semantic_errors++;
            } else {
                quadr("input", "", "", $3);
                ts_marquer_init($3);
                printf("Instruction input correcte.\n");
            }
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
    : STRING     { quadr("out", $1, "", ""); }
    | IDF
        {
            if (!ts_est_declare($1)) {
                printf(RED "ERREUR semantique: variable '%s' non declaree, "
                       "ligne %d, col %d" RESET "\n", $1, nb_ligne, nb_col);
                semantic_errors++;
            } else if (!ts_est_initialise($1)) {
                printf(RED "ERREUR semantique: variable '%s' utilisee sans "
                       "initialisation, ligne %d, col %d" RESET "\n",
                       $1, nb_ligne, nb_col);
                semantic_errors++;
            } else {
                quadr("out", $1, "", "");
            }
        }
    | NUM_INT
        {
            char s[20]; sprintf(s, "%d", $1);
            quadr("out", s, "", "");
        }
    | NUM_FLOAT
        {
            char s[20]; sprintf(s, "%f", $1);
            quadr("out", s, "", "");
        }
    ;

condition
    : expression OP_EQ expression
        { $$ = strdup(nouveau_temp()); quadr("==", $1, $3, $$); }
    | expression OP_NE expression
        { $$ = strdup(nouveau_temp()); quadr("!=", $1, $3, $$); }
    | expression OP_LT expression
        { $$ = strdup(nouveau_temp()); quadr("<",  $1, $3, $$); }
    | expression OP_GT expression
        { $$ = strdup(nouveau_temp()); quadr(">",  $1, $3, $$); }
    | expression OP_LE expression
        { $$ = strdup(nouveau_temp()); quadr("<=", $1, $3, $$); }
    | expression OP_GE expression
        { $$ = strdup(nouveau_temp()); quadr(">=", $1, $3, $$); }
    | condition AND condition
        { $$ = strdup(nouveau_temp()); quadr("AND", $1, $3, $$); }
    | condition OR  condition
        { $$ = strdup(nouveau_temp()); quadr("OR",  $1, $3, $$); }
    | NON SEP_LPAREN condition SEP_RPAREN
        { $$ = strdup(nouveau_temp()); quadr("NON", $3, "",  $$); }
    | SEP_LPAREN condition SEP_RPAREN
        { $$ = $2; }
    ;

expression
    : expression OP_ADD expression
        { $$ = strdup(nouveau_temp()); quadr("+", $1, $3, $$); }
    | expression OP_SUB expression
        { $$ = strdup(nouveau_temp()); quadr("-", $1, $3, $$); }
    | expression OP_MUL expression
        { $$ = strdup(nouveau_temp()); quadr("*", $1, $3, $$); }
    | expression OP_DIV expression
        {
            int div_zero = (strcmp($3, "0") == 0 || strcmp($3, "0.0") == 0);
            if (!div_zero) {
                const char *v = ts_get_val($3);
                if (v && (strcmp(v, "0") == 0 || strcmp(v, "0.0") == 0))
                    div_zero = 1;
            }
            if (div_zero) {
                printf(RED "ERREUR semantique: division par zero (diviseur '%s' vaut 0), ligne %d, col %d" RESET "\n",
                       $3, nb_ligne, nb_col);
                semantic_errors++;
            }
            $$ = strdup(nouveau_temp()); quadr("/", $1, $3, $$);
        }
    | SEP_LPAREN expression SEP_RPAREN
        { $$ = $2; }
    | SEP_LPAREN OP_ADD expression SEP_RPAREN
        { $$ = $3; }
    | SEP_LPAREN OP_SUB expression SEP_RPAREN
        { $$ = strdup(nouveau_temp()); quadr("NEG", $3, "", $$); }
    | IDF
        {
            if (!ts_est_declare($1)) {
                printf(RED "ERREUR semantique: variable '%s' non declaree, ligne %d, col %d" RESET "\n",
                       $1, nb_ligne, nb_col);
                semantic_errors++;
            } else if (!ts_est_initialise($1)) {
               
                printf(RED "ERREUR semantique: variable '%s' utilisee sans initialisation, ligne %d, col %d" RESET "\n",
                       $1, nb_ligne, nb_col);
                semantic_errors++;
            }
            $$ = $1;
        }
    | IDF SEP_LBRACKET expression SEP_RBRACKET
        {
            if (!ts_est_declare($1)) {
                printf(RED "ERREUR semantique: tableau '%s' non declare, ligne %d, col %d" RESET "\n",
                       $1, nb_ligne, nb_col);
                semantic_errors++;
            }
            $$ = strdup(nouveau_temp());
            quadr("TAB", $1, $3, $$);
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
    printf(RED "ERREUR syntaxique: %s, ligne %d, col %d" RESET "\n", msg, nb_ligne, nb_col);
    return 1;
}
