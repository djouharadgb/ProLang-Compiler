TITLE prolang.asm
;
; Code assembleur 8086 genere automatiquement par le compilateur Prolang
; USTHB 2025-2026 - SAYOUD Lynda
;

PILE SEGMENT STACK
    DW 100 DUP (?)     ; reserve 100 mots = 200 octets pour la pile
base_pile EQU $        ; etiquette base de la pile
PILE ENDS

DONNEE SEGMENT

    moyenne              DW 0           ; float (partie entiere)
    somme                DW 0
    z                    DW ?
    y                    DW ?
    x                    DW ?
    Max                  DW 100
    Tabint               DW 50 DUP(?)    ; tableau de 50 entiers
    Pi                   DW 3           ; float (partie entiere)
    Tabfloat             DW 30 DUP(?)    ; tableau de 30 entiers
    k                    DW ?
    j                    DW ?
    i                    DW ?
    b                    DW ?
    a                    DW ?
    _T6                  DW ?             ; temporaire compilateur
    _T8                  DW ?             ; temporaire compilateur
    _T9                  DW ?             ; temporaire compilateur
    _T10                 DW ?             ; temporaire compilateur
    _T11                 DW ?             ; temporaire compilateur
    _T12                 DW ?             ; temporaire compilateur
    _T13                 DW ?             ; temporaire compilateur
    _T14                 DW ?             ; temporaire compilateur
    _T15                 DW ?             ; temporaire compilateur
    _T16                 DW ?             ; temporaire compilateur
    _T17                 DW ?             ; temporaire compilateur
    _T18                 DW ?             ; temporaire compilateur
    _T19                 DW ?             ; temporaire compilateur
    _T20                 DW ?             ; temporaire compilateur
    _T21                 DW ?             ; temporaire compilateur
    _T22                 DW ?             ; temporaire compilateur
    _T23                 DW ?             ; temporaire compilateur
    _T24                 DW ?             ; temporaire compilateur
    _T25                 DW ?             ; temporaire compilateur
    _T26                 DW ?             ; temporaire compilateur
    _T27                 DW ?             ; temporaire compilateur
    _T28                 DW ?             ; temporaire compilateur
    _T29                 DW ?             ; temporaire compilateur
    _T30                 DW ?             ; temporaire compilateur
    _T31                 DW ?             ; temporaire compilateur
    _T32                 DW ?             ; temporaire compilateur
    _T33                 DW ?             ; temporaire compilateur
    _T34                 DW ?             ; temporaire compilateur
    _T35                 DW ?             ; temporaire compilateur
    _T37                 DW ?             ; temporaire compilateur
    _T38                 DW ?             ; temporaire compilateur
    _T39                 DW ?             ; temporaire compilateur
    _T40                 DW ?             ; temporaire compilateur
    _T41                 DW ?             ; temporaire compilateur
    _T36                 DW ?             ; temporaire compilateur
    _T42                 DW ?             ; temporaire compilateur
    _T43                 DW ?             ; temporaire compilateur
    _T44                 DW ?             ; temporaire compilateur
    _T45                 DW ?             ; temporaire compilateur
    _T46                 DW ?             ; temporaire compilateur
    _T47                 DW ?             ; temporaire compilateur
    _T48                 DW ?             ; temporaire compilateur
    _T49                 DW ?             ; temporaire compilateur
    _T50                 DW ?             ; temporaire compilateur
    _T51                 DW ?             ; temporaire compilateur
    _OUT_BUF             DB 10 DUP(?), '$' ; tampon affichage

DONNEE ENDS

LECODE SEGMENT

;---- Procedure d'affichage d'un entier dans AX ----
_PRINT_INT PROC NEAR
    ; Convertit AX en chaine et affiche via DOS INT 21h
    PUSH AX
    PUSH BX
    PUSH CX
    PUSH DX
    PUSH SI
    MOV SI, OFFSET _OUT_BUF
    ; Gerer le signe negatif
    CMP AX, 0
    JGE _PRINT_POS
    MOV BYTE PTR [SI], '-'
    INC SI
    NEG AX
_PRINT_POS:
    ; Divisor BX = 10, CX compte les chiffres
    MOV BX, 10
    MOV CX, 0
_DIV_LOOP:
    MOV DX, 0
    DIV BX             ; AX = AX / 10, DX = reste
    ADD DL, '0'
    PUSH DX            ; empiler le chiffre (LIFO => ordre inverse)
    INC CX
    CMP AX, 0
    JNE _DIV_LOOP
