#include "opt.h"
#include "quad.h"
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
        if (!isdigit((unsigned char)s[i])) return 0; // mor first T pas de digit
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
 lecture, ecriture, saut) qui doit etre preserve meme si son resultat n'est jamais lu.
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
0: t1 := x
1: x  := 5
2: t2 := t1 + 1
here ida habina nbdlo t1 par x c'est faut
 */
static int est_redefini_entre(const char *var, int debut, int fin)
{
    int i;
    for (i = debut; i < fin; i++) {
        if (est_saut(quad[i].oper)) continue; // we skip jumps, we dont assign there values
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

/* SIMPLIFICATION ALGEBRIQUE

   Principe : remplacer des operations couteuses ou sans effet
   par des formes equivalentes plus simples ou par leur resultat
   calcule directement a la compilation.

   Regles appliquees sur chaque quadruplet :
     X + 0  ou  0 + X   =>  X          
     X - 0              =>  X        
     X - X              =>  0       
     X * 0  ou  0 * X   =>  0        
     X * 1  ou  1 * X   =>  X        
     X * 2  ou  2 * X   =>  X + X    
     X / 1              =>  X         
     C1 op C2 (deux constantes) => resultat calcule a la compilation*/

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
            } else if (b_est_nb && atof(b) == 2.0) {       /* X * 2 => X + X (addition moins couteuse) */
                strcpy(op, "+"); strcpy(b, a); // a+a
                modifie = 1;
            } else if (a_est_nb && atof(a) == 2.0) {       /* 2 * X => X + X */
                strcpy(op, "+"); strcpy(a, b); //b+b
                modifie = 1;
            } else if (a_est_nb && b_est_nb) {             /* C * C => resultat direct */
                //res = 3 * 4 devient res := 12.
                char buf[64]; formater_nombre(atof(a) * atof(b), buf);
                strcpy(op, ":="); strcpy(a, buf); strcpy(b, "");
                modifie = 1;
            }

        } else if (strcmp(op, "/") == 0) {
            if (b_est_nb && atof(b) == 1.0) {              /* X / 1 => X */
                strcpy(op, ":="); strcpy(b, "");
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

/*SIMPLIFICATION DES CONSTANTES CHAINEES
   Principe : si deux quadruplets consecutifs effectuent chacun
   une addition ou soustraction avec une constante, et que le
   temporaire intermediaire n'est utilise qu'une seule fois,
   on peut fusionner les deux en un seul en calculant le decalage net.

   Pattern detecte (T utilise une seule fois) :
     quad[i] : T  = X  +/-  C1
     quad[j] : R  = T  +/-  C2
   => quad[j] : R  = X  +/-  (C1 op C2)  (calcul net)
      quad[i] : NOP                       (supprime)

   Exemple :
     t10 = j + 1              
     t11 = t10 - 1  =>  t11 = j 
  */
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
        if (!est_nombre(b1))       continue;  /* op2 doit etre une constante */
        if (!est_temporaire(res1)) continue;  /* le resultat doit etre un temporaire */
        if (compter_utilisations(res1) != 1) continue; /* T utilise exactement une fois */

        /* Chercher le quadruplet qui utilise res1, sans franchir un saut */
        int j, quad_usage = -1;
        for (j = i + 1; j < qc; j++) {
            if (est_saut(quad[j].oper)) break;               /* barriere : on arrete */
            if (strcmp(quad[j].res, res1) == 0) break;       /* res1 est redéfini dans le quad j, alors l’ancienne valeur de res1 n’est plus valable  */
            if (strcmp(quad[j].op1, res1) == 0 || strcmp(quad[j].op2, res1) == 0) {
                quad_usage = j; break; //   on trouve le quad qui utilise res1, on arrete la recherche
            }
        }
        if (quad_usage == -1) continue;

        char *op2 = quad[quad_usage].oper;
        char *a2  = quad[quad_usage].op1;
        char *b2  = quad[quad_usage].op2;

        /* Le second quad doit aussi etre +/- avec constante, T doit etre op1 */
        if (strcmp(op2, "+") != 0 && strcmp(op2, "-") != 0) continue;
        if (strcmp(a2, res1) != 0) continue;   /* T doit etre l'operande gauche */
        if (!est_nombre(b2)) continue;

        /* Calculer le decalage net des deux constantes */
        double c1 = atof(b1), c2 = atof(b2), net;
        char op_net[4];

        if      (strcmp(op1,"+")==0 && strcmp(op2,"+")==0) { net = c1+c2; strcpy(op_net, "+"); } // T = X + 3 | R = T + 5    =>   R = X + 8
        else if (strcmp(op1,"+")==0 && strcmp(op2,"-")==0) { net = c1-c2; strcpy(op_net,(net>=0)?"+":"-"); if(net<0)net=-net; } // T = X + 7 | R = T - 2    =>   R = X + 5
        else if (strcmp(op1,"-")==0 && strcmp(op2,"+")==0) { net = c2-c1; strcpy(op_net,(net>=0)?"+":"-"); if(net<0)net=-net; }// T = X - 3 |R = T + 8    =>   R = X + 5   (net = 8-3 = +5)
        else                                        { net = c1+c2; strcpy(op_net, "-"); }

        /* Mettre a jour le quadruplet d'usage avec le resultat fusionne */
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
        /* Le quadruplet intermediaire devient inutile */
        strcpy(quad[i].oper, "NOP");
        strcpy(quad[i].op1,  "");
        strcpy(quad[i].op2,  "");
        strcpy(quad[i].res,  "");
        modifie = 1;
    }
    return modifie;
}

