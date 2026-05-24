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
   UTILITAIRES INTERNES
 */

/*
 est_nombre : verifie si la chaine s represente un nombre valide
              (entier ou flottant, avec signe optionnel).
              on verifie que c'est un nombre avant de simplifier x+0=x
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
 est_utilise_comme_index : verifie si var apparait comme indice
 dans un acces tableau de la forme "Tab[var]" dans un champ res.
 compter_utilisations ne scanne que op1/op2 et rate ce cas.

 Exemple :
   (+, x, 1, T36)           <- T36 semble "0 utilisations" pour compter_utilisations
   (:=, T41, , Tabint[T36]) <- mais T36 est ici dans res en tant qu'indice !
 */
static int est_utilise_comme_index(const char *var)
{
    int i;
    for (i = 0; i < qc; i++) {
        const char *r   = quad[i].res;
        const char *lbr = strchr(r, '[');
        const char *rbr = strrchr(r, ']');
        if (!lbr || !rbr || rbr <= lbr) continue;

        /* Extraire l'indice entre crochets */
        char idx[100];
        int  n = (int)(rbr - lbr - 1);
        strncpy(idx, lbr + 1, n);
        idx[n] = '\0';

        if (strcmp(idx, var) == 0) return 1; /* var est un indice de tableau */
    }
    return 0;
}
/*
 est_temporaire : verifie si s est un temporaire genere par le compilateur.
 Un temporaire a la forme T0, T1, T2, ... (T majuscule + chiffres uniquement).
 */
