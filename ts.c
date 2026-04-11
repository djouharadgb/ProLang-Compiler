#include "ts.h"

static Symbol* table[TABLE_SIZE];

/* ============================================================
   Parametres FNV-1a pour 32 bits :
     - offset_basis = 2166136261 (0x811c9dc5)
     - prime         = 16777619  (0x01000193)
   ============================================================ */
unsigned int fnv1a_hash(const char *cle) {
    unsigned int hash = 2166136261u;  /* FNV offset basis */
    while (*cle) {
        hash ^= (unsigned char)(*cle);
        hash *= 16777619u;            /* FNV prime */
        cle++;
    }
    return hash % TABLE_SIZE;
}

/* Initialiser toute la table a NULL */
void ts_initialiser(void) {
    for (int i = 0; i < TABLE_SIZE; i++)
        table[i] = NULL;
}

/* Rechercher un symbole par nom */
Symbol* ts_rechercher(const char *nom) {
    unsigned int index = fnv1a_hash(nom);
    Symbol *courant = table[index];
    while (courant != NULL) {
        if (strcmp(courant->nom, nom) == 0)
            return courant;
        courant = courant->suivant;
    }
    return NULL;
}

/* Insertion generique (interne) */
static Symbol* ts_creer_symbole(const char *nom, TypeVar type, NatureVar nature, int taille) {
    Symbol *sym = (Symbol*)malloc(sizeof(Symbol));
    if (!sym) {
        fprintf(stderr, "Erreur: allocation memoire echouee\n");
        exit(1);
    }
    strncpy(sym->nom, nom, MAX_IDF);
    sym->nom[MAX_IDF] = '\0';
    sym->type    = type;
    sym->nature  = nature;
    sym->taille  = taille;
    sym->est_init = 0;
    sym->valeur.val_int = 0;
    sym->suivant = NULL;
    return sym;
}

/* Verifier double declaration */
int ts_double_declaration(const char *nom) {
    return (ts_rechercher(nom) != NULL) ? 1 : 0;
}

/* Inserer une variable simple */
int ts_inserer_variable(const char *nom, TypeVar type) {
    if (ts_double_declaration(nom)) {
        printf("ERREUR semantique: double declaration de '%s'\n", nom);
        return 0;
    }
    unsigned int index = fnv1a_hash(nom);
    Symbol *sym = ts_creer_symbole(nom, type, NATURE_VAR, 1);
    sym->suivant = table[index];
    table[index] = sym;
    return 1;
}

/* Inserer une constante */
int ts_inserer_constante(const char *nom, TypeVar type) {
    if (ts_double_declaration(nom)) {
        printf("ERREUR semantique: double declaration de '%s'\n", nom);
        return 0;
    }
    unsigned int index = fnv1a_hash(nom);
    Symbol *sym = ts_creer_symbole(nom, type, NATURE_CONST, 1);
    sym->est_init = 1;  /* une constante est toujours initialisee */
    sym->suivant = table[index];
    table[index] = sym;
    return 1;
}

/* Inserer un tableau */
int ts_inserer_tableau(const char *nom, TypeVar type, int taille) {
    if (ts_double_declaration(nom)) {
        printf("ERREUR semantique: double declaration de '%s'\n", nom);
        return 0;
    }
    if (taille <= 0) {
        printf("ERREUR semantique: taille de tableau invalide pour '%s'\n", nom);
        return 0;
    }
    unsigned int index = fnv1a_hash(nom);
    Symbol *sym = ts_creer_symbole(nom, type, NATURE_TABLEAU, taille);
    sym->suivant = table[index];
    table[index] = sym;
    return 1;
}

/* Verifier si declare */
int ts_est_declare(const char *nom) {
    return (ts_rechercher(nom) != NULL) ? 1 : 0;
}

/* Marquer comme initialise */
void ts_marquer_init(const char *nom) {
    Symbol *sym = ts_rechercher(nom);
    if (sym) sym->est_init = 1;
}

/* Verifier si c'est une constante */
int ts_est_constante(const char *nom) {
    Symbol *sym = ts_rechercher(nom);
    return (sym && sym->nature == NATURE_CONST) ? 1 : 0;
}

/* Afficher la table des symboles */
void ts_afficher(void) {
    printf("\n╔══════════════════════════════════════════════════════════════════╗\n");
    printf("║                    TABLE DES SYMBOLES                           ║\n");
    printf("╠════╦════════════════╦═════════╦══════════╦════════╦══════════════╣\n");
    printf("║ #  ║ Nom            ║ Type    ║ Nature   ║ Taille ║ Initialisee  ║\n");
    printf("╠════╬════════════════╬═════════╬══════════╬════════╬══════════════╣\n");

    int count = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        Symbol *sym = table[i];
        while (sym != NULL) {
            const char *type_str   = (sym->type == TYPE_INTEGER) ? "integer" : "float";
            const char *nature_str;
            switch(sym->nature) {
                case NATURE_VAR:     nature_str = "variable"; break;
                case NATURE_CONST:   nature_str = "const";    break;
                case NATURE_TABLEAU: nature_str = "tableau";  break;
                default:             nature_str = "???";      break;
            }
            printf("║ %-2d ║ %-14s ║ %-7s ║ %-8s ║ %-6d ║ %-12s ║\n",
                   count++, sym->nom, type_str, nature_str,
                   sym->taille, sym->est_init ? "oui" : "non");
            sym = sym->suivant;
        }
    }
    printf("╚════╩════════════════╩═════════╩══════════╩════════╩══════════════╝\n");
    printf("Total: %d symboles\n\n", count);
}

/* Liberer la memoire */
void ts_liberer(void) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Symbol *sym = table[i];
        while (sym != NULL) {
            Symbol *temp = sym;
            sym = sym->suivant;
            free(temp);
        }
        table[i] = NULL;
    }
}
