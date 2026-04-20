#include "opt.h"
#include "quad.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* =========================================================
   COULEURS (coherence avec le reste du projet)
   ========================================================= */
#define GRN  "\033[1;32m"
#define CYN  "\033[1;36m"
#define YLW  "\033[1;33m"
#define RESET "\033[0m"

/* =========================================================
   UTILITAIRES
   ========================================================= */

/* Retourne 1 si la chaine represente un nombre (entier ou flottant). */
static int is_num(const char *s) // this verify if s is a valid number (integer or floating-point)
{
    if (!s || !*s) return 0;
    const char *p = s;
    if (*p == '-' || *p == '+') p++;
    if (!*p) return 0;
    int has_dot = 0;
    for (; *p; p++) {
        if (*p == '.') {
            if (has_dot) return 0;
            has_dot = 1;
        } else if (!isdigit((unsigned char)*p)) return 0;
    }
    return 1;
}

/* Retourne 1 si s est un temporaire genere (T0, T1, ...). */
static int is_temp(const char *s)
{
    int i;
    if (!s || s[0] != 'T') return 0;
    for (i = 1; s[i]; i++)
        if (!isdigit((unsigned char)s[i])) return 0;
    return (i > 1);
}

/* Retourne 1 si l'operateur est un saut => barriere de flot de controle. */
static int is_branch(const char *op)
{
    return (strcmp(op, "BZ") == 0 || strcmp(op, "BR") == 0);
}

/* Retourne 1 si le quadruplet i a des effets de bord (ne doit pas etre supprime). */
static int has_sideeffect(int i)
{
    const char *op = quad[i].oper;
    return is_branch(op)
        || strcmp(op, "input") == 0
        || strcmp(op, "out")   == 0;
}

/* Compte le nombre total d'utilisations de `var` comme operande (op1 ou op2)
   dans l'ensemble de la table des quadruplets. */
static int count_uses(const char *var)
{
    int i, n = 0;
    if (!var || !*var) return 0;
    for (i = 0; i < qc; i++) {
        if (strcmp(quad[i].op1, var) == 0) n++;
        if (strcmp(quad[i].op2, var) == 0) n++;
    }
    return n;
}

/* Retourne 1 si `var` est re-definie (apparait en res, hors sauts)
   dans l'intervalle [from, to[. */
static int redefined_in(const char *var, int from, int to)
{
    int i;
    for (i = from; i < to; i++) {
        if (is_branch(quad[i].oper)) continue;
        if (strcmp(quad[i].res, var) == 0) return 1;
    }
    return 0;
}

/* Formate un nombre double en chaine (entier si possible, sinon %g). */
static void fmt_num(double v, char *buf)
{
    if (v == (long)v) sprintf(buf, "%ld", (long)v);
    else              sprintf(buf, "%g",  v);
}

/* =========================================================
   PASSE 1 – Simplification algebrique (quadruplet unique)

   Regles appliquees :
     X + 0,  0 + X             => X
     X - 0,  X - X             => X  /  0
     X * 0,  0 * X             => 0
     X * 1,  1 * X             => X
     X * 2,  2 * X             => X + X
     X / 1                     => X
     C1 op C2  (deux constantes) => resultat pliant
   ========================================================= */