/* PROPAGATION DE COPIE

   Principe : lorsqu'une variable dst est affectee a partir
   d'une variable src (dst := src), on remplace toutes les
   utilisations suivantes de dst par src directement.
   Cela elimine les affectations intermediaires inutiles.

   Condition d'arret : la propagation s'arrete si :
     - dst est redefini (sa valeur change)
     - src est redefini (la copie n'est plus valide)
     - un saut est rencontre (barriere de flot de controle)

     t1 = t2           =>  dst=t1, src=t2
     t3 = 4 * t1       =>  t3 = 4 * t2   (t1 remplace par t2)

   Cas impossible  :
     t1 = t2
     t1 = t3           =>  t1 redefini : propagation arretee
     t4 = t1 + t5      =>  t1 vaut t3 ici, pas t2
*/
static int propagation_copie(void)
{
    int modifie = 0, i;
    for (i = 0; i < qc; i++) {
        if (strcmp(quad[i].oper, ":=") != 0) continue; /* on cherche les affectations simples */
        const char *src = quad[i].op1;  /* valeur source */
        const char *dst = quad[i].res;  /* variable destination */
        if (!src || !*src || !dst || !*dst) continue;
        if (strcmp(src, dst) == 0) continue;  /* affectation a soi-meme : rien a faire */

        /* On ne propage que les temporaires (T0, T1, ...)
           Propager une variable utilisateur (i, j, x...) est dangereux
           dans les boucles : i:=1 puis i=i+1 => on ne peut pas remplacer i par 1 partout */
        if (!est_temporaire(dst)) continue;

        int j;
        for (j = i + 1; j < qc; j++) {
            const char *op = quad[j].oper;
            if (est_saut(op)) break;                        /* barriere de flot : on arrete */
            if (strcmp(quad[j].res, dst) == 0) break;      /* dst redefini : copie invalide */
            if (strcmp(quad[j].res, src) == 0) break;      /* src redefini : copie invalide */
            /* Remplacer les occurrences de dst par src */
            if (strcmp(quad[j].op1, dst) == 0) { strcpy(quad[j].op1, src); modifie = 1; }
            if (strcmp(quad[j].op2, dst) == 0) { strcpy(quad[j].op2, src); modifie = 1; }
        }
    }
    return modifie;
}

/* 
   PROPAGATION D'EXPRESSION

   Principe : si un temporaire T est defini par une affectation
   simple (T := src) et n'est utilise qu'une seule fois dans
   tout le code, on substitue directement src a la place de T
   au point d'utilisation, puis on supprime le quadruplet
   de definition (il devient NOP).

   Condition : src ne doit pas avoir ete redefini entre la
   definition de T et son unique utilisation.

   Exemple:
     t1 = expr         =>  T utilise une seule fois
     t3 = 4 * t1       =>  t3 = 4 * expr  (t1 supprime)
    */