static int est_temporaire(const char *s)
{
    int i;
    if (!s || s[0] != 'T') return 0;
    for (i = 1; s[i]; i++)
        if (!isdigit((unsigned char)s[i])) return 0; /* pas de digit => pas un temp */
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
 a_effet_de_bord : verifie si le quadruplet i produit un effet
 (lecture, ecriture, saut) qui doit etre preserve meme si son
 resultat n'est jamais lu.
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
 (en tant que res) dans les quadruplets de debut a fin-1.

 Exemple :
   0: t1 := x
   1: x  := 5      <- x redefini ici
   2: t2 := t1 + 1
 Si on voulait remplacer t1 par x, c'est faux car x a change entre 0 et 2.
 */
static int est_redefini_entre(const char *var, int debut, int fin)
{
    int i;
    for (i = debut; i < fin; i++) {
        if (est_saut(quad[i].oper)) continue; /* les sauts n'assignent pas de valeur */
        if (strcmp(quad[i].res, var) == 0) return 1;
    }
    return 0;
}

/*
 formater_nombre : convertit un double en chaine de caracteres.
 Si la valeur est entiere, l'affiche sans decimale (ex: 4 et non 4.000000).

 FIX : utilise long long au lieu de long pour eviter les debordements
 sur les valeurs superieures a 2^31 (ex: constantes flottantes larges).
 */
static void formater_nombre(double v, char *buf)
{
    if (v == (double)(long long)v) sprintf(buf, "%lld", (long long)v);
    else                           sprintf(buf, "%g",   v);
}

/*
 nom_base_tableau : si res est un acces tableau de la forme "Tab[indice]",
 extrait uniquement le nom de base "Tab" dans buf.
 Retourne buf si c'est un tableau, sinon retourne res inchange.

 POURQUOI : la passe 7 compare le res d'une affectation avec les variables
 lues en operande. Les TAB utilisent "Tabfloat" en op1, mais les affectations
 tableau ecrivent "Tabfloat[i]" en res. Sans extraction du nom de base,
 strcmp("Tabfloat[i]", "Tabfloat") echoue => le tableau est faussement
 considere comme mort et supprime, alors qu'il est encore lu.
 */
static const char *nom_base_tableau(const char *res, char *buf)
{
    const char *lbr = strchr(res, '[');
    if (!lbr) return res;                  /* pas un tableau : retourner tel quel */
    int n = (int)(lbr - res);
    strncpy(buf, res, n);
    buf[n] = '\0';
    return buf;
}


/* 
   PASSE 1 : PROPAGATION DE CONSTANTES SYMBOLIQUES

   Principe : si une variable est declaree CONST dans la table des symboles,
   remplacer toutes ses occurrences en operande par sa valeur numerique.

   Exemple :
     const MAX := 100
     t1 = i <= MAX   =>  t1 = i <= 100
*/
static int propagation_constantes_symboliques(void)
{
    int modifie = 0, i;
    for (i = 0; i < qc; i++) {
        char *champs[2] = { quad[i].op1, quad[i].op2 };
        int c;
        for (c = 0; c < 2; c++) {
            char *nom = champs[c];
            if (!nom || !*nom || est_nombre(nom)) continue;
            if (!ts_est_constante(nom)) continue;       /* uniquement les CONSTes */
            const char *val = ts_get_val(nom);
            if (!val || !*val || !est_nombre(val)) continue;
            strcpy(nom, val);                           /* remplacer le nom par la valeur */
            modifie = 1;
        }
    }
    return modifie;
}


/*
   PASSE 2 : SIMPLIFICATION ALGEBRIQUE

   Principe : remplacer des operations couteuses ou sans effet
   par des formes equivalentes plus simples ou par leur resultat
   calcule directement a la compilation.

   Regles appliquees sur chaque quadruplet :
     X + 0  ou  0 + X   =>  X
     X - 0              =>  X
     X - X              =>  0
     X * 0  ou  0 * X   =>  0
     X * 1  ou  1 * X   =>  X
     X * 2  ou  2 * X   =>  X + X    (addition moins couteuse que multiplication)
     X / 1              =>  X
     X / X              =>  1
     C1 op C2 (deux constantes) => resultat calcule a la compilation (folding)
*/
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
                strcpy(op, "+"); strcpy(b, a);              /* b = copie de a */
                modifie = 1;
            } else if (a_est_nb && atof(a) == 2.0) {       /* 2 * X => X + X */
                /* CORRECTION : sauvegarder X avant d'ecraser a */
                char tmp[100];
                strcpy(tmp, b);    /* tmp = X */
                strcpy(op, "+");
                strcpy(a, tmp);    /* a = X */
                strcpy(b, tmp);    /* b = X  (et non b=b comme avant) */
                modifie = 1;
            } else if (b_est_nb && atof(b) == 4.0) {       /* X * 4 => X * 2 * 2 */
                strcpy(b, "2"); modifie = 1;
            } else if (a_est_nb && atof(a) == 4.0) {
                strcpy(a, "2"); modifie = 1;
            } else if (b_est_nb && atof(b) == 8.0) {       /* X * 8 => X * 4 */
                strcpy(b, "4"); modifie = 1;
            } else if (a_est_nb && atof(a) == 8.0) {
                strcpy(a, "4"); modifie = 1;
            } else if (a_est_nb && b_est_nb) {             /* C * C => resultat direct */
                /* ex: res = 3 * 4 devient res := 12 */
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


/*
   PASSE 3 : SIMPLIFICATION DES CONSTANTES CHAINEES

   Principe : si deux quadruplets consecutifs effectuent chacun
   une addition ou soustraction avec une constante, et que le
   temporaire intermediaire n'est utilise qu'une seule fois,
   on peut fusionner les deux en un seul en calculant le decalage net.

   Pattern detecte (T utilise une seule fois) :
     quad[i] : T  = X  +/-  C1
     quad[j] : R  = T  +/-  C2
   => quad[j] : R  = X  +/-  (C1 op C2)   (calcul net a la compilation)
      quad[i] : NOP                         (supprime)

   Exemples : on fusuionne
     t10 = j + 1
     t11 = t10 - 1   =>  t11 = j        (1-1 = 0, affectation directe) */
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
        /* FIX : ne pas fusionner si T est utilise comme indice tableau dans un res */
        if (est_utilise_comme_index(res1)) continue;

        /* Chercher le quadruplet qui utilise res1, sans franchir un saut */
        int j, quad_usage = -1;
        for (j = i + 1; j < qc; j++) {
            if (est_saut(quad[j].oper)) break;               /* barriere : on arrete */
            if (strcmp(quad[j].res, res1) == 0) break;       /* res1 redefini : invalide */
            if (strcmp(quad[j].op1, res1) == 0 || strcmp(quad[j].op2, res1) == 0) {
                quad_usage = j; break; /* on a trouve le consommateur de res1 */
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

        if      (strcmp(op1,"+")==0 && strcmp(op2,"+")==0) { net = c1+c2; strcpy(op_net, "+"); } /* T=X+3, R=T+5 => R=X+8 */
        else if (strcmp(op1,"+")==0 && strcmp(op2,"-")==0) { net = c1-c2; strcpy(op_net,(net>=0)?"+":"-"); if(net<0)net=-net; } /* T=X+7, R=T-2 => R=X+5 */
        else if (strcmp(op1,"-")==0 && strcmp(op2,"+")==0) { net = c2-c1; strcpy(op_net,(net>=0)?"+":"-"); if(net<0)net=-net; } /* T=X-3, R=T+8 => R=X+5 */
        else                                               { net = c1+c2; strcpy(op_net, "-"); }  /* T=X-3, R=T-2 => R=X-5 */

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
        /* Le quadruplet intermediaire devient inutile : le marquer NOP */
        strcpy(quad[i].oper, "NOP");
        strcpy(quad[i].op1,  "");
        strcpy(quad[i].op2,  "");
        strcpy(quad[i].res,  "");
        modifie = 1;
    }
    return modifie;
}


/*
     PASSE 4 : FUSION TEMPORAIRE -> DESTINATION

     Principe : si un temporaire T est calcule puis immediatement
     recopie vers une destination, on remplace la destination du
     calcul par la destination finale et on supprime la copie.

     Pattern :
         quad[i]   :  T = expr
         quad[i+1] :  dst := T
     =>
         quad[i]   :  dst = expr
         quad[i+1] :  NOP

     On ne fusionne pas si quad[i+1] est cible d'un saut.
*/
static int fusion_temp_affectation(void)
{
        int modifie = 0, i;
        int cible[MAX_QUADS];
        memset(cible, 0, sizeof(cible));

        for (i = 0; i < qc; i++) {
                if (!est_saut(quad[i].oper)) continue;
                int t = atoi(quad[i].res);
                if (t >= 0 && t < qc) cible[t] = 1;
        }

        for (i = 0; i < qc - 1; i++) {
                if (cible[i + 1]) continue;
                if (strcmp(quad[i + 1].oper, ":=") != 0) continue;
                if (quad[i + 1].op2[0] != '\0') continue;

                const char *tmp = quad[i + 1].op1;
                if (!est_temporaire(tmp)) continue;
                if (strcmp(quad[i].res, tmp) != 0) continue;
                if (compter_utilisations(tmp) != 1) continue;
                if (est_utilise_comme_index(tmp)) continue;
                if (a_effet_de_bord(i) || a_effet_de_bord(i + 1)) continue;

                /* Fusionner vers la destination finale */
                strcpy(quad[i].res, quad[i + 1].res);
                strcpy(quad[i + 1].oper, "NOP");
                strcpy(quad[i + 1].op1,  "");
                strcpy(quad[i + 1].op2,  "");
                strcpy(quad[i + 1].res,  "");
                modifie = 1;
        }
        return modifie;
}


/* 
   PASSE 4 : PROPAGATION DE COPIE

   Principe : lorsqu'un temporaire dst est affecte a partir d'une
   valeur src (dst := src), on remplace toutes les utilisations
   suivantes de dst par src directement.
   Cela elimine les affectations intermediaires inutiles.

   Condition d'arret : la propagation s'arrete si :
      dst est redefini (sa valeur change)
      src est redefini (la copie n'est plus valide)
      un saut est rencontre (barriere de flot de controle)

   Exemple :
     t1 = t2          => dst=t1, src=t2
     t3 = 4 * t1      => t3 = 4 * t2   (t1 remplace par t2)
*/
static int propagation_copie(void)
{
    int modifie = 0, i;
    for (i = 0; i < qc; i++) {
        if (strcmp(quad[i].oper, ":=") != 0) continue; /* chercher les affectations simples */
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


/* PASSE 5 : PROPAGATION D'EXPRESSION
kifkif mais cette fois ci avec une expression
   Condition : src ne doit pas avoir ete redefini entre la
   definition de T et son unique utilisation.

   Exemple :
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
        /* FIX : ne pas propager si T est utilise comme indice tableau dans un res */
        if (est_utilise_comme_index(res)) continue;

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


/* PASSE 6 : ELIMINATION DES EXPRESSIONS REDONDANTES 

   Exemple :
     t1 = a + b        =>  memorisee dans la table
     t2 = c * d
     t3 = a + b        =>  REDONDANT : remplace par t3 := t1
     t4 = t3 * e       =>  apres propagation : t4 = t1 * e

   Commutativite : a+b et b+a sont considerees identiques pour + et *.  */
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
        /* FIX CSE : inclure les lectures de tableau (TAB) dans la CSE.
           TAB Tabint[i] recalcule plusieurs fois avec le meme indice
           dans le meme bloc de base => redondant, on peut reutiliser
           le premier resultat.
           ATTENTION : une ecriture tableau (:=, val, , Tab[idx]) invalide
           toutes les entrees TAB de ce tableau dans la table. */
        int est_tab   = strcmp(op,"TAB") == 0;

        if (est_arith || est_comp || est_tab) {
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

            /* FIX CSE tableaux : si res est un acces tableau "Tab[idx]",
               invalider toutes les entrees TAB dont l'operande1 est ce
               meme tableau de base. Ex: Tabint[i] := val invalide toutes
               les entrees TAB de Tabint dans la table CSE, car on ne sait
               plus si la valeur lue sera la meme apres ecriture. */
            char base_res_buf[100];
            const char *base_res = nom_base_tableau(res, base_res_buf);
            int res_est_tableau = (base_res != res);

            for (k = 0; k < nb_expr; ) {
                int invalider = 0;
                if (strcmp(table_expr[k].resultat,  res) == 0
                 || strcmp(table_expr[k].operande1, res) == 0
                 || strcmp(table_expr[k].operande2, res) == 0)
                    invalider = 1;
                if (!invalider && res_est_tableau
                 && strcmp(table_expr[k].operateur, "TAB") == 0
                 && strcmp(table_expr[k].operande1, base_res) == 0)
                    invalider = 1;
                if (invalider) {
                    memmove(&table_expr[k], &table_expr[k+1],
                            (nb_expr - k - 1) * sizeof(ExpressionDisponible));
                    nb_expr--;
                } else k++;
            }
        }
    }
    return modifie;
}


/*
   PASSE 7 : ELIMINATION DE VARIABLES MORTES 
on supprime var jamais utilisee
 */
static int elimination_variables_mortes(void)
{
    int modifie = 0, i;

    /*
     Construire l'ensemble des variables utilisateur lues au moins
     une fois comme operande dans tout le code intermediaire.
     Une variable presente dans cet ensemble ne peut jamais etre
     eliminee, quelle que soit sa position.
     */
    char lues[MAX_QUADS][100];
    int  nb_lues = 0;

    for (i = 0; i < qc; i++) {
        const char *ops[2] = { quad[i].op1, quad[i].op2 };
        int k;
        for (k = 0; k < 2; k++) {
            const char *v = ops[k];
            if (!v || !*v || est_nombre(v) || est_temporaire(v)) continue;
            /* variable utilisateur lue en operande : la conserver */
            int deja = 0, m;
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
     * FIX BUG TABLEAUX : pour "Tabfloat[i]" en res, on compare le nom
     * de base "Tabfloat" (extrait par nom_base_tableau) avec la liste
     * des variables lues, pas la chaine complete "Tabfloat[i]".
     */
    for (i = 0; i < qc; i++) {
        if (a_effet_de_bord(i)) continue;
        if (strcmp(quad[i].oper, ":=") != 0) continue;
        const char *res = quad[i].res;
        if (!res || !*res || est_temporaire(res)) continue;

        /* Extraire le nom de base si c'est un acces tableau "Tab[indice]" */
        char base_buf[100];
        const char *base = nom_base_tableau(res, base_buf);

        /* Cette variable (ou ce tableau) est-il lu quelque part ? */
        int lue = 0, k;
        for (k = 0; k < nb_lues; k++)
            if (strcmp(lues[k], base) == 0) { lue = 1; break; }

        if (!lue) {
            /* Jamais lue => affectation inutile => NOP */
            strcpy(quad[i].oper, "NOP");
            strcpy(quad[i].op1,  "");
            strcpy(quad[i].op2,  "");
            strcpy(quad[i].res,  "");
            modifie = 1;
        }
    }
    return modifie;
}


/* 
   PASSE 8 : ELIMINATION DE CODE INUTILE + REMAPPAGE DES SAUTS

   Principe : supprimer les quadruplets dont le calcul n'a
   aucun impact sur le resultat final du programme.
   Deux categories sont eliminees :
     1. Les NOP explicites (marques par les passes precedentes)
     2. Les temporaires dont le resultat n'est jamais relu
        (compter_utilisations == 0) : le calcul est inutile

   Exemple general:
     x   = t3       =>  x n'est jamais utilise apres => supprime
     a[t2] = t5     =>  conserve
     a[t4] = t3     =>  conserve
  */
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

        /* Supprimer les temporaires dont le resultat n'est jamais lu
           et qui ne sont pas utilises comme indice de tableau dans un res */
        if (est_temporaire(res) && compter_utilisations(res) == 0 && !est_utilise_comme_index(res))
            a_supprimer[i] = 1;
    }

    /* 
       ETAPE 1 : construire la table de remappage old_idx -> new_idx
     */
    int remap[MAX_QUADS];
    int nouveau_qc = 0;
    for (i = 0; i < qc; i++) {
        if (a_supprimer[i])
            remap[i] = -1;           /* quad supprime : n'a plus d'indice */
        else
            remap[i] = nouveau_qc++;
    }

    /* 
       ETAPE 2 : mettre a jour les cibles BZ / BR avant la compaction.
       Pour un saut vers old_target :
         - si old_target est supprime, avancer au premier quad conserve
         - sinon utiliser remap[old_target]
       Si old_target >= qc c'est la cible "sortie programme" : on la
       laisse pointer vers nouveau_qc (label de fin genere par asm8086).
      */
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

    /*
       ETAPE 2b : FIX BR MORT
       Apres remappage, un BR dont la cible == son propre nouvel indice + 1
       est un saut vers l'instruction suivante : il n'a aucun effet et
       peut etre supprime sans changer le comportement du programme.

       Exemple :
         quad 23 : BZ T21 -> 25   (conserve, apres remap -> new_idx 25)
         quad 24 : BR -> 25       (apres remap new_idx=24, cible=25=24+1 => mort)
         quad 25 : ...
  */
    for (i = 0; i < qc; i++) {
        if (a_supprimer[i]) continue;
        if (strcmp(quad[i].oper, "BR") != 0) continue;

        int cible      = atoi(quad[i].res);  /* cible deja remappee */
        int mon_nouvel_idx = remap[i];        /* mon futur indice apres compaction */

        /* Si la cible est exactement l'instruction suivante : saut inutile */
        if (cible == mon_nouvel_idx + 1) {
            a_supprimer[i] = 1;  /* marquer pour suppression */
        }
    }

    /*
       ETAPE 3 : compacter le tableau en retirant les quads marques
        */
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

/*   Chaque passe retourne 1 si elle a modifie au moins un quadruplet.
   La boucle recommence tant qu'au moins une passe a produit un changement.   
   Chaque passe retourne 1 si elle a modifie au moins un quadruplet.
*/


void optimiser_quadruplets(void)
{
    int modifie, iterations = 0;

    printf(CYN "\n============================================================\n" RESET);
    printf(CYN " OPTIMISATION DU CODE INTERMEDIAIRE (9 passes)\n" RESET);
    printf(CYN "============================================================\n" RESET);

    do {
        /* Passe 1 : remplacer les constantes symboliques par leur valeur */
        modifie  = propagation_constantes_symboliques();

        /* Passe 2 : simplifier les operations algebriques triviales */
        modifie |= simplification_algebrique();

        /* Passe 3 : fusionner deux +/- de constantes consecutives
           (FIX : ne pas fusionner si le temporaire est un indice tableau) */
        modifie |= simplification_constantes_chainees();

        /* Passe 4 : fusionner T calcule + affectation immediate */
        modifie |= fusion_temp_affectation();

        /* Passe 5 : propager les copies (dst:=src => remplacer dst par src) */
        modifie |= propagation_copie();

          /* Passe 6 : propager les expressions a usage unique vers leur site
           (FIX : ne pas propager si le temporaire est un indice tableau) */
        modifie |= propagation_expression();

        /* Passe 7 : eliminer les recalculs d'expressions deja disponibles */
        modifie |= elimination_expressions_redondantes();

          /* Passe 8 : supprimer les variables utilisateur jamais lues
           (FIX tableaux : compare le nom de base, pas "Tab[i]") */
        modifie |= elimination_variables_mortes();

          /* Passe 9 : supprimer les NOP et les calculs morts + corriger sauts
           (FIX remappage BZ/BR + FIX BR vers instruction suivante) */
        modifie |= elimination_code_inutile();

        iterations++;
    } while (modifie && iterations < 30);

    printf(GRN "[Optimisation terminee en %d iteration(s), 9 passes]\n" RESET, iterations);
}