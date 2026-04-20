#ifndef OPT_H
#define OPT_H

/*
 * opt.h  –  Optimisation du code intermediaire
 *
 * Passes implementees (selon cours USTHB 2025-2026) :
 *   1. Simplification algebrique
 *   2. Propagation de copie
 *   3. Propagation d'expression  (reduction de copies intermediaires)
 *   4. Elimination des expressions redondantes (CSE)
 *   5. Elimination de code inutile (dead code)
 *
 * Toutes les passes sont appliquees en boucle jusqu'a point fixe.
 */

void optimiser_quadruplets(void);

#endif /* OPT_H */