static int propagation_expression(void)
{
    int modifie = 0, i;
    for (i = 0; i < qc; i++) {
        /* Seulement les affectations simples vers un temporaire */
        if (strcmp(quad[i].oper, ":=") != 0) continue;
        const char *res = quad[i].res;  /* temporaire a propager */
        const char *src = quad[i].op1;  /* valeur source a substituer */
        if (!est_temporaire(res)) continue;
        if (!src || !*src) continue;

        /* T doit etre utilise exactement une seule fois dans tout le code */
        if (compter_utilisations(res) != 1) continue;

        /* Trouver le quadruplet qui utilise T */
        int j, quad_usage = -1;
        for (j = i + 1; j < qc; j++) {
            if (est_saut(quad[j].oper)) break;
            if (est_redefini_entre(res, j, j + 1)) break;
            if (strcmp(quad[j].op1, res) == 0 || strcmp(quad[j].op2, res) == 0) {
                quad_usage = j; break;
            }
        }
        if (quad_usage == -1) continue;

        /* src ne doit pas avoir change entre la definition et le site d'usage */
        if (!est_nombre(src) && est_redefini_entre(src, i + 1, quad_usage)) continue;

        /* Substituer T par src au site d'utilisation */
        if (strcmp(quad[quad_usage].op1, res) == 0) { strcpy(quad[quad_usage].op1, src); modifie = 1; }
        if (strcmp(quad[quad_usage].op2, res) == 0) { strcpy(quad[quad_usage].op2, src); modifie = 1; }

        /* Le quadruplet de definition devient inutile : le marquer NOP */
        strcpy(quad[i].oper, "NOP");
        strcpy(quad[i].op1,  "");
        strcpy(quad[i].op2,  "");
        strcpy(quad[i].res,  "");
    }
    return modifie;
}

/* ELIMINATION DES EXPRESSIONS REDONDANTES

   Principe : maintenir une table des expressions deja calculees.
   Si la meme expression apparait a nouveau et que ses operandes
   n'ont pas ete modifies, remplacer le recalcul par une simple
   copie du resultat precedent, evitant ainsi le recalcul inutile.

   Exemple (diapo 104) :
     t1 = a + b        =>  memorisee dans la table
     t2 = c * d
     t3 = a + b        =>  REDONDANT : remplace par t3 := t1
     t4 = t3 * e       =>  apres propagation : t4 = t1 * e
*/
#define MAX_EXPR_DISPO 256

/* Structure representant une expression disponible dans la table */
typedef struct {
    char operateur[100]; /* operateur : +, -, *, /, ==, ... */
    char operande1[100]; /* premier operande de l'expression */
    char operande2[100]; /* second operande de l'expression  */
    char resultat[100];  /* temporaire qui stocke le resultat */
} ExpressionDisponible;

