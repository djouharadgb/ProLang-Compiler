#include "asm8086.h"
#include "quad.h"
#include "ts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Macro : ecrire une ligne dans le fichier asm */
#define EMIT(fmt, ...)  fprintf(f, fmt "\n", ##__VA_ARGS__)

/* Virgule fixe pour les floats (2 decimales) */
#define SCALE 100

/* Marque les temporaires Tn qui portent des valeurs float (virgule fixe) */
static int temp_is_float[MAX_QUADS];


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

/* Verifie si s est un temporaire Tn (T suivi de chiffres) */
static int est_temporaire_asm(const char *s)
{
    int i;
    if (!s || s[0] != 'T') return 0;
    for (i = 1; s[i]; i++)
        if (!isdigit((unsigned char)s[i])) return 0;
    return (i > 1);
}

/* Extrait l'indice du temporaire Tn, ou -1 si ce n'en est pas un */
static int indice_temp(const char *s)
{
    return est_temporaire_asm(s) ? atoi(s + 1) : -1;
}

/* Convertit un litteral float en virgule fixe (valeur * SCALE) */
static long scale_literal(const char *s)
{
    double v = atof(s);
    if (v >= 0)
        return (long)(v * SCALE + 0.5);
    return (long)(v * SCALE - 0.5);
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

/* Detecte & verifie si un acces tableau "nom[indice]" et extrait nom, indice.
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

/* Determine si un operande doit etre traite comme float (virgule fixe) */
static int symbole_est_float(const char *s)
{
    if (!s || !*s) return 0;
    if (est_nombre(s)) return est_flottant(s);

    /* Temporaire Tn */
    int tidx = indice_temp(s);
    if (tidx >= 0 && tidx < MAX_QUADS) return temp_is_float[tidx];

    /* Acces tableau : on teste le type du nom de base */
    char ntab[64], idx[64];
    if (est_tableau_access(s, ntab, idx))
        return symbole_est_float(ntab);

    /* Symbole dans la TS */
    const char *type = ts_get_type(s);
    return (type && strcmp(type, "float") == 0);
}

/* Inference simple des types float pour les temporaires Tn */
static void inferer_types_temporaires(void)
{
    int i, changed;
    memset(temp_is_float, 0, sizeof(temp_is_float));

    do {
        changed = 0;
        for (i = 0; i < qc; i++) {
            const char *op  = quad[i].oper;
            const char *a   = quad[i].op1;
            const char *b   = quad[i].op2;
            const char *res = quad[i].res;

            int tidx = indice_temp(res);
            int aidx = indice_temp(a);
            int bidx = indice_temp(b);

            int a_is_float   = symbole_est_float(a);
            int b_is_float   = symbole_est_float(b);
            int res_is_float = symbole_est_float(res);

            if (strcmp(op, ":=") == 0) {
                if (tidx >= 0 && (a_is_float || res_is_float) && !temp_is_float[tidx]) {
                    temp_is_float[tidx] = 1; changed = 1;
                }
                if (res_is_float && aidx >= 0 && !temp_is_float[aidx]) {
                    temp_is_float[aidx] = 1; changed = 1;
                }
                continue;
            }

            if (strcmp(op, "TAB") == 0) {
                if (tidx >= 0 && a_is_float && !temp_is_float[tidx]) {
                    temp_is_float[tidx] = 1; changed = 1;
                }
                continue;
            }

            if (strcmp(op, "NEG") == 0) {
                if (tidx >= 0 && (a_is_float || res_is_float) && !temp_is_float[tidx]) {
                    temp_is_float[tidx] = 1; changed = 1;
                }
                if (res_is_float && aidx >= 0 && !temp_is_float[aidx]) {
                    temp_is_float[aidx] = 1; changed = 1;
                }
                continue;
            }

            if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 ||
                strcmp(op, "*") == 0 || strcmp(op, "/") == 0) {
                if (tidx >= 0 && (a_is_float || b_is_float || res_is_float) && !temp_is_float[tidx]) {
                    temp_is_float[tidx] = 1; changed = 1;
                }
                if (res_is_float) {
                    if (aidx >= 0 && !temp_is_float[aidx]) { temp_is_float[aidx] = 1; changed = 1; }
                    if (bidx >= 0 && !temp_is_float[bidx]) { temp_is_float[bidx] = 1; changed = 1; }
                }
                continue;
            }
        }
    } while (changed);
}

