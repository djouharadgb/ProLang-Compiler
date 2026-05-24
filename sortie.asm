TITLE prolang.asm
; Code 8086 genere automatiquement

PILE SEGMENT STACK
    DW 100 DUP (?)
base_pile EQU $
PILE ENDS

DONNEE SEGMENT

    res                  DW ?
    compteur             DW ?
    x                    DW ?
    k                    DW ?
    j                    DW ?
    i                    DW ?
    _T0                  DW ?          ; temporaire
    _T1                  DW ?          ; temporaire
    _T2                  DW ?          ; temporaire
    _T5                  DW ?          ; temporaire
    _T6                  DW ?          ; temporaire
    _T7                  DW ?          ; temporaire
    _T10                 DW ?          ; temporaire
    _T11                 DW ?          ; temporaire
    _T12                 DW ?          ; temporaire
    _T15                 DW ?          ; temporaire
    _T16                 DW ?          ; temporaire
    _T17                 DW ?          ; temporaire
    _T20                 DW ?          ; temporaire
    _T21                 DW ?          ; temporaire
    _T22                 DW ?          ; temporaire
    _T23                 DW ?          ; temporaire
    _T27                 DW ?          ; temporaire
    _T28                 DW ?          ; temporaire
    _T29                 DW ?          ; temporaire
    _T30                 DW ?          ; temporaire
    _OUT_BUF             DB 10 DUP(?), '$'

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

Debut:
    ASSUME CS:LECODE, DS:DONNEE, SS:PILE

    MOV AX, DONNEE
    MOV DS, AX
    MOV AX, PILE
    MOV SS, AX
    MOV SP, base_pile

L0:
    ; [0] (:=, 0, , res)
    MOV AX, 0
    MOV res, AX

    ; [1] (:=, 0, , i)
    MOV AX, 0
    MOV i, AX

L2:
    ; [2] (<=, i, 2, T0)
    MOV AX, i
    CMP AX, 2
    JLE _CMP2_V
    MOV _T0, 0
    JMP _CMP2_S
_CMP2_V:
    MOV _T0, 1
_CMP2_S:

    ; [3] (BZ, T0, , 11)
    MOV AX, _T0
    CMP AX, 0
    JE  L11

    ; [4] (:=, 0, , j)
    MOV AX, 0
    MOV j, AX

L5:
    ; [5] (<=, j, 3, T1)
    MOV AX, j
    CMP AX, 3
    JLE _CMP5_V
    MOV _T1, 0
    JMP _CMP5_S
_CMP5_V:
    MOV _T1, 1
_CMP5_S:

    ; [6] (BZ, T1, , 10)
    MOV AX, _T1
    CMP AX, 0
    JE  L10

    ; [7] (+, res, 1, T2)
    MOV AX, res
    ADD AX, 1
    MOV _T2, AX

    ; [8] (:=, T2, , res)
    MOV AX, _T2
    MOV res, AX

    ; [9] (BR, , , 5)
    JMP L5

L10:
    ; [10] (BR, , , 2)
    JMP L2

L11:
    ; [11] (out, "CAS1 - for dans for, res attendu 12: ", , )
    MOV AH, 02h
    MOV DL, 67   ; 'C'
    INT 21h
    MOV AH, 02h
    MOV DL, 65   ; 'A'
    INT 21h
    MOV AH, 02h
    MOV DL, 83   ; 'S'
    INT 21h
    MOV AH, 02h
    MOV DL, 49   ; '1'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 45   ; '-'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 102   ; 'f'
    INT 21h
    MOV AH, 02h
    MOV DL, 111   ; 'o'
    INT 21h
    MOV AH, 02h
    MOV DL, 114   ; 'r'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 100   ; 'd'
    INT 21h
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 110   ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 115   ; 's'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 102   ; 'f'
    INT 21h
    MOV AH, 02h
    MOV DL, 111   ; 'o'
    INT 21h
    MOV AH, 02h
    MOV DL, 114   ; 'r'
    INT 21h
    MOV AH, 02h
    MOV DL, 44   ; ','
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 114   ; 'r'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 115   ; 's'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 116   ; 't'
    INT 21h
    MOV AH, 02h
    MOV DL, 116   ; 't'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 110   ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 100   ; 'd'
    INT 21h
    MOV AH, 02h
    MOV DL, 117   ; 'u'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 49   ; '1'
    INT 21h
    MOV AH, 02h
    MOV DL, 50   ; '2'
    INT 21h
    MOV AH, 02h
    MOV DL, 58   ; ':'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [12] (out, res, , )
    MOV AX, res
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [13] (:=, 0, , res)
    MOV AX, 0
    MOV res, AX

    ; [14] (:=, 0, , i)
    MOV AX, 0
    MOV i, AX

