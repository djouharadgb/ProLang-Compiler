/* ================================================================
Fonction de hachage : FNV-1  (Fowler–Noll–Vo, variante 1)
     hash = FNV_OFFSET_BASIS
     pour chaque octet b du nom :
         hash = hash * FNV_PRIME
         hash = hash XOR b
   ================================================================ */
#include "ts.h"
#define FNV_OFFSET_BASIS 2166136261u  /* valeur initiale du hash FNV-1  */
#define FNV_PRIME        16777619u    /* nombre premier FNV pour 32 bits */

NoeudTS *hashTable[HASH_SIZE]; /* tableau de têtes de listes chaînées     */
int      cpt  = 0;             /* nb total d'entrées IDF/CONST/TABLEAU    */
int      cptm = 0;             /* nb de mots-clés enregistrés             */
int      cpts = 0;             /* nb de séparateurs enregistrés           */

TypeSM tabM[MAX_SM];           /* table linéaire des mots-clés            */
TypeSM tabS[MAX_SM];           /* table linéaire des séparateurs          */

int semantic_errors = 0;       /* compteur d'erreurs sémantiques          */

static unsigned int fnv1(const char *nom) {
    unsigned int hash = FNV_OFFSET_BASIS; 

    /* On parcourt chaque octet de la chaîne */
    while (*nom) {
        hash = hash * FNV_PRIME;  /* étape 1 : multiplier par le premier FNV   */
        hash = hash ^ (unsigned char)(*nom); /* étape 2 : XOR avec l'octet courant */
        nom++;                    /* passer à l'octet suivant                  */
    }

    return hash % HASH_SIZE; 
}

/*RECHERCHE INTERNE */

static NoeudTS *ts_find(const char *nom) {
    unsigned int h = fnv1(nom);     /* calculer l'indice de hachage           */
    NoeudTS *courant = hashTable[h]; /* partir de la tête de la liste du seau */

    while (courant != NULL) {
        /* Vérifier que le noeud est actif et que le nom correspond */
        if (courant->state == 1 && strcmp(courant->name, nom) == 0)
            return courant; /* trouvé : retourner le pointeur */
        courant = courant->suivant; /* sinon, passer au suivant */
    }

    return NULL; /* non trouvé */
}

/*CRÉATION D'UN NOUVEAU NOEUD*/

static NoeudTS *creer_noeud(const char *nom, const char *code,
                             const char *type_str, const char *val_str) {
    NoeudTS *n = (NoeudTS *)malloc(sizeof(NoeudTS));
    if (!n) {
        fprintf(stderr, "ERREUR: malloc échoué pour NoeudTS\n");
        exit(1);
    }
    /* Remplir les champs */
    n->state = 1;                                          /* noeud actif          */
    strncpy(n->name, nom,      MAX_IDF); n->name[MAX_IDF] = '\0'; /* copier le nom */
    strncpy(n->code, code,     9);       n->code[9]       = '\0'; /* catégorie     */
    strncpy(n->type, type_str, 9);       n->type[9]       = '\0'; /* type          */

    /* Valeur initiale : vide ou la valeur fournie */
    if (val_str && val_str[0] != '\0')
        strncpy(n->val, val_str, 19);
    else
        n->val[0] = '\0';
    n->val[19] = '\0';

    n->suivant = NULL; /* pas encore de suivant dans la liste */

    return n;
}

/*INSERTION INTERNE dans la table de hachage*/

static void ts_inserer_noeud(NoeudTS *n) {
    unsigned int h = fnv1(n->name); /* calculer le seau destination */

    /* Insérer en tête de liste : le nouveau noeud pointe vers l'ancienne tête */
    n->suivant = hashTable[h];
    hashTable[h] = n;             

    cpt++; 
}

void ts_initialiser(void) {
    /* Mettre toutes les têtes de listes à NULL */
    for (int i = 0; i < HASH_SIZE; i++)
        hashTable[i] = NULL;

    /* Réinitialiser les tables auxiliaires */
    for (int i = 0; i < MAX_SM; i++) {
        tabM[i].state = 0; /* case libre */
        tabS[i].state = 0;
    }

    /* Remettre les compteurs à zéro */
    cpt  = 0;
    cptm = 0;
    cpts = 0;
    semantic_errors = 0;
}

