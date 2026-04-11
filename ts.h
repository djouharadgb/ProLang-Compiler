#ifndef TS_H
#define TS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 256   /* Taille de la table de hachage */
#define MAX_IDF    14    /* Longueur max d'un identificateur */

/* Types possibles */
typedef enum {
    TYPE_INTEGER,
    TYPE_FLOAT
} TypeVar;

/* Nature de l'entree */
typedef enum {
    NATURE_VAR,
    NATURE_CONST,
    NATURE_TABLEAU
} NatureVar;

/* Structure d'une entree dans la table des symboles */
typedef struct Symbol {
    char   nom[MAX_IDF + 1];
    TypeVar type;
    NatureVar nature;
    int    taille;         /* taille du tableau (1 si variable simple) */
    int    est_init;       /* 1 si initialisee, 0 sinon */
    union {
        int    val_int;
        float  val_float;
    } valeur;
    struct Symbol *suivant; /* chainage pour collision */
} Symbol;


/* Fonction de hachage FNV-1a */
unsigned int fnv1a_hash(const char *cle);

/* Initialiser la table des symboles */
void ts_initialiser(void);

/* Rechercher un symbole — retourne NULL si non trouve */
Symbol* ts_rechercher(const char *nom);

/* Inserer une variable simple */
int ts_inserer_variable(const char *nom, TypeVar type);

/* Inserer une constante */
int ts_inserer_constante(const char *nom, TypeVar type);

/* Inserer un tableau */
int ts_inserer_tableau(const char *nom, TypeVar type, int taille);

/* Verifier si un symbole est declare */
int ts_est_declare(const char *nom);

/* Marquer un symbole comme initialise */
void ts_marquer_init(const char *nom);

/* Verifier la double declaration — retourne 1 si deja declare */
int ts_double_declaration(const char *nom);

/* Verifier si c'est une constante */
int ts_est_constante(const char *nom);

/* Afficher la table des symboles */
void ts_afficher(void);

/* Liberer la memoire de la table */
void ts_liberer(void);

#endif 
