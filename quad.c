#include "quad.h"

Quadruplet quads[MAX_QUADS];
int qc = 0;
int temp_count = 0;

char* nouveau_temp(void) {
    static char temp[20];
    sprintf(temp, "T%d", temp_count++);
    return temp;
}

/* Generer un quadruplet */
int generer_quad(const char *op, const char *arg1, const char *arg2, const char *result) {
    if (qc >= MAX_QUADS) {
        fprintf(stderr, "Erreur: depassement du nombre max de quadruplets\n");
        exit(1);
    }
    strncpy(quads[qc].op,     op     ? op     : "", 9); //if its null use empty string
    strncpy(quads[qc].arg1,   arg1   ? arg1   : "", MAX_ARG - 1);
    strncpy(quads[qc].arg2,   arg2   ? arg2   : "", MAX_ARG - 1); // -1 when its bigger than max arg to add \0
    strncpy(quads[qc].result, result ? result : "", MAX_ARG - 1);
    return qc++;
}

/* Mettre a jour un quadruplet */
void maj_quad(int index, const char *result) {
    if (index >= 0 && index < qc) {
        strncpy(quads[index].result, result ? result : "", MAX_ARG - 1);
    }
}

/* Afficher les quadruplets */
void afficher_quads(void) {
    printf("\n╔═══════════════════════════════════════════════════╗\n");
    printf("║          CODE INTERMEDIAIRE (Quadruplets)         ║\n");
    printf("╠═════╦══════════╦════════════╦════════════╦════════╣\n");
    printf("║  #  ║ Operateur║ Arg1       ║ Arg2       ║ Result ║\n");
    printf("╠═════╬══════════╬════════════╬════════════╬════════╣\n");
    for (int i = 0; i < qc; i++) {
        printf("║ %-3d ║ %-8s ║ %-10s ║ %-10s ║ %-6s ║\n",
               i, quads[i].op, quads[i].arg1, quads[i].arg2, quads[i].result);
    }
    printf("╚═════╩══════════╩════════════╩════════════╩════════╝\n");
    printf("Total: %d quadruplets\n\n", qc);
}
