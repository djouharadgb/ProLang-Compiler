#include "asm8086.h"
#include "quad.h"
#include "ts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Macro : ecrire une ligne dans le fichier asm */
#define EMIT(fmt, ...)  fprintf(f, fmt "\n", ##__VA_ARGS__)


/* Verifie si s est un nombre (ex: 42, -3, 1.5) */
// utilisee pour detecter les constantes dans les optimisations (ex: x+0 => x)
static int est_nombre(const char *s)
{
    if (!s || !*s) return 0;
    const char *p = s;
    if (*p == '-' || *p == '+') p++;
    if (!*p) return 0;
    int point = 0;
    for (; *p; p++) {
        if (*p == '.') { if (point) return 0; point = 1; }
        else if (!isdigit((unsigned char)*p)) return 0;
    }
    return 1;
}

/* Verifie si s est un nombre flottant (contient un point decimal) */
static int est_flottant(const char *s)
{
    if (!est_nombre(s)) return 0;
    return strchr(s, '.') != NULL;
}

/* Convertit un nom de variable en identifiant asm valide.
   Les temporaires T0, T1... deviennent _T0, _T1... */
static const char *nom_asm(const char *s)
{
    static char buf[128];
    if (!s || !*s) return s;
    if (s[0] == 'T' && isdigit((unsigned char)s[1])) {
        snprintf(buf, sizeof(buf), "_T%s", s + 1);
        return buf;
    }
    return s;
}

/* Detecte & verifie si un acces tableau "nom[indice]" et extrait les deux parties.
   Retourne 1 si c'est un acces tableau, 0 sinon. */
static int est_tableau_access(const char *s, char *nom_tab, char *indice)
{
    const char *lbr = strchr(s, '[');
    const char *rbr = strrchr(s, ']');
    if (!lbr || !rbr || rbr < lbr) return 0;
    int n = (int)(lbr - s);
    strncpy(nom_tab, s, n); nom_tab[n] = '\0';
    int m = (int)(rbr - lbr - 1);
    strncpy(indice, lbr + 1, m); indice[m] = '\0'; 
    return 1;
}

/* Emet un saut de ligne dans la sortie (separateur visuel)- affichage brk */
static void emit_newline(FILE *f)
{
    EMIT("    MOV AH, 02h");
    EMIT("    MOV DL, 0Ah");
    EMIT("    INT 21h");
}

/* CHARGEMENT ET STOCKAGE (AX / BX) */

/*
 charger_dans_reg : place la valeur de 'src' dans le registre 'reg' (AX ou BX).
  - Nombre litteral   => MOV reg, valeur  (partie entiere uniquement pour 8086)
- Acces tableau     => calcul index*2, puis MOV reg, tab[SI]
   - Variable/temp     => MOV reg, nom
 */
static void charger_dans_reg(FILE *f, const char *src, const char *reg)
{
    char ntab[64], idx[64];

    if (est_nombre(src)) {
        /* FIX : cast en long pour truncature correcte des flottants
           ex: 3.141590 => 3, 2.500000 => 2 (8086 ne gere que les entiers) */
        EMIT("    MOV %s, %ld", reg, (long)atof(src));

    } else if (est_tableau_access(src, ntab, idx)) {
        /* Calcul de l'offset : chaque element fait 2 octets (DW) */
        if (est_nombre(idx))
            EMIT("    MOV SI, %ld", atol(idx) * 2);
        else {
            EMIT("    MOV SI, %s", nom_asm(idx));
            EMIT("    ADD SI, SI   ; SI = indice * 2");
        }
        EMIT("    MOV %s, %s[SI]", reg, nom_asm(ntab));

    } else {
        EMIT("    MOV %s, %s", reg, nom_asm(src));
    }
}

/*
 * stocker_ax_dans : stocke AX dans la destination 'dst'.
 *   - Acces tableau => T[SI] = AX
 *   - Variable/temp => MOV nom, AX
 */
static void stocker_ax_dans(FILE *f, const char *dst)
{
    char ntab[64], idx[64];

    if (est_tableau_access(dst, ntab, idx)) {
        if (est_nombre(idx))
            EMIT("    MOV SI, %ld", atol(idx) * 2);
        else {
            EMIT("    MOV SI, %s", nom_asm(idx));
            EMIT("    ADD SI, SI");
        }
        EMIT("    MOV %s[SI], AX", nom_asm(ntab));
    } else {
        EMIT("    MOV %s, AX", nom_asm(dst));
    }
}

