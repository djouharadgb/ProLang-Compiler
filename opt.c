#include "opt.h"
#include "quad.h"
#include "ts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define GRN  "\033[1;32m"
#define CYN  "\033[1;36m"
#define YLW  "\033[1;33m"
#define RESET "\033[0m"


/*
 est_nombre : verifie si la chaine s represente un nombre valide
            (entier ou flottant, avec signe optionnel).
            on verifie qu cest un nombre avant de simplifier x+0=x
 */
static int est_nombre(const char *s)
{
    if (!s || !*s) return 0;
    const char *p = s;
    if (*p == '-' || *p == '+') p++;  /* signe optionnel */
    if (!*p) return 0;                /* signe seul => invalide */
    int a_point = 0;
    for (; *p; p++) {
        if (*p == '.') {
            if (a_point) return 0;    /* deux points => invalide (ex: 3.1.4) */
            a_point = 1;
        } else if (!isdigit((unsigned char)*p)) return 0; /* lettre => pas un nombre */
    }
    return 1;
}

/*
  est_temporaire : verifie si s est un temporaire genere par le compilateur.
  Un temporaire a la forme T0, T1, T2, ... (T majuscule + chiffres) */
static int est_temporaire(const char *s)
{
    int i;
    if (!s || s[0] != 'T') return 0;
    for (i = 1; s[i]; i++)
        if (!isdigit((unsigned char)s[i])) return 0;
    return (i > 1); /* au moins un chiffre apres le T */
}

/*
 est_saut : verifie si l'operateur est un saut (BZ ou BR).
 */
static int est_saut(const char *op)
{
    return (strcmp(op, "BZ") == 0 || strcmp(op, "BR") == 0);
}

/*
 * a_effet_de_bord : verifie si le quadruplet i produit un effet
 * (lecture, ecriture, saut) qui doit etre preserve meme si son resultat n'est jamais lu.
 */
static int a_effet_de_bord(int i)
{
    const char *op = quad[i].oper;
    return est_saut(op)
        || strcmp(op, "input") == 0   /* lecture clavier : effet observable */
        || strcmp(op, "out")   == 0;  /* affichage ecran : effet observable */
}

/*
 compter_utilisations : compte combien de fois la variable var
 apparait comme operande (op1 ou op2) dans toute la table des quadruplets.
 */
static int compter_utilisations(const char *var)
{
    int i, n = 0;
    if (!var || !*var) return 0;
    for (i = 0; i < qc; i++) {
        if (strcmp(quad[i].op1, var) == 0) n++;
        if (strcmp(quad[i].op2, var) == 0) n++;
    }
    return n;
}

/*
 est_redefini_entre : verifie si la variable var est redefinie
 entre les indices debut (inclus) et fin (exclu).
 Exemple :
   0: t1 := x
   1: x  := 5
   2: t2 := t1 + 1
   => si on cherche a propager t1=x, x est redefini en 1 : propagation invalide
 */
static int est_redefini_entre(const char *var, int debut, int fin)
{
    int i;
    for (i = debut; i < fin; i++) {
        if (est_saut(quad[i].oper)) continue;
        if (strcmp(quad[i].res, var) == 0) return 1;
    }
    return 0;
}

/*
 * formater_nombre : convertit un double en chaine de caracteres.
 * Si la valeur est entiere, l'affiche sans decimale (ex: 4 et non 4.000000).
 */
static void formater_nombre(double v, char *buf)
{
    if (v == (long)v) sprintf(buf, "%ld", (long)v);
    else              sprintf(buf, "%g",  v);
}

/* ==========================================================================
   PASSE 1 : PROPAGATION DE CONSTANTES SYMBOLIQUES

   Principe : si la table des symboles indique qu'un nom est une CONST
   avec une valeur numerique connue, remplacer toutes ses occurrences
   dans les operandes des quadruplets par cette valeur litterale.
   Cela permet aux passes suivantes (simplification algebrique, etc.)
   de plier les calculs impliquant des constantes nommees.

   Exemple :
     const N = 5;
     T0 := x * N    =>  T0 := x * 5
     T1 := N + 3    =>  T1 := 8  (apres simplification algebrique)
   ========================================================================== */
