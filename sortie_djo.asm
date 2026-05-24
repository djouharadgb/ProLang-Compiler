TITLE prolang.asm
; Code 8086 genere automatiquement :))

PILE SEGMENT STACK
    DW 100 DUP (?)
base_pile EQU $
PILE ENDS

DONNEE SEGMENT

    moyenne              DW 0   ; float fixe (scale=100) (val orig: 0.000000)
    somme                DW 0
    z                    DW ?
    y                    DW ?
    x                    DW ?
    Max                  DW 100
    Tabint               DW 50 DUP(0)  ; tableau
    Pi                   DW 314   ; float fixe (scale=100) (val orig: 3.141590)
    Tabfloat             DW 30 DUP(0)  ; tableau
    k                    DW ?
    j                    DW ?
    i                    DW ?
    b                    DW ?
    a                    DW ?
    _T0                  DW ?          ; temporaire
    _T2                  DW ?          ; temporaire
    _T4                  DW ?          ; temporaire
    _T6                  DW ?          ; temporaire
    _T7                  DW ?          ; temporaire
    _T8                  DW ?          ; temporaire
    _T9                  DW ?          ; temporaire
    _T10                 DW ?          ; temporaire
    _T11                 DW ?          ; temporaire
    _T12                 DW ?          ; temporaire
    _T13                 DW ?          ; temporaire
    _T15                 DW ?          ; temporaire
    _T16                 DW ?          ; temporaire
    _T18                 DW ?          ; temporaire
    _T20                 DW ?          ; temporaire
    _T21                 DW ?          ; temporaire
    _T22                 DW ?          ; temporaire
    _T24                 DW ?          ; temporaire
    _T27                 DW ?          ; temporaire
    _T28                 DW ?          ; temporaire
    _T29                 DW ?          ; temporaire
    _T30                 DW ?          ; temporaire
    _T31                 DW ?          ; temporaire
    _T33                 DW ?          ; temporaire
    _T34                 DW ?          ; temporaire
    _T36                 DW ?          ; temporaire
    _T37                 DW ?          ; temporaire
    _T38                 DW ?          ; temporaire
    _T39                 DW ?          ; temporaire
    _T40                 DW ?          ; temporaire
    _T42                 DW ?          ; temporaire
    _T43                 DW ?          ; temporaire
    _T44                 DW ?          ; temporaire
    _T45                 DW ?          ; temporaire
    _T46                 DW ?          ; temporaire
    _OUT_BUF             DB 12 DUP(?), '$'

DONNEE ENDS

LECODE SEGMENT

;---- _PRINT_INT : affiche AX en decimal ----
_PRINT_INT PROC NEAR
    PUSH AX
    PUSH BX
    PUSH CX
    PUSH DX
    PUSH SI
    MOV SI, OFFSET _OUT_BUF
    CMP AX, 0
    JGE _PRINT_POS
    MOV BYTE PTR [SI], '-'
    INC SI
    NEG AX
_PRINT_POS:
    MOV BX, 10
    MOV CX, 0
_DIV_LOOP:
    MOV DX, 0
    DIV BX          ; AX = AX/10, DX = reste
    ADD DL, '0'
    PUSH DX         ; empiler les chiffres (ordre inverse)
    INC CX
    CMP AX, 0
    JNE _DIV_LOOP
_POP_LOOP:
    POP DX
    MOV [SI], DL
    INC SI
    LOOP _POP_LOOP
    MOV BYTE PTR [SI], '$'
    MOV AH, 09h
    MOV DX, OFFSET _OUT_BUF
    INT 21h
    POP SI
    POP DX
    POP CX
    POP BX
    POP AX
    RET
_PRINT_INT ENDP

;---- _PRINT_FIXED : affiche AX en fixe (scale=100) ----
_PRINT_FIXED PROC NEAR
    PUSH AX
    PUSH BX
    PUSH CX
    PUSH DX
    PUSH SI
    CMP AX, 0
    JGE _PF_POS
    MOV AH, 02h
    MOV DL, '-'
    INT 21h
    NEG AX