/* =========================================================
   COLLECTE DES SYMBOLES (segment de donnees)
   ========================================================= */

#define MAX_SYMS 512

typedef struct {
    char nom[128];
    char type[16];  /* "integer" ou "float" */
    char val[64];   /* valeur initiale ou "" */
    int  taille;    /* 0 = scalaire, >0 = tableau */
    int  est_temp;  /* 1 = temporaire compilateur  */
} SymInfo;

static SymInfo syms[MAX_SYMS];
static int     nb_syms = 0;

static int sym_existe(const char *nom)
{
    int i;
    for (i = 0; i < nb_syms; i++)
        if (strcmp(syms[i].nom, nom) == 0) return 1;
    return 0;
}

static void sym_ajouter(const char *nom, const char *type,
                        const char *val, int taille, int est_temp)
{
    if (nb_syms >= MAX_SYMS || sym_existe(nom)) return;
    strncpy(syms[nb_syms].nom,  nom,  sizeof(syms[0].nom)  - 1);
    strncpy(syms[nb_syms].type, type, sizeof(syms[0].type) - 1);
    strncpy(syms[nb_syms].val,  val ? val : "", sizeof(syms[0].val) - 1);
    syms[nb_syms].taille   = taille;
    syms[nb_syms].est_temp = est_temp;
    nb_syms++;
}

/* Parcourt la TS et les quadruplets pour collecter tous les symboles */
static void collecter_symboles(void)
{
    int i;
    nb_syms = 0;

    /* Variables, constantes, tableaux depuis la table des symboles */
    for (i = 0; i < HASH_SIZE; i++) {
        NoeudTS *n = hashTable[i];
        while (n) {
            if (n->state == 1) {
                int taille = (strcmp(n->code, "TABLEAU") == 0) ? atoi(n->val) : 0;
                const char *val_init = (taille > 0) ? "" :
                    ((strcmp(n->code,"CONST")==0 || strlen(n->val)>0) ? n->val : "");
                sym_ajouter(n->name, n->type, val_init, taille, 0);
            }
            n = n->suivant;
        }
    }

    /* Temporaires T0, T1, ... depuis les quadruplets */
    for (i = 0; i < qc; i++) {
        const char *fields[3] = { quad[i].op1, quad[i].op2, quad[i].res };
        int f;
        for (f = 0; f < 3; f++) {
            const char *s = fields[f];
            if (!s || !*s || est_nombre(s)) continue;

            /* Temporaire simple */
            if (s[0]=='T' && isdigit((unsigned char)s[1]) && !strchr(s,'['))
                sym_ajouter(s, "integer", "", 0, 1);

            /* Indice de tableau qui serait un temporaire */
            char ntab[64], idx[64];
            if (est_tableau_access(s, ntab, idx))
                if (idx[0]=='T' && isdigit((unsigned char)idx[1]))
                    sym_ajouter(idx, "integer", "", 0, 1);
        }
    }
}

/* =========================================================
   SEGMENT DE DONNEES
   ========================================================= */

static void emettre_segment_donnees(FILE *f)
{
    int i;
    EMIT("DONNEE SEGMENT");
    EMIT("");

    for (i = 0; i < nb_syms; i++) {
        SymInfo *s = &syms[i];
        const char *anom = nom_asm(s->nom);

        if (s->taille > 0) {
            EMIT("    %-20s DW %d DUP(?)  ; tableau", anom, s->taille);
        } else if (s->est_temp) {
            EMIT("    %-20s DW ?          ; temporaire", anom);
        } else if (strlen(s->val) > 0 && strcmp(s->val,"oui") != 0) {
            /* Variable/constante initialisee.
               FIX : les flottants sont stockes comme entiers (partie entiere)
               car le 8086 en mode reel ne supporte pas les flottants natifs.
               ex: 2.500000 => DW 2,  3.141590 => DW 3
               Un commentaire indique la valeur flottante originale. */
            long v = (long)atof(s->val);
            if (est_flottant(s->val))
                EMIT("    %-20s DW %ld   ; float tronque (val orig: %s)", anom, v, s->val);
            else
                EMIT("    %-20s DW %ld", anom, v);
        } else {
            EMIT("    %-20s DW ?", anom);
        }
    }

    /* Tampon pour l'affichage (INT 21h attend '$' comme terminateur).
       FIX : taille portee a 12 pour couvrir -32768 (6 chiffres + signe + marge). */
    EMIT("    _OUT_BUF             DB 12 DUP(?), '$'");
    EMIT("");
    EMIT("DONNEE ENDS");
    EMIT("");
}

