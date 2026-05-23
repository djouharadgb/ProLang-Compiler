/*
 * asm8086.c  -  Generation de code assembleur 8086
 *
 * Cours USTHB 2025-2026, SAYOUD Lynda, diapositives 115-148.
 *
 * Traduit chaque quadruplet (oper, op1, op2, res) en une ou plusieurs
 * instructions assembleur 8086 et produit un fichier .asm complet
 * (pile, donnees, code) pret a etre assemble avec MASM/TASM.
 *
 * Operateurs traites :
 *   :=              affectation simple
 *   + - * /         arithmetique binaire
 *   NEG             negation unaire
 *   == != < > <= >= comparaison  (produit 0 ou 1 dans un temporaire)
 *   AND OR NON      logique booleenne
 *   BZ              saut conditionnel (si res == 0, sauter a l'adresse)
 *   BR              saut inconditionnel
 *   TAB             lecture tableau  (res = TAB[indice])
 *   :=TAB           ecriture tableau (TAB[indice] = val)  -- encodee comme ":="
 *   input           lecture clavier  (DOS INT 21h / appel simplifie)
 *   out             affichage ecran  (DOS INT 21h)
 */

#include "asm8086.h"
#include "quad.h"
#include "ts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* =========================================================
   MACROS DE CONFORT
   ========================================================= */
#define EMIT(fmt, ...)  fprintf(f, fmt "\n", ##__VA_ARGS__)
#define EMITL(lbl, fmt, ...) fprintf(f, "%s: " fmt "\n", lbl, ##__VA_ARGS__)

/* =========================================================
   UTILITAIRES
   ========================================================= */

/* Verifie si s est un literral numerique (entier ou flottant) */
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

/* Verifie si la chaine contient un point (=> flottant) */
static int est_flottant(const char *s)
{
    return s && strchr(s, '.') != NULL;
}

/*
 * nom_asm : convertit un nom de variable/temporaire en identifiant
 * valide pour l'assembleur 8086 (evite les conflits avec les mots-cles).
 * Les temporaires T0, T1, ... deviennent _T0, _T1, ...
 * Les tableaux de la forme "nom[idx]" sont geres separement.
 */
static const char *nom_asm(const char *s)
{
    static char buf[128];
    if (!s || !*s) return s;
    /* Temporaires generes par le compilateur : prefixe _ pour securite */
    if (s[0] == 'T' && isdigit((unsigned char)s[1])) {
        snprintf(buf, sizeof(buf), "_T%s", s + 1);
        return buf;
    }
    return s;
}

/*
 * est_tableau_access : detecte si la chaine a la forme "nom[indice]"
 * et extrait le nom du tableau et l'indice dans les buffers fournis.
 * Retourne 1 si oui, 0 sinon.
 */
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

/* =========================================================
   COLLECTE DES SYMBOLES DU SEGMENT DE DONNEES
   ========================================================= */

/*
 * On parcourt la table des symboles (TS) pour extraire toutes les
 * variables, constantes et tableaux a declarer dans DONNEE SEGMENT.
 * Les temporaires (T0, T1, ...) generes par le compilateur sont aussi
 * collectes depuis les quadruplets.
 */

#define MAX_SYMS 512

typedef struct {
    char  nom[128];
    char  type[16];   /* "integer" ou "float" */
    char  val[64];    /* valeur initiale ou "" */
    int   taille;     /* 0 = scalaire, >0 = tableau */
    int   est_temp;   /* 1 = temporaire compile */
} SymInfo;

static SymInfo syms[MAX_SYMS];
static int     nb_syms = 0;

/* Verifie si le symbole est deja dans la liste */
static int sym_existe(const char *nom)
{
    int i;
    for (i = 0; i < nb_syms; i++)
        if (strcmp(syms[i].nom, nom) == 0) return 1;
    return 0;
}

/* Ajoute un symbole */
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