static int propagation_constantes_symboliques(void)
{
    int modifie = 0, i;
    for (i = 0; i < qc; i++) {
        /* Tester op1 et op2 de chaque quadruplet */
        char *champs[2] = { quad[i].op1, quad[i].op2 };
        int c;
        for (c = 0; c < 2; c++) {
            char *nom = champs[c];
            if (!nom || !*nom || est_nombre(nom)) continue; /* deja un litteral */
            if (!ts_est_constante(nom)) continue;           /* pas une constante TS */
            const char *val = ts_get_val(nom);
            if (!val || !*val || !est_nombre(val)) continue; /* valeur non numerique */
            /* Substituer le nom par la valeur litterale */
            strcpy(nom, val);
            modifie = 1;
        }
    }
    return modifie;
}

/* ==========================================================================
   PASSE 2 : SIMPLIFICATION ALGEBRIQUE

   Remplacer des operations sans effet ou calculables a la compilation
   par des formes equivalentes plus simples.

   Regles appliquees :
     X + 0  ou  0 + X   =>  X
     X - 0              =>  X
     X - X              =>  0
     X * 0  ou  0 * X   =>  0
     X * 1  ou  1 * X   =>  X
     X * 2  ou  2 * X   =>  X + X   (addition moins couteuse)
     X * 4  ou  4 * X   =>  X * 2   (reduit en deux iterations a X+X)
     X * 8  ou  8 * X   =>  X * 4   (idem, reduit progressivement)
     X / 1              =>  X
     X / X              =>  1       (si X != 0, cas statiquement sur)
     C1 op C2           =>  resultat calcule a la compilation (folding)
   ========================================================================== */
static int simplification_algebrique(void)
{
    int modifie = 0, i;
    for (i = 0; i < qc; i++) {
        char *op = quad[i].oper;
        char *a  = quad[i].op1;
        char *b  = quad[i].op2;
        int a_est_nb = est_nombre(a), b_est_nb = est_nombre(b);

        if (strcmp(op, "+") == 0) {
            if (b_est_nb && atof(b) == 0.0) {              /* X + 0 => X */
                strcpy(op, ":="); strcpy(b, "");
                modifie = 1;
            } else if (a_est_nb && atof(a) == 0.0) {       /* 0 + X => X */
                strcpy(op, ":="); strcpy(a, b); strcpy(b, "");
                modifie = 1;
            } else if (a_est_nb && b_est_nb) {             /* C + C => resultat direct */
                char buf[64]; formater_nombre(atof(a) + atof(b), buf);
                strcpy(op, ":="); strcpy(a, buf); strcpy(b, "");
                modifie = 1;
            }

        } else if (strcmp(op, "-") == 0) {
            if (b_est_nb && atof(b) == 0.0) {              /* X - 0 => X */
                strcpy(op, ":="); strcpy(b, "");
                modifie = 1;
            } else if (!a_est_nb && !b_est_nb && strcmp(a, b) == 0) { /* X - X => 0 */
                strcpy(op, ":="); strcpy(a, "0"); strcpy(b, "");
                modifie = 1;
            } else if (a_est_nb && b_est_nb) {             /* C - C => resultat direct */
                char buf[64]; formater_nombre(atof(a) - atof(b), buf);
                strcpy(op, ":="); strcpy(a, buf); strcpy(b, "");
                modifie = 1;
            }

        } else if (strcmp(op, "*") == 0) {
            if ((a_est_nb && atof(a) == 0.0) || (b_est_nb && atof(b) == 0.0)) {
                strcpy(op, ":="); strcpy(a, "0"); strcpy(b, ""); /* X * 0 => 0 */
                modifie = 1;
            } else if (b_est_nb && atof(b) == 1.0) {       /* X * 1 => X */
                strcpy(op, ":="); strcpy(b, "");
                modifie = 1;
            } else if (a_est_nb && atof(a) == 1.0) {       /* 1 * X => X */
                strcpy(op, ":="); strcpy(a, b); strcpy(b, "");
                modifie = 1;
            } else if (b_est_nb && atof(b) == 2.0) {       /* X * 2 => X + X */
                strcpy(op, "+"); strcpy(b, a);
                modifie = 1;
            } else if (a_est_nb && atof(a) == 2.0) {       /* 2 * X => X + X */
                strcpy(op, "+"); strcpy(a, b); strcpy(b, a);
                modifie = 1;
            } else if (b_est_nb && atof(b) == 4.0) {       /* X * 4 => X * 2 (iter suivante: X+X) */
                strcpy(b, "2");
                modifie = 1;
            } else if (a_est_nb && atof(a) == 4.0) {       /* 4 * X => 2 * X */
                strcpy(a, "2");
                modifie = 1;
            } else if (b_est_nb && atof(b) == 8.0) {       /* X * 8 => X * 4 */
                strcpy(b, "4");
                modifie = 1;
            } else if (a_est_nb && atof(a) == 8.0) {       /* 8 * X => 4 * X */
                strcpy(a, "4");
                modifie = 1;
            } else if (a_est_nb && b_est_nb) {             /* C * C => resultat direct */
                char buf[64]; formater_nombre(atof(a) * atof(b), buf);
                strcpy(op, ":="); strcpy(a, buf); strcpy(b, "");
                modifie = 1;
            }

        } else if (strcmp(op, "/") == 0) {
            if (b_est_nb && atof(b) == 1.0) {              /* X / 1 => X */
                strcpy(op, ":="); strcpy(b, "");
                modifie = 1;
            } else if (!a_est_nb && !b_est_nb && strcmp(a, b) == 0) { /* X / X => 1 */
                strcpy(op, ":="); strcpy(a, "1"); strcpy(b, "");
                modifie = 1;
            } else if (a_est_nb && b_est_nb && atof(b) != 0.0) { /* C / C => resultat direct */
                char buf[64]; formater_nombre(atof(a) / atof(b), buf);
                strcpy(op, ":="); strcpy(a, buf); strcpy(b, "");
                modifie = 1;
            }
        }
    }
    return modifie;
}

