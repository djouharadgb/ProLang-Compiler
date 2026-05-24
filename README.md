# ProLang Mini-Compiler

Mini-compilateur pour le langage ProLang, realise dans le cadre du module Compilation, Master 1 Informatique, USTHB, annee universitaire 2025/2026.

Le compilateur couvre toutes les phases classiques de la compilation : analyse lexicale, analyse syntaxique, analyse semantique, generation de code intermediaire sous forme de quadruplets, optimisation du code intermediaire, et generation du code objet en assembleur 8086.

## Structure du projet

```
.
├── prolang.l              Analyseur lexical (Flex)
├── Syn.y                  Grammaire et actions semantiques (Bison)
├── ts.c / ts.h            Table des symboles (hachage FNV-1)
├── quad.c / quad.h        Table des quadruplets
├── opt.c / opt.h          Optimiseur de code intermediaire
├── asm8086.c / asm8086.h  Generateur de code assembleur 8086
├── source_eval.txt        Fichier source d'evaluation fourni
├── fichier_test_all_errors.txt   Programme test couvrant toutes les erreurs implementees
├── erreur_traitee.txt     Liste complete des erreurs detectees et leur traitement
├── quadruples_optimisees.txt     Quadruplets optimises produits sur source_eval.txt
├── sortie_djo.asm         Code assembleur 8086 genere sur source_eval.txt
├── documentation_optimisation_generation_code_objet.html   Documentation technique detaillee
├── prolang.bat            Script de compilation et execution sous Windows
└── prolang / prolang.exe  Executable du compilateur
```

## Le langage ProLang

La description complete du langage (identificateurs, types, declarations, instructions, operateurs, commentaires) est fournie dans `Projet_M1_compil_25-26.pdf`. Structure generale d'un programme :

```
BeginProject nom_programme;

Setup :
    %% declarations ici

Run :
{
    //* instructions ici *//
}

EndProject ;
```

## Phases du compilateur

### 1. Analyse lexicale : `prolang.l`

Le fichier Flex parcourt le source caractere par caractere et produit les tokens transmis a Bison. Chaque regle tient a jour deux compteurs globaux `nb_ligne` et `nb_col` pour localiser precisement toute erreur.

Sont reconnus : les mots-cles du langage (`BeginProject`, `Setup`, `Run`, `define`, `const`, `if`, `loop`, `for`, `input`, `out`, ...), les identificateurs valides, les constantes entieres et reelles, les chaines de caracteres, les operateurs arithmetiques et de comparaison, l'operateur d'affectation (`<--`), l'operateur d'initialisation (`=`), les separateurs (`;`, `:`, `,`, `|`, `[`, `]`, `{`, `}`, `(`, `)`), et les commentaires mono-ligne (`%%`) et multi-lignes (`//* ... *//`) via un etat exclusif Flex.

Chaque mot-cle reconnu est insere dans la table des mots-cles (`tabM`). Chaque separateur ou operateur est insere dans la table des separateurs (`tabS`).

Erreurs lexicales detectees, avec numero de ligne et colonne : identificateur commencant par un tiret bas, identificateur depassant 14 caracteres, identificateur se terminant par un tiret bas, identificateur contenant deux tirets bas consecutifs, constante reelle mal formee (chiffre manquant apres le point, ex: `3.`), constante entiere hors des bornes [-32768, 32767], caractere inconnu non reconnu par aucune regle.

### 2. Analyse syntaxico-semantique : `Syn.y`

Le fichier Bison definit la grammaire complete du langage et declenche les actions semantiques integrees directement dans les regles de production.

La grammaire couvre la structure globale du programme, les declarations de variables simples avec ou sans initialisation, les tableaux, les constantes, la declaration groupee de plusieurs variables par le separateur `|`, les instructions d'affectation simple et sur tableau, les conditions if/then/else/endIf, les boucles while imbriquees et les boucles for imbriquees (chacune avec une pile de contexte dediee), et les instructions input/output acceptant des listes d'elements.

Analyses semantiques realisees : double declaration d'un identifiant, utilisation d'une variable non declaree dans une expression ou affectation ou input/output, utilisation d'une variable sans initialisation prealable, affectation vers un identifiant de type CONST, acces a un tableau non declare, declaration d'un tableau avec une taille nulle ou negative, declaration d'un tableau avec plusieurs noms, division par zero detectable statiquement (diviseur litteral 0 ou variable dont la valeur connue est 0 dans la table des symboles).