static int elimination_expressions_redondantes(void)
{
    ExpressionDisponible table_expr[MAX_EXPR_DISPO]; /* expressions deja calculees */
    int nb_expr = 0;   /* nombre d'expressions dans la table */
    int modifie = 0, i;

    for (i = 0; i < qc; i++) {
        const char *op  = quad[i].oper;
        char       *a   = quad[i].op1;
        char       *b   = quad[i].op2;
        char       *res = quad[i].res;
        int k;

        /* Un saut invalide toute la table : on ne sait plus quelles
           expressions restent valides apres le branchement */
        if (est_saut(op)) { nb_expr = 0; continue; }

        if (strcmp(op, "NOP") == 0) continue;

        int est_arith = strcmp(op,"+")==0 || strcmp(op,"-")==0
                     || strcmp(op,"*")==0 || strcmp(op,"/")==0;
        int est_comp  = strcmp(op,"==")==0 || strcmp(op,"!=")==0
                     || strcmp(op,"<=")==0 || strcmp(op,">=")==0
                     || strcmp(op,"<") ==0 || strcmp(op,">") ==0;

        if (est_arith || est_comp) {
            /* Chercher si cette expression est deja dans la table */
            int idx_trouve = -1;
            for (k = 0; k < nb_expr; k++) {
                if (strcmp(table_expr[k].operateur, op) != 0) continue;
                /* Correspondance directe : memes operandes, meme ordre */
                int correspondance_directe = strcmp(table_expr[k].operande1, a) == 0
                                          && strcmp(table_expr[k].operande2, b) == 0;
                /* Commutativite pour + et * : a+b == b+a */
                int correspondance_comm = (strcmp(op,"+")==0 || strcmp(op,"*")==0)
                                       && strcmp(table_expr[k].operande1, b) == 0
                                       && strcmp(table_expr[k].operande2, a) == 0;
                if (correspondance_directe || correspondance_comm) {
                    idx_trouve = k; break;
                }
            }

            if (idx_trouve >= 0) {
                /* Expression deja calculee : remplacer par copie du resultat precedent */
                strcpy(quad[i].oper, ":=");
                strcpy(quad[i].op1,  table_expr[idx_trouve].resultat);
                strcpy(quad[i].op2,  "");
                modifie = 1;
            } else {
                /* Invalider les expressions dont un operande est res */
                for (k = 0; k < nb_expr; ) {
                    if (strcmp(table_expr[k].resultat,  res) == 0
                     || strcmp(table_expr[k].operande1, res) == 0
                     || strcmp(table_expr[k].operande2, res) == 0) {
                        memmove(&table_expr[k], &table_expr[k+1],
                                (nb_expr - k - 1) * sizeof(ExpressionDisponible));
                        nb_expr--;
                    } else k++;
                }
                /* Ajouter la nouvelle expression a la table */
                if (nb_expr < MAX_EXPR_DISPO) {
                    strcpy(table_expr[nb_expr].operateur, op);
                    strcpy(table_expr[nb_expr].operande1, a);
                    strcpy(table_expr[nb_expr].operande2, b);
                    strcpy(table_expr[nb_expr].resultat,  res);
                    nb_expr++;
                }
            }

        } else if (strcmp(op, ":=") == 0) {
            /* Une affectation peut changer la valeur de res :
               invalider toutes les expressions qui en dependent */
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

/* ELIMINATION DE CODE INUTILE

   Principe : supprimer les quadruplets dont le calcul n'a
   aucun impact sur le resultat final du programme.
   Deux categories sont eliminees :
     1. Les NOP explicites (marques par les passes precedentes)
     2. Les temporaires dont le resultat n'est jamais relu
        (count_uses == 0) : le calcul est inutile

   Exemple (diapo 107) :
     x   = t3       =>  x n'est jamais utilise apres
     a[t2] = t5     =>  conserve
     a[t4] = t3     =>  conserve
   Apres : le quadruplet "x = t3" est supprime.*/
static int elimination_code_inutile(void)
{
    int modifie = 0, i;
    int a_supprimer[MAX_QUADS];
    memset(a_supprimer, 0, sizeof(a_supprimer));

    for (i = 0; i < qc; i++) {
        if (a_effet_de_bord(i)) continue;  /* ne jamais supprimer les effets de bord */
        const char *op  = quad[i].oper;
        const char *res = quad[i].res;

        /* Supprimer les NOP laisses par les passes precedentes */
        if (strcmp(op, "NOP") == 0) { a_supprimer[i] = 1; continue; }

        /* Supprimer les temporaires dont le resultat n'est jamais lu */
        if (est_temporaire(res) && compter_utilisations(res) == 0)
            a_supprimer[i] = 1;
    }

    /* Compacter le tableau en retirant les quadruplets marques */
    int nouveau_qc = 0;
    for (i = 0; i < qc; i++) {
        if (!a_supprimer[i]) {
            if (i != nouveau_qc) quad[nouveau_qc] = quad[i];
            nouveau_qc++;
        } else modifie = 1;
    }
    qc = nouveau_qc;
    return modifie;
}

/*
   OPTIMISER_QUADRUPLETS : point d'entree de l'optimisation

   Ordre des passes a chaque iteration :
     1. Simplification algebrique          (X*1=>X, X+0=>X, C*C=>resultat)
     2. Simplification constantes chainees (t=j+1; r=t-1 => r=j)
     3. Propagation de copie               (y:=x => remplace y par x apres)
     4. Propagation d'expression           (T:=src utilise 1 fois => inline)
     5. Elimination des expressions redond.(a+b calcule 2 fois => reutiliser)
     6. Elimination de code inutile        (NOP et temporaires morts => retirer)

   Chaque passe retourne 1 si elle a modifie au moins un quadruplet.
   La boucle recommence tant qu'au moins une passe a produit un changement.
  */
void optimiser_quadruplets(void)
{
    int modifie, iterations = 0;

    printf(CYN "\n============================================================\n" RESET);
    printf(CYN " OPTIMISATION DU CODE INTERMEDIAIRE\n" RESET);
    printf(CYN "============================================================\n" RESET);

    do {
        /* Passe 1 : simplifier les operations algebriques triviales */
        modifie  = simplification_algebrique();

        /* Passe 2 : fusionner deux +/- de constantes consecutives */
        modifie |= simplification_constantes_chainees();

        /* Passe 3 : propager les copies (dst:=src => remplacer dst par src) */
        modifie |= propagation_copie();

        /* Passe 4 : propager les expressions a usage unique vers leur site */
        modifie |= propagation_expression();

        /* Passe 5 : eliminer les recalculs d'expressions deja disponibles */
        modifie |= elimination_expressions_redondantes();

        /* Passe 6 : supprimer les NOP et les calculs dont le resultat est mort */
        modifie |= elimination_code_inutile();

        iterations++;
    } while (modifie && iterations < 30);

    printf(GRN "[Optimisation terminee en %d iteration(s)]\n" RESET, iterations);
}