/* ==========================================================================
   PASSE 3 : SIMPLIFICATION DES CONSTANTES CHAINEES

   Si deux quadruplets consecutifs effectuent chacun une addition ou
   soustraction avec une constante, et que le temporaire intermediaire
   n'est utilise qu'une seule fois, fusionner les deux en un seul.

   Pattern detecte (T utilise une seule fois) :
     quad[i] : T  = X  +/-  C1
     quad[j] : R  = T  +/-  C2
   =>
     quad[j] : R  = X  +/-  (C1 op C2)  (calcul net a la compilation)
     quad[i] : NOP                       (supprime)

   Exemple :
     t10 = j + 1
     t11 = t10 - 1  =>  t11 = j  (les constantes s'annulent)
   ========================================================================== */
static int simplification_constantes_chainees(void)
{
    int modifie = 0, i;
    for (i = 0; i < qc; i++) {
        char *op1  = quad[i].oper;
        char *a1   = quad[i].op1;
        char *b1   = quad[i].op2;
        char *res1 = quad[i].res;

        /* Le premier quad doit etre +/- avec une constante en op2 */
        if (strcmp(op1, "+") != 0 && strcmp(op1, "-") != 0) continue;
        if (!est_nombre(b1))       continue;
        if (!est_temporaire(res1)) continue;
        if (compter_utilisations(res1) != 1) continue;

        /* Chercher le quadruplet qui utilise res1, sans franchir un saut */
        int j, quad_usage = -1;
        for (j = i + 1; j < qc; j++) {
            if (est_saut(quad[j].oper)) break;
            if (strcmp(quad[j].res, res1) == 0) break;
            if (strcmp(quad[j].op1, res1) == 0 || strcmp(quad[j].op2, res1) == 0) {
                quad_usage = j; break;
            }
        }
        if (quad_usage == -1) continue;

        char *op2 = quad[quad_usage].oper;
        char *a2  = quad[quad_usage].op1;
        char *b2  = quad[quad_usage].op2;

        /* Le second quad doit aussi etre +/- avec constante, T doit etre op1 */
        if (strcmp(op2, "+") != 0 && strcmp(op2, "-") != 0) continue;
        if (strcmp(a2, res1) != 0) continue;
        if (!est_nombre(b2)) continue;

        /* Calculer le decalage net des deux constantes */
        double c1 = atof(b1), c2 = atof(b2), net;
        char op_net[4];

        if      (strcmp(op1,"+")==0 && strcmp(op2,"+")==0) { net = c1+c2; strcpy(op_net, "+"); }
        else if (strcmp(op1,"+")==0 && strcmp(op2,"-")==0) { net = c1-c2; strcpy(op_net,(net>=0)?"+":"-"); if(net<0)net=-net; }
        else if (strcmp(op1,"-")==0 && strcmp(op2,"+")==0) { net = c2-c1; strcpy(op_net,(net>=0)?"+":"-"); if(net<0)net=-net; }
        else                                               { net = c1+c2; strcpy(op_net, "-"); }

        if (net == 0.0) {
            /* Les constantes s'annulent : R = X directement */
            strcpy(quad[quad_usage].oper, ":=");
            strcpy(quad[quad_usage].op1,  a1);
            strcpy(quad[quad_usage].op2,  "");
        } else {
            char buf[64]; formater_nombre(net, buf);
            strcpy(quad[quad_usage].oper, op_net);
            strcpy(quad[quad_usage].op1,  a1);
            strcpy(quad[quad_usage].op2,  buf);
        }
        strcpy(quad[i].oper, "NOP");
        strcpy(quad[i].op1,  "");
        strcpy(quad[i].op2,  "");
        strcpy(quad[i].res,  "");
        modifie = 1;
    }
    return modifie;
}