_POP_LOOP:
    POP DX
    MOV [SI], DL
    INC SI
    LOOP _POP_LOOP
    MOV BYTE PTR [SI], '$'  ; terminateur DOS
    MOV AH, 09h
    MOV DX, OFFSET _OUT_BUF
    INT 21h
    ; Afficher un espace separateur
    MOV AH, 02h
    MOV DL, ' '
    INT 21h
    POP SI
    POP DX
    POP CX
    POP BX
    POP AX
    RET
_PRINT_INT ENDP

;---- Procedure de lecture d'un entier depuis le clavier ----
_READ_INT PROC NEAR
    ; Lit les caracteres un a un (INT 21h AH=01h) et construit l'entier
    PUSH BX
    PUSH CX
    PUSH DX
    MOV BX, 0     ; accumulateur
    MOV CX, 0     ; signe (0=positif, 1=negatif)
_READ_CHAR:
    MOV AH, 01h
    INT 21h       ; AL = caractere lu (avec echo)
    CMP AL, 0Dh   ; Entree (CR) ?
    JE  _READ_DONE
    CMP AL, '-'
    JNE _READ_DIGIT
    MOV CX, 1     ; marquer negatif
    JMP _READ_CHAR
_READ_DIGIT:
    SUB AL, '0'   ; convertir ASCII -> chiffre
    CBW            ; etendre AL vers AX
    XCHG AX, BX
    MOV DX, 10
    MUL DX        ; AX = BX * 10
    ADD AX, BX    ; AX = AX + nouveau chiffre
    XCHG AX, BX
    JMP _READ_CHAR
_READ_DONE:
    MOV AX, BX
    CMP CX, 1
    JNE _READ_POS
    NEG AX
_READ_POS:
    ; newline
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h
    POP DX
    POP CX
    POP BX
    RET
_READ_INT ENDP

Debut:
    ASSUME CS:LECODE, DS:DONNEE, SS:PILE

    ; Initialisation des segments (diapo 136)
    MOV AX, DONNEE
    MOV DS, AX         ; DS pointe sur le segment de donnees
    MOV AX, PILE
    MOV SS, AX         ; SS pointe sur le segment de pile
    MOV SP, base_pile  ; SP pointe sur la base de la pile (pile vide)

L0:
    ; [0] (:=, 0, , somme)
    MOV AX, 0
    MOV somme, AX

    ; [1] (:=, 0.000000, , moyenne)
    MOV AX, 0
    MOV moyenne, AX

    ; [2] (:=, 3.141590, , Pi)
    MOV AX, 3
    MOV Pi, AX

    ; [3] (:=, 100, , Max)
    MOV AX, 100
    MOV Max, AX

    ; [4] (:=, 10, , x)
    MOV AX, 10
    MOV x, AX

    ; [5] (:=, 5, , y)
    MOV AX, 5
    MOV y, AX

    ; [6] (:=, 2, , z)
    MOV AX, 2
    MOV z, AX

    ; [7] (:=, 2.500000, , a)
    MOV AX, 2
    MOV a, AX

    ; [8] (:=, 11.2832, , b)
    MOV AX, 11
    MOV b, AX

    ; [9] (:=, 20, , Tabint[0])
    MOV AX, 20
    MOV SI, 0
    MOV Tabint[SI], AX

    ; [10] (:=, 7.3916, , Tabfloat[1])
    MOV AX, 7
    MOV SI, 2
    MOV Tabfloat[SI], AX

    ; [11] (>, 10, 5, T6)
    MOV AX, 10
    CMP AX, 5
    JG _CMP11_V
    MOV _T6, 0
    JMP _CMP11_S
_CMP11_V:
    MOV _T6, 1
_CMP11_S:

    ; [12] (<, 2, 15, T8)
    MOV AX, 2
    CMP AX, 15
    JL _CMP12_V
    MOV _T8, 0
    JMP _CMP12_S
_CMP12_V:
    MOV _T8, 1
_CMP12_S:

    ; [13] (AND, T6, T8, T9)
    MOV AX, _T6
    MOV BX, _T8
    AND AX, BX
    MOV _T9, AX

    ; [14] (==, 5, 0, T10)
    MOV AX, 5
    CMP AX, 0
    JE _CMP14_V
    MOV _T10, 0
    JMP _CMP14_S
_CMP14_V:
    MOV _T10, 1
_CMP14_S:

    ; [15] (NON, T10, , T11)
    MOV AX, _T10
    CMP AX, 0
    JE  _NON15_U
    MOV AX, 0
    JMP _NON15_F