L15:
    ; [15] (<, i, 3, T5)
    MOV AX, i
    CMP AX, 3
    JL _CMP15_V
    MOV _T5, 0
    JMP _CMP15_S
_CMP15_V:
    MOV _T5, 1
_CMP15_S:

    ; [16] (BZ, T5, , 24)
    MOV AX, _T5
    CMP AX, 0
    JE  L24

    ; [17] (:=, 0, , j)
    MOV AX, 0
    MOV j, AX

L18:
    ; [18] (<=, j, 4, T6)
    MOV AX, j
    CMP AX, 4
    JLE _CMP18_V
    MOV _T6, 0
    JMP _CMP18_S
_CMP18_V:
    MOV _T6, 1
_CMP18_S:

    ; [19] (BZ, T6, , 23)
    MOV AX, _T6
    CMP AX, 0
    JE  L23

    ; [20] (+, res, 1, T7)
    MOV AX, res
    ADD AX, 1
    MOV _T7, AX

    ; [21] (:=, T7, , res)
    MOV AX, _T7
    MOV res, AX

    ; [22] (BR, , , 18)
    JMP L18

L23:
    ; [23] (BR, , , 15)
    JMP L15

L24:
    ; [24] (out, "CAS2 - for dans while, res attendu 15: ", , )
    MOV AH, 02h
    MOV DL, 67   ; 'C'
    INT 21h
    MOV AH, 02h
    MOV DL, 65   ; 'A'
    INT 21h
    MOV AH, 02h
    MOV DL, 83   ; 'S'
    INT 21h
    MOV AH, 02h
    MOV DL, 50   ; '2'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 45   ; '-'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 102   ; 'f'
    INT 21h
    MOV AH, 02h
    MOV DL, 111   ; 'o'
    INT 21h
    MOV AH, 02h
    MOV DL, 114   ; 'r'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 100   ; 'd'
    INT 21h
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 110   ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 115   ; 's'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 119   ; 'w'
    INT 21h
    MOV AH, 02h
    MOV DL, 104   ; 'h'
    INT 21h
    MOV AH, 02h
    MOV DL, 105   ; 'i'
    INT 21h
    MOV AH, 02h
    MOV DL, 108   ; 'l'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 44   ; ','
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 114   ; 'r'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 115   ; 's'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 116   ; 't'
    INT 21h
    MOV AH, 02h
    MOV DL, 116   ; 't'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 110   ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 100   ; 'd'
    INT 21h
    MOV AH, 02h
    MOV DL, 117   ; 'u'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 49   ; '1'
    INT 21h
    MOV AH, 02h
    MOV DL, 53   ; '5'
    INT 21h
    MOV AH, 02h
    MOV DL, 58   ; ':'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [25] (out, res, , )
    MOV AX, res
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [26] (:=, 0, , res)
    MOV AX, 0
    MOV res, AX

    ; [27] (:=, 0, , i)
    MOV AX, 0
    MOV i, AX

L28:
    ; [28] (<=, i, 2, T10)
    MOV AX, i
    CMP AX, 2
    JLE _CMP28_V
    MOV _T10, 0
    JMP _CMP28_S
_CMP28_V:
    MOV _T10, 1
_CMP28_S:

    ; [29] (BZ, T10, , 37)
    MOV AX, _T10
    CMP AX, 0
    JE  L37

    ; [30] (:=, 0, , j)
    MOV AX, 0
    MOV j, AX

L31:
    ; [31] (<, j, 4, T11)
    MOV AX, j
    CMP AX, 4
    JL _CMP31_V
    MOV _T11, 0
    JMP _CMP31_S
_CMP31_V:
    MOV _T11, 1