/* ==========================================================================
   PASSE 4 : PROPAGATION DE COPIE

   Lorsqu'un temporaire dst est affecte a partir d'une valeur src
   (dst := src), remplacer toutes les utilisations suivantes de dst
   par src directement. Cela elimine les copies intermediaires inutiles.

   Conditions d'arret de la propagation :
     - dst est redefini (sa valeur change)
     - src est redefini (la copie n'est plus valide)
     - un saut est rencontre (barriere de flot de controle)

   Exemple :
     t1 := t2              =>  dst=t1, src=t2
     t3 := 4 * t1          =>  t3 := 4 * t2  (t1 remplace par t2)
   ========================================================================== */
static int propagation_copie(void)
{
    int modifie = 0, i;
    for (i = 0; i < qc; i++) {
        if (strcmp(quad[i].oper, ":=") != 0) continue;
        const char *src = quad[i].op1;
        const char *dst = quad[i].res;
        if (!src || !*src || !dst || !*dst) continue;
        if (strcmp(src, dst) == 0) continue;

        /* On ne propage que les temporaires (T0, T1, ...).
           Propager une variable utilisateur est dangereux dans les boucles :
           i:=1 puis i=i+1 => on ne peut pas remplacer i par 1 partout. */
        if (!est_temporaire(dst)) continue;

        int j;
        for (j = i + 1; j < qc; j++) {
            const char *op = quad[j].oper;
            if (est_saut(op)) break;
            if (strcmp(quad[j].res, dst) == 0) break;
            if (strcmp(quad[j].res, src) == 0) break;
            if (strcmp(quad[j].op1, dst) == 0) { strcpy(quad[j].op1, src); modifie = 1; }
            if (strcmp(quad[j].op2, dst) == 0) { strcpy(quad[j].op2, src); modifie = 1; }
        }
    }
    return modifie;
}

/* ==========================================================================
   PASSE 5 : PROPAGATION D'EXPRESSION

   Si un temporaire T est defini par une affectation simple (T := src)
   et n'est utilise qu'une seule fois dans tout le code, substituer
   directement src a la place de T au point d'utilisation, puis
   supprimer le quadruplet de definition (il devient NOP).

   Condition : src ne doit pas avoir ete redefini entre la definition
   de T et son unique utilisation.

   Exemple :
     t1 := expr            =>  T utilise une seule fois
     t3 := 4 * t1          =>  t3 := 4 * expr  (t1 supprime)
   ========================================================================== */
static int propagation_expression(void)
{
    int modifie = 0, i;
    for (i = 0; i < qc; i++) {
        if (strcmp(quad[i].oper, ":=") != 0) continue;
        const char *res = quad[i].res;
        const char *src = quad[i].op1;
        if (!est_temporaire(res)) continue;
        if (!src || !*src) continue;

        if (compter_utilisations(res) != 1) continue;

        int j, quad_usage = -1;
        for (j = i + 1; j < qc; j++) {
            if (est_saut(quad[j].oper)) break;
            if (est_redefini_entre(res, j, j + 1)) break;
            if (strcmp(quad[j].op1, res) == 0 || strcmp(quad[j].op2, res) == 0) {
                quad_usage = j; break;
            }
        }
        if (quad_usage == -1) continue;

        if (!est_nombre(src) && est_redefini_entre(src, i + 1, quad_usage)) continue;

        if (strcmp(quad[quad_usage].op1, res) == 0) { strcpy(quad[quad_usage].op1, src); modifie = 1; }
        if (strcmp(quad[quad_usage].op2, res) == 0) { strcpy(quad[quad_usage].op2, src); modifie = 1; }

        strcpy(quad[i].oper, "NOP");
        strcpy(quad[i].op1,  "");
        strcpy(quad[i].op2,  "");
        strcpy(quad[i].res,  "");
    }
    return modifie;
}