_NON15_U:
    MOV AX, 1
_NON15_F:
    MOV _T11, AX

    ; [16] (OR, T9, T11, T12)
    MOV AX, _T9
    MOV BX, _T11
    OR  AX, BX
    MOV _T12, AX

    ; [17] (BZ, T12, , 50)
    MOV AX, _T12
    CMP AX, 0
    JE  L50            ; sauter si condition fausse

    ; [18] (+, x, y, T13)
    MOV AX, x
    MOV BX, y
    ADD AX, BX
    MOV _T13, AX

    ; [19] (+, T13, z, T14)
    MOV AX, _T13
    MOV BX, z
    ADD AX, BX
    MOV _T14, AX

    ; [20] (:=, T14, , somme)
    MOV AX, _T14
    MOV somme, AX

    ; [21] (:=, 0, , i)
    MOV AX, 0
    MOV i, AX

    ; [22] (<=, 0, 10, T15)
    MOV AX, 0
    CMP AX, 10
    JLE _CMP22_V
    MOV _T15, 0
    JMP _CMP22_S
_CMP22_V:
    MOV _T15, 1
_CMP22_S:

    ; [23] (BZ, T15, , 49)
    MOV AX, _T15
    CMP AX, 0
    JE  L49            ; sauter si condition fausse

    ; [24] (TAB, Tabint, i, T16)
    MOV SI, i
    ADD SI, SI
    MOV AX, Tabint[SI]
    MOV _T16, AX

    ; [25] (+, T16, i, T17)
    MOV AX, _T16
    MOV BX, i
    ADD AX, BX
    MOV _T17, AX

    ; [26] (:=, T17, , Tabint[i])
    MOV AX, _T17
    MOV SI, i
    ADD SI, SI
    MOV Tabint[SI], AX

    ; [27] (<, i, 5, T18)
    MOV AX, i
    CMP AX, 5
    JL _CMP27_V
    MOV _T18, 0
    JMP _CMP27_S
_CMP27_V:
    MOV _T18, 1
_CMP27_S:

    ; [28] (TAB, Tabint, i, T19)
    MOV SI, i
    ADD SI, SI
    MOV AX, Tabint[SI]
    MOV _T19, AX

L29:
    ; [29] (>, T19, 10, T20)
    MOV AX, _T19
    CMP AX, 10
    JG _CMP29_V
    MOV _T20, 0
    JMP _CMP29_S
_CMP29_V:
    MOV _T20, 1
_CMP29_S:

    ; [30] (AND, T18, T20, T21)
    MOV AX, _T18
    MOV BX, _T20
    AND AX, BX
    MOV _T21, AX

    ; [31] (BZ, T21, , 43)
    MOV AX, _T21
    CMP AX, 0
    JE  L43            ; sauter si condition fausse

    ; [32] (TAB, Tabint, i, T22)
    MOV SI, i
    ADD SI, SI
    MOV AX, Tabint[SI]
    MOV _T22, AX

    ; [33] (*, T22, 1.500000, T23)
    MOV AX, _T22
    MOV BX, 1
    IMUL BX            ; DX:AX = AX * BX (signe)
    MOV _T23, AX

    ; [34] (:=, T23, , Tabfloat[i])
    MOV AX, _T23
    MOV SI, i
    ADD SI, SI
    MOV Tabfloat[SI], AX

    ; [35] (BR, , , 46)
    JMP L46

    ; [36] (TAB, Tabint, i, T24)
    MOV SI, i
    ADD SI, SI
    MOV AX, Tabint[SI]
    MOV _T24, AX

    ; [37] (/, T24, 2.000000, T25)
    MOV AX, _T24
    MOV BX, 2
    CWD                ; etendre AX vers DX:AX (signe)
    IDIV BX            ; AX = quotient, DX = reste
    MOV _T25, AX

    ; [38] (:=, T25, , Tabfloat[i])
    MOV AX, _T25
    MOV SI, i
    ADD SI, SI
    MOV Tabfloat[SI], AX

    ; [39] (+, i, 1, T26)
    MOV AX, i
    ADD AX, 1
    MOV _T26, AX

    ; [40] (:=, T26, , i)
    MOV AX, _T26
    MOV i, AX

    ; [41] (BR, , , 29)
    JMP L29

    ; [42] (BR, , , 51)
    JMP L51

