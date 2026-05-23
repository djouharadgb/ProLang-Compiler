TITLE prolang.asm
; Code 8086 genere automatiquement

PILE SEGMENT STACK
    DW 100 DUP (?)
base_pile EQU $
PILE ENDS

DONNEE SEGMENT

    res                  DW ?
    y                    DW ?
    x                    DW ?
    Max                  DW 100
    b                    DW ?
    a                    DW ?
    _T0                  DW ?          ; temporaire
    _T1                  DW ?          ; temporaire
    _T2                  DW ?          ; temporaire
    _T3                  DW ?          ; temporaire
    _T5                  DW ?          ; temporaire
    _T6                  DW ?          ; temporaire
    _T7                  DW ?          ; temporaire
    _T8                  DW ?          ; temporaire
    _T11                 DW ?          ; temporaire
    _T12                 DW ?          ; temporaire
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
    ; [0] (:=, 100, , Max)
    MOV AX, 100
    MOV Max, AX

    ; [1] (:=, 10, , a)
    MOV AX, 10
    MOV a, AX

    ; [2] (:=, 3, , b)
    MOV AX, 3
    MOV b, AX

    ; [3] (+, a, b, T0)
    MOV AX, a
    MOV BX, b
    ADD AX, BX
    MOV _T0, AX

    ; [4] (:=, T0, , res)
    MOV AX, _T0
    MOV res, AX

    ; [5] (out, "a + b = ", , )
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 43   ; '+'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 98   ; 'b'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 61   ; '='
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [6] (out, res, , )
    MOV AX, res
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [7] (-, a, b, T1)
    MOV AX, a
    MOV BX, b
    SUB AX, BX
    MOV _T1, AX

    ; [8] (:=, T1, , res)
    MOV AX, _T1
    MOV res, AX

    ; [9] (out, "a - b = ", , )
    MOV AH, 02h
    MOV DL, 97   ; 'a'
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
    MOV DL, 98   ; 'b'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 61   ; '='
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [10] (out, res, , )
    MOV AX, res
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [11] (*, a, b, T2)
    MOV AX, a
    MOV BX, b
    IMUL BX
    MOV _T2, AX

    ; [12] (:=, T2, , res)
    MOV AX, _T2
    MOV res, AX

    ; [13] (out, "a * b = ", , )
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 42   ; '*'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 98   ; 'b'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 61   ; '='
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [14] (out, res, , )
    MOV AX, res
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [15] (/, a, b, T3)
    MOV AX, a
    MOV BX, b
    CWD            ; etendre AX -> DX:AX
    IDIV BX
    MOV _T3, AX

    ; [16] (:=, T3, , res)
    MOV AX, _T3
    MOV res, AX

    ; [17] (out, "a / b = ", , )
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 47   ; '/'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 98   ; 'b'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 61   ; '='
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [18] (out, res, , )
    MOV AX, res
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [19] (+, T0, T0, T5)
    MOV AX, _T0
    MOV BX, _T0
    ADD AX, BX
    MOV _T5, AX

    ; [20] (:=, T5, , res)
    MOV AX, _T5
    MOV res, AX

    ; [21] (out, "(a+b)*2 = ", , )
    MOV AH, 02h
    MOV DL, 40   ; '('
    INT 21h
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 43   ; '+'
    INT 21h
    MOV AH, 02h
    MOV DL, 98   ; 'b'
    INT 21h
    MOV AH, 02h
    MOV DL, 41   ; ')'
    INT 21h
    MOV AH, 02h
    MOV DL, 42   ; '*'
    INT 21h
    MOV AH, 02h
    MOV DL, 50   ; '2'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 61   ; '='
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [22] (out, res, , )
    MOV AX, res
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [23] (*, a, a, T6)
    MOV AX, a
    MOV BX, a
    IMUL BX
    MOV _T6, AX

    ; [24] (*, b, b, T7)
    MOV AX, b
    MOV BX, b
    IMUL BX
    MOV _T7, AX

    ; [25] (+, T6, T7, T8)
    MOV AX, _T6
    MOV BX, _T7
    ADD AX, BX
    MOV _T8, AX

    ; [26] (:=, T8, , res)
    MOV AX, _T8
    MOV res, AX

    ; [27] (out, "a*a + b*b = ", , )
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 42   ; '*'
    INT 21h
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 43   ; '+'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 98   ; 'b'
    INT 21h
    MOV AH, 02h
    MOV DL, 42   ; '*'
    INT 21h
    MOV AH, 02h
    MOV DL, 98   ; 'b'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 61   ; '='
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [28] (out, res, , )
    MOV AX, res
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [29] (*, T0, T1, T11)
    MOV AX, _T0
    MOV BX, _T1
    IMUL BX
    MOV _T11, AX

    ; [30] (:=, T11, , res)
    MOV AX, _T11
    MOV res, AX

    ; [31] (out, "(a+b)*(a-b) = ", , )
    MOV AH, 02h
    MOV DL, 40   ; '('
    INT 21h
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 43   ; '+'
    INT 21h
    MOV AH, 02h
    MOV DL, 98   ; 'b'
    INT 21h
    MOV AH, 02h
    MOV DL, 41   ; ')'
    INT 21h
    MOV AH, 02h
    MOV DL, 42   ; '*'
    INT 21h
    MOV AH, 02h
    MOV DL, 40   ; '('
    INT 21h
    MOV AH, 02h
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 45   ; '-'
    INT 21h
    MOV AH, 02h
    MOV DL, 98   ; 'b'
    INT 21h
    MOV AH, 02h
    MOV DL, 41   ; ')'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 61   ; '='
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [32] (out, res, , )
    MOV AX, res
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [33] (:=, 7.500000, , x)
    MOV AX, 7
    MOV x, AX

    ; [34] (:=, 2.500000, , y)
    MOV AX, 2
    MOV y, AX

    ; [35] (out, "x = ", , )
    MOV AH, 02h
    MOV DL, 120   ; 'x'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 61   ; '='
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [36] (out, x, , )
    MOV AX, x
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [37] (out, "y = ", , )
    MOV AH, 02h
    MOV DL, 121   ; 'y'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 61   ; '='
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [38] (out, y, , )
    MOV AX, y
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [39] (-, Max, a, T12)
    MOV AX, Max
    MOV BX, a
    SUB AX, BX
    MOV _T12, AX

    ; [40] (:=, T12, , res)
    MOV AX, _T12
    MOV res, AX

    ; [41] (out, "MAX - a = ", , )
    MOV AH, 02h
    MOV DL, 77   ; 'M'
    INT 21h
    MOV AH, 02h
    MOV DL, 65   ; 'A'
    INT 21h
    MOV AH, 02h
    MOV DL, 88   ; 'X'
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
    MOV DL, 97   ; 'a'
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 61   ; '='
    INT 21h
    MOV AH, 02h
    MOV DL, 32   ; ' '
    INT 21h
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [42] (out, res, , )
    MOV AX, res
    CALL _PRINT_INT
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

L43:
    MOV AH, 4Ch
    INT 21h

LECODE ENDS

END Debut