_PF_POS:
    MOV BX, 100
    XOR DX, DX
    DIV BX          ; AX = partie entiere, DX = fraction
    PUSH DX
    CALL _PRINT_INT
    POP DX
    MOV AH, 02h
    MOV DL, '.'
    INT 21h
    MOV AX, DX
    MOV BX, 10
    XOR DX, DX
    DIV BX          ; AX = dizaine, DX = unite
    PUSH DX
    ADD AL, '0'
    MOV DL, AL
    MOV AH, 02h
    INT 21h
    POP DX
    ADD DL, '0'
    MOV AH, 02h
    INT 21h
    POP SI
    POP DX
    POP CX
    POP BX
    POP AX
    RET
_PRINT_FIXED ENDP

;---- _READ_INT : lit un entier depuis le clavier ----
_READ_INT PROC NEAR
    PUSH BX
    PUSH CX
    PUSH DX
    MOV BX, 0   ; accumulateur
    MOV CX, 0   ; signe (0=positif, 1=negatif)
_READ_CHAR:
    MOV AH, 01h
    INT 21h     ; AL = caractere lu
    CMP AL, 0Dh ; Entree ?
    JE  _READ_DONE
    CMP AL, '-'
    JNE _READ_DIGIT
    MOV CX, 1
    JMP _READ_CHAR
_READ_DIGIT:
    SUB AL, '0'
    CBW
    XCHG AX, BX
    MOV DX, 10
    MUL DX
    ADD AX, BX
    XCHG AX, BX
    JMP _READ_CHAR
_READ_DONE:
    MOV AX, BX
    CMP CX, 1
    JNE _READ_POS
    NEG AX
_READ_POS:
    POP DX
    POP CX
    POP BX
    RET
_READ_INT ENDP

;---- _READ_FIXED : lit un entier et applique l'echelle ----
_READ_FIXED PROC NEAR
    PUSH BX
    CALL _READ_INT
    MOV BX, 100
    IMUL BX
    POP BX
    RET
_READ_FIXED ENDP

Debut:
    ASSUME CS:LECODE, DS:DONNEE, SS:PILE

    MOV AX, DONNEE
    MOV DS, AX
    MOV AX, PILE
    MOV SS, AX
    MOV SP, base_pile

L0:
    ; [0] (:=, 0, , somme)
    MOV AX, 0
    MOV somme, AX

    ; [1] (:=, 0.000000, , moyenne)
    MOV AX, 0
    MOV moyenne, AX

    ; [2] (:=, 10, , x)
    MOV AX, 10
    MOV x, AX

    ; [3] (:=, 5, , y)
    MOV AX, 5
    MOV y, AX

    ; [4] (:=, 2, , z)
    MOV AX, 2
    MOV z, AX

    ; [5] (:=, 2.500000, , a)
    MOV AX, 250
    MOV a, AX

    ; [6] (+, a, 3.141590, T0)
    MOV AX, a
    MOV BX, 314
    ADD AX, BX
    MOV _T0, AX

    ; [7] (+, T0, T0, b)
    MOV AX, _T0
    MOV BX, _T0
    ADD AX, BX
    MOV b, AX

    ; [8] (*, y, z, T2)
    MOV AX, y
    MOV BX, z
    IMUL BX
    MOV _T2, AX

    ; [9] (+, x, T2, Tabint[0])
    MOV AX, x
    MOV BX, _T2
    ADD AX, BX
    MOV SI, 0
    MOV Tabint[SI], AX

    ; [10] (+, b, 3.500000, T4)
    MOV AX, b
    MOV BX, 350
    ADD AX, BX
    MOV _T4, AX

    ; [11] (/, T4, 2.000000, Tabfloat[1])
    MOV AX, _T4
    MOV BX, 200
    MOV CX, 100
    IMUL CX
    IDIV BX
    MOV SI, 2
    MOV Tabfloat[SI], AX

    ; [12] (>, x, y, T6)
    MOV AX, x
    MOV BX, y
    CMP AX, BX
    JG _CMP12_V
    MOV _T6, 0
    JMP _CMP12_S
