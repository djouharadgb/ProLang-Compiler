#ifndef TS_H
#define TS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_IDF     14          /* longueur max d'un identificateur  */
#define HASH_SIZE   64          /* nombre de cases dans la table de hachage*/
#define MAX_SM      100         /* taille des tables mots-clés / séparateurs*/

typedef enum {
    TYPE_INTEGER,   /* type entier  (integer) */
    TYPE_FLOAT      /* type réel    (float)   */
} TypeVar;

/* ================================================================
   NOEUD de la liste chaînée  (entrée dans la table des symboles)
   ================================================================ */
typedef struct NoeudTS {
    char  name[MAX_IDF + 1]; /* nom de l'identificateur / constante / tableau */
    char  code[10];          /* catégorie : "IDF", "CONST", "TABLEAU"          */
    char  type[10];          /* type      : "integer" ou "float"               */
    char  val[20];           /* valeur initiale ou taille (tableau)            */
    int   state;             /* 1 = occupé, 0 = supprimé        */

   struct NoeudTS *suivant; /* pointeur vers le prochain noeud  */
} NoeudTS;

extern NoeudTS *hashTable[HASH_SIZE]; /* tableau de pointeurs (têtes de listes) */
extern int      cpt;                  /* nombre total d'entrées dans la TS       */

typedef struct {
    int  state;       /* 1 = occupé */
    char name[20];    /* lexème du mot-clé         */
    char type[20];    /* token associé              */
} TypeSM;

extern TypeSM tabM[MAX_SM]; /* table des mots-clés   */
extern TypeSM tabS[MAX_SM]; /* table des séparateurs */
extern int    cptm;         /* nb de mots-clés       */
extern int    cpts;         /* nb de séparateurs     */
extern int semantic_errors;


/* Initialisation / libération */
void ts_initialiser(void);
void ts_liberer(void);

/* Vérifications */
int  ts_double_declaration(const char *nom);
int  ts_est_declare       (const char *nom);
int  ts_est_constante     (const char *nom);

/* Lecture / écriture de valeur */
void        ts_marquer_init(const char *nom);
void        ts_set_val     (const char *nom, const char *val);
const char *ts_get_val     (const char *nom);

/* Insertions dans la TS principale */
int  ts_inserer_variable (const char *nom, TypeVar type);
int  ts_inserer_constante(const char *nom, TypeVar type);
int  ts_inserer_tableau  (const char *nom, TypeVar type, int taille);

/* Insertions dans les tables auxiliaires */
void ts_inserer_mc (const char *nom, const char *token);
void ts_inserer_sep(const char *nom, const char *token);

/* Affichage */
void ts_afficher(void);

/* Récupérer le type d'un symbole ("integer" ou "float") */
const char *ts_get_type(const char *nom);

/* Récupérer la taille d'un tableau (retourne -1 si pas un tableau) */
int ts_get_taille(const char *nom);

/* Vérifier si une variable a été initialisée */
int ts_est_initialise(const char *nom);
#endif 