static int pass_algebraic(void)
{
    int changed = 0, i;
    for (i = 0; i < qc; i++) {
        char *op  = quad[i].oper;
        char *a   = quad[i].op1;
        char *b   = quad[i].op2;
        int an = is_num(a), bn = is_num(b);

        if (strcmp(op, "+") == 0) {
            if (bn && atof(b) == 0.0) {                    /* X + 0 => X */
                strcpy(op, ":="); strcpy(b, "");
                changed = 1;
            } else if (an && atof(a) == 0.0) {             /* 0 + X => X */
                strcpy(op, ":="); strcpy(a, b); strcpy(b, "");
                changed = 1;
            } else if (an && bn) {                         /* C + C */
                char buf[64]; fmt_num(atof(a) + atof(b), buf);
                strcpy(op, ":="); strcpy(a, buf); strcpy(b, "");
                changed = 1;
            }

        } else if (strcmp(op, "-") == 0) {
            if (bn && atof(b) == 0.0) {                    /* X - 0 => X */
                strcpy(op, ":="); strcpy(b, "");
                changed = 1;
            } else if (!an && !bn && strcmp(a, b) == 0) {  /* X - X => 0 */
                strcpy(op, ":="); strcpy(a, "0"); strcpy(b, "");
                changed = 1;
            } else if (an && bn) {                         /* C - C */
                char buf[64]; fmt_num(atof(a) - atof(b), buf);
                strcpy(op, ":="); strcpy(a, buf); strcpy(b, "");
                changed = 1;
            }

        } else if (strcmp(op, "*") == 0) {
            if ((an && atof(a) == 0.0) || (bn && atof(b) == 0.0)) { /* X*0 */
                strcpy(op, ":="); strcpy(a, "0"); strcpy(b, "");
                changed = 1;
            } else if (bn && atof(b) == 1.0) {             /* X * 1 */
                strcpy(op, ":="); strcpy(b, "");
                changed = 1;
            } else if (an && atof(a) == 1.0) {             /* 1 * X */
                strcpy(op, ":="); strcpy(a, b); strcpy(b, "");
                changed = 1;
            } else if (bn && atof(b) == 2.0) {             /* X * 2 => X+X */
                strcpy(op, "+"); strcpy(b, a);
                changed = 1;
            } else if (an && atof(a) == 2.0) {             /* 2 * X => X+X */
                strcpy(op, "+"); strcpy(a, b);
                changed = 1;
            } else if (an && bn) {                         /* C * C */
                char buf[64]; fmt_num(atof(a) * atof(b), buf);
                strcpy(op, ":="); strcpy(a, buf); strcpy(b, "");
                changed = 1;
            }

        } else if (strcmp(op, "/") == 0) {
            if (bn && atof(b) == 1.0) {                    /* X / 1 */
                strcpy(op, ":="); strcpy(b, "");
                changed = 1;
            } else if (an && bn && atof(b) != 0.0) {       /* C / C */
                char buf[64]; fmt_num(atof(a) / atof(b), buf);
                strcpy(op, ":="); strcpy(a, buf); strcpy(b, "");
                changed = 1;
            }
        }
    }
    return changed;
}

/* =========================================================
   PASSE 2 – Peephole : annulation de constantes chainees

   Pattern detecte (T utilise une seule fois) :
     quad[i] : T  = X  +/-  C1
     quad[j] : R  = T  +/-  C2   (j > i, T est op1)
   => collapse en une seule operation R = X +/- net
      quad[i] devient NOP.

   Exemples du cours :
     t10 = j + 1 ; t11 = t10 - 1  =>  t11 = j      (j+1-1=j)
     t16 = j + 1 ; t17 = t16 - 1  =>  t17 = j
   ========================================================= */
static int pass_peephole(void)
{
    int changed = 0, i;
    for (i = 0; i < qc; i++) {
        char *op1  = quad[i].oper;
        char *a1   = quad[i].op1;
        char *b1   = quad[i].op2;
        char *res1 = quad[i].res;

        /* Premier quad : T = X +/- C (C est op2) */
        if (strcmp(op1, "+") != 0 && strcmp(op1, "-") != 0) continue;
        if (!is_num(b1))  continue;
        if (!is_temp(res1)) continue;
        if (count_uses(res1) != 1) continue;  /* T utilise exactement une fois */

        /* Cherche le quad j > i qui utilise res1, sans croiser une barriere */
        int j, found = -1;
        for (j = i + 1; j < qc; j++) {
            if (is_branch(quad[j].oper)) break;
            /* Si res1 est redefini avant son usage => abandon */
            if (!is_branch(quad[j].oper) && strcmp(quad[j].res, res1) == 0) break;
            if (strcmp(quad[j].op1, res1) == 0 || strcmp(quad[j].op2, res1) == 0) {
                found = j; break;
            }
        }
        if (found == -1) continue;

        char *op2 = quad[found].oper;
        char *a2  = quad[found].op1;
        char *b2  = quad[found].op2;

        /* Second quad : R = T +/- C2, T doit etre op1 */
        if (strcmp(op2, "+") != 0 && strcmp(op2, "-") != 0) continue;
        if (strcmp(a2, res1) != 0) continue;   /* T doit etre l'operande gauche */
        if (!is_num(b2)) continue;

        /* Calcul du decalage net */
        double c1 = atof(b1), c2 = atof(b2), net;
        char net_op[4];

        if (strcmp(op1, "+") == 0 && strcmp(op2, "+") == 0)      { net = c1 + c2; strcpy(net_op, "+"); }
        else if (strcmp(op1, "+") == 0 && strcmp(op2, "-") == 0) { net = c1 - c2; strcpy(net_op, (net >= 0) ? "+" : "-"); if (net < 0) net = -net; }
        else if (strcmp(op1, "-") == 0 && strcmp(op2, "+") == 0) { net = c2 - c1; strcpy(net_op, (net >= 0) ? "+" : "-"); if (net < 0) net = -net; }
        else /* - - */                                             { net = c1 + c2; strcpy(net_op, "-"); }

        /* Mise a jour de quad[found] */
        if (net == 0.0) {
            strcpy(quad[found].oper, ":=");
            strcpy(quad[found].op1,  a1);
            strcpy(quad[found].op2,  "");
        } else {
            char buf[64]; fmt_num(net, buf);
            strcpy(quad[found].oper, net_op);
            strcpy(quad[found].op1,  a1);
            strcpy(quad[found].op2,  buf);
        }
        /* quad[i] devient NOP */
        strcpy(quad[i].oper, "NOP");
        strcpy(quad[i].op1,  "");
        strcpy(quad[i].op2,  "");
        strcpy(quad[i].res,  "");
        changed = 1;
    }
    return changed;
}