_CMP12_V:
    MOV _T6, 1
_CMP12_S:

    ; [13] (+, x, y, T7)
    MOV AX, x
    MOV BX, y
    ADD AX, BX
    MOV _T7, AX

    ; [14] (<, z, T7, T8)
    MOV AX, z
    MOV BX, _T7
    CMP AX, BX
    JL _CMP14_V
    MOV _T8, 0
    JMP _CMP14_S
_CMP14_V:
    MOV _T8, 1
_CMP14_S:

    ; [15] (AND, T6, T8, T9)
    MOV AX, _T6
    MOV BX, _T8
    AND AX, BX
    MOV _T9, AX

    ; [16] (==, y, 0, T10)
    MOV AX, y
    CMP AX, 0
    JE _CMP16_V
    MOV _T10, 0
    JMP _CMP16_S
_CMP16_V:
    MOV _T10, 1
_CMP16_S:

    ; [17] (NON, T10, , T11)
    MOV AX, _T10
    CMP AX, 0
    JE  _NON17_U
    MOV AX, 0
    JMP _NON17_F
_NON17_U:
    MOV AX, 1
_NON17_F:
    MOV _T11, AX

    ; [18] (OR, T9, T11, T12)
    MOV AX, _T9
    MOV BX, _T11
    OR AX, BX
    MOV _T12, AX

    ; [19] (BZ, T12, , 39)
    MOV AX, _T12
    CMP AX, 0
    JE  L39

    ; [20] (+, x, y, T13)
    MOV AX, x
    MOV BX, y
    ADD AX, BX
    MOV _T13, AX

    ; [21] (+, T13, z, somme)
    MOV AX, _T13
    MOV BX, z
    ADD AX, BX
    MOV somme, AX

    ; [22] (:=, 0, , i)
    MOV AX, 0
    MOV i, AX

L23:
    ; [23] (<=, i, 10, T15)
    MOV AX, i
    CMP AX, 10
    JLE _CMP23_V
    MOV _T15, 0
    JMP _CMP23_S
_CMP23_V:
    MOV _T15, 1
_CMP23_S:

    ; [24] (BZ, T15, , 38)
    MOV AX, _T15
    CMP AX, 0
    JE  L38

    ; [25] (TAB, Tabint, i, T16)
    MOV SI, i
    ADD SI, SI
    MOV AX, Tabint[SI]
    MOV _T16, AX

    ; [26] (+, T16, i, Tabint[i])
    MOV AX, _T16
    MOV BX, i
    ADD AX, BX
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

    ; [28] (>, T16, 10, T20)
    MOV AX, _T16
    CMP AX, 10
    JG _CMP28_V
    MOV _T20, 0
    JMP _CMP28_S
_CMP28_V:
    MOV _T20, 1
_CMP28_S:

    ; [29] (AND, T18, T20, T21)
    MOV AX, _T18
    MOV BX, _T20
    AND AX, BX
    MOV _T21, AX

    ; [30] (BZ, T21, , 34)
    MOV AX, _T21
    CMP AX, 0
    JE  L34

    ; [31] (TAB, Tabint, i, T22)
    MOV SI, i
    ADD SI, SI
    MOV AX, Tabint[SI]
    MOV BX, 100
    IMUL BX
    MOV _T22, AX

    ; [32] (*, T22, 1.500000, Tabfloat[i])
    MOV AX, _T22
    MOV BX, 150
    IMUL BX
    MOV BX, 100
    IDIV BX
    MOV SI, i
    ADD SI, SI
    MOV Tabfloat[SI], AX

    ; [33] (BR, , , 36)
    JMP L36