/* LIBÉRATION DE LA MÉMOIRE*/
void ts_liberer(void) {
    /* Parcourir chaque seau de la table de hachage */
    for (int i = 0; i < HASH_SIZE; i++) {
        NoeudTS *courant = hashTable[i]; /* tête de la liste du seau i */

        /* Libérer tous les noeuds de la liste chaînée */
        while (courant != NULL) {
            NoeudTS *suivant = courant->suivant; /* sauvegarder le suivant avant free */
            free(courant);                       /* libérer le noeud courant          */
            courant = suivant;                   /* passer au suivant                 */
        }

        hashTable[i] = NULL; /* remettre la tête à NULL */
    }

    /* Remettre les compteurs à zéro */
    cpt  = 0;
    cptm = 0;
    cpts = 0;
    semantic_errors = 0;
}

/* VÉRIFICATIONS */

/* Retourne 1 si le nom est déjà déclaré dans la TS, 0 sinon */
int ts_double_declaration(const char *nom) {
    return ts_find(nom) != NULL ? 1 : 0;
}

/* Retourne 1 si le nom est déclaré (identique à double_declaration ici) */
int ts_est_declare(const char *nom) {
    return ts_find(nom) != NULL ? 1 : 0;
}

/* Retourne 1 si le nom correspond à une constante  */
int ts_est_constante(const char *nom) {
    NoeudTS *n = ts_find(nom);
    /* Vérifier que le noeud existe ET que son code est "CONST" */
    return (n != NULL && strcmp(n->code, "CONST") == 0) ? 1 : 0;
}


/* Marquer qu'une variable a été initialisée */
void ts_marquer_init(const char *nom) {
    NoeudTS *n = ts_find(nom);
    if (n != NULL && n->val[0] == '\0')  /* val vide ? */
        strcpy(n->val, "oui");           /* marquer comme initialisée */
}

/* Affecter une valeur concrète à un symbole (ex: valeur d'initialisation) */
void ts_set_val(const char *nom, const char *val) {
    NoeudTS *n = ts_find(nom);
    if (n != NULL) {
        strncpy(n->val, val ? val : "", 19); /* copier la valeur, max 19 chars */
        n->val[19] = '\0';
    }
}

/* Retourner la valeur stockée pour un symbole  */
const char *ts_get_val(const char *nom) {
    NoeudTS *n = ts_find(nom);
    if (n != NULL) return n->val; /* retourner le champ val du noeud */
    return NULL;                  /* symbole introuvable              */
}

/*INSERTIONS DANS LA TABLE PRINCIPALE*/

/* Insérer une VARIABLE simple (IDF) */
int ts_inserer_variable(const char *nom, TypeVar type) {
    /* Vérifier la double déclaration */
    if (ts_double_declaration(nom)) {
        printf("ERREUR semantique: double declaration de '%s'\n", nom);
        semantic_errors++;
        return 0; /* échec */
    }

    /* Déterminer la chaîne de type */
    const char *type_str = (type == TYPE_INTEGER) ? "integer" : "float";

    /* Créer le noeud et l'insérer dans la table de hachage */
    NoeudTS *n = creer_noeud(nom, "IDF", type_str, "");
    ts_inserer_noeud(n); /* insertion en tête du seau correspondant */

    return 1; /* succès */
}

/* Insérer une CONSTANTE (CONST) */
int ts_inserer_constante(const char *nom, TypeVar type) {
    /* Vérifier la double déclaration */
    if (ts_double_declaration(nom)) {
        printf("ERREUR semantique: double declaration de '%s'\n", nom);
        semantic_errors++;
        return 0;
    }

    const char *type_str = (type == TYPE_INTEGER) ? "integer" : "float";

    /* Créer le noeud avec le code "CONST" */
    NoeudTS *n = creer_noeud(nom, "CONST", type_str, "");
    ts_inserer_noeud(n);

    return 1;
}

/* Insérer un TABLEAU */
int ts_inserer_tableau(const char *nom, TypeVar type, int taille) {
    /* Vérifier la double déclaration */
    if (ts_double_declaration(nom)) {
        printf("ERREUR semantique: double declaration de '%s'\n", nom);
        semantic_errors++;
        return 0;
    }

    /* La taille doit être un entier strictement positif */
    if (taille <= 0) {
        printf("ERREUR semantique: taille invalide pour '%s'\n", nom);
        semantic_errors++;
        return 0;
    }

    const char *type_str = (type == TYPE_INTEGER) ? "integer" : "float";

    /* Stocker la taille sous forme de chaîne dans le champ val */
    char val_str[20];
    sprintf(val_str, "%d", taille);

    /* Créer le noeud avec le code "TABLEAU" et la taille comme valeur */
    NoeudTS *n = creer_noeud(nom, "TABLEAU", type_str, val_str);
    ts_inserer_noeud(n);

    return 1;
}