/* ==========================================================================
   PASSE 6 : ELIMINATION DES EXPRESSIONS REDONDANTES (CSE)

   Maintenir une table des expressions deja calculees.
   Si la meme expression apparait a nouveau et que ses operandes
   n'ont pas ete modifies, remplacer le recalcul par une simple
   copie du resultat precedent.

   Exemple :
     t1 := a + b        =>  memorisee dans la table
     t2 := c * d
     t3 := a + b        =>  REDONDANT : remplace par t3 := t1
     t4 := t3 * e       =>  apres propagation : t4 := t1 * e

   Note : un saut invalide toute la table (on ne sait plus quelles
   expressions restent valides apres un branchement).
   ========================================================================== */
#define MAX_EXPR_DISPO 256

typedef struct {
    char operateur[100];
    char operande1[100];
    char operande2[100];
    char resultat[100];
} ExpressionDisponible;

static int elimination_expressions_redondantes(void)
{
    ExpressionDisponible table_expr[MAX_EXPR_DISPO];
    int nb_expr = 0;
    int modifie = 0, i;

    for (i = 0; i < qc; i++) {
        const char *op  = quad[i].oper;
        char       *a   = quad[i].op1;
        char       *b   = quad[i].op2;
        char       *res = quad[i].res;
        int k;

        if (est_saut(op)) { nb_expr = 0; continue; }
        if (strcmp(op, "NOP") == 0) continue;

        int est_arith = strcmp(op,"+")==0 || strcmp(op,"-")==0
                     || strcmp(op,"*")==0 || strcmp(op,"/")==0;
        int est_comp  = strcmp(op,"==")==0 || strcmp(op,"!=")==0
                     || strcmp(op,"<=")==0 || strcmp(op,">=")==0
                     || strcmp(op,"<") ==0 || strcmp(op,">") ==0;

        if (est_arith || est_comp) {
            int idx_trouve = -1;
            for (k = 0; k < nb_expr; k++) {
                if (strcmp(table_expr[k].operateur, op) != 0) continue;
                int direct = strcmp(table_expr[k].operande1, a) == 0
                          && strcmp(table_expr[k].operande2, b) == 0;
                int comm   = (strcmp(op,"+")==0 || strcmp(op,"*")==0)
                          && strcmp(table_expr[k].operande1, b) == 0
                          && strcmp(table_expr[k].operande2, a) == 0;
                if (direct || comm) { idx_trouve = k; break; }
            }

            if (idx_trouve >= 0) {
                strcpy(quad[i].oper, ":=");
                strcpy(quad[i].op1,  table_expr[idx_trouve].resultat);
                strcpy(quad[i].op2,  "");
                modifie = 1;
            } else {
                for (k = 0; k < nb_expr; ) {
                    if (strcmp(table_expr[k].resultat,  res) == 0
                     || strcmp(table_expr[k].operande1, res) == 0
                     || strcmp(table_expr[k].operande2, res) == 0) {
                        memmove(&table_expr[k], &table_expr[k+1],
                                (nb_expr - k - 1) * sizeof(ExpressionDisponible));
                        nb_expr--;
                    } else k++;
                }
                if (nb_expr < MAX_EXPR_DISPO) {
                    strcpy(table_expr[nb_expr].operateur, op);
                    strcpy(table_expr[nb_expr].operande1, a);
                    strcpy(table_expr[nb_expr].operande2, b);
                    strcpy(table_expr[nb_expr].resultat,  res);
                    nb_expr++;
                }
            }

        } else if (strcmp(op, ":=") == 0) {
            for (k = 0; k < nb_expr; ) {
                if (strcmp(table_expr[k].resultat,  res) == 0
                 || strcmp(table_expr[k].operande1, res) == 0
                 || strcmp(table_expr[k].operande2, res) == 0) {
                    memmove(&table_expr[k], &table_expr[k+1],
                            (nb_expr - k - 1) * sizeof(ExpressionDisponible));
                    nb_expr--;
                } else k++;
            }
        }
    }
    return modifie;
}