/* Met l'echelle SCALE dans un registre (conversion int -> float fixe) */
static void scale_reg(FILE *f, const char *reg)
{
    if (strcmp(reg, "AX") == 0) {
        EMIT("    MOV BX, %d", SCALE);
        EMIT("    IMUL BX");
        return;
    }

    /* Utiliser AX comme tampon pour ne pas ecraser reg */
    EMIT("    PUSH AX");
    EMIT("    MOV AX, %s", reg);
    EMIT("    MOV CX, %d", SCALE);
    EMIT("    IMUL CX");
    EMIT("    MOV %s, AX", reg);
    EMIT("    POP AX");
}

/* Charger un operande en virgule fixe (valeur * SCALE) */
static void charger_dans_reg_fp(FILE *f, const char *src, const char *reg)
{
    char ntab[64], idx[64];

    if (est_nombre(src)) {
        EMIT("    MOV %s, %ld", reg, scale_literal(src));
        return;
    }

    if (est_tableau_access(src, ntab, idx)) {
        if (est_nombre(idx))
            EMIT("    MOV SI, %ld", atol(idx) * 2);
        else {
            EMIT("    MOV SI, %s", nom_asm(idx));
            EMIT("    ADD SI, SI   ; SI = indice * 2");
        }
        EMIT("    MOV %s, %s[SI]", reg, nom_asm(ntab));

        /* Si le tableau est entier, convertir en float fixe */
        if (!symbole_est_float(ntab))
            scale_reg(f, reg);
        return;
    }

    EMIT("    MOV %s, %s", reg, nom_asm(src));
    if (!symbole_est_float(src))
        scale_reg(f, reg);
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
        /* ex: 3.141590 => 3, 2.500000 => 2 (8086 ne gere que les entiers) */
        EMIT("    MOV %s, %ld", reg, (long)atof(src)); // mov reg, val

    } else if (est_tableau_access(src, ntab, idx)) {
        /* Calcul de l'offset : chaque element fait 2 octets (DW) */
        if (est_nombre(idx))
            EMIT("    MOV SI, %ld", atol(idx) * 2); // mov SI, idx*2
        else { // indice non numerique (variable ou temporaire)
            EMIT("    MOV SI, %s", nom_asm(idx)); // mov SI, idx
            EMIT("    ADD SI, SI   ; SI = indice * 2");// add SI, SI
        }
        EMIT("    MOV %s, %s[SI]", reg, nom_asm(ntab));// mov reg, tab[SI]

    } else {
        EMIT("    MOV %s, %s", reg, nom_asm(src)); // mov reg, src
    }
}

/*
 stocker_ax_dans : stocke AX dans la destination 'dst'.
   - Acces tableau => T[SI] = AX
   - Variable/temp => MOV nom, AX
 */
static void stocker_ax_dans(FILE *f, const char *dst)
{
    char ntab[64], idx[64];

    if (est_tableau_access(dst, ntab, idx)) {
        if (est_nombre(idx))
            EMIT("    MOV SI, %ld", atol(idx) * 2); // mov SI, idx*2 car idx cste we know value cste
        else {
            EMIT("    MOV SI, %s", nom_asm(idx));// mov SI, idx 
            EMIT("    ADD SI, SI");// add SI, SI car idx est var 
        }
        EMIT("    MOV %s[SI], AX", nom_asm(ntab)); // mov tab[SI], AX
    } else {
        EMIT("    MOV %s, AX", nom_asm(dst));// mov dst, AX
    }
}

/* COLLECTE DES SYMBOLES (segment de donnees)*/

#define MAX_SYMS 512

typedef struct { //structure symbole 
    char nom[128];
    char type[16];  /* "integer" ou "float" */
    char val[64];   /* valeur initiale ou "" */
    int  taille;    /* 0 = scalaire, >0 = tableau */
    int  est_temp;  /* 1 = temporaire compilateur  */
} SymInfo;

static SymInfo syms[MAX_SYMS]; // table de symboles collectes pour le segment de donnees (variables, constantes, tableaux, temporaires)
static int     nb_syms = 0;

static int sym_existe(const char *nom) // Verifie si un symbole de nom 'nom' existe deja dans syms[]
{
    int i;
    for (i = 0; i < nb_syms; i++)
        if (strcmp(syms[i].nom, nom) == 0) return 1;
    return 0;
}

