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


/* -------------------------------------------------------------------------
   UTILITAIRES INTERNES
   ------------------------------------------------------------------------- */

/*
 est_nombre : verifie si la chaine s represente un nombre valide
              (entier ou flottant, avec signe optionnel).
 */
static int est_nombre(const char *s)
{
    if (!s || !*s) return 0;
    const char *p = s;
    if (*p == '-' || *p == '+') p++;
    if (!*p) return 0;
    int a_point = 0;
    for (; *p; p++) {
        if (*p == '.') {
            if (a_point) return 0;
            a_point = 1;
        } else if (!isdigit((unsigned char)*p)) return 0;
    }
    return 1;
}

/*
 est_temporaire : verifie si s est un temporaire T0, T1, T2, ...
 */
static int est_temporaire(const char *s)
{
    int i;
    if (!s || s[0] != 'T') return 0;
    for (i = 1; s[i]; i++)
        if (!isdigit((unsigned char)s[i])) return 0;
    return (i > 1);
}

/*
 est_saut : verifie si l'operateur est un saut (BZ ou BR).
 */
static int est_saut(const char *op)
{
    return (strcmp(op, "BZ") == 0 || strcmp(op, "BR") == 0);
}

/*
 a_effet_de_bord : effets observables (saut, entree, sortie).
 */
static int a_effet_de_bord(int i)
{
    const char *op = quad[i].oper;
    return est_saut(op)
        || strcmp(op, "input") == 0
        || strcmp(op, "out")   == 0;
}

/*
 compter_utilisations : nombre d'apparitions de var en op1 ou op2.
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
 est_redefini_entre : verifie si var est redefinie entre debut et fin.
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
 formater_nombre : double -> chaine sans decimale inutile.
 */
static void formater_nombre(double v, char *buf)
{
    if (v == (long)v) sprintf(buf, "%ld", (long)v);
    else              sprintf(buf, "%g",  v);
}

/* =========================================================================
   PASSE 1 : PROPAGATION DE CONSTANTES SYMBOLIQUES
   ========================================================================= */
static int propagation_constantes_symboliques(void)
{
    int modifie = 0, i;
    for (i = 0; i < qc; i++) {
        char *champs[2] = { quad[i].op1, quad[i].op2 };
        int c;
        for (c = 0; c < 2; c++) {
            char *nom = champs[c];
            if (!nom || !*nom || est_nombre(nom)) continue;
            if (!ts_est_constante(nom)) continue;
            const char *val = ts_get_val(nom);
            if (!val || !*val || !est_nombre(val)) continue;
            strcpy(nom, val);
            modifie = 1;
        }
    }
    return modifie;
}

/* =========================================================================
   PASSE 2 : SIMPLIFICATION ALGEBRIQUE
   ========================================================================= */