/* ==========================================================================
   PASSE 7 : ELIMINATION DE VARIABLES MORTES (passe arriere)

   Une affectation "x := ..." sur une variable utilisateur (non temporaire)
   est inutile si x n'est plus jamais lue apres ce point.
   On fait une passe arriere : une variable est "vivante" si elle est
   lue avant d'etre reecrite.

   Les effets de bord (out, input, sauts) sont toujours conserves.
   Les operandes d'un "out" sont marques vivants car ils sont consommes.

   Exemple :
     x := 5              x assigne
     out(y)              x jamais lue apres => affectation inutile
   => x := 5 supprime (devient NOP)

   Note : cette passe complete elimination_code_inutile qui ne
   traite que les temporaires. Ici on traite les variables nommees.
   ========================================================================== */
static int elimination_variables_mortes(void)
{
    /* Table de vivacite indexee par nom de variable */
    char vivantes[MAX_QUADS][100];
    int  nb_vivantes = 0;
    int  modifie = 0;
    int  i, k, m;

    /* Passe arriere : partir de la fin du code */
    for (i = qc - 1; i >= 0; i--) {
        const char *op  = quad[i].oper;
        const char *a   = quad[i].op1;
        const char *b   = quad[i].op2;
        const char *res = quad[i].res;

        if (strcmp(op, "NOP") == 0) continue;

        /* Les effets de bord sont toujours gardes ; leurs operandes sont vivants */
        if (a_effet_de_bord(i)) {
            /* Marquer les operandes comme vivants */
            const char *ops[2] = { a, b };
            for (k = 0; k < 2; k++) {
                const char *v = ops[k];
                if (!v || !*v || est_nombre(v)) continue;
                int deja = 0;
                for (m = 0; m < nb_vivantes; m++)
                    if (strcmp(vivantes[m], v) == 0) { deja = 1; break; }
                if (!deja && nb_vivantes < MAX_QUADS)
                    strcpy(vivantes[nb_vivantes++], v);
            }
            continue;
        }

        /* Pour les affectations simples de variables utilisateur */
        if (res && *res && !est_temporaire(res) && strcmp(op, ":=") == 0) {
            int vivant = 0;
            for (k = 0; k < nb_vivantes; k++)
                if (strcmp(vivantes[k], res) == 0) { vivant = 1; break; }

            if (!vivant) {
                /* Variable morte : supprimer l'affectation */
                strcpy(quad[i].oper, "NOP");
                strcpy(quad[i].op1,  "");
                strcpy(quad[i].op2,  "");
                strcpy(quad[i].res,  "");
                modifie = 1;
                continue;
            }

            /* Variable vivante : la retirer de la table (elle vient d'etre definie) */
            for (k = 0; k < nb_vivantes; k++) {
                if (strcmp(vivantes[k], res) == 0) {
                    memmove(vivantes[k], vivantes[k + 1],
                            (nb_vivantes - k - 1) * sizeof(vivantes[0]));
                    nb_vivantes--;
                    break;
                }
            }
        }

        /* Marquer les operandes du quadruplet courant comme vivants */
        const char *ops[2] = { a, b };
        for (k = 0; k < 2; k++) {
            const char *v = ops[k];
            if (!v || !*v || est_nombre(v)) continue;
            int deja = 0;
            for (m = 0; m < nb_vivantes; m++)
                if (strcmp(vivantes[m], v) == 0) { deja = 1; break; }
            if (!deja && nb_vivantes < MAX_QUADS)
                strcpy(vivantes[nb_vivantes++], v);
        }
    }
    return modifie;
}

/* ==========================================================================
   PASSE 8 : ELIMINATION DE CODE INUTILE

   Supprimer les quadruplets dont le calcul n'a aucun impact sur le
   resultat final du programme. Deux categories sont eliminees :
     1. Les NOP explicites (marques par les passes precedentes)
     2. Les temporaires dont le resultat n'est jamais relu (dead temps)

   Les quadruplets a effet de bord (input, out, BZ, BR) sont
   toujours conserves.

   Exemple :
     T3 := x + y         T3 jamais relu => supprime
     out(z)              conserve (effet de bord)
   ========================================================================== */