/*
 * Parcourt la table de hachage des symboles pour reconstruire la liste
 * de tout ce qui est a declarer dans le segment de donnees.
 */
static void collecter_symboles(void)
{
    int i;
    nb_syms = 0;

    /* --- Variables, constantes et tableaux depuis la TS --- */
    for (i = 0; i < HASH_SIZE; i++) {
        NoeudTS *n = hashTable[i];
        while (n) {
            if (n->state == 1) {
                int taille = 0;
                if (strcmp(n->code, "TABLEAU") == 0)
                    taille = atoi(n->val);
                const char *val_init = "";
                if (strcmp(n->code, "CONST") == 0 || strlen(n->val) > 0)
                    val_init = n->val;
                /* Pour les tableaux la val est la taille, pas une valeur init */
                if (taille > 0) val_init = "";
                sym_ajouter(n->name, n->type, val_init, taille, 0);
            }
            n = n->suivant;
        }
    }

    /* --- Temporaires T0, T1, ... depuis les quadruplets --- */
    for (i = 0; i < qc; i++) {
        const char *fields[4] = {
            quad[i].op1, quad[i].op2, quad[i].res, NULL
        };
        int f;
        for (f = 0; f < 3; f++) {
            const char *s = fields[f];
            if (!s || !*s) continue;
            if (est_nombre(s)) continue;

            /* Temporaire direct */
            if (s[0] == 'T' && isdigit((unsigned char)s[1]) && !strchr(s, '[')) {
                if (!sym_existe(s))
                    sym_ajouter(s, "integer", "", 0, 1);
            }

            /* Acces tableau (nom[indice]) - on extrait le nom du tableau */
            char ntab[64], idx[64];
            if (est_tableau_access(s, ntab, idx)) {
                /* Le tableau est deja dans la TS, rien de plus a faire ici */
                /* Mais si l'indice est un temporaire, l'ajouter */
                if (idx[0] == 'T' && isdigit((unsigned char)idx[1]) && !sym_existe(idx))
                    sym_ajouter(idx, "integer", "", 0, 1);
            }
        }
    }
}

/* =========================================================
   EMISSION DU SEGMENT DE DONNEES
   ========================================================= */

/*
 * Pour l'assembleur 8086 (diapo 137-141) :
 *   - entier scalaire  -> DW (2 octets, word)
 *   - flottant scalaire -> DD (4 octets, dword) ; note: en 8086 pur les
 *     flottants ne sont pas directement supportes par le FPU sans co-processeur
 *     8087, mais on les declare en DD et on les manipule via AX:DX.
 *     Dans ce projet on les traite comme des entiers 16-bit pour simplifier.
 *   - tableau d'entiers -> DW taille DUP (?)
 *   - temporaire        -> DW ? (non initialise)
 */
static void emettre_segment_donnees(FILE *f)
{
    int i;
    EMIT("DONNEE SEGMENT");
    EMIT("");

    for (i = 0; i < nb_syms; i++) {
        SymInfo *s = &syms[i];
        /* Nom de la variable dans l'assembleur */
        const char *anom = nom_asm(s->nom);
        int is_float = (strcmp(s->type, "float") == 0);

        if (s->taille > 0) {
            /* Tableau : taille DUP(?) */
            EMIT("    %-20s DW %d DUP(?)    ; tableau de %d entiers",
                 anom, s->taille, s->taille);

        } else if (s->est_temp) {
            /* Temporaire : non initialise */
            EMIT("    %-20s DW ?             ; temporaire compilateur", anom);

        } else if (strlen(s->val) > 0
                   && strcmp(s->val, "oui") != 0) { /* "oui" = juste marque init, pas de valeur */
            /* Variable avec valeur d'initialisation concrete */
            /* Variable/constante initialisee */
            if (is_float) {
                /* On stocke la partie entiere uniquement (simplification 8086) */
                long v = (long)atof(s->val);
                EMIT("    %-20s DW %ld           ; float (partie entiere)", anom, v);
            } else {
                EMIT("    %-20s DW %s", anom, s->val);
            }
        } else {
            /* Variable non initialisee */
            EMIT("    %-20s DW ?", anom);
        }
    }

    /* Chaine pour l'affichage OUT (DOS INT 21h attend '$' comme terminateur) */
    EMIT("    _OUT_BUF             DB 10 DUP(?), '$' ; tampon affichage");
    EMIT("");
    EMIT("DONNEE ENDS");
    EMIT("");
}