/* =========================================================
   PASSE 3 – Propagation de copie

   Pour chaque affectation  y := x  : substitue y par x
   dans les op1/op2 de tous les quadruplets suivants,
   jusqu'a ce que y ou x soit redefini ou qu'une barriere
   soit rencontree.
   ========================================================= */
static int pass_copy_prop(void)
{
    int changed = 0, i;
    for (i = 0; i < qc; i++) {
        if (strcmp(quad[i].oper, ":=") != 0) continue;
        const char *src = quad[i].op1;
        const char *dst = quad[i].res;
        if (!src || !*src || !dst || !*dst) continue;
        if (strcmp(src, dst) == 0) continue;  /* copie triviale */

        int j;
        for (j = i + 1; j < qc; j++) {
            const char *op = quad[j].oper;
            if (is_branch(op)) break;
            /* Arret si dst ou src sont redefinis */
            if (strcmp(quad[j].res, dst) == 0) break;
            if (strcmp(quad[j].res, src) == 0) break;
            /* Remplacement des utilisations de dst */
            if (strcmp(quad[j].op1, dst) == 0) { strcpy(quad[j].op1, src); changed = 1; }
            if (strcmp(quad[j].op2, dst) == 0) { strcpy(quad[j].op2, src); changed = 1; }
        }
    }
    return changed;
}

/* =========================================================
   PASSE 4 – Propagation d'expression

   Si un temporaire T est calcule par une affectation simple
   (:= src) et utilise exactement une fois, on remplace son
   utilisation par src et on marque le quad comme NOP.
   (Cas ":=" uniquement, les expressions binaires etant
   traitees par les autres passes.)
   ========================================================= */
static int pass_expr_prop(void)
{
    int changed = 0, i;
    for (i = 0; i < qc; i++) {
        /* Seulement les affectations simples vers un temporaire */
        if (strcmp(quad[i].oper, ":=") != 0) continue;
        const char *res = quad[i].res;
        const char *src = quad[i].op1;
        if (!is_temp(res)) continue;
        if (!src || !*src)  continue;

        /* T doit etre utilise exactement une fois */
        if (count_uses(res) != 1) continue;

        /* Trouver le site d'utilisation */
        int j, site = -1;
        for (j = i + 1; j < qc; j++) {
            if (is_branch(quad[j].oper)) break;
            if (redefined_in(res, j, j + 1)) break;
            if (strcmp(quad[j].op1, res) == 0 || strcmp(quad[j].op2, res) == 0) {
                site = j; break;
            }
        }
        if (site == -1) continue;

        /* src ne doit pas avoir ete redefini entre i+1 et site */
        if (!is_num(src) && redefined_in(src, i + 1, site)) continue;

        /* Substitution */
        if (strcmp(quad[site].op1, res) == 0) { strcpy(quad[site].op1, src); changed = 1; }
        if (strcmp(quad[site].op2, res) == 0) { strcpy(quad[site].op2, src); changed = 1; }

        /* Marquer le quad d'affectation comme NOP */
        strcpy(quad[i].oper, "NOP");
        strcpy(quad[i].op1,  "");
        strcpy(quad[i].op2,  "");
        strcpy(quad[i].res,  "");
    }
    return changed;
}

/* =========================================================
   PASSE 5 – Elimination des expressions redondantes (CSE)

   Maintient un ensemble d'expressions disponibles (oper,op1,op2).
   Si l'expression courante a deja ete calculee et que ses
   operandes n'ont pas change => remplace par une copie du
   resultat precedent.
   L'ensemble est vide apres une barriere.
   ========================================================= */
#define MAX_AVAIL 256

typedef struct {
    char oper[100];
    char op1[100];
    char op2[100];
    char res[100];
} AvailExpr;