Le compteur `semantic_errors` est incremente a chaque erreur semantique. La generation du code assembleur n'est declenchee qu'en l'absence d'erreur semantique.

La generation des quadruplets a lieu directement dans les actions Bison, au moment ou chaque construction est reduite. Les branchements conditionnels (BZ) et inconditionnels (BR) sont emis avec backpatching : l'adresse cible est d'abord laissee vide puis mise a jour par `updateQuad` une fois la cible connue.

### 3. Table des symboles : `ts.c`

La table des symboles est implementee sous forme de table de hachage ouverte (chaining) avec la fonction de hachage FNV-1 (Fowler-Noll-Vo, 32 bits). Chaque noeud contient le nom du symbole (max 14 caracteres), sa categorie (`IDF`, `CONST` ou `TABLEAU`), son type (`integer` ou `float`), sa valeur d'initialisation ou la taille du tableau, et un indicateur d'etat actif.

Trois tables distinctes sont gerees en parallele : la table de hachage principale pour les variables, constantes et tableaux, `tabM` pour les mots-cles enregistres lors de l'analyse lexicale, et `tabS` pour les separateurs et operateurs.

Fonctions disponibles : insertion de variable, constante ou tableau, verification de double declaration et de declaration, verification d'initialisation, lecture et ecriture de la valeur, lecture du type, lecture de la taille d'un tableau. L'affichage final presente les trois tables sous forme de tableaux ASCII.

### 4. Table des quadruplets : `quad.c`

Un quadruplet a la forme `(operateur, op1, op2, resultat)`. La table `quad[]` est un tableau statique de taille `MAX_QUADS`, le compteur global `qc` designant le prochain emplacement libre.

`quadr(oper, op1, op2, res)` ajoute un quadruplet en fin de table. `updateQuad(num, colonne, val)` met a jour un champ specifique d'un quadruplet existant, utilise pour le backpatching des sauts. `nouveau_temp()` genere un nom de temporaire unique de la forme T0, T1, T2, ... `afficher_qdr()` affiche toute la table.

Operateurs utilises dans les quadruplets : `:=`, `+`, `-`, `*`, `/`, `NEG`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `AND`, `OR`, `NON`, `BZ`, `BR`, `TAB`, `input`, `out`, `NOP`.

### 5. Optimiseur de code intermediaire : `opt.c`

L'optimiseur applique plusieurs passes successives sur la table des quadruplets, repetees en boucle tant qu'au moins une modification est effectuee afin de propager les effets de chaque passe sur les suivantes.

**Passe 1 : Propagation de constantes symboliques**
Les symboles declares `CONST` dans la table des symboles sont remplaces par leur valeur numerique dans les operandes des quadruplets. Exemple : avec `const MAX = 100`, toutes les occurrences de `MAX` en op1 ou op2 deviennent `100`.

**Passe 2 : Simplification algebrique**
Regles appliquees sur chaque quadruplet : `X + 0` => `X`, `0 + X` => `X`, `X - 0` => `X`, `X - X` => `0`, `X * 0` => `0`, `X * 1` => `X`, `X * 2` => `X + X`, reduction de `X * 4` et `X * 8` en multiplications successives par 2, `X / 1` => `X`, `X / X` => `1`, et calcul direct a la compilation (constant folding) quand les deux operandes sont des constantes.

**Passe 3 : Simplification des constantes chainees**
Deux operations consecutives `+/-` avec des constantes portant sur le meme temporaire utilise une seule fois sont fusionnees en une seule. Exemple : `T = X + 3`, `R = T - 3` => `R = X`, le temporaire intermediaire disparaissant.

**Passe 4a : Fusion temporaire vers destination**
Quand un temporaire est calcule puis immediatement recopie vers une destination, on ecrit directement le resultat dans la destination finale et on supprime la copie. Pattern : `T = expr`, `dst := T` => `dst = expr`, `NOP`.

**Passe 4b : Propagation de copie**
Quand un temporaire `T` est affecte depuis une valeur `src`, toutes ses utilisations suivantes jusqu'au prochain saut ou redefinition sont remplacees par `src`.

**Passe 5 : Propagation d'expression**
Si un temporaire utilise exactement une fois peut etre substitue directement au site de consommation, la definition intermediaire est supprimee (NOP).