static int elimination_code_inutile(void)
{
    int modifie = 0, i;
    int a_supprimer[MAX_QUADS];
    memset(a_supprimer, 0, sizeof(a_supprimer));

    for (i = 0; i < qc; i++) {
        if (a_effet_de_bord(i)) continue;
        const char *op  = quad[i].oper;
        const char *res = quad[i].res;
        if (strcmp(op, "NOP") == 0) { a_supprimer[i] = 1; continue; }
        if (est_temporaire(res) && compter_utilisations(res) == 0)
            a_supprimer[i] = 1;
    }

    /* Build a remapping table: old index -> new index */
    int remap[MAX_QUADS];
    int nouveau_qc = 0;
    for (i = 0; i < qc; i++) {
        remap[i] = a_supprimer[i] ? -1 : nouveau_qc++;
    }

    /* Update all BZ/BR jump targets using the remap table */
    for (i = 0; i < qc; i++) {
        if (a_supprimer[i]) continue;
        if (strcmp(quad[i].oper, "BZ") == 0 || strcmp(quad[i].oper, "BR") == 0) {
            int old_target = atoi(quad[i].res);
            if (old_target >= 0 && old_target < qc) {
                /* If the target itself was removed, forward to next kept quad */
                int new_target = old_target;
                while (new_target < qc && remap[new_target] == -1)
                    new_target++;
                char buf[20];
                sprintf(buf, "%d", (new_target < qc) ? remap[new_target] : nouveau_qc);
                strcpy(quad[i].res, buf);
            }
        }
    }

    /* Compact the array */
    nouveau_qc = 0;
    for (i = 0; i < qc; i++) {
        if (!a_supprimer[i]) {
            if (i != nouveau_qc) quad[nouveau_qc] = quad[i];
            nouveau_qc++;
        } else modifie = 1;
    }
    qc = nouveau_qc;
    return modifie;
}

/* ==========================================================================
   OPTIMISER_QUADRUPLETS : point d'entree de l'optimisation

   Les 8 passes sont appliquees en boucle jusqu'a point fixe
   (aucune modification produite). La boucle garantit que des
   simplifications en cascade (ex: X*8 -> X*4 -> X*2 -> X+X)
   sont toutes appliquees.

   Ordre des passes a chaque iteration :
     1. Propagation constantes symboliques  (const N=5 => remplace N par 5)
     2. Simplification algebrique           (X*1=X, X+0=X, X*2=X+X, C op C)
     3. Simplification constantes chainees  (t=j+1; r=t-1 => r=j)
     4. Propagation de copie               (y:=x => remplace y par x apres)
     5. Propagation d'expression            (T:=src utilise 1 fois => inline)
     6. Elimination expressions redondantes (a+b calcule 2 fois => reutiliser)
     7. Elimination variables mortes        (x:=5 mais x jamais relu => NOP)
     8. Elimination de code inutile         (NOP et temporaires morts => retirer)
   ========================================================================== */
void optimiser_quadruplets(void)
{
    int modifie, iterations = 0;

    printf(CYN "\n============================================================\n" RESET);
    printf(CYN " OPTIMISATION DU CODE INTERMEDIAIRE (8 passes)\n" RESET);
    printf(CYN "============================================================\n" RESET);

    do {
        /* Passe 1 : substituer les constantes nommees par leur valeur */
        modifie  = propagation_constantes_symboliques();

        /* Passe 2 : simplifier les operations algebriques triviales */
        modifie |= simplification_algebrique();

        /* Passe 3 : fusionner deux +/- de constantes consecutives */
        modifie |= simplification_constantes_chainees();

        /* Passe 4 : propager les copies (dst:=src => remplacer dst par src) */
        modifie |= propagation_copie();

        /* Passe 5 : propager les expressions a usage unique vers leur site */
        modifie |= propagation_expression();

        /* Passe 6 : eliminer les recalculs d'expressions deja disponibles */
        modifie |= elimination_expressions_redondantes();

        /* Passe 7 : supprimer les affectations a des variables jamais relues */
        modifie |= elimination_variables_mortes();

        /* Passe 8 : supprimer les NOP et les temporaires morts */
        modifie |= elimination_code_inutile();

        iterations++;
    } while (modifie && iterations < 30);

    printf(GRN "[Optimisation terminee en %d iteration(s), 8 passes]\n" RESET, iterations);
}