/* =========================================================
   PROCEDURES UTILITAIRES (print / read)
   ========================================================= */

/* Procedure d'affichage d'un entier (AX) via DOS INT 21h */
static void emettre_proc_print(FILE *f)
{
    EMIT(";---- _PRINT_INT : affiche AX en decimal ----");
    EMIT("_PRINT_INT PROC NEAR");
    EMIT("    PUSH AX"); EMIT("    PUSH BX"); EMIT("    PUSH CX");
    EMIT("    PUSH DX"); EMIT("    PUSH SI");
    EMIT("    MOV SI, OFFSET _OUT_BUF");
    EMIT("    CMP AX, 0");
    EMIT("    JGE _PRINT_POS");
    EMIT("    MOV BYTE PTR [SI], '-'");
    EMIT("    INC SI");
    EMIT("    NEG AX");
    EMIT("_PRINT_POS:");
    EMIT("    MOV BX, 10");
    EMIT("    MOV CX, 0");
    EMIT("_DIV_LOOP:");
    EMIT("    MOV DX, 0");
    EMIT("    DIV BX          ; AX = AX/10, DX = reste");
    EMIT("    ADD DL, '0'");
    EMIT("    PUSH DX         ; empiler les chiffres (ordre inverse)");
    EMIT("    INC CX");
    EMIT("    CMP AX, 0");
    EMIT("    JNE _DIV_LOOP");
    EMIT("_POP_LOOP:");
    EMIT("    POP DX");
    EMIT("    MOV [SI], DL");
    EMIT("    INC SI");
    EMIT("    LOOP _POP_LOOP");
    EMIT("    MOV BYTE PTR [SI], '$'");
    EMIT("    MOV AH, 09h");
    EMIT("    MOV DX, OFFSET _OUT_BUF");
    EMIT("    INT 21h");
    EMIT("    POP SI"); EMIT("    POP DX"); EMIT("    POP CX");
    EMIT("    POP BX"); EMIT("    POP AX");
    EMIT("    RET");
    EMIT("_PRINT_INT ENDP");
    EMIT("");
}

/* Procedure de lecture d'un entier depuis le clavier (resultat dans AX) */
static void emettre_proc_input(FILE *f)
{
    EMIT(";---- _READ_INT : lit un entier depuis le clavier ----");
    EMIT("_READ_INT PROC NEAR");
    EMIT("    PUSH BX"); EMIT("    PUSH CX"); EMIT("    PUSH DX");
    EMIT("    MOV BX, 0   ; accumulateur");
    EMIT("    MOV CX, 0   ; signe (0=positif, 1=negatif)");
    EMIT("_READ_CHAR:");
    EMIT("    MOV AH, 01h");
    EMIT("    INT 21h     ; AL = caractere lu");
    EMIT("    CMP AL, 0Dh ; Entree ?");
    EMIT("    JE  _READ_DONE");
    EMIT("    CMP AL, '-'");
    EMIT("    JNE _READ_DIGIT");
    EMIT("    MOV CX, 1");
    EMIT("    JMP _READ_CHAR");
    EMIT("_READ_DIGIT:");
    EMIT("    SUB AL, '0'");
    EMIT("    CBW");
    EMIT("    XCHG AX, BX");
    EMIT("    MOV DX, 10");
    EMIT("    MUL DX");
    EMIT("    ADD AX, BX");
    EMIT("    XCHG AX, BX");
    EMIT("    JMP _READ_CHAR");
    EMIT("_READ_DONE:");
    EMIT("    MOV AX, BX");
    EMIT("    CMP CX, 1");
    EMIT("    JNE _READ_POS");
    EMIT("    NEG AX");
    EMIT("_READ_POS:");
    EMIT("    POP DX"); EMIT("    POP CX"); EMIT("    POP BX");
    EMIT("    RET");
    EMIT("_READ_INT ENDP");
    EMIT("");
}

/* =========================================================
   LABELS DE SAUT
   ========================================================= */