L34:
    ; [34] (TAB, Tabint, i, T24)
    MOV SI, i
    ADD SI, SI
    MOV AX, Tabint[SI]
    MOV BX, 100
    IMUL BX
    MOV _T24, AX

    ; [35] (/, T24, 2.000000, Tabfloat[i])
    MOV AX, _T24
    MOV BX, 200
    MOV CX, 100
    IMUL CX
    IDIV BX
    MOV SI, i
    ADD SI, SI
    MOV Tabfloat[SI], AX

L36:
    ; [36] (+, i, 1, i)
    MOV AX, i
    ADD AX, 1
    MOV i, AX

    ; [37] (BR, , , 23)
    JMP L23

L38:
    ; [38] (BR, , , 40)
    JMP L40

L39:
    ; [39] (:=, 0, , somme)
    MOV AX, 0
    MOV somme, AX

L40:
    ; [40] (<=, x, 100, T27)
    MOV AX, x
    CMP AX, 100
    JLE _CMP40_V
    MOV _T27, 0
    JMP _CMP40_S
_CMP40_V:
    MOV _T27, 1
_CMP40_S:

    ; [41] (!=, y, 0, T28)
    MOV AX, y
    CMP AX, 0
    JNE _CMP41_V
    MOV _T28, 0
    JMP _CMP41_S
_CMP41_V:
    MOV _T28, 1
_CMP41_S:

    ; [42] (<, z, 10, T29)
    MOV AX, z
    CMP AX, 10
    JL _CMP42_V
    MOV _T29, 0
    JMP _CMP42_S
_CMP42_V:
    MOV _T29, 1
_CMP42_S:

    ; [43] (OR, T28, T29, T30)
    MOV AX, _T28
    MOV BX, _T29
    OR AX, BX
    MOV _T30, AX

    ; [44] (AND, T27, T30, T31)
    MOV AX, _T27
    MOV BX, _T30
    AND AX, BX
    MOV _T31, AX

    ; [45] (BZ, T31, , 59)
    MOV AX, _T31
    CMP AX, 0
    JE  L59

    ; [46] (+, x, 1, x)
    MOV AX, x
    ADD AX, 1
    MOV x, AX

L47:
    ; [47] (==, x, y, T33)
    MOV AX, x
    MOV BX, y
    CMP AX, BX
    JE _CMP47_V
    MOV _T33, 0
    JMP _CMP47_S
_CMP47_V:
    MOV _T33, 1
_CMP47_S:

    ; [48] (NON, T33, , T34)
    MOV AX, _T33
    CMP AX, 0
    JE  _NON48_U
    MOV AX, 0
    JMP _NON48_F
_NON48_U:
    MOV AX, 1
_NON48_F:
    MOV _T34, AX

    ; [49] (BZ, T34, , 58)
    MOV AX, _T34
    CMP AX, 0
    JE  L58

    ; [50] (+, y, 1, y)
    MOV AX, y
    ADD AX, 1
    MOV y, AX

    ; [51] (+, x, 1, T36)
    MOV AX, x
    ADD AX, 1
    MOV _T36, AX

    ; [52] (TAB, Tabint, 0, T37)
    MOV SI, 0
    MOV AX, Tabint[SI]
    MOV _T37, AX

    ; [53] (TAB, Tabint, 1, T38)
    MOV SI, 2
    MOV AX, Tabint[SI]
    MOV _T38, AX

    ; [54] (+, T37, T38, T39)
    MOV AX, _T37
    MOV BX, _T38
    ADD AX, BX
    MOV _T39, AX

    ; [55] (-, x, y, T40)
    MOV AX, x
    MOV BX, y
    SUB AX, BX
    MOV _T40, AX

    ; [56] (*, T39, T40, Tabint[T36])
    MOV AX, _T39
    MOV BX, _T40
    IMUL BX
    MOV SI, _T36
    ADD SI, SI
    MOV Tabint[SI], AX

    ; [57] (BR, , , 47)
    JMP L47