_CMP31_S:

    ; [32] (BZ, T11, , 36)
    MOV AX, _T11
    CMP AX, 0
    JE  L36

    ; [33] (+, res, 1, T12)
    MOV AX, res
    ADD AX, 1
    MOV _T12, AX

    ; [34] (:=, T12, , res)
    MOV AX, _T12
    MOV res, AX

    ; [35] (BR, , , 31)
    JMP L31

L36:
    ; [36] (BR, , , 28)
    JMP L28

L37:
    ; [37] (out, "CAS3 - while dans for, res attendu 12: ", , )
    MOV AH, 02h
    MOV DL, 67   ; 'C'
    INT 21h
    MOV AH, 02h
    MOV DL, 65   ; 'A'
    INT 21h
    MOV AH, 02h
    MOV DL, 83   ; 'S'
    INT 21h
    MOV AH, 02h
    MOV DL, 51   ; '3'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 45   ; '-'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 119   ; 'w'
    INT 21h
    MOV AH, 02h
    MOV DL, 104   ; 'h'
    INT 21h
    MOV AH, 02h
    MOV DL, 105   ; 'i'
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
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 110   ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 115   ; 's'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 102   ; 'f'
    INT 21h
    MOV AH, 02h
    MOV DL, 111   ; 'o'
    INT 21h
    MOV AH, 02h
    MOV DL, 114   ; 'r'
    INT 21h
    MOV AH, 02h
    MOV DL, 44   ; ','
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 114   ; 'r'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 115   ; 's'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 116   ; 't'
    INT 21h
    MOV AH, 02h
    MOV DL, 116   ; 't'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 110   ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 100   ; 'd'
    INT 21h
    MOV AH, 02h
    MOV DL, 117   ; 'u'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 49   ; '1'
    INT 21h
    MOV AH, 02h
    MOV DL, 50   ; '2'
    INT 21h
    MOV AH, 02h
    MOV DL, 58   ; ':'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [38] (out, res, , )
    MOV AX, res
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [39] (:=, 0, , res)
    MOV AX, 0
    MOV res, AX

    ; [40] (:=, 0, , i)
    MOV AX, 0
    MOV i, AX

L41:
    ; [41] (<, i, 3, T15)
    MOV AX, i
    CMP AX, 3
    JL _CMP41_V
    MOV _T15, 0
    JMP _CMP41_S
_CMP41_V:
    MOV _T15, 1
_CMP41_S:

    ; [42] (BZ, T15, , 50)
    MOV AX, _T15
    CMP AX, 0
    JE  L50

    ; [43] (:=, 0, , j)
    MOV AX, 0
    MOV j, AX

L44:
    ; [44] (<, j, 3, T16)
    MOV AX, j
    CMP AX, 3
    JL _CMP44_V
    MOV _T16, 0
    JMP _CMP44_S
_CMP44_V:
    MOV _T16, 1
_CMP44_S:

    ; [45] (BZ, T16, , 49)
    MOV AX, _T16
    CMP AX, 0
    JE  L49

    ; [46] (+, res, 1, T17)
    MOV AX, res
    ADD AX, 1
    MOV _T17, AX

    ; [47] (:=, T17, , res)
    MOV AX, _T17
    MOV res, AX

    ; [48] (BR, , , 44)
    JMP L44

L49:
    ; [49] (BR, , , 41)
    JMP L41

L50:
    ; [50] (out, "CAS4 - while dans while, res attendu 9: ", , )
    MOV AH, 02h
    MOV DL, 67   ; 'C'
    INT 21h
    MOV AH, 02h
    MOV DL, 65   ; 'A'
    INT 21h
    MOV AH, 02h
    MOV DL, 83   ; 'S'
    INT 21h
    MOV AH, 02h
    MOV DL, 52   ; '4'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 45   ; '-'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 119   ; 'w'
    INT 21h
    MOV AH, 02h
    MOV DL, 104   ; 'h'
    INT 21h
    MOV AH, 02h
    MOV DL, 105   ; 'i'
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
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 110   ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 115   ; 's'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 119   ; 'w'
    INT 21h
    MOV AH, 02h
    MOV DL, 104   ; 'h'
    INT 21h
    MOV AH, 02h
    MOV DL, 105   ; 'i'
    INT 21h
    MOV AH, 02h
    MOV DL, 108   ; 'l'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 44   ; ','
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 114   ; 'r'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 115   ; 's'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 116   ; 't'
    INT 21h
    MOV AH, 02h
    MOV DL, 116   ; 't'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 110   ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 100   ; 'd'
    INT 21h
    MOV AH, 02h
    MOV DL, 117   ; 'u'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 57   ; '9'
    INT 21h
    MOV AH, 02h
    MOV DL, 58   ; ':'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [51] (out, res, , )
    MOV AX, res
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [52] (:=, 0, , res)
    MOV AX, 0
    MOV res, AX

    ; [53] (:=, 0, , i)
    MOV AX, 0
    MOV i, AX