/* =========================================================
   GENERATION DE CODE : utilitaires de chargement
   ========================================================= */

/*
 * charger_dans_ax : emet les instructions pour placer la valeur de
 * l'operande 'src' dans le registre AX.
 *   - Si src est un nombre litteral : MOV AX, valeur
 *   - Si src est un acces tableau  : calcul index * 2, MOV AX, tab[SI]
 *   - Sinon (variable/temporaire)  : MOV AX, nom
 */
static void charger_dans_ax(FILE *f, const char *src)
{
    char ntab[64], idx[64];

    if (est_nombre(src)) {
        /* Valeur immediate (diapo 125, adressage immediat) */
        long v = (long)atof(src);
        EMIT("    MOV AX, %ld", v);

    } else if (est_tableau_access(src, ntab, idx)) {
        /* Acces tableau T[i] : SI = i*2, AX = T[SI]  (diapo 141) */
        if (est_nombre(idx)) {
            long offset = atol(idx) * 2;
            EMIT("    MOV SI, %ld", offset);
        } else {
            EMIT("    MOV SI, %s", nom_asm(idx));
            EMIT("    ADD SI, SI         ; SI = indice * 2 (DW = 2 octets)");
        }
        EMIT("    MOV AX, %s[SI]", nom_asm(ntab));

    } else {
        /* Variable ou temporaire (diapo 125, registre <-> memoire) */
        EMIT("    MOV AX, %s", nom_asm(src));
    }
}

/*
 * charger_dans_bx : idem pour BX (utile pour le second operande
 * d'une operation binaire afin de ne pas ecraser AX).
 */
static void charger_dans_bx(FILE *f, const char *src)
{
    char ntab[64], idx[64];

    if (est_nombre(src)) {
        long v = (long)atof(src);
        EMIT("    MOV BX, %ld", v);
    } else if (est_tableau_access(src, ntab, idx)) {
        if (est_nombre(idx)) {
            long offset = atol(idx) * 2;
            EMIT("    MOV SI, %ld", offset);
        } else {
            EMIT("    MOV SI, %s", nom_asm(idx));
            EMIT("    ADD SI, SI");
        }
        EMIT("    MOV BX, %s[SI]", nom_asm(ntab));
    } else {
        EMIT("    MOV BX, %s", nom_asm(src));
    }
}

/*
 * stocker_ax_dans : emet les instructions pour stocker AX dans
 * la destination 'dst'.
 *   - Si dst est un acces tableau : calcul index * 2, T[SI] = AX
 *   - Sinon                       : MOV nom, AX
 */
static void stocker_ax_dans(FILE *f, const char *dst)
{
    char ntab[64], idx[64];

    if (est_tableau_access(dst, ntab, idx)) {
        /* Ecriture tableau T[i] = AX  (diapo 141) */
        if (est_nombre(idx)) {
            long offset = atol(idx) * 2;
            EMIT("    MOV SI, %ld", offset);
        } else {
            EMIT("    MOV SI, %s", nom_asm(idx));
            EMIT("    ADD SI, SI");
        }
        EMIT("    MOV %s[SI], AX", nom_asm(ntab));
    } else {
        EMIT("    MOV %s, AX", nom_asm(dst));
    }
}

/* =========================================================
   CONVERSION ENTIER -> CHAINE (pour OUT)
   Procedure itoa_proc en assembleur, appelee via CALL.
   On inclut une mini-routine dans le segment de code qui convertit
   AX en chaine decimale dans _OUT_BUF puis appelle INT 21h.
   ========================================================= */