L58:
    ; [58] (BR, , , 40)
    JMP L40

L59:
    ; [59] (:=, 1, , j)
    MOV AX, 1
    MOV j, AX

L60:
    ; [60] (<=, j, 20, T42)
    MOV AX, j
    CMP AX, 20
    JLE _CMP60_V
    MOV _T42, 0
    JMP _CMP60_S
_CMP60_V:
    MOV _T42, 1
_CMP60_S:

    ; [61] (BZ, T42, , 70)
    MOV AX, _T42
    CMP AX, 0
    JE  L70

    ; [62] (-, j, 1, T43)
    MOV AX, j
    SUB AX, 1
    MOV _T43, AX

    ; [63] (TAB, Tabfloat, T43, T44)
    MOV SI, _T43
    ADD SI, SI
    MOV AX, Tabfloat[SI]
    MOV _T44, AX

    ; [64] (TAB, Tabfloat, j, T45)
    MOV SI, j
    ADD SI, SI
    MOV AX, Tabfloat[SI]
    MOV _T45, AX

    ; [65] (+, T44, T45, T46)
    MOV AX, _T44
    MOV BX, _T45
    ADD AX, BX
    MOV _T46, AX

    ; [66] (/, T46, 2.000000, Tabfloat[j])
    MOV AX, _T46
    MOV BX, 200
    MOV CX, 100
    IMUL CX
    IDIV BX
    MOV SI, j
    ADD SI, SI
    MOV Tabfloat[SI], AX

    ; [67] (+, moyenne, T45, moyenne)
    MOV AX, moyenne
    MOV BX, _T45
    ADD AX, BX
    MOV moyenne, AX

    ; [68] (+, j, 1, j)
    MOV AX, j
    ADD AX, 1
    MOV j, AX

    ; [69] (BR, , , 60)
    JMP L60

L70:
    ; [70] (/, moyenne, 20.000000, moyenne)
    MOV AX, moyenne
    MOV BX, 2000
    MOV CX, 100
    IMUL CX
    IDIV BX
    MOV moyenne, AX

    ; [71] (input, , , x)
    CALL _READ_INT   ; resultat dans AX
    MOV x, AX

    ; [72] (out, "Valeur finale de x: ", , )
    MOV AH, 02h
    MOV DL, 86   ; 'V'
    INT 21h
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 108   ; 'l'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 117   ; 'u'
    INT 21h
    MOV AH, 02h
    MOV DL, 114   ; 'r'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 102   ; 'f'
    INT 21h
    MOV AH, 02h
    MOV DL, 105   ; 'i'
    INT 21h
    MOV AH, 02h
    MOV DL, 110   ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 108   ; 'l'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 100   ; 'd'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 120   ; 'x'
    INT 21h
    MOV AH, 02h
    MOV DL, 58   ; ':'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h

    ; [73] (out, x, , )
    MOV AX, x
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [74] (out, "Somme: ", , )
    MOV AH, 02h
    MOV DL, 83   ; 'S'
    INT 21h
    MOV AH, 02h
    MOV DL, 111   ; 'o'
    INT 21h
    MOV AH, 02h
    MOV DL, 109   ; 'm'
    INT 21h
    MOV AH, 02h
    MOV DL, 109   ; 'm'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 58   ; ':'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h

    ; [75] (out, somme, , )
    MOV AX, somme
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [76] (out, "Moyenne: ", , )
    MOV AH, 02h
    MOV DL, 77   ; 'M'
    INT 21h
    MOV AH, 02h
    MOV DL, 111   ; 'o'
    INT 21h
    MOV AH, 02h
    MOV DL, 121   ; 'y'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 110   ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 110   ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 58   ; ':'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h

    ; [77] (out, moyenne, , )
    MOV AX, moyenne
    CALL _PRINT_FIXED
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

L78:
    MOV AH, 4Ch
    INT 21h

LECODE ENDS

END Debut
