#include "asm8086.h"
#include "quad.h"
#include "ts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* Macro : ecrire une ligne dans le fichier asm */
#define EMIT(fmt, ...)  fprintf(f, fmt "\n", ##__VA_ARGS__)

/* =========================================================
   VIRGULE FIXE (FIXED-POINT) — FACTEUR D'ECHELLE = 100
   =========================================================
   Toutes les valeurs (entiers ET flottants) sont stockees
   multipliees par SCALE = 100 dans les registres et en memoire.

   Exemples de representation interne :
     2.5      =>  250   (2.5   * 100)
     3.141590 =>  314   (arrondi de 3.14159 * 100)
     10       => 1000   (10    * 100)
     0        =>    0   (0     * 100)
     5        =>  500   (5     * 100)

   Regles de calcul en virgule fixe :
     ADD / SUB  => normal, les deux operandes sont deja * 100
                   (a*100) +/- (b*100) = (a+/-b)*100  => correct
     MUL        => IMUL puis IDIV 100  (pour redonner l'echelle)
                   (a*100) * (b*100) = (a*b)*10000  => diviser par 100 => (a*b)*100
     DIV        => MUL numerateur par 100 puis IDIV
                   (a*100)*100 / (b*100) = (a/b)*100  => correct
     CMP        => direct, les deux cotes sont a la meme echelle

   Affichage (_PRINT_FIXED) :
     AX = valeur * 100  (ex: 314 pour 3.14,  1000 pour 10,  250 pour 2.5)
     => affiche  AX / 100  (partie entiere)
     => affiche  '.'
     => affiche  AX % 100  (deux chiffres decimaux, avec zero de tete si besoin)

   Saisie (_READ_FIXED) :
     Lit les chiffres entiers, les multiplie par 100 (partie entiere),
     puis lit optionnellement '.' et jusqu'a 2 chiffres decimaux.
     Retourne AX = valeur * 100 (virgule fixe).
   ========================================================= */
#define SCALE 100


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