**Passe 6 : Elimination des expressions redondantes (CSE)**
Une table des expressions disponibles est maintenue. Si une expression `op1 operateur op2` a deja ete calculee et que ses operandes n'ont pas ete redefinies depuis, le nouveau calcul est remplace par une simple copie du temporaire precedent. La commutativite est prise en compte pour `+` et `*`.

**Passe 7 : Elimination de code mort**
Les quadruplets dont le resultat n'est jamais utilise et qui n'ont pas d'effet de bord (pas de saut, pas de input, pas de out) sont marques NOP.

**Compaction finale**
Les quadruplets NOP sont supprimes de la table. Les references de saut (BZ, BR) sont mises a jour pour pointer vers les nouveaux indices apres compaction.

Les passes evitent de modifier les temporaires utilises comme indices de tableau dans un champ `res` (detection par `est_utilise_comme_index`). Les barrieres de flot (BZ, BR) interrompent les propagations locales pour ne pas traverser un branchement.

### 6. Generateur de code assembleur 8086 : `asm8086.c`

Le generateur parcourt la table des quadruplets apres optimisation et produit un fichier `.asm` compatible avec l'assembleur 8086.

Sont generes : une section `.data` avec la declaration de toutes les variables (DW pour integer et float en virgule fixe), des tableaux (autant de DW que la taille declaree), et des chaines de caracteres pour `out`; une section `.code` avec des procedures standard de multiplication et division 16 bits; les affectations simples via MOV; les operations arithmetiques `+`, `-`, `*`, `/` avec gestion des temporaires dans les registres AX/BX/CX/DX; la negation unaire via NEG AX; les acces tableaux avec calcul d'adresse par indice; les comparaisons produisant 0 ou 1 dans un temporaire; les operateurs logiques AND, OR, NON sur valeurs booleennes 0/1; les branchements conditionnels BZ et inconditionnels BR avec etiquettes auto-generees `L0`, `L1`, ...; les instructions `input` et `out` via les interruptions DOS INT 21h; et la gestion des flottants par virgule fixe (multiplication par 100 a l'entree, division pour l'affichage).

## Compilation et utilisation

Sous Linux :

```bash
flex prolang.l
bison -d Syn.y
gcc lex.yy.c Syn.tab.c ts.c quad.c opt.c asm8086.c -o prolang -lfl
```

Sous Windows :

```bat
prolang.bat
```

Execution :

```bash
./prolang < source_eval.txt
```

Le compilateur lit le programme source sur l'entree standard et produit sur la sortie standard les messages d'analyse, la table des symboles complete, les quadruplets avant optimisation, les quadruplets apres optimisation, puis genere le fichier `.asm` 8086.

## Fichiers de test fournis

`source_eval.txt` est le programme source complet utilise pour l'evaluation. Il contient des declarations de variables, constantes et tableaux, des affectations simples et sur tableau, des boucles while et for imbriquees, des conditions if/else, et des instructions input/output.

`fichier_test_all_errors.txt` est un programme volontairement errone couvrant toutes les erreurs implementees : identificateurs mal formes (commence par `_`, trop long, finit par `_`, tirets bas consecutifs), constante entiere hors bornes, flottant mal forme, caractere inconnu, double declaration, variable non declaree, variable non initialisee, modification d'une constante, taille de tableau invalide, division par zero.

`erreur_traitee.txt` liste chaque erreur traitee, sa categorie (lexicale, syntaxique, semantique), le message affiche et la construction qui la declenche.

`quadruples_optimisees.txt` contient la sortie complete des quadruplets apres toutes les passes d'optimisation appliquees sur `source_eval.txt`.

`sortie_djo.asm` est le code assembleur 8086 genere sur `source_eval.txt`.

`documentation_optimisation_generation_code_objet.html` est la documentation technique detaillant chaque passe d'optimisation et la logique de generation du code objet.

## Format des messages d'erreur

Toutes les erreurs sont affichees en rouge sur le terminal et suivent le format :

```
TYPE_ERREUR: description, ligne L, col C
```

Exemples :

```
ERREUR lexicale: identificateur '_bad' ne doit pas commencer par '_', ligne 5, col 3
ERREUR lexicale: identificateur 'trop_long_idf_xx' depasse 14 caracteres, ligne 8, col 1
ERREUR semantique: double declaration de 'x', ligne 12, col 1
ERREUR semantique: variable 'y' utilisee sans initialisation, ligne 20, col 5
ERREUR semantique: division par zero (diviseur '0' vaut 0), ligne 25, col 10
ERREUR syntaxique: syntax error, ligne 30, col 8
```

