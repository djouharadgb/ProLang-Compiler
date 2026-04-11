#ifndef TS_H
#define TS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TS   200   /* taille table IDF / CONST / TABLEAU */
#define MAX_SM    50   /* taille tables mots-cles et separateurs */
#define MAX_IDF   19   /* longueur max d'un identifiant */

/* Types et natures — gardes pour compatibilite avec Syn.y */
typedef enum { TYPE_INTEGER, TYPE_FLOAT }             TypeVar;
typedef enum { NATURE_VAR, NATURE_CONST, NATURE_TABLEAU } NatureVar;

/* -------- Table des IDFs / Constantes / Tableaux -------- */
typedef struct {
    int  state;        /* 0: libre, 1: occupe            */
    char name[20];     /* nom de l'identifiant           */
    char code[20];     /* "IDF", "CONST" ou "TABLEAU"   */
    char type[20];     /* "integer" ou "float"           */
    char val[20];      /* valeur initiale / taille / "" */
} TypeTS;

/* -------- Table des mots-cles et separateurs -------- */
typedef struct {
    int  state;
    char name[20];     /* texte du lexeme (ex: "if", ";") */
    char type[20];     /* token Bison (ex: "IF_MC")        */
} TypeSM;

extern TypeTS TS[MAX_TS];
extern TypeSM tabM[MAX_SM];   /* mots-cles   */
extern TypeSM tabS[MAX_SM];   /* separateurs */
extern int cpt, cptm, cpts;

/* Compteur global d'erreurs semantiques */
extern int semantic_errors;

/* -------- API -------- */
void ts_initialiser(void);

int  ts_inserer_variable (const char *nom, TypeVar type);
int  ts_inserer_constante(const char *nom, TypeVar type);
int  ts_inserer_tableau  (const char *nom, TypeVar type, int taille);

void ts_inserer_mc (const char *nom, const char *token);
void ts_inserer_sep(const char *nom, const char *token);

int  ts_est_declare       (const char *nom);
int  ts_est_constante     (const char *nom);
int  ts_double_declaration(const char *nom);
void ts_marquer_init      (const char *nom);
void        ts_set_val    (const char *nom, const char *val);
const char *ts_get_val    (const char *nom);

void ts_afficher(void);
void ts_liberer (void);

#endif