static int pass_cse(void)
{
    AvailExpr avail[MAX_AVAIL];
    int navail = 0;
    int changed = 0, i;

    for (i = 0; i < qc; i++) {
        const char *op  = quad[i].oper;
        char       *a   = quad[i].op1;
        char       *b   = quad[i].op2;
        char       *res = quad[i].res;
        int k;

        /* Barriere : vider le stock d'expressions disponibles */
        if (is_branch(op)) { navail = 0; continue; }

        /* NOP : pas d'interet */
        if (strcmp(op, "NOP") == 0) continue;

        int is_arith = strcmp(op,"+") == 0 || strcmp(op,"-") == 0
                    || strcmp(op,"*") == 0 || strcmp(op,"/") == 0;
        int is_cmp   = strcmp(op,"==") == 0 || strcmp(op,"!=") == 0
                    || strcmp(op,"<=") == 0 || strcmp(op,">=") == 0
                    || strcmp(op,"<")  == 0 || strcmp(op,">")  == 0;

        if (is_arith || is_cmp) {
            /* Rechercher l'expression dans le stock */
            int found = -1;
            for (k = 0; k < navail; k++) {
                if (strcmp(avail[k].oper, op) != 0) continue;
                int direct = strcmp(avail[k].op1, a) == 0
                          && strcmp(avail[k].op2, b) == 0;
                /* Commutativite pour + et * */
                int comm = (strcmp(op,"+") == 0 || strcmp(op,"*") == 0)
                        && strcmp(avail[k].op1, b) == 0
                        && strcmp(avail[k].op2, a) == 0;
                if (direct || comm) { found = k; break; }
            }

            if (found >= 0) {
                /* Expression deja disponible : remplacer par copie */
                strcpy(quad[i].oper, ":=");
                strcpy(quad[i].op1,  avail[found].res);
                strcpy(quad[i].op2,  "");
                changed = 1;
                /* Pas besoin d'ajouter dans avail (on vient de la remplacer) */
            } else {
                /* Invalider les entrees dont res figure comme operande */
                for (k = 0; k < navail; ) {
                    if (strcmp(avail[k].res, res) == 0
                     || strcmp(avail[k].op1, res) == 0
                     || strcmp(avail[k].op2, res) == 0) {
                        memmove(&avail[k], &avail[k+1],
                                (navail - k - 1) * sizeof(AvailExpr));
                        navail--;
                    } else k++;
                }
                /* Ajouter l'expression au stock */
                if (navail < MAX_AVAIL) {
                    strcpy(avail[navail].oper, op);
                    strcpy(avail[navail].op1,  a);
                    strcpy(avail[navail].op2,  b);
                    strcpy(avail[navail].res,  res);
                    navail++;
                }
            }

        } else if (strcmp(op, ":=") == 0) {
            /* Affectation : invalider les expressions utilisant res */
            for (k = 0; k < navail; ) {
                if (strcmp(avail[k].res, res) == 0
                 || strcmp(avail[k].op1, res) == 0
                 || strcmp(avail[k].op2, res) == 0) {
                    memmove(&avail[k], &avail[k+1],
                            (navail - k - 1) * sizeof(AvailExpr));
                    navail--;
                } else k++;
            }
        }
    }
    return changed;
}

/* =========================================================
   PASSE 6 – Elimination de code inutile (dead code)

   Supprime :
     - les NOP explicites
     - tout quadruplet dont le resultat est un temporaire
       jamais utilise (count_uses == 0) et sans effet de bord.
   ========================================================= */
static int pass_dead_code(void)
{
    int changed = 0, i;
    int dead[MAX_QUADS];
    memset(dead, 0, sizeof(dead));

    for (i = 0; i < qc; i++) {
        if (has_sideeffect(i)) continue;
        const char *op  = quad[i].oper;
        const char *res = quad[i].res;

        if (strcmp(op, "NOP") == 0) { dead[i] = 1; continue; }

        /* Temporaire dont la valeur n'est jamais lue */
        if (is_temp(res) && count_uses(res) == 0)
            dead[i] = 1;
    }

    /* Compactage du tableau */
    int new_qc = 0;
    for (i = 0; i < qc; i++) {
        if (!dead[i]) {
            if (i != new_qc) quad[new_qc] = quad[i];
            new_qc++;
        } else changed = 1;
    }
    qc = new_qc;
    return changed;
}

/* =========================================================
   POINT D'ENTREE : boucle a point fixe sur toutes les passes
   ========================================================= */
void optimize_quads(void)
{
    int changed, iterations = 0;

    printf(CYN "\n============================================================\n" RESET);
    printf(CYN " OPTIMISATION DU CODE INTERMEDIAIRE\n" RESET);
    printf(CYN "============================================================\n" RESET);

    do {
        changed  = pass_algebraic();
        changed |= pass_peephole();
        changed |= pass_copy_prop();
        changed |= pass_expr_prop();
        changed |= pass_cse();
        changed |= pass_dead_code();
        iterations++;
    } while (changed && iterations < 30);

    printf(GRN "[Optimisation terminee en %d iteration(s)]\n" RESET, iterations);
    printf(CYN "============================================================\n" RESET);
}