static int simplification_algebrique(void)
{
    int modifie = 0, i;
    for (i = 0; i < qc; i++) {
        char *op = quad[i].oper;
        char *a  = quad[i].op1;
        char *b  = quad[i].op2;
        int a_est_nb = est_nombre(a), b_est_nb = est_nombre(b);

        if (strcmp(op, "+") == 0) {
            if (b_est_nb && atof(b) == 0.0) {
                strcpy(op, ":="); strcpy(b, "");
                modifie = 1;
            } else if (a_est_nb && atof(a) == 0.0) {
                strcpy(op, ":="); strcpy(a, b); strcpy(b, "");
                modifie = 1;
            } else if (a_est_nb && b_est_nb) {
                char buf[64]; formater_nombre(atof(a) + atof(b), buf);
                strcpy(op, ":="); strcpy(a, buf); strcpy(b, "");
                modifie = 1;
            }

        } else if (strcmp(op, "-") == 0) {
            if (b_est_nb && atof(b) == 0.0) {
                strcpy(op, ":="); strcpy(b, "");
                modifie = 1;
            } else if (!a_est_nb && !b_est_nb && strcmp(a, b) == 0) {
                strcpy(op, ":="); strcpy(a, "0"); strcpy(b, "");
                modifie = 1;
            } else if (a_est_nb && b_est_nb) {
                char buf[64]; formater_nombre(atof(a) - atof(b), buf);
                strcpy(op, ":="); strcpy(a, buf); strcpy(b, "");
                modifie = 1;
            }

        } else if (strcmp(op, "*") == 0) {
            if ((a_est_nb && atof(a) == 0.0) || (b_est_nb && atof(b) == 0.0)) {
                strcpy(op, ":="); strcpy(a, "0"); strcpy(b, "");
                modifie = 1;
            } else if (b_est_nb && atof(b) == 1.0) {
                strcpy(op, ":="); strcpy(b, "");
                modifie = 1;
            } else if (a_est_nb && atof(a) == 1.0) {
                strcpy(op, ":="); strcpy(a, b); strcpy(b, "");
                modifie = 1;
            } else if (b_est_nb && atof(b) == 2.0) {
                /* X * 2 => X + X  (b devient une copie de a) */
                strcpy(op, "+"); strcpy(b, a);
                modifie = 1;
            } else if (a_est_nb && atof(a) == 2.0) {
                /* 2 * X => X + X
                   FIX bug original : on copie d'abord a dans un tampon,
                   puis on ecrit a=X et b=X sans ecraser la source. */
                char tmp[100];
                strcpy(tmp, b);          /* sauvegarde de X */
                strcpy(op, "+");
                strcpy(a, tmp);          /* a = X */
                strcpy(b, tmp);          /* b = X */
                modifie = 1;
            } else if (b_est_nb && atof(b) == 4.0) {
                strcpy(b, "2"); modifie = 1;
            } else if (a_est_nb && atof(a) == 4.0) {
                strcpy(a, "2"); modifie = 1;
            } else if (b_est_nb && atof(b) == 8.0) {
                strcpy(b, "4"); modifie = 1;
            } else if (a_est_nb && atof(a) == 8.0) {
                strcpy(a, "4"); modifie = 1;
            } else if (a_est_nb && b_est_nb) {
                char buf[64]; formater_nombre(atof(a) * atof(b), buf);
                strcpy(op, ":="); strcpy(a, buf); strcpy(b, "");
                modifie = 1;
            }

        } else if (strcmp(op, "/") == 0) {
            if (b_est_nb && atof(b) == 1.0) {
                strcpy(op, ":="); strcpy(b, "");
                modifie = 1;
            } else if (!a_est_nb && !b_est_nb && strcmp(a, b) == 0) {
                strcpy(op, ":="); strcpy(a, "1"); strcpy(b, "");
                modifie = 1;
            } else if (a_est_nb && b_est_nb && atof(b) != 0.0) {
                char buf[64]; formater_nombre(atof(a) / atof(b), buf);
                strcpy(op, ":="); strcpy(a, buf); strcpy(b, "");
                modifie = 1;
            }
        }
    }
    return modifie;
}

/* =========================================================================
   PASSE 3 : SIMPLIFICATION DES CONSTANTES CHAINEES
   ========================================================================= */
