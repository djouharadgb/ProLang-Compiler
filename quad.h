#ifndef QUAD_H
#define QUAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Declaration de la structure quadruplets */
typedef struct {
    char oper[100];
    char op1[100];
    char op2[100];
    char res[100];
} qdr;

#define MAX_QUADS 1000

extern qdr quad[MAX_QUADS];

/* Compteur de quadruplets */
extern int qc;
extern int temp_count;

/* Generer un nouveau temporaire Ti */
char *nouveau_temp(void);

/* Ajouter un quadruplet */
void quadr(char oper[], char op1[], char op2[], char res[]);

/* Mettre a jour un champ d'un quadruplet (colon: 0=oper,1=op1,2=op2,3=res) */
void updateQuad(int num_quad, int colon_quad, char val[]);

/* Afficher les quadruplets */
void afficher_qdr(void);

#endif