static void label_pour(int idx, char *buf) { sprintf(buf, "L%d", idx); }

/* Marque les indices cibles de BZ/BR (on posera un label a ces endroits) */
static void marquer_cibles(int *marque)
{
    int i;
    memset(marque, 0, qc * sizeof(int));
    for (i = 0; i < qc; i++) {
        if (strcmp(quad[i].oper,"BZ")==0 || strcmp(quad[i].oper,"BR")==0) {
            int cible = atoi(quad[i].res);
            if (cible >= 0 && cible < qc) marque[cible] = 1;
        }
    }
    if (qc > 0) marque[0] = 1; /* le debut est toujours une cible */
}

/* =========================================================
   TRADUCTION D'UN QUADRUPLET
   ========================================================= */

static void traduire_quadruplet(FILE *f, int idx, int *marque)
{
    const char *op  = quad[idx].oper;
    const char *a   = quad[idx].op1;
    const char *b   = quad[idx].op2;
    const char *res = quad[idx].res;
    char lbl[32];

    /* Placer un label si cet indice est une cible de saut */
    if (marque[idx]) { label_pour(idx, lbl); EMIT("%s:", lbl); }

    /* Commentaire de reference */
    EMIT("    ; [%d] (%s, %s, %s, %s)", idx, op, a, b, res);

    /* ---- Affectation simple  res := a ---- */
    if (strcmp(op, ":=") == 0) {
        charger_dans_reg(f, a, "AX");
        stocker_ax_dans(f, res);
        return;
    }

    /* ---- Addition et Soustraction  res = a +/- b ----
       On factorise les deux car le schema est identique. */
    if (strcmp(op,"+")==0 || strcmp(op,"-")==0) {
        const char *mnem = (strcmp(op,"+")==0) ? "ADD" : "SUB";
        charger_dans_reg(f, a, "AX");
        if (est_nombre(b))
            /* FIX : cast en long pour truncature correcte des flottants
               ex: 1.500000 => 1, 2.000000 => 2 */
            EMIT("    %s AX, %ld", mnem, (long)atof(b));
        else {
            charger_dans_reg(f, b, "BX");
            EMIT("    %s AX, BX", mnem);
        }
        stocker_ax_dans(f, res);
        return;
    }

    /* ---- Multiplication et Division  res = a * / b ----
       Schema identique : charger BX, puis IMUL ou IDIV.
       La division a besoin de CWD pour etendre AX vers DX:AX. */
    if (strcmp(op,"*")==0 || strcmp(op,"/")==0) {
        int est_div = (strcmp(op,"/") == 0);
        charger_dans_reg(f, a, "AX");

        /* FIX Bug2 : multiplication par un flottant non entier (ex: 1.500000).
           La troncature (long)atof("1.500000") = 1 donnait un resultat faux.
           Cas n.5 : x * n.5  =>  x * (2n+1) puis SAR AX, 1  (division entiere par 2).
           Ex: x * 1.5  =>  MOV BX, 3 ; IMUL BX ; SAR AX, 1
               x * 2.5  =>  MOV BX, 5 ; IMUL BX ; SAR AX, 1                           */
        if (!est_div && est_nombre(b) && est_flottant(b)) {
            double fval = atof(b);
            long   ival = (long)fval;
            if (fval == ival + 0.5) {
                EMIT("    MOV BX, %ld", 2 * ival + 1);
                EMIT("    IMUL BX");
                EMIT("    SAR AX, 1");
                stocker_ax_dans(f, res);
                return;
            }
        }

        if (est_nombre(b))
            /* FIX : cast en long pour truncature correcte des flottants */
            EMIT("    MOV BX, %ld", (long)atof(b));
        else
            charger_dans_reg(f, b, "BX");
        if (est_div) EMIT("    CWD            ; etendre AX -> DX:AX");
        EMIT("    %s BX", est_div ? "IDIV" : "IMUL");
        stocker_ax_dans(f, res);
        return;
    }

    /* ---- Negation unaire  res = -a ---- */
    if (strcmp(op, "NEG") == 0) {
        charger_dans_reg(f, a, "AX");
        EMIT("    NEG AX");
        stocker_ax_dans(f, res);
        return;
    }

    /* ---- Comparaisons  res = (a op b)  =>  0 ou 1 ----
       Table de correspondance operateur -> saut conditionnel. */
    if (strcmp(op,"==")==0 || strcmp(op,"!=")==0 ||
        strcmp(op,"<") ==0 || strcmp(op,">") ==0 ||
        strcmp(op,"<=")==0 || strcmp(op,">=")==0) {

        /* Trouver le mnemonique de saut */
        const char *jcond = "JE";
        if      (strcmp(op,"!=") == 0) jcond = "JNE";
        else if (strcmp(op,"<")  == 0) jcond = "JL";
        else if (strcmp(op,">")  == 0) jcond = "JG";
        else if (strcmp(op,"<=") == 0) jcond = "JLE";
        else if (strcmp(op,">=") == 0) jcond = "JGE";

        char lbl_vrai[32], lbl_suite[32];
        sprintf(lbl_vrai,  "_CMP%d_V", idx);
        sprintf(lbl_suite, "_CMP%d_S", idx);

        charger_dans_reg(f, a, "AX");
        if (est_nombre(b))
            /* FIX : cast en long pour truncature correcte des flottants */
            EMIT("    CMP AX, %ld", (long)atof(b));
        else {
            charger_dans_reg(f, b, "BX");
            EMIT("    CMP AX, BX");
        }
        EMIT("    %s %s", jcond, lbl_vrai);
        EMIT("    MOV %s, 0", nom_asm(res));
        EMIT("    JMP %s", lbl_suite);
        EMIT("%s:", lbl_vrai);
        EMIT("    MOV %s, 1", nom_asm(res));
        EMIT("%s:", lbl_suite);
        return;
    }

    /* ---- AND et OR logiques  (meme schema, mnemonique different) ---- */
    if (strcmp(op,"AND")==0 || strcmp(op,"OR")==0) {
        charger_dans_reg(f, a, "AX");
        charger_dans_reg(f, b, "BX");
        EMIT("    %s AX, BX", op);   /* AND AX,BX  ou  OR AX,BX */
        stocker_ax_dans(f, res);
        return;
    }

    /* ---- NON logique : 0->1, non-zero->0 ---- */
    if (strcmp(op, "NON") == 0) {
        char lbl_un[32], lbl_fin[32];
        sprintf(lbl_un,  "_NON%d_U", idx);
        sprintf(lbl_fin, "_NON%d_F", idx);
        charger_dans_reg(f, a, "AX");
        EMIT("    CMP AX, 0");
        EMIT("    JE  %s", lbl_un);
        EMIT("    MOV AX, 0");
        EMIT("    JMP %s", lbl_fin);
        EMIT("%s:", lbl_un);
        EMIT("    MOV AX, 1");
        EMIT("%s:", lbl_fin);
        stocker_ax_dans(f, res);
        return;
    }

    /* ---- Saut conditionnel  BZ : si a == 0, aller a res ---- */
    if (strcmp(op, "BZ") == 0) {
        char cible[32];
        label_pour(atoi(res), cible);
        charger_dans_reg(f, a, "AX");
        EMIT("    CMP AX, 0");
        EMIT("    JE  %s", cible);
        return;
    }

    /* ---- Saut inconditionnel  BR : aller a res ---- */
    if (strcmp(op, "BR") == 0) {
        char cible[32];
        label_pour(atoi(res), cible);
        EMIT("    JMP %s", cible);
        return;
    }

    /* ---- Lecture tableau  res = TAB[b] ---- */
    if (strcmp(op, "TAB") == 0) {
        if (est_nombre(b))
            EMIT("    MOV SI, %ld", atol(b) * 2);
        else {
            EMIT("    MOV SI, %s", nom_asm(b));
            EMIT("    ADD SI, SI");
        }
        EMIT("    MOV AX, %s[SI]", nom_asm(a));
        stocker_ax_dans(f, res);
        return;
    }

    /* ---- Affichage  out variable ou chaine ---- */
    if (strcmp(op, "out") == 0) {
        if (a && a[0] == '"') {
            /* Chaine litterale : afficher caractere par caractere.
               FIX : gestion de toutes les sequences d'echappement courantes
               (\n, \t, \\, \") au lieu de \n seulement. */
            const char *p = a + 1;
            while (*p && *p != '"') {
                if (*p == '\\') {
                    p++;
                    switch (*p) {
                        case 'n':   /* saut de ligne */
                            emit_newline(f);
                            break;
                        case 't':   /* tabulation horizontale */
                            EMIT("    MOV AH, 02h");
                            EMIT("    MOV DL, 09h  ; tabulation");
                            EMIT("    INT 21h");
                            break;
                        case '\\':  /* antislash litteral */
                            EMIT("    MOV AH, 02h");
                            EMIT("    MOV DL, 92   ; '\\'");
                            EMIT("    INT 21h");
                            break;
                        case '"':   /* guillemet litteral */
                            EMIT("    MOV AH, 02h");
                            EMIT("    MOV DL, 34   ; '\"'");
                            EMIT("    INT 21h");
                            break;
                        default:    /* sequence inconnue : ignorer */
                            break;
                    }
                    p++;
                    continue;
                }
                EMIT("    MOV AH, 02h");
                EMIT("    MOV DL, %d   ; '%c'", (unsigned char)*p, *p);
                EMIT("    INT 21h");
                p++;
            }
        } else {
            /* Variable ou nombre : appel _PRINT_INT */
            charger_dans_reg(f, a, "AX");
            EMIT("    CALL _PRINT_INT");
        }
        /* FIX Bug5 : une chaine litterale (ex: "Somme: ") est un libelle qui
           precede la valeur sur la meme ligne => pas de saut de ligne apres.
           Une variable ou un nombre est toujours l'element final => saut de ligne.
           Cela traite correctement  out("label: ", x)  =>  "label: 42\n"
           sans fusionner plusieurs instructions out() sur une seule ligne.    */
        if (!(a && a[0] == '"'))
            emit_newline(f);
        return;
    }

    /* ---- Lecture clavier  input -> res ---- */
    if (strcmp(op, "input") == 0) {
        EMIT("    CALL _READ_INT   ; resultat dans AX");
        stocker_ax_dans(f, res);
        return;
    }

    /* ---- NOP : rien a generer ---- */
    if (strcmp(op, "NOP") == 0) return;

    /* Operateur inconnu */
    EMIT("    ; *** operateur non reconnu : '%s' ***", op);
}