static void emettre_proc_print(FILE *f)
{
    EMIT(";---- Procedure d'affichage d'un entier dans AX ----");
    EMIT("_PRINT_INT PROC NEAR");
    EMIT("    ; Convertit AX en chaine et affiche via DOS INT 21h");
    EMIT("    PUSH AX");
    EMIT("    PUSH BX");
    EMIT("    PUSH CX");
    EMIT("    PUSH DX");
    EMIT("    PUSH SI");
    EMIT("    MOV SI, OFFSET _OUT_BUF");
    EMIT("    ; Gerer le signe negatif");
    EMIT("    CMP AX, 0");
    EMIT("    JGE _PRINT_POS");
    EMIT("    MOV BYTE PTR [SI], '-'");
    EMIT("    INC SI");
    EMIT("    NEG AX");
    EMIT("_PRINT_POS:");
    EMIT("    ; Divisor BX = 10, CX compte les chiffres");
    EMIT("    MOV BX, 10");
    EMIT("    MOV CX, 0");
    EMIT("_DIV_LOOP:");
    EMIT("    MOV DX, 0");
    EMIT("    DIV BX             ; AX = AX / 10, DX = reste");
    EMIT("    ADD DL, '0'");
    EMIT("    PUSH DX            ; empiler le chiffre (LIFO => ordre inverse)");
    EMIT("    INC CX");
    EMIT("    CMP AX, 0");
    EMIT("    JNE _DIV_LOOP");
    EMIT("_POP_LOOP:");
    EMIT("    POP DX");
    EMIT("    MOV [SI], DL");
    EMIT("    INC SI");
    EMIT("    LOOP _POP_LOOP");
    EMIT("    MOV BYTE PTR [SI], '$'  ; terminateur DOS");
    EMIT("    MOV AH, 09h");
    EMIT("    MOV DX, OFFSET _OUT_BUF");
    EMIT("    INT 21h");
    EMIT("    ; Afficher un espace separateur");
    EMIT("    MOV AH, 02h");
    EMIT("    MOV DL, ' '");
    EMIT("    INT 21h");
    EMIT("    POP SI");
    EMIT("    POP DX");
    EMIT("    POP CX");
    EMIT("    POP BX");
    EMIT("    POP AX");
    EMIT("    RET");
    EMIT("_PRINT_INT ENDP");
    EMIT("");
}

/* =========================================================
   PROCEDURE DE LECTURE CLAVIER (INPUT)
   Lit un entier depuis stdin via INT 21h buffered input.
   Stocke le resultat (valeur ASCII-decoded) dans AX.
   La variable cible sera affectee ensuite.
   ========================================================= */
static void emettre_proc_input(FILE *f)
{
    EMIT(";---- Procedure de lecture d'un entier depuis le clavier ----");
    EMIT("_READ_INT PROC NEAR");
    EMIT("    ; Lit les caracteres un a un (INT 21h AH=01h) et construit l'entier");
    EMIT("    PUSH BX");
    EMIT("    PUSH CX");
    EMIT("    PUSH DX");
    EMIT("    MOV BX, 0     ; accumulateur");
    EMIT("    MOV CX, 0     ; signe (0=positif, 1=negatif)");
    EMIT("_READ_CHAR:");
    EMIT("    MOV AH, 01h");
    EMIT("    INT 21h       ; AL = caractere lu (avec echo)");
    EMIT("    CMP AL, 0Dh   ; Entree (CR) ?");
    EMIT("    JE  _READ_DONE");
    EMIT("    CMP AL, '-'");
    EMIT("    JNE _READ_DIGIT");
    EMIT("    MOV CX, 1     ; marquer negatif");
    EMIT("    JMP _READ_CHAR");
    EMIT("_READ_DIGIT:");
    EMIT("    SUB AL, '0'   ; convertir ASCII -> chiffre");
    EMIT("    CBW            ; etendre AL vers AX");
    EMIT("    XCHG AX, BX");
    EMIT("    MOV DX, 10");
    EMIT("    MUL DX        ; AX = BX * 10");
    EMIT("    ADD AX, BX    ; AX = AX + nouveau chiffre");
    EMIT("    XCHG AX, BX");
    EMIT("    JMP _READ_CHAR");
    EMIT("_READ_DONE:");
    EMIT("    MOV AX, BX");
    EMIT("    CMP CX, 1");
    EMIT("    JNE _READ_POS");
    EMIT("    NEG AX");
    EMIT("_READ_POS:");
    EMIT("    ; newline");
    EMIT("    MOV AH, 02h");
    EMIT("    MOV DL, 0Ah");
    EMIT("    INT 21h");
    EMIT("    POP DX");
    EMIT("    POP CX");
    EMIT("    POP BX");
    EMIT("    RET");
    EMIT("_READ_INT ENDP");
    EMIT("");
}