static int simplification_constantes_chainees(void)
{
    int modifie = 0, i;
    for (i = 0; i < qc; i++) {
        char *op1  = quad[i].oper;
        char *a1   = quad[i].op1;
        char *b1   = quad[i].op2;
        char *res1 = quad[i].res;

        if (strcmp(op1, "+") != 0 && strcmp(op1, "-") != 0) continue;
        if (!est_nombre(b1))       continue;
        if (!est_temporaire(res1)) continue;
        if (compter_utilisations(res1) != 1) continue;

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

        if (strcmp(op2, "+") != 0 && strcmp(op2, "-") != 0) continue;
        if (strcmp(a2, res1) != 0) continue;
        if (!est_nombre(b2)) continue;

        double c1 = atof(b1), c2 = atof(b2), net;
        char op_net[4];

        if      (strcmp(op1,"+")==0 && strcmp(op2,"+")==0) { net = c1+c2; strcpy(op_net, "+"); }
        else if (strcmp(op1,"+")==0 && strcmp(op2,"-")==0) { net = c1-c2; strcpy(op_net,(net>=0)?"+":"-"); if(net<0)net=-net; }
        else if (strcmp(op1,"-")==0 && strcmp(op2,"+")==0) { net = c2-c1; strcpy(op_net,(net>=0)?"+":"-"); if(net<0)net=-net; }
        else                                               { net = c1+c2; strcpy(op_net, "-"); }

        if (net == 0.0) {
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

/* =========================================================================
   PASSE 4 : PROPAGATION DE COPIE
   ========================================================================= */
static int propagation_copie(void)
{
    int modifie = 0, i;
    for (i = 0; i < qc; i++) {
        if (strcmp(quad[i].oper, ":=") != 0) continue;
        const char *src = quad[i].op1;
        const char *dst = quad[i].res;
        if (!src || !*src || !dst || !*dst) continue;
        if (strcmp(src, dst) == 0) continue;
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

/* =========================================================================
   PASSE 5 : PROPAGATION D'EXPRESSION
   ========================================================================= */
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

/* =========================================================================
   PASSE 6 : ELIMINATION DES EXPRESSIONS REDONDANTES (CSE)
   ========================================================================= */
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

/* =========================================================================
   PASSE 7 : ELIMINATION DE VARIABLES MORTES — VERSION CORRIGEE

   PROBLEME DE L'ANCIENNE VERSION :
   L'analyse de vivacite en passe arriere LINEAIRE est incorrecte en
   presence de boucles. Le schema suivant tue systematiquement les
   increments de boucle :

       [i := 0]                  <- init
       [cond] T = i <= N         <- scanne EN DERNIER dans le sens arriere
       [BZ T -> exit]
       ...corps...
       [T2 = i + 1]
       [i := T2]                 <- scanne EN PREMIER ; i pas encore "vivant"
       [BR -> cond]              ->  i est marque mort et SUPPRIME (BUG)

   SOLUTION : on ne supprime comme "mort" que les variables utilisateur
   qui ne sont plus lues NULLE PART dans tout le programme (analyse
   globale simple), en excluant toutes les variables qui apparaissent
   dans au moins un operande de quadruplet non-NOP.  C'est une
   approximation conservatrice (pas de faux positifs) sans danger pour
   les boucles.
   ========================================================================= */
static int elimination_variables_mortes(void)
{
    int modifie = 0, i;

    /*
     * Construire l'ensemble des variables utilisateur lues au moins
     * une fois comme operande dans tout le code intermediaire.
     * Une variable presente dans cet ensemble ne peut jamais etre
     * eliminee, quelle que soit sa position.
     */
    char lues[MAX_QUADS][100];
    int  nb_lues = 0;

    for (i = 0; i < qc; i++) {
        const char *ops[2] = { quad[i].op1, quad[i].op2 };
        int k;
        for (k = 0; k < 2; k++) {
            const char *v = ops[k];
            if (!v || !*v || est_nombre(v) || est_temporaire(v)) continue;
            /* variable utilisateur utilisee en lecture */
            int deja = 0;
            int m;
            for (m = 0; m < nb_lues; m++)
                if (strcmp(lues[m], v) == 0) { deja = 1; break; }
            if (!deja && nb_lues < MAX_QUADS)
                strcpy(lues[nb_lues++], v);
        }
        /* Les operandes de BZ (cible numerique dans res) ne sont pas des vars */
    }

    /*
     * Supprimer uniquement les affectations ":=" vers des variables
     * utilisateur qui ne sont JAMAIS lues nulle part.
     * Cela elimine les affectations vraiment inutiles (ex: variable
     * declaree mais jamais utilisee) sans toucher aux variables de boucle.
     */
    for (i = 0; i < qc; i++) {
        if (a_effet_de_bord(i)) continue;
        if (strcmp(quad[i].oper, ":=") != 0) continue;
        const char *res = quad[i].res;
        if (!res || !*res || est_temporaire(res)) continue;

        /* Cette variable est-elle lue quelque part ? */
        int lue = 0;
        int k;
        for (k = 0; k < nb_lues; k++)
            if (strcmp(lues[k], res) == 0) { lue = 1; break; }

        if (!lue) {
            strcpy(quad[i].oper, "NOP");
            strcpy(quad[i].op1,  "");
            strcpy(quad[i].op2,  "");
            strcpy(quad[i].res,  "");
            modifie = 1;
        }
    }
    return modifie;
}

/* =========================================================================
   PASSE 8 : ELIMINATION DE CODE INUTILE
   + REMAPPAGE DES CIBLES BZ/BR (BUG 1 CORRIGE)

   Apres compaction du tableau, tous les indices absolus stockes dans
   le champ res des BZ/BR deviennent invalides.  On construit une table
   de remappage old_idx -> new_idx avant de compacter, puis on met a
   jour chaque cible de saut.
   ========================================================================= */
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

    /* ------------------------------------------------------------------
       ETAPE 1 : construire la table de remappage old -> new
       ------------------------------------------------------------------ */
    int remap[MAX_QUADS];
    int nouveau_qc = 0;
    for (i = 0; i < qc; i++) {
        if (a_supprimer[i])
            remap[i] = -1;          /* quad supprime : n'a plus d'indice */
        else
            remap[i] = nouveau_qc++;
    }

    /* ------------------------------------------------------------------
       ETAPE 2 : mettre a jour les cibles BZ / BR
       Pour un saut vers old_target :
         - si old_target est supprime, avancer au premier quad conserve
           (le saut doit atterrir "apres" le NOP supprime)
         - sinon utiliser remap[old_target]
       Si old_target >= qc c'est la cible "sortie programme" : on la
       laisse pointer vers nouveau_qc (label de fin genere par asm8086).
       ------------------------------------------------------------------ */
    for (i = 0; i < qc; i++) {
        if (a_supprimer[i]) continue;
        if (!est_saut(quad[i].oper)) continue;

        int old_target = atoi(quad[i].res);

        if (old_target < 0 || old_target >= qc) {
            /* Cible hors tableau = label de fin du programme */
            char buf[20];
            sprintf(buf, "%d", nouveau_qc);
            strcpy(quad[i].res, buf);
            continue;
        }

        /* Avancer si le quad cible lui-meme est supprime */
        int t = old_target;
        while (t < qc && a_supprimer[t])
            t++;

        int new_target = (t < qc) ? remap[t] : nouveau_qc;
        char buf[20];
        sprintf(buf, "%d", new_target);
        strcpy(quad[i].res, buf);
    }

    /* ------------------------------------------------------------------
       ETAPE 3 : compacter le tableau
       ------------------------------------------------------------------ */
    nouveau_qc = 0;
    for (i = 0; i < qc; i++) {
        if (!a_supprimer[i]) {
            if (i != nouveau_qc) quad[nouveau_qc] = quad[i];
            nouveau_qc++;
        } else {
            modifie = 1;
        }
    }
    qc = nouveau_qc;
    return modifie;
}

/* =========================================================================
   OPTIMISER_QUADRUPLETS : point d'entree
   ========================================================================= */
void optimiser_quadruplets(void)
{
    int modifie, iterations = 0;

    printf(CYN "\n============================================================\n" RESET);
    printf(CYN " OPTIMISATION DU CODE INTERMEDIAIRE (8 passes)\n" RESET);
    printf(CYN "============================================================\n" RESET);

    do {
        modifie  = propagation_constantes_symboliques();
        modifie |= simplification_algebrique();
        modifie |= simplification_constantes_chainees();
        modifie |= propagation_copie();
        modifie |= propagation_expression();
        modifie |= elimination_expressions_redondantes();
        modifie |= elimination_variables_mortes();
        modifie |= elimination_code_inutile();   /* remappe BZ/BR avant compaction */

        iterations++;
    } while (modifie && iterations < 30);

    printf(GRN "[Optimisation terminee en %d iteration(s), 8 passes]\n" RESET, iterations);
}