/*INSERTIONS DANS LES TABLES AUXILIAIRES  */

/* Insérer un mot-clé dans tabM (si pas déjà présent) */
void ts_inserer_mc(const char *nom, const char *token) {
    /* Vérifier qu'il n'est pas déjà enregistré */
    for (int i = 0; i < cptm; i++)
        if (tabM[i].state == 1 && strcmp(tabM[i].name, nom) == 0)
            return; /* déjà présent : ne rien faire */

    /* Vérifier qu'il reste de la place */
    if (cptm >= MAX_SM) return;

    /* Insérer à la prochaine case libre */
    tabM[cptm].state = 1;
    strncpy(tabM[cptm].name, nom,   19); tabM[cptm].name[19] = '\0';
    strncpy(tabM[cptm].type, token, 19); tabM[cptm].type[19] = '\0';
    cptm++;
}

/* Insérer un séparateur dans tabS (si pas déjà présent) */
void ts_inserer_sep(const char *nom, const char *token) {
    /* Vérifier qu'il n'est pas déjà enregistré */
    for (int i = 0; i < cpts; i++)
        if (tabS[i].state == 1 && strcmp(tabS[i].name, nom) == 0)
            return; /* déjà présent */

    if (cpts >= MAX_SM) return;

    tabS[cpts].state = 1;
    strncpy(tabS[cpts].name, nom,   19); tabS[cpts].name[19] = '\0';
    strncpy(tabS[cpts].type, token, 19); tabS[cpts].type[19] = '\0';
    cpts++;
}

/*AFFICHAGE DES TROIS TABLES*/
void ts_afficher(void) {

    /* ---- Table principale : IDF / CONST / TABLEAU ---- */
    printf("\n/****** Table des symboles (Hachage FNV-1 + Listes chainees) ******/\n");
    printf("+------+----+--------------------+----------+-----------+--------------------+\n");
    printf("| Seau | #  | Nom                | Code     | Type      | Val / Taille       |\n");
    printf("+------+----+--------------------+----------+-----------+--------------------+\n");

    int compteur = 0; /* numéro de ligne affiché */

    /* Parcourir chaque seau de la table de hachage */
    for (int h = 0; h < HASH_SIZE; h++) {
        NoeudTS *courant = hashTable[h]; /* tête de la liste du seau h */

        /* Parcourir toute la liste chaînée du seau */
        while (courant != NULL) {
            if (courant->state == 1) { /* afficher uniquement les noeuds actifs */
                printf("| %-4d | %-2d | %-18s | %-8s | %-9s | %-18s |\n",
                       h,                            /* numéro du seau        */
                       compteur++,                   /* numéro de ligne       */
                       courant->name,                /* nom du symbole        */
                       courant->code,                /* IDF / CONST / TABLEAU */
                       courant->type,                /* integer / float       */
                       courant->val[0] ? courant->val : "-"); /* valeur ou - */
            }
            courant = courant->suivant; /* passer au noeud suivant de la liste */
        }
    }

    printf("+------+----+--------------------+----------+-----------+--------------------+\n");
    printf("Total: %d entree(s)  |  Taille de la table: %d seaux\n", cpt, HASH_SIZE);

    /* ---- Afficher aussi les collisions (seaux avec > 1 entrée) ---- */
    printf("\n--- Statistiques de hachage ---\n");
    int seaux_utilises = 0;
    int max_chaine     = 0;
    for (int h = 0; h < HASH_SIZE; h++) {
        int len = 0;
        NoeudTS *c = hashTable[h];
        while (c) { len++; c = c->suivant; }  /* longueur de la liste du seau h */
        if (len > 0) seaux_utilises++;
        if (len > max_chaine) max_chaine = len;
    }
    printf("Seaux utilises : %d / %d\n", seaux_utilises, HASH_SIZE);
    printf("Longueur max d'une liste chainee : %d\n", max_chaine);

    /* ---- Table des mots-clés ---- */
    printf("\n/****** Table des mots-cles ******/\n");
    printf("+----+--------------------+--------------------+\n");
    printf("| #  | Mot-cle            | Token              |\n");
    printf("+----+--------------------+--------------------+\n");
    for (int i = 0; i < cptm; i++)
        if (tabM[i].state == 1)
            printf("| %-2d | %-18s | %-18s |\n", i, tabM[i].name, tabM[i].type);
    printf("+----+--------------------+--------------------+\n");
    printf("Total: %d mot(s)-cle(s)\n", cptm);

    /* ---- Table des séparateurs ---- */
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