/* =========================================================
   COMPTEUR DE LABELS pour BZ/BR
   Chaque quadruplet BZ/BR a besoin d'un label de saut.
   On genere des labels L0, L1, L2, ... associes aux indices.
   ========================================================= */

/*
 * label_pour : retourne le label assembleur correspondant a un index
 * de quadruplet (ex: index 5 => "L5").
 */
static void label_pour(int idx, char *buf)
{
    sprintf(buf, "L%d", idx);
}

/*
 * indices_cibles : construit un tableau marque[qc] ou marque[i]=1
 * si l'indice i est une cible de saut (BZ ou BR) => on placera un label.
 */
static void marquer_cibles(int *marque)
{
    int i;
    memset(marque, 0, qc * sizeof(int));
    for (i = 0; i < qc; i++) {
        if (strcmp(quad[i].oper, "BZ") == 0 || strcmp(quad[i].oper, "BR") == 0) {
            if (quad[i].res && quad[i].res[0] != '\0') {
                int cible = atoi(quad[i].res);
                if (cible >= 0 && cible < qc)
                    marque[cible] = 1;
            }
        }
    }
    /* La premiere instruction est toujours une cible implicite (Debut:) */
    if (qc > 0) marque[0] = 1;
}

/* =========================================================
   TRADUCTION D'UN QUADRUPLET
   ========================================================= */

