#include "ts.h"

TypeTS TS[MAX_TS];
TypeSM tabM[MAX_SM];
TypeSM tabS[MAX_SM];
int cpt = 0, cptm = 0, cpts = 0;

int semantic_errors = 0;

/* ---- Initialisation ---- */
void ts_initialiser(void) {
    for (int i = 0; i < MAX_TS; i++) TS[i].state = 0;
    for (int i = 0; i < MAX_SM; i++) { tabM[i].state = 0; tabS[i].state = 0; }
    cpt = 0; cptm = 0; cpts = 0;
    semantic_errors = 0;
}

/* ---- Recherche interne ---- */
static int ts_find(const char *nom) {
    for (int i = 0; i < MAX_TS; i++)
        if (TS[i].state == 1 && strcmp(TS[i].name, nom) == 0) return i;
    return -1;
}

static int ts_next_free(void) {
    for (int i = 0; i < MAX_TS; i++)
        if (TS[i].state == 0) return i;
    return -1;
}

/* ---- API ---- */
int ts_double_declaration(const char *nom) { return ts_find(nom) >= 0 ? 1 : 0; }
int ts_est_declare       (const char *nom) { return ts_find(nom) >= 0 ? 1 : 0; }

int ts_est_constante(const char *nom) {
    int i = ts_find(nom);
    return (i >= 0 && strcmp(TS[i].code, "CONST") == 0) ? 1 : 0;
}

void ts_marquer_init(const char *nom) {
    int i = ts_find(nom);
    if (i >= 0 && TS[i].val[0] == '\0') strcpy(TS[i].val, "oui");
}

void ts_set_val(const char *nom, const char *val) {
    int i = ts_find(nom);
    if (i >= 0) { strncpy(TS[i].val, val ? val : "", 19); TS[i].val[19] = '\0'; }
}

const char *ts_get_val(const char *nom) {
    int i = ts_find(nom);
    if (i >= 0) return TS[i].val;
    return NULL;
}

int ts_inserer_variable(const char *nom, TypeVar type) {
    if (ts_double_declaration(nom)) {
        printf("ERREUR semantique: double declaration de '%s'\n", nom);
        semantic_errors++;
        return 0;
    }
    int i = ts_next_free();
    if (i < 0) { fprintf(stderr, "ERREUR: table des symboles pleine\n"); return 0; }
    TS[i].state = 1;
    strncpy(TS[i].name, nom, MAX_IDF); TS[i].name[MAX_IDF] = '\0';
    strcpy(TS[i].code, "IDF");
    strcpy(TS[i].type, type == TYPE_INTEGER ? "integer" : "float");
    TS[i].val[0] = '\0';
    cpt++;
    return 1;
}

int ts_inserer_constante(const char *nom, TypeVar type) {
    if (ts_double_declaration(nom)) {
        printf("ERREUR semantique: double declaration de '%s'\n", nom);
        semantic_errors++;
        return 0;
    }
    int i = ts_next_free();
    if (i < 0) { fprintf(stderr, "ERREUR: table des symboles pleine\n"); return 0; }
    TS[i].state = 1;
    strncpy(TS[i].name, nom, MAX_IDF); TS[i].name[MAX_IDF] = '\0';
    strcpy(TS[i].code, "CONST");
    strcpy(TS[i].type, type == TYPE_INTEGER ? "integer" : "float");
    TS[i].val[0] = '\0';
    cpt++;
    return 1;
}

int ts_inserer_tableau(const char *nom, TypeVar type, int taille) {
    if (ts_double_declaration(nom)) {
        printf("ERREUR semantique: double declaration de '%s'\n", nom);
        semantic_errors++;
        return 0;
    }
    if (taille <= 0) {
        printf("ERREUR semantique: taille invalide pour '%s'\n", nom);
        semantic_errors++;
        return 0;
    }
    int i = ts_next_free();
    if (i < 0) { fprintf(stderr, "ERREUR: table des symboles pleine\n"); return 0; }
    TS[i].state = 1;
    strncpy(TS[i].name, nom, MAX_IDF); TS[i].name[MAX_IDF] = '\0';
    strcpy(TS[i].code, "TABLEAU");
    strcpy(TS[i].type, type == TYPE_INTEGER ? "integer" : "float");
    sprintf(TS[i].val, "%d", taille);
    cpt++;
    return 1;
}

void ts_inserer_mc(const char *nom, const char *token) {
    for (int i = 0; i < cptm; i++)
        if (tabM[i].state == 1 && strcmp(tabM[i].name, nom) == 0) return;
    if (cptm >= MAX_SM) return;
    tabM[cptm].state = 1;
    strncpy(tabM[cptm].name, nom,   19); tabM[cptm].name[19] = '\0';
    strncpy(tabM[cptm].type, token, 19); tabM[cptm].type[19] = '\0';
    cptm++;
}

void ts_inserer_sep(const char *nom, const char *token) {
    for (int i = 0; i < cpts; i++)
        if (tabS[i].state == 1 && strcmp(tabS[i].name, nom) == 0) return;
    if (cpts >= MAX_SM) return;
    tabS[cpts].state = 1;
    strncpy(tabS[cpts].name, nom,   19); tabS[cpts].name[19] = '\0';
    strncpy(tabS[cpts].type, token, 19); tabS[cpts].type[19] = '\0';
    cpts++;
}

/* ---- Affichage 3 tables ---- */
void ts_afficher(void) {
    /* Table IDF / CONST / TABLEAU */
    printf("\n/****** Table des symboles IDF / CONST / TABLEAU ******/\n");
    printf("+----+--------------------+----------+-----------+--------------------+\n");
    printf("| #  | Nom                | Code     | Type      | Val / Taille       |\n");
    printf("+----+--------------------+----------+-----------+--------------------+\n");
    for (int i = 0, n = 0; i < MAX_TS && n < cpt; i++) {
        if (TS[i].state == 1) {
            printf("| %-2d | %-18s | %-8s | %-9s | %-18s |\n",
                   n++, TS[i].name, TS[i].code, TS[i].type,
                   TS[i].val[0] ? TS[i].val : "-");
        }
    }
    printf("+----+--------------------+----------+-----------+--------------------+\n");
    printf("Total: %d entree(s)\n", cpt);

    /* Table des mots-cles */
    printf("\n/****** Table des mots-cles ******/\n");
    printf("+----+--------------------+--------------------+\n");
    printf("| #  | Mot-cle            | Token              |\n");
    printf("+----+--------------------+--------------------+\n");
    for (int i = 0; i < cptm; i++)
        if (tabM[i].state == 1)
            printf("| %-2d | %-18s | %-18s |\n", i, tabM[i].name, tabM[i].type);
    printf("+----+--------------------+--------------------+\n");
    printf("Total: %d mot(s)-cle(s)\n", cptm);

    /* Table des separateurs */
    printf("\n/****** Table des separateurs ******/\n");
    printf("+----+--------------------+--------------------+\n");
    printf("| #  | Separateur         | Token              |\n");
    printf("+----+--------------------+--------------------+\n");
    for (int i = 0; i < cpts; i++)
        if (tabS[i].state == 1)
            printf("| %-2d | %-18s | %-18s |\n", i, tabS[i].name, tabS[i].type);
    printf("+----+--------------------+--------------------+\n");
    printf("Total: %d separateur(s)\n\n", cpts);
}

void ts_liberer(void) {
    ts_initialiser();
}

