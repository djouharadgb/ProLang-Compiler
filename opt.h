#ifndef OPT_H
#define OPT_H

/*
  opt.h  -  Optimisation du code intermediaire

  Passes implementees (selon cours USTHB 2025-2026) :
    1. Propagation de constantes symboliques
         const N = 5 => remplace N par 5 dans tous les quadruplets,
         permettant aux passes suivantes de plier les calculs.

    2. Simplification algebrique
         X + 0 => X,  X * 1 => X,  X * 0 => 0,  X - X => 0,
         X / X => 1,  X / 1 => X,
         X * 2 => X + X,  X * 4 => X * 2 (reduit en 2 iter. a X+X),
         X * 8 => X * 4,  C1 op C2 => resultat (constant folding).

    3. Simplification des constantes chainees
         T = X + C1 ; R = T + C2  =>  R = X + (C1+C2), T supprime.

    4. Propagation de copie
         T := src  =>  remplace T par src dans les quadruplets suivants
         (temporaires seulement, barriere sur les sauts).

    5. Propagation d'expression
         T := src utilise une seule fois => inline T au site d'usage,
         quadruplet de definition supprime (NOP).

    6. Elimination des expressions redondantes (CSE)
         a + b calcule deux fois => reutiliser le premier temporaire.
         Commutativite prise en compte pour + et *.

    7. Elimination de variables mortes (passe arriere)
         x := ... suivi d'aucune lecture de x => affectation supprimee.
         Complete la passe 8 qui ne traite que les temporaires.

    8. Elimination de code inutile
         Supprime les NOP et les temporaires dont le resultat n'est
         jamais relu (dead temporaries).

  Toutes les passes sont appliquees en boucle jusqu'a point fixe
  (30 iterations maximum). Les chaines de reduction (ex: X*8 -> X*4
  -> X*2 -> X+X) sont resolues automatiquement sur plusieurs tours.
 */

void optimiser_quadruplets(void);

#endif
