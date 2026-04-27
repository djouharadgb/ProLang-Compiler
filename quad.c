#include "quad.h"
#define RED   "\033[1;31m"
#define RESET "\033[0m"

qdr quad[MAX_QUADS];
int qc = 0;
int temp_count = 0;

char *nouveau_temp(void) {
    static char temp[20];
    sprintf(temp, "T%d", temp_count++); /*gere les ecriture de type T1 */
    return temp;
}

/* 1- Fonction d'ajout d'un quadruplet a une table de quadruplets */
void quadr(char oper[], char op1[], char op2[], char res[]) {
    if (qc >= MAX_QUADS) {
        fprintf(stderr, RED "Erreur: depassement du nombre max de quadruplets" RESET "\n");
        exit(1);
    }
    strcpy(quad[qc].oper, oper ? oper : "");
    strcpy(quad[qc].op1,  op1  ? op1  : "");
    strcpy(quad[qc].op2,  op2  ? op2  : "");
    strcpy(quad[qc].res,  res  ? res  : "");
    qc++;
}

/* 2- Fonction qui permet de mettre a jour les quadruplets
   selon la position : (0=oper, 1=op1, 2=op2, 3=res)       */
void updateQuad(int num_quad, int colon_quad, char val[]) {
    if (num_quad < 0 || num_quad >= qc) return;
    if      (colon_quad == 0) strcpy(quad[num_quad].oper, val);
    else if (colon_quad == 1) strcpy(quad[num_quad].op1,  val);
    else if (colon_quad == 2) strcpy(quad[num_quad].op2,  val);
    else if (colon_quad == 3) strcpy(quad[num_quad].res,  val);
}

/* Fonction d'affichage des quadruplets generes */
void afficher_qdr(void) {
    printf("*********************Les Quadruplets***********************\n");
    int i;
    for (i = 0; i < qc; i++) {
        printf("\n %d - (%s, %s, %s, %s)",
               i, quad[i].oper, quad[i].op1, quad[i].op2, quad[i].res);
               printf("\n");
    }
}