/*
   val_en_fixe : convertit une chaine representant un nombre
   (entier ou flottant) en sa valeur virgule fixe (val * SCALE).
   L'arrondi est applique pour eviter les erreurs de troncature.
   Ex: "3.141590" => 314,  "10" => 1000,  "2.500000" => 250,  "1.5" => 150.
*/
static long val_en_fixe(const char *s)
{
    double v = atof(s);
    /* round() pour eviter 3.14159*100 = 314.159 => troncature a 314 (correct ici) */
    return (long)round(v * SCALE);
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
  - Nombre litteral   => MOV reg, val_en_fixe(src)   (mis a l'echelle *100)
  - Acces tableau     => calcul index*2, puis MOV reg, tab[SI]
  - Variable/temp     => MOV reg, nom
    Note : les variables en memoire sont TOUJOURS stockees en virgule fixe (*100).
    Les indices de tableau sont en virgule fixe => on divise par SCALE avant l'acces.
 */
static void charger_dans_reg(FILE *f, const char *src, const char *reg)
{
    char ntab[64], idx[64];

    if (est_nombre(src)) {
        /* Constante litterale : convertir en virgule fixe une fois pour toutes.
           Ex: "3.141590" => MOV reg, 314 ;  "10" => MOV reg, 1000 */
        EMIT("    MOV %s, %ld", reg, val_en_fixe(src)); // mov reg, val*SCALE

    } else if (est_tableau_access(src, ntab, idx)) {
        /* Calcul de l'offset : chaque element fait 2 octets (DW) */
        if (est_nombre(idx))
            /* Indice entier litteral (ex: Tabint[0]) : offset = idx * 2 directement */
            EMIT("    MOV SI, %ld", atol(idx) * 2); // mov SI, idx*2
        else {
            /* Indice variable ou temporaire : valeur en virgule fixe (*100).
               Il faut la ramener a sa valeur entiere reelle avant de calculer l'offset.
               Sequence : SI = (idx_fixe / SCALE) * 2
               On utilise AX comme registre de travail (sera ecrase par la lecture). */
            EMIT("    MOV AX, %s", nom_asm(idx));      // AX = idx * 100 (valeur fixe)
            EMIT("    CWD");                             // etendre signe DX:AX
            EMIT("    MOV BX, %d", SCALE);               // BX = 100
            EMIT("    IDIV BX");                         // AX = idx entier reel
            EMIT("    MOV SI, AX");                      // SI = idx entier
            EMIT("    ADD SI, SI   ; SI = indice * 2"); // SI = offset octet
        }
        EMIT("    MOV %s, %s[SI]", reg, nom_asm(ntab)); // mov reg, tab[SI]

    } else {
        EMIT("    MOV %s, %s", reg, nom_asm(src)); // mov reg, src
    }
}

/*
 stocker_ax_dans : stocke AX dans la destination 'dst'.
   - Acces tableau => T[SI] = AX  (avec decodage virgule fixe pour l'indice)
   - Variable/temp => MOV nom, AX
   Note : AX contient une valeur virgule fixe (*100), coherent avec la memoire.
 */
static void stocker_ax_dans(FILE *f, const char *dst)
{
    char ntab[64], idx[64];

    if (est_tableau_access(dst, ntab, idx)) {
        if (est_nombre(idx))
            EMIT("    MOV SI, %ld", atol(idx) * 2); // mov SI, idx*2 car idx cste we know value cste
        else {
            /* Meme logique que charger_dans_reg : l'indice est en virgule fixe,
               on divise par SCALE pour retrouver l'entier reel, puis *2 pour l'offset.
               AX contient la valeur a stocker => on le sauvegarde sur la pile. */
            EMIT("    PUSH AX");                        // sauvegarder la valeur a ecrire
            EMIT("    MOV AX, %s", nom_asm(idx));       // AX = idx * 100
            EMIT("    CWD");                             // etendre signe
            EMIT("    MOV BX, %d", SCALE);               // BX = 100
            EMIT("    IDIV BX");                         // AX = idx entier reel
            EMIT("    MOV SI, AX");                      // SI = idx entier
            EMIT("    ADD SI, SI");                      // SI = idx * 2 (offset octet)
            EMIT("    POP AX");                          // restaurer la valeur a ecrire
        }
        EMIT("    MOV %s[SI], AX", nom_asm(ntab)); // mov tab[SI], AX
    } else {
        EMIT("    MOV %s, AX", nom_asm(dst)); // mov dst, AX
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
            if (s[0]=='T' && isdigit((unsigned char)s[1]) && !strchr(s,'['))
                sym_ajouter(s, "integer", "", 0, 1);

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

        if (s->taille > 0) {  // taille sup 0, tableau
            EMIT("    %-20s DW %d DUP(?)  ; tableau", anom, s->taille); // ex: T0[10] => _T0 DW 10 DUP(?)
        } else if (s->est_temp) { 
            EMIT("    %-20s DW ?          ; temporaire", anom); //temporaire simple ex: T0 => _T0 DW ?
        } else if (strlen(s->val) > 0 && strcmp(s->val,"oui") != 0) {
            /* Variable/constante initialisee.
               VIRGULE FIXE : on stocke val * SCALE en memoire.
               Ex: 2.500000 => DW 250,  10 => DW 1000,  3.141590 => DW 314
               Ainsi toutes les valeurs sont coherentes en memoire et dans les registres. */
            long vf = val_en_fixe(s->val);
            if (est_flottant(s->val))
                EMIT("    %-20s DW %ld   ; fixe: %s * %d = %ld", anom, vf, s->val, SCALE, vf);
            else
                EMIT("    %-20s DW %ld   ; fixe: %s * %d = %ld", anom, vf, s->val, SCALE, vf);
        } else {
            EMIT("    %-20s DW ?", anom);
        }
    }

    /* Tampon pour l'affichage (INT 21h attend '$' comme terminateur).
       Taille 16 : signe(1) + 5 chiffres entiers + '.'(1) + 2 decimales + '$'(1) + marge. */
    EMIT("    _OUT_BUF             DB 16 DUP(?), '$'");
    EMIT("");
    EMIT("DONNEE ENDS");
    EMIT("");
}

/* =========================================================
   PROCEDURES UTILITAIRES (print / read)
   ========================================================= */

/*
   _PRINT_INT_RAW : sous-procedure interne.
   Affiche BX en decimal positif sans signe (usage interne par _PRINT_FIXED).
   Preserve tous les registres sauf AX (utilisé comme registre de travail).
*/
static void emettre_proc_print(FILE *f)
{
    EMIT(";---- _PRINT_INT_RAW : affiche BX en decimal positif (usage interne) ----");
    EMIT("_PRINT_INT_RAW PROC NEAR");
    EMIT("    PUSH AX"); EMIT("    PUSH CX"); EMIT("    PUSH DX"); EMIT("    PUSH SI");
    EMIT("    MOV AX, BX          ; AX = valeur a afficher");
    EMIT("    MOV SI, OFFSET _OUT_BUF");
    EMIT("    MOV BX, 10          ; diviseur decimal");
    EMIT("    MOV CX, 0           ; compteur de chiffres");
    EMIT("_RAW_DIV:");
    EMIT("    MOV DX, 0");
    EMIT("    DIV BX              ; AX = AX/10, DX = chiffre (reste)");
    EMIT("    ADD DL, '0'         ; convertir en caractere ASCII");
    EMIT("    PUSH DX             ; empiler les chiffres (ordre inverse pour depiler)");
    EMIT("    INC CX");
    EMIT("    CMP AX, 0");
    EMIT("    JNE _RAW_DIV");
    EMIT("_RAW_POP:");
    EMIT("    POP DX");
    EMIT("    MOV [SI], DL        ; ecrire chiffre dans le tampon");
    EMIT("    INC SI");
    EMIT("    LOOP _RAW_POP");
    EMIT("    MOV BYTE PTR [SI], '$' ; terminateur INT 21h");
    EMIT("    MOV AH, 09h");
    EMIT("    MOV DX, OFFSET _OUT_BUF");
    EMIT("    INT 21h             ; afficher la chaine");
    EMIT("    POP SI"); EMIT("    POP DX"); EMIT("    POP CX"); EMIT("    POP AX");
    EMIT("    RET");
    EMIT("_PRINT_INT_RAW ENDP");
    EMIT("");

    /*
       _PRINT_FIXED : affiche la valeur virgule fixe contenue dans AX.
       AX = valeur * SCALE  (ex: 314 pour 3.14, 1000 pour 10.00, -250 pour -2.50)

       Algorithme :
         1. Si AX < 0 : afficher '-', NEG AX  (travailler en positif)
         2. DX = AX % SCALE  (partie decimale, 0..99)
            AX = AX / SCALE  (partie entiere)
         3. Afficher AX via _PRINT_INT_RAW
         4. Afficher '.'
         5. Si DX < 10 : afficher zero de tete ('0')
            Afficher DX via _PRINT_INT_RAW
    */
    EMIT(";---- _PRINT_FIXED : affiche AX en virgule fixe (AX = valeur * %d) ----", SCALE);
    EMIT(";     ex: AX=314  => '3.14'   AX=1000 => '10.00'   AX=-250 => '-2.50'");
    EMIT("_PRINT_FIXED PROC NEAR");
    EMIT("    PUSH AX"); EMIT("    PUSH BX"); EMIT("    PUSH CX"); EMIT("    PUSH DX");

    /* Gestion du signe */
    EMIT("    CMP AX, 0");
    EMIT("    JGE _FIX_POS");
    EMIT("    MOV AH, 02h");
    EMIT("    MOV DL, '-'         ; afficher le signe moins");
    EMIT("    INT 21h");
    EMIT("    NEG AX              ; travailler avec la valeur absolue");
    EMIT("_FIX_POS:");

    /* Diviser par SCALE pour separer partie entiere et decimale */
    EMIT("    MOV BX, %d          ; diviseur = SCALE = 100", SCALE);
    EMIT("    MOV DX, 0");
    EMIT("    DIV BX              ; AX = partie entiere, DX = partie decimale (0..99)");
    EMIT("    PUSH DX             ; sauvegarder la partie decimale");

    /* Afficher la partie entiere */
    EMIT("    MOV BX, AX          ; BX = partie entiere pour _PRINT_INT_RAW");
    EMIT("    CALL _PRINT_INT_RAW");

    /* Afficher le point decimal */
    EMIT("    MOV AH, 02h");
    EMIT("    MOV DL, '.'         ; point decimal");
    EMIT("    INT 21h");

    /* Afficher la partie decimale (avec zero de tete si < 10) */
    EMIT("    POP AX              ; AX = partie decimale (0..99)");
    EMIT("    CMP AX, 10          ; ex: 5 => afficher '05' et non '5'");
    EMIT("    JGE _FIX_NO_ZERO");
    EMIT("    MOV AH, 02h");
    EMIT("    MOV DL, '0'         ; zero de tete");
    EMIT("    INT 21h");
    EMIT("_FIX_NO_ZERO:");
    EMIT("    MOV BX, AX          ; BX = partie decimale pour _PRINT_INT_RAW");
    EMIT("    CALL _PRINT_INT_RAW");

    EMIT("    POP DX"); EMIT("    POP CX"); EMIT("    POP BX"); EMIT("    POP AX");
    EMIT("    RET");
    EMIT("_PRINT_FIXED ENDP");
    EMIT("");
}

/*
   _READ_FIXED : lit un nombre (entier ou decimal) depuis le clavier.
   Retourne AX = valeur * SCALE (virgule fixe).

   Algorithme :
     - Lire chiffres entiers  => accumuler dans BX (valeur brute)
     - Si on rencontre '.'   => passer en mode decimal
       Lire jusqu'a 2 chiffres decimaux => accumuler dans CX (0..99)
       Un seul chiffre decimal => multiplier par 10 (ex: '.5' => CX=50)
     - Resultat = BX * SCALE + CX
     - Appliquer le signe si '-' en tete
*/
static void emettre_proc_input(FILE *f)
{
    EMIT(";---- _READ_FIXED : lit un entier ou decimal depuis le clavier ----");
    EMIT(";     retourne AX = valeur * %d (virgule fixe)", SCALE);
    EMIT(";     ex: '3.14' => AX=314   '10' => AX=1000   '2.5' => AX=250   '-1.5' => AX=-150");
    EMIT("_READ_FIXED PROC NEAR");
    EMIT("    PUSH BX"); EMIT("    PUSH CX"); EMIT("    PUSH DX"); EMIT("    PUSH SI");
    EMIT("    PUSH BP");
    EMIT("    MOV BX, 0   ; partie entiere (valeur brute)");
    EMIT("    MOV CX, 0   ; partie decimale (0..99)");
    EMIT("    MOV SI, 0   ; SI=0 => mode entier, SI=1 => mode decimal");
    EMIT("    MOV DX, 0   ; nombre de chiffres decimaux lus");
    EMIT("    MOV BP, 0   ; BP=0 positif, BP=1 negatif");

    EMIT("_RFIXED_CHAR:");
    EMIT("    MOV AH, 01h");
    EMIT("    INT 21h     ; AL = caractere saisi");

    /* Fin de saisie */
    EMIT("    CMP AL, 0Dh ; Entree (CR) ?");
    EMIT("    JE  _RFIXED_DONE");

    /* Signe moins */
    EMIT("    CMP AL, '-'");
    EMIT("    JNE _RFIXED_NOT_MINUS");
    EMIT("    MOV BP, 1   ; marquer negatif");
    EMIT("    JMP _RFIXED_CHAR");
    EMIT("_RFIXED_NOT_MINUS:");

    /* Point decimal : passer en mode decimal */
    EMIT("    CMP AL, '.'");
    EMIT("    JNE _RFIXED_NOT_DOT");
    EMIT("    MOV SI, 1   ; activer le mode decimal");
    EMIT("    JMP _RFIXED_CHAR");
    EMIT("_RFIXED_NOT_DOT:");

    /* Filtrer les non-chiffres */
    EMIT("    CMP AL, '0'");
    EMIT("    JL  _RFIXED_CHAR");
    EMIT("    CMP AL, '9'");
    EMIT("    JG  _RFIXED_CHAR");
    EMIT("    SUB AL, '0' ; convertir ASCII => valeur 0..9");
    EMIT("    CBW         ; AL => AX");

    EMIT("    CMP SI, 1   ; mode decimal ?");
    EMIT("    JE  _RFIXED_DEC_DIGIT");

    /* Mode entier : BX = BX * 10 + chiffre */
    EMIT("    PUSH AX");
    EMIT("    MOV AX, BX");
    EMIT("    MOV BX, 10");
    EMIT("    MUL BX");
    EMIT("    MOV BX, AX");
    EMIT("    POP AX");
    EMIT("    ADD BX, AX  ; BX = partie entiere accumulee");
    EMIT("    JMP _RFIXED_CHAR");

    /* Mode decimal : accepter jusqu'a 2 chiffres */
    EMIT("_RFIXED_DEC_DIGIT:");
    EMIT("    CMP DX, 2   ; deja 2 chiffres decimaux lus ?");
    EMIT("    JGE _RFIXED_CHAR ; ignorer les chiffres en trop");
    EMIT("    INC DX");
    EMIT("    CMP DX, 1   ; premier chiffre decimal ?");
    EMIT("    JNE _RFIXED_DEC2");
    /* Premier chiffre decimal : il represente les dixiemes => *10 pour avoir 0..90 */
    EMIT("    MOV CX, AX");
    EMIT("    PUSH BX");
    EMIT("    MOV BX, 10");
    EMIT("    MUL BX      ; AX = chiffre * 10 (ex: 5 => 50 = 0.50)");
    EMIT("    MOV CX, AX  ; CX = dizaine");
    EMIT("    POP BX");
    EMIT("    JMP _RFIXED_CHAR");
    /* Deuxieme chiffre decimal : il represente les centiemes => ajouter directement */
    EMIT("_RFIXED_DEC2:");
    EMIT("    ADD CX, AX  ; CX = dizaine + unite => 0..99");
    EMIT("    JMP _RFIXED_CHAR");

    EMIT("_RFIXED_DONE:");
    /* Combiner : resultat = partie_entiere * SCALE + partie_decimale */
    EMIT("    MOV AX, BX       ; AX = partie entiere brute");
    EMIT("    MOV BX, %d", SCALE);
    EMIT("    MUL BX           ; AX = partie_entiere * SCALE");
    EMIT("    ADD AX, CX       ; AX += partie decimale (0..99)");
    /* Appliquer le signe */
    EMIT("    CMP BP, 1");
    EMIT("    JNE _RFIXED_POS");
    EMIT("    NEG AX");
    EMIT("_RFIXED_POS:");
    EMIT("    POP BP");
    EMIT("    POP SI"); EMIT("    POP DX"); EMIT("    POP CX"); EMIT("    POP BX");
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
        charger_dans_reg(f, a, "AX");
        stocker_ax_dans(f, res);
        return;
    }

    /* Addition et Soustraction  res = a +/- b
       On factorise les deux car le schema est identique.
       VIRGULE FIXE : (a*100) +/- (b*100) = (a+/-b)*100 => correct sans ajustement. */
    if (strcmp(op,"+")==0 || strcmp(op,"-")==0) {
        const char *mnem = (strcmp(op,"+")==0) ? "ADD" : "SUB";
        charger_dans_reg(f, a, "AX");
        if (est_nombre(b))
            /* constante : mettre a l'echelle avant l'operation */
            EMIT("    %s AX, %ld", mnem, val_en_fixe(b)); // optimisations : x+0 => x, x-0 => x
        else {
            charger_dans_reg(f, b, "BX");
            EMIT("    %s AX, BX", mnem);
        }
        stocker_ax_dans(f, res);
        return;
    }

    /* Multiplication et Division  res = a * / b
       VIRGULE FIXE :
         MUL : AX=(a*100), BX=(b*100)
               IMUL BX  =>  DX:AX = (a*100)*(b*100) = (a*b)*10000
               IDIV 100 =>  AX    = (a*b)*100  (echelle correcte)
         DIV : AX=(a*100), BX=(b*100)
               On veut (a/b)*100 comme resultat.
               (a*100)*100 / (b*100) = (a/b)*100
               => IMUL 100 sur le numerateur, puis IDIV (b*100) */
    if (strcmp(op,"*")==0 || strcmp(op,"/")==0) {
        int est_div = (strcmp(op,"/") == 0);
        charger_dans_reg(f, a, "AX");

        if (est_nombre(b))
            EMIT("    MOV BX, %ld", val_en_fixe(b)); // optimisation : x*1 => x, x/1 => x
        else
            charger_dans_reg(f, b, "BX");

        if (!est_div) {
            /* MUL virgule fixe :
               (a*100) * (b*100) = (a*b)*10000  => diviser par 100 => (a*b)*100 */
            EMIT("    CWD            ; etendre AX -> DX:AX pour IMUL signe");
            EMIT("    IMUL BX        ; DX:AX = (a*SCALE)*(b*SCALE) = (a*b)*SCALE^2");
            EMIT("    MOV BX, %d", SCALE);
            EMIT("    IDIV BX        ; AX = (a*b)*SCALE  (virgule fixe correct)");
        } else {
            /* DIV virgule fixe :
               numerateur = (a*100)*100 = a*10000
               denominateur = b*100
               resultat = a*10000 / (b*100) = (a/b)*100  (echelle correcte) */
            EMIT("    MOV CX, BX     ; sauvegarder BX = (b*SCALE)");
            EMIT("    MOV BX, %d     ; BX = SCALE", SCALE);
            EMIT("    IMUL BX        ; DX:AX = (a*SCALE)*SCALE = a*SCALE^2");
            EMIT("    MOV BX, CX     ; restaurer BX = (b*SCALE)");
            EMIT("    IDIV BX        ; AX = a*SCALE^2 / (b*SCALE) = (a/b)*SCALE");
        }
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
       Table de correspondance operateur -> saut conditionnel.
       VIRGULE FIXE : les deux operandes sont a la meme echelle (*100),
       la comparaison directe est donc correcte (a*100 > b*100 <=> a > b). */
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
            EMIT("    CMP AX, %ld", val_en_fixe(b)); // comparer les deux valeurs a l'echelle
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

    /* AND et OR logiques  (meme schema, mnemonique different)
       Les valeurs 0 et 1 ne sont pas en virgule fixe (resultats de comparaison).
       Le AND/OR bitwise sur 0 et 1 est correct sans ajustement d'echelle. */
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

    /* Saut conditionnel  BZ : si a == 0, aller a res
       Les valeurs de condition (T6, T8...) sont 0 ou 1 (non scalees). */
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

    /* Lecture tableau  res = TAB[b]
       Si b est un entier litteral (indice statique) : offset = b * 2 directement.
       Si b est une variable (en virgule fixe) : diviser par SCALE, puis *2. */
    if (strcmp(op, "TAB") == 0) {
        if (est_nombre(b))
            EMIT("    MOV SI, %ld", atol(b) * 2); // indice entier litteral => *2 direct
        else {
            /* Indice variable : valeur en virgule fixe => diviser par SCALE puis *2 */
            EMIT("    MOV AX, %s", nom_asm(b));    // AX = b * SCALE
            EMIT("    CWD");                         // etendre signe
            EMIT("    MOV BX, %d", SCALE);           // BX = SCALE
            EMIT("    IDIV BX");                     // AX = b entier reel
            EMIT("    MOV SI, AX");
            EMIT("    ADD SI, SI");                  // SI = b * 2 (offset octet)
        }
        EMIT("    MOV AX, %s[SI]", nom_asm(a));
        stocker_ax_dans(f, res);
        return;
    }

    /* Affichage  out variable ou chaine */
    if (strcmp(op, "out") == 0) {
        if (a && a[0] == '"') {
            /* Chaine litterale : afficher caractere par caractere */
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
            /* Variable ou nombre : appel _PRINT_FIXED
               La valeur est en virgule fixe (*100), _PRINT_FIXED gere l'affichage decimal. */
            charger_dans_reg(f, a, "AX");
            EMIT("    CALL _PRINT_FIXED");
        }
        if (!(a && a[0] == '"'))
            emit_newline(f);
        return;
    }

    /* Lecture clavier  input -> res
       _READ_FIXED retourne AX = valeur * SCALE (virgule fixe). */
    if (strcmp(op, "input") == 0) {
        EMIT("    CALL _READ_FIXED   ; resultat dans AX (virgule fixe, AX = valeur * %d)", SCALE);
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
    EMIT("; Code 8086 genere automatiquement :))");
    EMIT("; Arithmetique en virgule fixe : toutes les valeurs sont stockees * %d", SCALE);
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