L54:
    ; [54] (<=, i, 2, T20)
    MOV AX, i
    CMP AX, 2
    JLE _CMP54_V
    MOV _T20, 0
    JMP _CMP54_S
_CMP54_V:
    MOV _T20, 1
_CMP54_S:

    ; [55] (BZ, T20, , 67)
    MOV AX, _T20
    CMP AX, 0
    JE  L67

    ; [56] (:=, 0, , j)
    MOV AX, 0
    MOV j, AX

L57:
    ; [57] (<=, j, 2, T21)
    MOV AX, j
    CMP AX, 2
    JLE _CMP57_V
    MOV _T21, 0
    JMP _CMP57_S
_CMP57_V:
    MOV _T21, 1
_CMP57_S:

    ; [58] (BZ, T21, , 66)
    MOV AX, _T21
    CMP AX, 0
    JE  L66

    ; [59] (:=, 0, , k)
    MOV AX, 0
    MOV k, AX

L60:
    ; [60] (<=, k, 2, T22)
    MOV AX, k
    CMP AX, 2
    JLE _CMP60_V
    MOV _T22, 0
    JMP _CMP60_S
_CMP60_V:
    MOV _T22, 1
_CMP60_S:

    ; [61] (BZ, T22, , 65)
    MOV AX, _T22
    CMP AX, 0
    JE  L65

    ; [62] (+, res, 1, T23)
    MOV AX, res
    ADD AX, 1
    MOV _T23, AX

    ; [63] (:=, T23, , res)
    MOV AX, _T23
    MOV res, AX

    ; [64] (BR, , , 60)
    JMP L60

L65:
    ; [65] (BR, , , 57)
    JMP L57

L66:
    ; [66] (BR, , , 54)
    JMP L54

L67:
    ; [67] (out, "CAS5 - for dans for dans for, res attendu 27: ", , )
    MOV AH, 02h
    MOV DL, 67   ; 'C'
    INT 21h
    MOV AH, 02h
    MOV DL, 65   ; 'A'
    INT 21h
    MOV AH, 02h
    MOV DL, 83   ; 'S'
    INT 21h
    MOV AH, 02h
    MOV DL, 53   ; '5'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 45   ; '-'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 102   ; 'f'
    INT 21h
    MOV AH, 02h
    MOV DL, 111   ; 'o'
    INT 21h
    MOV AH, 02h
    MOV DL, 114   ; 'r'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 100   ; 'd'
    INT 21h
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 110   ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 115   ; 's'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 102   ; 'f'
    INT 21h
    MOV AH, 02h
    MOV DL, 111   ; 'o'
    INT 21h
    MOV AH, 02h
    MOV DL, 114   ; 'r'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 100   ; 'd'
    INT 21h
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 110   ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 115   ; 's'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 102   ; 'f'
    INT 21h
    MOV AH, 02h
    MOV DL, 111   ; 'o'
    INT 21h
    MOV AH, 02h
    MOV DL, 114   ; 'r'
    INT 21h
    MOV AH, 02h
    MOV DL, 44   ; ','
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 114   ; 'r'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 115   ; 's'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 116   ; 't'
    INT 21h
    MOV AH, 02h
    MOV DL, 116   ; 't'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 110   ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 100   ; 'd'
    INT 21h
    MOV AH, 02h
    MOV DL, 117   ; 'u'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 50   ; '2'
    INT 21h
    MOV AH, 02h
    MOV DL, 55   ; '7'
    INT 21h
    MOV AH, 02h
    MOV DL, 58   ; ':'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [68] (out, res, , )
    MOV AX, res
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [69] (:=, 0, , res)
    MOV AX, 0
    MOV res, AX

    ; [70] (:=, 0, , compteur)
    MOV AX, 0
    MOV compteur, AX