/* =========================================================
   POINT D'ENTREE PRINCIPAL
   ========================================================= */

void generer_asm(const char *nom_fichier)
{
    if (!nom_fichier || !nom_fichier[0])
        nom_fichier = "sortie.asm";

    FILE *f = fopen(nom_fichier, "w");
    if (!f) {
        fprintf(stderr, "ERREUR: impossible de creer '%s'\n", nom_fichier);
        return;
    }

    printf("\n=== Generation assembleur 8086 : %s ===\n", nom_fichier);

    collecter_symboles();

    /* En-tete */
    EMIT("TITLE prolang.asm");
    EMIT("; Code 8086 genere automatiquement");
    EMIT("");

    /* Segment de pile */
    EMIT("PILE SEGMENT STACK");
    EMIT("    DW 100 DUP (?)");
    EMIT("base_pile EQU $");
    EMIT("PILE ENDS");
    EMIT("");

    /* Segment de donnees */
    emettre_segment_donnees(f);

    /* Segment de code */
    EMIT("LECODE SEGMENT");
    EMIT("");
    emettre_proc_print(f);
    emettre_proc_input(f);

    /* Point d'entree */
    EMIT("Debut:");
    EMIT("    ASSUME CS:LECODE, DS:DONNEE, SS:PILE");
    EMIT("");
    EMIT("    MOV AX, DONNEE");
    EMIT("    MOV DS, AX");
    EMIT("    MOV AX, PILE");
    EMIT("    MOV SS, AX");
    EMIT("    MOV SP, base_pile");
    EMIT("");

    /* Marquer les cibles de saut puis traduire */
    int *marque = (int *)calloc(qc + 1, sizeof(int));
    if (!marque) { fclose(f); return; }
    marquer_cibles(marque);

    int i;
    for (i = 0; i < qc; i++) {
        traduire_quadruplet(f, i, marque);
        EMIT("");
    }

    /* Label de fin + terminaison DOS */
    char lbl_fin[32];
    label_pour(qc, lbl_fin);
    EMIT("%s:", lbl_fin);
    EMIT("    MOV AH, 4Ch");
    EMIT("    INT 21h");
    EMIT("");
    EMIT("LECODE ENDS");
    EMIT("");
    EMIT("END Debut");

    free(marque);
    fclose(f);
    printf("[Fichier genere : %s]\n", nom_fichier);
}