static void sym_ajouter(const char *nom, const char *type, //inserer tabe syminfo
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
/*variable : TS
Temp: Quads 
 */
static void collecter_symboles(void) 
{
    int i;
    nb_syms = 0;

    /* Determiner quels temporaires transportent des floats */
    inferer_types_temporaires();

    /* Variables, constantes, tableaux depuis la table des symboles */
    for (i = 0; i < HASH_SIZE; i++) {
        NoeudTS *n = hashTable[i];
        while (n) {
            if (n->state == 1) {
                int taille = (strcmp(n->code, "TABLEAU") == 0) ? atoi(n->val) : 0; //tableau on recupere sa taille n->val, if not t=0
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
            if (s[0]=='T' && isdigit((unsigned char)s[1]) && !strchr(s,'[')) {
                int tidx = indice_temp(s);
                const char *ttype = (tidx >= 0 && tidx < MAX_QUADS && temp_is_float[tidx])
                    ? "float" : "integer";
                sym_ajouter(s, ttype, "", 0, 1);
            }

            /* Indice de tableau qui serait un temporaire */
            char ntab[64], idx[64];
            if (est_tableau_access(s, ntab, idx)) //recupere nom et idx tableau
                if (idx[0]=='T' && isdigit((unsigned char)idx[1]))
                    sym_ajouter(idx, "integer", "", 0, 1);
        }
    }
}

/*SEGMENT DE DONNEES*/

static void emettre_segment_donnees(FILE *f)
{
    int i;
    EMIT("DONNEE SEGMENT");
    EMIT("");

    for (i = 0; i < nb_syms; i++) {
        SymInfo *s = &syms[i];
        const char *anom = nom_asm(s->nom);
        int is_float = (strcmp(s->type, "float") == 0);

        if (s->taille > 0) {  // taille sup 0, tableau
            EMIT("    %-20s DW %d DUP(0)  ; tableau", anom, s->taille); // ex: T0[10] => _T0 DW 10 DUP(0)
        } else if (s->est_temp) { 
            EMIT("    %-20s DW ?          ; temporaire", anom); //temporaire simple ex: T0 => _T0 DW ?
        } else if (strlen(s->val) > 0 && strcmp(s->val,"oui") != 0) { // constante ou variable initialisee (on traite "oui" comme une valeur de tableau non initialisee)

            if (is_float) {
                long v = scale_literal(s->val);
                EMIT("    %-20s DW %ld   ; float fixe (scale=%d) (val orig: %s)", anom, v, SCALE, s->val);
            } else {
                long v = (long)atof(s->val);
                EMIT("    %-20s DW %ld", anom, v);
            }
        } else {
            EMIT("    %-20s DW ?", anom);
        }
    }

    /* Tampon pour l'affichage (INT 21h attend '$' comme terminateur).*/
    EMIT("    _OUT_BUF             DB 12 DUP(?), '$'");
    EMIT("");
    EMIT("DONNEE ENDS");
    EMIT("");
}

/*  PROCEDURES  (print / read)*/

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

/* Procedure d'affichage d'un float en virgule fixe (AX) */
static void emettre_proc_print_fixed(FILE *f)
{
    EMIT(";---- _PRINT_FIXED : affiche AX en fixe (scale=%d) ----", SCALE);
    EMIT("_PRINT_FIXED PROC NEAR");
    EMIT("    PUSH AX"); EMIT("    PUSH BX"); EMIT("    PUSH CX");
    EMIT("    PUSH DX"); EMIT("    PUSH SI");
    EMIT("    CMP AX, 0");
    EMIT("    JGE _PF_POS");
    EMIT("    MOV AH, 02h");
    EMIT("    MOV DL, '-'");
    EMIT("    INT 21h");
    EMIT("    NEG AX");
    EMIT("_PF_POS:");
    EMIT("    MOV BX, %d", SCALE);
    EMIT("    XOR DX, DX");
    EMIT("    DIV BX          ; AX = partie entiere, DX = fraction");
    EMIT("    PUSH DX");
    EMIT("    CALL _PRINT_INT");
    EMIT("    POP DX");
    EMIT("    MOV AH, 02h");
    EMIT("    MOV DL, '.'");
    EMIT("    INT 21h");
    EMIT("    MOV AX, DX");
    EMIT("    MOV BX, 10");
    EMIT("    XOR DX, DX");
    EMIT("    DIV BX          ; AX = dizaine, DX = unite");
    EMIT("    PUSH DX");
    EMIT("    ADD AL, '0'");
    EMIT("    MOV DL, AL");
    EMIT("    MOV AH, 02h");
    EMIT("    INT 21h");
    EMIT("    POP DX");
    EMIT("    ADD DL, '0'");
    EMIT("    MOV AH, 02h");
    EMIT("    INT 21h");
    EMIT("    POP SI"); EMIT("    POP DX"); EMIT("    POP CX");
    EMIT("    POP BX"); EMIT("    POP AX");
    EMIT("    RET");
    EMIT("_PRINT_FIXED ENDP");
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

/* Procedure de lecture d'un float fixe (partie entiere * SCALE) */
static void emettre_proc_input_fixed(FILE *f)
{
    EMIT(";---- _READ_FIXED : lit un entier et applique l'echelle ----");
    EMIT("_READ_FIXED PROC NEAR");
    EMIT("    PUSH BX");
    EMIT("    CALL _READ_INT");
    EMIT("    MOV BX, %d", SCALE);
    EMIT("    IMUL BX");
    EMIT("    POP BX");
    EMIT("    RET");
    EMIT("_READ_FIXED ENDP");
    EMIT("");
}

/* LABELS DE SAUT */

static void label_pour(int idx, char *buf) { sprintf(buf, "L%d", idx); } // genere un label unique pour l'indice de quadruplet donne (ex: L42 pour idx=42)

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

/* TRADUCTION D'UN QUADRUPLET*/

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
    EMIT("    ; [%d] (%s, %s, %s, %s)", idx, op, a, b, res); //quad commentaire dans asm pour reference

    /* Affectation simple  res := a */
    if (strcmp(op, ":=") == 0) {
        if (symbole_est_float(res) || symbole_est_float(a)) {
            if (symbole_est_float(res)) {
                charger_dans_reg_fp(f, a, "AX");
            } else {
                charger_dans_reg_fp(f, a, "AX");
                EMIT("    MOV BX, %d", SCALE);
                EMIT("    CWD");
                EMIT("    IDIV BX   ; conversion float fixe -> entier");
            }
            stocker_ax_dans(f, res);
            return;
        }
        charger_dans_reg(f, a, "AX");
        stocker_ax_dans(f, res);
        return;
    }

    /* Addition et Soustraction  res = a +/- b 
       On factorise les deux car le schema est identique. */
    if (strcmp(op,"+")==0 || strcmp(op,"-")==0) {
        const char *mnem = (strcmp(op,"+")==0) ? "ADD" : "SUB";
        if (symbole_est_float(a) || symbole_est_float(b) || symbole_est_float(res)) {
            charger_dans_reg_fp(f, a, "AX");
            charger_dans_reg_fp(f, b, "BX");
            EMIT("    %s AX, BX", mnem);
            stocker_ax_dans(f, res);
            return;
        }
        charger_dans_reg(f, a, "AX");
        if (est_nombre(b))
           
            EMIT("    %s AX, %ld", mnem, (long)atof(b));// optimisations : x+0 => x, x-0 => x
        else {
            charger_dans_reg(f, b, "BX");
            EMIT("    %s AX, BX", mnem);
        }
        stocker_ax_dans(f, res);
        return;
    }

    /* Multiplication et Division  res = a * / b 
       Schema identique : charger BX, puis IMUL ou IDIV.
       La division a besoin de CWD pour etendre AX vers DX:AX. */
    if (strcmp(op,"*")==0 || strcmp(op,"/")==0) {
        int est_div = (strcmp(op,"/") == 0);
        if (symbole_est_float(a) || symbole_est_float(b) || symbole_est_float(res)) {
            charger_dans_reg_fp(f, a, "AX");
            charger_dans_reg_fp(f, b, "BX");
            if (est_div) {
                EMIT("    MOV CX, %d", SCALE);
                EMIT("    IMUL CX");
                EMIT("    IDIV BX");
            } else {
                EMIT("    IMUL BX");
                EMIT("    MOV BX, %d", SCALE);
                EMIT("    IDIV BX");
            }
            stocker_ax_dans(f, res);
            return;
        }
        charger_dans_reg(f, a, "AX");

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

        EMIT("    MOV BX, %ld", (long)atof(b)); // optimisation : x*1 => x, x/1 => x
        else
            charger_dans_reg(f, b, "BX");
        if (est_div) EMIT("    CWD            ; etendre AX -> DX:AX");
        EMIT("    %s BX", est_div ? "IDIV" : "IMUL");
        stocker_ax_dans(f, res);
        return;
    }

    /* Negation unaire  res = -a */
    if (strcmp(op, "NEG") == 0) {
        charger_dans_reg(f, a, "AX");
        EMIT("    NEG AX");
        stocker_ax_dans(f, res);
        return;
    }

    /* Comparaisons  res = (a op b)  =>  0 ou 1
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

        if (symbole_est_float(a) || symbole_est_float(b)) {
            charger_dans_reg_fp(f, a, "AX");
            charger_dans_reg_fp(f, b, "BX");
            EMIT("    CMP AX, BX");
        } else {
            charger_dans_reg(f, a, "AX");
            if (est_nombre(b))
                EMIT("    CMP AX, %ld", (long)atof(b));
            else {
                charger_dans_reg(f, b, "BX");
                EMIT("    CMP AX, BX");
            }
        }
        EMIT("    %s %s", jcond, lbl_vrai);
        EMIT("    MOV %s, 0", nom_asm(res));
        EMIT("    JMP %s", lbl_suite);
        EMIT("%s:", lbl_vrai);
        EMIT("    MOV %s, 1", nom_asm(res));
        EMIT("%s:", lbl_suite);
        return;
    }

    /* AND et OR logiques  (meme schema, mnemonique different)*/
    if (strcmp(op,"AND")==0 || strcmp(op,"OR")==0) {
        charger_dans_reg(f, a, "AX");
        charger_dans_reg(f, b, "BX");
        EMIT("    %s AX, BX", op);   /* AND AX,BX  ou  OR AX,BX */
        stocker_ax_dans(f, res);
        return;
    }

    /* NON logique : 0->1, non-zero->0 */
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

    /* Saut conditionnel  BZ : si a == 0, aller a res */
    if (strcmp(op, "BZ") == 0) {
        char cible[32];
        label_pour(atoi(res), cible);
        charger_dans_reg(f, a, "AX");
        EMIT("    CMP AX, 0");
        EMIT("    JE  %s", cible);
        return;
    }

    /* Saut inconditionnel  BR : aller a res*/
    if (strcmp(op, "BR") == 0) {
        char cible[32];
        label_pour(atoi(res), cible);
        EMIT("    JMP %s", cible);
        return;
    }

    /* Lecture tableau  res = TAB[b] */
    if (strcmp(op, "TAB") == 0) {
        if (est_nombre(b))
            EMIT("    MOV SI, %ld", atol(b) * 2);
        else {
            EMIT("    MOV SI, %s", nom_asm(b));
            EMIT("    ADD SI, SI");
        }
        EMIT("    MOV AX, %s[SI]", nom_asm(a));
        if (symbole_est_float(res) && !symbole_est_float(a))
            scale_reg(f, "AX");
        else if (!symbole_est_float(res) && symbole_est_float(a)) {
            EMIT("    MOV BX, %d", SCALE);
            EMIT("    CWD");
            EMIT("    IDIV BX   ; conversion float fixe -> entier");
        }
        stocker_ax_dans(f, res);
        return;
    }

    /* Affichage  out variable ou chaine */
    if (strcmp(op, "out") == 0) {
        if (a && a[0] == '"') {
           

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
            /* Variable ou nombre : appel _PRINT_INT / _PRINT_FIXED */
            if (symbole_est_float(a)) {
                charger_dans_reg_fp(f, a, "AX");
                EMIT("    CALL _PRINT_FIXED");
            } else {
                charger_dans_reg(f, a, "AX");
                EMIT("    CALL _PRINT_INT");
            }
        }
        if (!(a && a[0] == '"'))
            emit_newline(f);
        return;
    }

    /* Lecture clavier  input -> res  */
    if (strcmp(op, "input") == 0) {
        if (symbole_est_float(res))
            EMIT("    CALL _READ_FIXED   ; resultat dans AX");
        else
            EMIT("    CALL _READ_INT   ; resultat dans AX");
        stocker_ax_dans(f, res);
        return;
    }

    /* NOP : rien a generer  */
    if (strcmp(op, "NOP") == 0) return;

    /* Operateur inconnu */
    EMIT("    ; *** operateur non reconnu : '%s' ***", op);
}

/* POINT D'ENTREE PRINCIPAL */

void generer_asm(const char *nom_fichier)
{
    if (!nom_fichier || !nom_fichier[0])
        nom_fichier = "sortie_djo.asm";

    FILE *f = fopen(nom_fichier, "w");
    if (!f) {
        fprintf(stderr, "ERREUR: impossible de creer '%s'\n", nom_fichier);
        return;
    }

    printf("\n=== Generation assembleur 8086 : %s ===\n", nom_fichier);

    collecter_symboles();

    /* En-tete */
    EMIT("TITLE prolang.asm");
    EMIT("; Code 8086 genere automatiquement :))");
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
    emettre_proc_print_fixed(f);
    emettre_proc_input(f);
    emettre_proc_input_fixed(f);

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