L71:
    ; [71] (<, compteur, 2, T27)
    MOV AX, compteur
    CMP AX, 2
    JL _CMP71_V
    MOV _T27, 0
    JMP _CMP71_S
_CMP71_V:
    MOV _T27, 1
_CMP71_S:

    ; [72] (BZ, T27, , 84)
    MOV AX, _T27
    CMP AX, 0
    JE  L84

    ; [73] (:=, 0, , i)
    MOV AX, 0
    MOV i, AX

L74:
    ; [74] (<=, i, 2, T28)
    MOV AX, i
    CMP AX, 2
    JLE _CMP74_V
    MOV _T28, 0
    JMP _CMP74_S
_CMP74_V:
    MOV _T28, 1
_CMP74_S:

    ; [75] (BZ, T28, , 83)
    MOV AX, _T28
    CMP AX, 0
    JE  L83

    ; [76] (:=, 0, , j)
    MOV AX, 0
    MOV j, AX

L77:
    ; [77] (<, j, 3, T29)
    MOV AX, j
    CMP AX, 3
    JL _CMP77_V
    MOV _T29, 0
    JMP _CMP77_S
_CMP77_V:
    MOV _T29, 1
_CMP77_S:

    ; [78] (BZ, T29, , 82)
    MOV AX, _T29
    CMP AX, 0
    JE  L82

    ; [79] (+, res, 1, T30)
    MOV AX, res
    ADD AX, 1
    MOV _T30, AX

    ; [80] (:=, T30, , res)
    MOV AX, _T30
    MOV res, AX

    ; [81] (BR, , , 77)
    JMP L77

L82:
    ; [82] (BR, , , 74)
    JMP L74

L83:
    ; [83] (BR, , , 71)
    JMP L71

L84:
    ; [84] (out, "CAS6 - while dans for dans while, res attendu 18: ", , )
    MOV AH, 02h
    MOV DL, 67   ; 'C'
    INT 21h
    MOV AH, 02h
    MOV DL, 65   ; 'A'
    INT 21h
    MOV AH, 02h
    MOV DL, 83   ; 'S'
    INT 21h
    MOV AH, 02h
    MOV DL, 54   ; '6'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 45   ; '-'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 119   ; 'w'
    INT 21h
    MOV AH, 02h
    MOV DL, 104   ; 'h'
    INT 21h
    MOV AH, 02h
    MOV DL, 105   ; 'i'
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
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 110   ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 115   ; 's'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 102   ; 'f'
    INT 21h
    MOV AH, 02h
    MOV DL, 111   ; 'o'
    INT 21h
    MOV AH, 02h
    MOV DL, 114   ; 'r'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 100   ; 'd'
    INT 21h
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 110   ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 115   ; 's'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 119   ; 'w'
    INT 21h
    MOV AH, 02h
    MOV DL, 104   ; 'h'
    INT 21h
    MOV AH, 02h
    MOV DL, 105   ; 'i'
    INT 21h
    MOV AH, 02h
    MOV DL, 108   ; 'l'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 44   ; ','
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 114   ; 'r'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 115   ; 's'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 116   ; 't'
    INT 21h
    MOV AH, 02h
    MOV DL, 116   ; 't'
    INT 21h
    MOV AH, 02h
    MOV DL, 101   ; 'e'
    INT 21h
    MOV AH, 02h
    MOV DL, 110   ; 'n'
    INT 21h
    MOV AH, 02h
    MOV DL, 100   ; 'd'
    INT 21h
    MOV AH, 02h
    MOV DL, 117   ; 'u'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 49   ; '1'
    INT 21h
    MOV AH, 02h
    MOV DL, 56   ; '8'
    INT 21h
    MOV AH, 02h
    MOV DL, 58   ; ':'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [85] (out, res, , )
    MOV AX, res
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

L86:
    MOV AH, 4Ch
    INT 21h

LECODE ENDS

END Debut
