#ifndef QUAD_H
#define QUAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Format: (operateur, argument1, argument2, resultat) */

#define MAX_QUADS  1000
#define MAX_ARG    50

typedef struct {
    char op[10];
    char arg1[MAX_ARG];
    char arg2[MAX_ARG];
    char result[MAX_ARG];
} Quadruplet;

/* Compteur de quadruplets */
extern int qc;              /* quad counter (prochain indice libre) */
extern Quadruplet quads[];  /* tableau de quadruplets */
extern int temp_count;      /* compteur de temporaires */

/* Generer un nouveau temporaire Ti */
char* nouveau_temp(void);

/* Generer un quadruplet — retourne l'indice */
int generer_quad(const char *op, const char *arg1, const char *arg2, const char *result);

/* Mettre a jour le champ result d'un quadruplet existant */
void maj_quad(int index, const char *result);

/* Afficher tous les quadruplets */
void afficher_quads(void);

#endif 