static void traduire_quadruplet(FILE *f, int idx, int *marque_label)
{
    const char *op  = quad[idx].oper;
    const char *a   = quad[idx].op1;
    const char *b   = quad[idx].op2;
    const char *res = quad[idx].res;
    char lbl[32];

    /* Placer un label si cet indice est une cible de saut */
    if (marque_label[idx]) {
        label_pour(idx, lbl);
        EMIT("%s:", lbl);
    }

    /* ---- Commentaire de reference ---- */
    EMIT("    ; [%d] (%s, %s, %s, %s)", idx, op, a, b, res);

    /* ============================================================
       AFFECTATION SIMPLE  res := a
       diapo 125 : MOV var, val / MOV reg, var
       ============================================================ */
    if (strcmp(op, ":=") == 0) {
        /* Verifier si res est un acces tableau */
        char ntab_res[64], idx_res[64];
        int res_est_tab = est_tableau_access(res, ntab_res, idx_res);

        charger_dans_ax(f, a);
        if (res_est_tab) {
            stocker_ax_dans(f, res);
        } else {
            stocker_ax_dans(f, res);
        }
        return;
    }

    /* ============================================================
       ADDITION   res = a + b
       diapo 127 : ADD AX, BX
       ============================================================ */
    if (strcmp(op, "+") == 0) {
        charger_dans_ax(f, a);
        if (est_nombre(b)) {
            long v = (long)atof(b);
            EMIT("    ADD AX, %ld", v);
        } else {
            charger_dans_bx(f, b);
            EMIT("    ADD AX, BX");
        }
        stocker_ax_dans(f, res);
        return;
    }

    /* ============================================================
       SOUSTRACTION   res = a - b
       diapo 127 : SUB AX, BX
       ============================================================ */
    if (strcmp(op, "-") == 0) {
        charger_dans_ax(f, a);
        if (est_nombre(b)) {
            long v = (long)atof(b);
            EMIT("    SUB AX, %ld", v);
        } else {
            charger_dans_bx(f, b);
            EMIT("    SUB AX, BX");
        }
        stocker_ax_dans(f, res);
        return;
    }

    /* ============================================================
       MULTIPLICATION   res = a * b
       8086 : MUL src  (AX = AX * src, resultat dans DX:AX)
       On utilise IMUL (signe) pour coherence avec les entiers signes.
       ============================================================ */
    if (strcmp(op, "*") == 0) {
        charger_dans_ax(f, a);
        if (est_nombre(b)) {
            long v = (long)atof(b);
            EMIT("    MOV BX, %ld", v);
        } else {
            charger_dans_bx(f, b);
        }
        EMIT("    IMUL BX            ; DX:AX = AX * BX (signe)");
        /* On ne garde que AX (16 bits) - suffisant pour les programmes du cours */
        stocker_ax_dans(f, res);
        return;
    }

    /* ============================================================
       DIVISION   res = a / b
       8086 : IDIV src  (AX = DX:AX / src, AX=quotient, DX=reste)
       DX doit etre etendu signe depuis AX avant (CWD).
       ============================================================ */
    if (strcmp(op, "/") == 0) {
        charger_dans_ax(f, a);
        if (est_nombre(b)) {
            long v = (long)atof(b);
            EMIT("    MOV BX, %ld", v);
        } else {
            charger_dans_bx(f, b);
        }
        EMIT("    CWD                ; etendre AX vers DX:AX (signe)");
        EMIT("    IDIV BX            ; AX = quotient, DX = reste");
        stocker_ax_dans(f, res);
        return;
    }

    /* ============================================================
       NEGATION UNAIRE   res = -a
       ============================================================ */
    if (strcmp(op, "NEG") == 0) {
        charger_dans_ax(f, a);
        EMIT("    NEG AX");
        stocker_ax_dans(f, res);
        return;
    }

    /* ============================================================
       COMPARAISONS   res = (a op b)  => res vaut 1 (vrai) ou 0 (faux)
       Principe (diapo 128-130) :
         CMP AX, BX   ; flags mis a jour
         Jcond VRAI   ; si condition vraie, sauter
         MOV res, 0   ; sinon res = 0
         JMP SUITE
         VRAI: MOV res, 1
         SUITE:
       ============================================================ */
    if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
        strcmp(op, "<")  == 0 || strcmp(op, ">")  == 0 ||
        strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0) {

        /* Mnemonique de saut conditionnel correspondant */
        const char *jcond = "JE"; /* == */
        if      (strcmp(op, "!=") == 0) jcond = "JNE";
        else if (strcmp(op, "<")  == 0) jcond = "JL";
        else if (strcmp(op, ">")  == 0) jcond = "JG";
        else if (strcmp(op, "<=") == 0) jcond = "JLE";
        else if (strcmp(op, ">=") == 0) jcond = "JGE";

        /* Labels locaux uniques pour ce quadruplet */
        char lbl_vrai[32], lbl_suite[32];
        sprintf(lbl_vrai,  "_CMP%d_V", idx);
        sprintf(lbl_suite, "_CMP%d_S", idx);

        charger_dans_ax(f, a);
        if (est_nombre(b)) {
            long v = (long)atof(b);
            EMIT("    CMP AX, %ld", v);
        } else {
            charger_dans_bx(f, b);
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

    /* ============================================================
       LOGIQUE AND / OR / NON
       res = a AND b  =>  res = a & b (pour des valeurs 0/1)
       On utilise AND/OR sur AX et BX directement.
       ============================================================ */
    if (strcmp(op, "AND") == 0) {
        charger_dans_ax(f, a);
        charger_dans_bx(f, b);
        EMIT("    AND AX, BX");
        stocker_ax_dans(f, res);
        return;
    }
    if (strcmp(op, "OR") == 0) {
        charger_dans_ax(f, a);
        charger_dans_bx(f, b);
        EMIT("    OR  AX, BX");
        stocker_ax_dans(f, res);
        return;
    }
    if (strcmp(op, "NON") == 0) {
        /* NON logique : si AX == 0 => 1, sinon => 0  */
        char lbl_un[32], lbl_fin[32];
        sprintf(lbl_un,  "_NON%d_U", idx);
        sprintf(lbl_fin, "_NON%d_F", idx);
        charger_dans_ax(f, a);
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

    /* ============================================================
       SAUT CONDITIONNEL  BZ cond _ cible
       Si cond == 0 (faux), sauter au label Lcible.
       diapo 130 : CMP + JE
       ============================================================ */
    if (strcmp(op, "BZ") == 0) {
        charger_dans_ax(f, a);
        EMIT("    CMP AX, 0");
        char cible_lbl[32];
        label_pour(atoi(res), cible_lbl);
        EMIT("    JE  %s            ; sauter si condition fausse", cible_lbl);
        return;
    }

    /* ============================================================
       SAUT INCONDITIONNEL  BR _ _ cible
       diapo 130 : JMP label
       ============================================================ */
    if (strcmp(op, "BR") == 0) {
        char cible_lbl[32];
        label_pour(atoi(res), cible_lbl);
        EMIT("    JMP %s", cible_lbl);
        return;
    }

    /* ============================================================
       ACCES TABLEAU (lecture)  res = TAB[indice]
       Genere par le compilateur avec operateur "TAB".
       diapo 141 : MOV SI, indice*2 / MOV AX, T[SI]
       ============================================================ */
    if (strcmp(op, "TAB") == 0) {
        /* a = nom du tableau, b = indice */
        if (est_nombre(b)) {
            long offset = atol(b) * 2;
            EMIT("    MOV SI, %ld", offset);
        } else {
            EMIT("    MOV SI, %s", nom_asm(b));
            EMIT("    ADD SI, SI");
        }
        EMIT("    MOV AX, %s[SI]", nom_asm(a));
        stocker_ax_dans(f, res);
        return;
    }

    /* ============================================================
       SORTIE  out variable/literal
       Appel de la procedure _PRINT_INT pour les nombres.
       Pour les chaines (STRING), on les inclut inline dans les donnees.
       ============================================================ */
    if (strcmp(op, "out") == 0) {
        /* Detecter si c'est une chaine litterale (commence et finit par ") */
        if (a && a[0] == '"') {
            /* Chaine : afficher via INT 21h AH=09h directement */
            /* On a besoin d'un label unique pour cette chaine dans le segment donnees */
            /* Simple approche : afficher caractere par caractere via AH=02h */
            EMIT("    ; OUT chaine (affichage caractere par caractere)");
            const char *p = a + 1; /* sauter le guillemet ouvrant */
            while (*p && *p != '"') {
                unsigned char c = (unsigned char)*p;
                if (c == '\\' && *(p+1) == 'n') {
                    EMIT("    MOV AH, 02h");
                    EMIT("    MOV DL, 0Ah");
                    EMIT("    INT 21h");
                    p += 2; continue;
                }
                EMIT("    MOV AH, 02h");
                EMIT("    MOV DL, %d       ; '%c'", c, (c >= 32 && c < 127) ? c : '?');
                EMIT("    INT 21h");
                p++;
            }
            /* Newline apres la chaine */
            EMIT("    MOV AH, 02h");
            EMIT("    MOV DL, 0Ah");
            EMIT("    INT 21h");
        } else {
            /* Nombre ou variable : appel _PRINT_INT */
            charger_dans_ax(f, a);
            EMIT("    CALL _PRINT_INT");
            /* Newline */
            EMIT("    MOV AH, 02h");
            EMIT("    MOV DL, 0Ah");
            EMIT("    INT 21h");
        }
        return;
    }

    /* ============================================================
       ENTREE  input _ _ variable
       Appel de la procedure _READ_INT.
       ============================================================ */
    if (strcmp(op, "input") == 0) {
        EMIT("    CALL _READ_INT     ; resultat dans AX");
        stocker_ax_dans(f, res);
        return;
    }

    /* ============================================================
       NOP : ne rien generer
       ============================================================ */
    if (strcmp(op, "NOP") == 0) {
        EMIT("    NOP");
        return;
    }

    /* Operateur inconnu : commentaire d'avertissement */
    EMIT("    ; *** AVERTISSEMENT: operateur non reconnu '%s' ***", op);
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
        fprintf(stderr, "ERREUR: impossible de creer le fichier '%s'\n", nom_fichier);
        return;
    }

    printf("\n=== Generation du code assembleur 8086 : %s ===\n", nom_fichier);

    /* ----- Collecte de tous les symboles a declarer ----- */
    collecter_symboles();

    /* ----- En-tete du programme (diapo 118) ----- */
    EMIT("TITLE prolang.asm");
    EMIT(";");
    EMIT("; Code assembleur 8086 genere automatiquement par le compilateur Prolang");
    EMIT("; USTHB 2025-2026 - SAYOUD Lynda");
    EMIT(";");
    EMIT("");

    /* ----- Segment de pile (diapo 133-136) ----- */
    EMIT("PILE SEGMENT STACK");
    EMIT("    DW 100 DUP (?)     ; reserve 100 mots = 200 octets pour la pile");
    EMIT("base_pile EQU $        ; etiquette base de la pile");
    EMIT("PILE ENDS");
    EMIT("");

    /* ----- Segment de donnees (diapo 137-141) ----- */
    emettre_segment_donnees(f);

    /* ----- Segment de code (diapo 142) ----- */
    EMIT("LECODE SEGMENT");
    EMIT("");

    /* Procedures utilitaires (print et read) */
    emettre_proc_print(f);
    emettre_proc_input(f);

    /* Point d'entree du programme */
    EMIT("Debut:");
    EMIT("    ASSUME CS:LECODE, DS:DONNEE, SS:PILE");
    EMIT("");
    EMIT("    ; Initialisation des segments (diapo 136)");
    EMIT("    MOV AX, DONNEE");
    EMIT("    MOV DS, AX         ; DS pointe sur le segment de donnees");
    EMIT("    MOV AX, PILE");
    EMIT("    MOV SS, AX         ; SS pointe sur le segment de pile");
    EMIT("    MOV SP, base_pile  ; SP pointe sur la base de la pile (pile vide)");
    EMIT("");

    /* Marquer les cibles de saut */
    int *marque = (int *)calloc(qc + 1, sizeof(int));
    if (!marque) { fclose(f); return; }
    marquer_cibles(marque);

    /* ----- Traduire chaque quadruplet ----- */
    int i;
    for (i = 0; i < qc; i++) {
        traduire_quadruplet(f, i, marque);
        EMIT("");
    }

    /* Etiquette de fin (cible potentielle de BR vers la fin) */
    char lbl_fin[32];
    label_pour(qc, lbl_fin);
    EMIT("%s:", lbl_fin);

    /* Terminaison DOS (diapo 145 : MOV AH, 4Ch / INT 21h) */
    EMIT("    ; Terminaison du programme (DOS)");
    EMIT("    MOV AH, 4Ch");
    EMIT("    INT 21h");
    EMIT("");
    EMIT("LECODE ENDS");
    EMIT("");
    EMIT("; ----- Fin du programme -----");
    EMIT("END Debut");

    free(marque);
    fclose(f);

    printf("[Code assembleur genere avec succes : %s]\n", nom_fichier);
}
