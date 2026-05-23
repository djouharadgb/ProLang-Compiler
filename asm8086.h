#ifndef ASM8086_H
#define ASM8086_H

/*
 asm8086.h  –  Generation de code assembleur 8086
 
 Traduit les quadruplets du code intermediaire (apres optimisation)
 en instructions assembleur 8086 conformes au cours USTHB 2025-2026
 
 Structure du fichier .asm genere :
  PILE  SEGMENT STACK   ; segment de pile (100 mots)
   DONNEE SEGMENT        ; variables entiers (DW) et flottants (DD)
   LECODE SEGMENT        ; code traduit depuis les quadruplets
     Debut:
       ASSUME ...
       MOV AX, DONNEE / MOV DS, AX  ; init segment donnees
       ... instructions traduites ...
     MOV AH, 4Ch / INT 21h        ; terminaison DOS
   LECODE ENDS
   END Debut
 */

/*
 generer_asm : point d'entree principal.
 nom_fichier : chemin du fichier .asm a creer (ex: "out.asm").
 Si nom_fichier == NULL, "sortie.asm" est utilise par defaut.
 */
void generer_asm(const char *nom_fichier);

#endif 