L43:
    ; [43] (:=, 0, , somme)
    MOV AX, 0
    MOV somme, AX

    ; [44] (<=, x, Max, T27)
    MOV AX, x
    MOV BX, Max
    CMP AX, BX
    JLE _CMP44_V
    MOV _T27, 0
    JMP _CMP44_S
_CMP44_V:
    MOV _T27, 1
_CMP44_S:

    ; [45] (!=, y, 0, T28)
    MOV AX, y
    CMP AX, 0
    JNE _CMP45_V
    MOV _T28, 0
    JMP _CMP45_S
_CMP45_V:
    MOV _T28, 1
_CMP45_S:

L46:
    ; [46] (<, z, 10, T29)
    MOV AX, z
    CMP AX, 10
    JL _CMP46_V
    MOV _T29, 0
    JMP _CMP46_S
_CMP46_V:
    MOV _T29, 1
_CMP46_S:

    ; [47] (OR, T28, T29, T30)
    MOV AX, _T28
    MOV BX, _T29
    OR  AX, BX
    MOV _T30, AX

    ; [48] (AND, T27, T30, T31)
    MOV AX, _T27
    MOV BX, _T30
    AND AX, BX
    MOV _T31, AX

L49:
    ; [49] (BZ, T31, , 73)
    MOV AX, _T31
    CMP AX, 0
    JE  L73            ; sauter si condition fausse

L50:
    ; [50] (+, x, 1, T32)
    MOV AX, x
    ADD AX, 1
    MOV _T32, AX

L51:
    ; [51] (:=, T32, , x)
    MOV AX, _T32
    MOV x, AX

    ; [52] (==, T32, y, T33)
    MOV AX, _T32
    MOV BX, y
    CMP AX, BX
    JE _CMP52_V
    MOV _T33, 0
    JMP _CMP52_S
_CMP52_V:
    MOV _T33, 1
_CMP52_S:

    ; [53] (NON, T33, , T34)
    MOV AX, _T33
    CMP AX, 0
    JE  _NON53_U
    MOV AX, 0
    JMP _NON53_F
_NON53_U:
    MOV AX, 1
_NON53_F:
    MOV _T34, AX

    ; [54] (BZ, T34, , 72)
    MOV AX, _T34
    CMP AX, 0
    JE  L72            ; sauter si condition fausse

    ; [55] (+, y, 1, T35)
    MOV AX, y
    ADD AX, 1
    MOV _T35, AX

    ; [56] (:=, T35, , y)
    MOV AX, _T35
    MOV y, AX

    ; [57] (TAB, Tabint, 0, T37)
    MOV SI, 0
    MOV AX, Tabint[SI]
    MOV _T37, AX

    ; [58] (TAB, Tabint, 1, T38)
    MOV SI, 2
    MOV AX, Tabint[SI]
    MOV _T38, AX

L59:
    ; [59] (+, T37, T38, T39)
    MOV AX, _T37
    MOV BX, _T38
    ADD AX, BX
    MOV _T39, AX

    ; [60] (-, x, T35, T40)
    MOV AX, x
    MOV BX, _T35
    SUB AX, BX
    MOV _T40, AX

    ; [61] (*, T39, T40, T41)
    MOV AX, _T39
    MOV BX, _T40
    IMUL BX            ; DX:AX = AX * BX (signe)
    MOV _T41, AX

    ; [62] (:=, T41, , Tabint[T36])
    MOV AX, _T41
    MOV SI, _T36
    ADD SI, SI
    MOV Tabint[SI], AX

    ; [63] (BR, , , 59)
    JMP L59

    ; [64] (BR, , , 51)
    JMP L51

    ; [65] (:=, 1, , j)
    MOV AX, 1
    MOV j, AX

    ; [66] (<=, 1, 20, T42)
    MOV AX, 1
    CMP AX, 20
    JLE _CMP66_V
    MOV _T42, 0
    JMP _CMP66_S
_CMP66_V:
    MOV _T42, 1
_CMP66_S:

    ; [67] (BZ, T42, , 88)
    MOV AX, _T42
    CMP AX, 0
    JE  L88            ; sauter si condition fausse

    ; [68] (-, j, 1, T43)
    MOV AX, j
    SUB AX, 1
    MOV _T43, AX

    ; [69] (TAB, Tabfloat, T43, T44)
    MOV SI, _T43
    ADD SI, SI
    MOV AX, Tabfloat[SI]
    MOV _T44, AX

    ; [70] (TAB, Tabfloat, j, T45)
    MOV SI, j
    ADD SI, SI
    MOV AX, Tabfloat[SI]
    MOV _T45, AX

    ; [71] (+, T44, T45, T46)
    MOV AX, _T44
    MOV BX, _T45
    ADD AX, BX
    MOV _T46, AX

L72:
    ; [72] (/, T46, 2.000000, T47)
    MOV AX, _T46
    MOV BX, 2
    CWD                ; etendre AX vers DX:AX (signe)
    IDIV BX            ; AX = quotient, DX = reste
    MOV _T47, AX

L73:
    ; [73] (:=, T47, , Tabfloat[j])
    MOV AX, _T47
    MOV SI, j
    ADD SI, SI
    MOV Tabfloat[SI], AX

L74:
    ; [74] (TAB, Tabfloat, j, T48)
    MOV SI, j
    ADD SI, SI
    MOV AX, Tabfloat[SI]
    MOV _T48, AX

    ; [75] (+, moyenne, T48, T49)
    MOV AX, moyenne
    MOV BX, _T48
    ADD AX, BX
    MOV _T49, AX

    ; [76] (:=, T49, , moyenne)
    MOV AX, _T49
    MOV moyenne, AX

    ; [77] (+, j, 1, T50)
    MOV AX, j
    ADD AX, 1
    MOV _T50, AX

    ; [78] (:=, T50, , j)
    MOV AX, _T50
    MOV j, AX

    ; [79] (BR, , , 74)
    JMP L74

    ; [80] (/, moyenne, 20.000000, T51)
    MOV AX, moyenne
    MOV BX, 20
    CWD                ; etendre AX vers DX:AX (signe)
    IDIV BX            ; AX = quotient, DX = reste
    MOV _T51, AX

    ; [81] (:=, T51, , moyenne)
    MOV AX, _T51
    MOV moyenne, AX

    ; [82] (input, , , x)
    CALL _READ_INT     ; resultat dans AX
    MOV x, AX

    ; [83] (out, "Valeur finale de x: ", , )
    ; OUT chaine (affichage caractere par caractere)
    MOV AH, 02h
    MOV DL, 86       ; 'V'
    INT 21h
    MOV AH, 02h
    MOV DL, 97       ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 108       ; 'l'
    INT 21h
    MOV AH, 02h
    MOV DL, 101       ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 117       ; 'u'
    INT 21h
    MOV AH, 02h
    MOV DL, 114       ; 'r'
    INT 21h
    MOV AH, 02h
    MOV DL, 32       ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 102       ; 'f'
    INT 21h
    MOV AH, 02h
    MOV DL, 105       ; 'i'
    INT 21h
    MOV AH, 02h
    MOV DL, 110       ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 97       ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 108       ; 'l'
    INT 21h
    MOV AH, 02h
    MOV DL, 101       ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 32       ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 100       ; 'd'
    INT 21h
    MOV AH, 02h
    MOV DL, 101       ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 32       ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 120       ; 'x'
    INT 21h
    MOV AH, 02h
    MOV DL, 58       ; ':'
    INT 21h
    MOV AH, 02h
    MOV DL, 32       ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [84] (out, x, , )
    MOV AX, x
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [85] (out, "Somme: ", , )
    ; OUT chaine (affichage caractere par caractere)
    MOV AH, 02h
    MOV DL, 83       ; 'S'
    INT 21h
    MOV AH, 02h
    MOV DL, 111       ; 'o'
    INT 21h
    MOV AH, 02h
    MOV DL, 109       ; 'm'
    INT 21h
    MOV AH, 02h
    MOV DL, 109       ; 'm'
    INT 21h
    MOV AH, 02h
    MOV DL, 101       ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 58       ; ':'
    INT 21h
    MOV AH, 02h
    MOV DL, 32       ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [86] (out, somme, , )
    MOV AX, somme
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [87] (out, "Moyenne: ", , )
    ; OUT chaine (affichage caractere par caractere)
    MOV AH, 02h
    MOV DL, 77       ; 'M'
    INT 21h
    MOV AH, 02h
    MOV DL, 111       ; 'o'
    INT 21h
    MOV AH, 02h
    MOV DL, 121       ; 'y'
    INT 21h
    MOV AH, 02h
    MOV DL, 101       ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 110       ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 110       ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 101       ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 58       ; ':'
    INT 21h
    MOV AH, 02h
    MOV DL, 32       ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

L88:
    ; [88] (out, T51, , )
    MOV AX, _T51
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

L89:
    ; Terminaison du programme (DOS)
    MOV AH, 4Ch
    INT 21h

LECODE ENDS

; ----- Fin du programme -----
END Debut
