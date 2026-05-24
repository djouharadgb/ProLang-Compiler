TITLE prolang.asm
; Code 8086 genere automatiquement :))
; Arithmetique en virgule fixe : toutes les valeurs sont stockees * 100

PILE SEGMENT STACK
    DW 100 DUP (?)
base_pile EQU $
PILE ENDS

DONNEE SEGMENT

    moyenne              DW 0   ; fixe: 0.000000 * 100 = 0
    somme                DW 0   ; fixe: 0 * 100 = 0
    z                    DW ?
    y                    DW ?
    x                    DW ?
    Max                  DW 10000   ; fixe: 100 * 100 = 10000
    Tabint               DW 50 DUP(?)  ; tableau
    Pi                   DW 314   ; fixe: 3.141590 * 100 = 314
    Tabfloat             DW 30 DUP(?)  ; tableau
    k                    DW ?
    j                    DW ?
    i                    DW ?
    b                    DW ?
    a                    DW ?
    _T0                  DW ?          ; temporaire
    _T1                  DW ?          ; temporaire
    _T2                  DW ?          ; temporaire
    _T3                  DW ?          ; temporaire
    _T4                  DW ?          ; temporaire
    _T5                  DW ?          ; temporaire
    _T6                  DW ?          ; temporaire
    _T7                  DW ?          ; temporaire
    _T8                  DW ?          ; temporaire
    _T9                  DW ?          ; temporaire
    _T10                 DW ?          ; temporaire
    _T11                 DW ?          ; temporaire
    _T12                 DW ?          ; temporaire
    _T13                 DW ?          ; temporaire
    _T14                 DW ?          ; temporaire
    _T15                 DW ?          ; temporaire
    _T16                 DW ?          ; temporaire
    _T17                 DW ?          ; temporaire
    _T18                 DW ?          ; temporaire
    _T19                 DW ?          ; temporaire
    _T20                 DW ?          ; temporaire
    _T21                 DW ?          ; temporaire
    _T22                 DW ?          ; temporaire
    _T23                 DW ?          ; temporaire
    _T24                 DW ?          ; temporaire
    _T25                 DW ?          ; temporaire
    _T26                 DW ?          ; temporaire
    _T27                 DW ?          ; temporaire
    _T28                 DW ?          ; temporaire
    _T29                 DW ?          ; temporaire
    _T30                 DW ?          ; temporaire
    _T31                 DW ?          ; temporaire
    _T32                 DW ?          ; temporaire
    _T33                 DW ?          ; temporaire
    _T34                 DW ?          ; temporaire
    _T35                 DW ?          ; temporaire
    _T36                 DW ?          ; temporaire
    _T37                 DW ?          ; temporaire
    _T38                 DW ?          ; temporaire
    _T39                 DW ?          ; temporaire
    _T40                 DW ?          ; temporaire
    _T41                 DW ?          ; temporaire
    _T42                 DW ?          ; temporaire
    _T43                 DW ?          ; temporaire
    _T44                 DW ?          ; temporaire
    _T45                 DW ?          ; temporaire
    _T46                 DW ?          ; temporaire
    _T47                 DW ?          ; temporaire
    _T48                 DW ?          ; temporaire
    _T49                 DW ?          ; temporaire
    _T50                 DW ?          ; temporaire
    _T51                 DW ?          ; temporaire
    _OUT_BUF             DB 16 DUP(?), '$'

DONNEE ENDS

LECODE SEGMENT

;---- _PRINT_INT_RAW : affiche BX en decimal positif (usage interne) ----
_PRINT_INT_RAW PROC NEAR
    PUSH AX
    PUSH CX
    PUSH DX
    PUSH SI
    MOV AX, BX          ; AX = valeur a afficher
    MOV SI, OFFSET _OUT_BUF
    MOV BX, 10          ; diviseur decimal
    MOV CX, 0           ; compteur de chiffres
_RAW_DIV:
    MOV DX, 0
    DIV BX              ; AX = AX/10, DX = chiffre (reste)
    ADD DL, '0'         ; convertir en caractere ASCII
    PUSH DX             ; empiler les chiffres (ordre inverse pour depiler)
    INC CX
    CMP AX, 0
    JNE _RAW_DIV
_RAW_POP:
    POP DX
    MOV [SI], DL        ; ecrire chiffre dans le tampon
    INC SI
    LOOP _RAW_POP
    MOV BYTE PTR [SI], '$' ; terminateur INT 21h
    MOV AH, 09h
    MOV DX, OFFSET _OUT_BUF
    INT 21h             ; afficher la chaine
    POP SI
    POP DX
    POP CX
    POP AX
    RET
_PRINT_INT_RAW ENDP

;---- _PRINT_FIXED : affiche AX en virgule fixe (AX = valeur * 100) ----
;     ex: AX=314  => '3.14'   AX=1000 => '10.00'   AX=-250 => '-2.50'
_PRINT_FIXED PROC NEAR
    PUSH AX
    PUSH BX
    PUSH CX
    PUSH DX
    CMP AX, 0
    JGE _FIX_POS
    MOV AH, 02h
    MOV DL, '-'         ; afficher le signe moins
    INT 21h
    NEG AX              ; travailler avec la valeur absolue
_FIX_POS:
    MOV BX, 100          ; diviseur = SCALE = 100
    MOV DX, 0
    DIV BX              ; AX = partie entiere, DX = partie decimale (0..99)
    PUSH DX             ; sauvegarder la partie decimale
    MOV BX, AX          ; BX = partie entiere pour _PRINT_INT_RAW
    CALL _PRINT_INT_RAW
    MOV AH, 02h
    MOV DL, '.'         ; point decimal
    INT 21h
    POP AX              ; AX = partie decimale (0..99)
    CMP AX, 10          ; ex: 5 => afficher '05' et non '5'
    JGE _FIX_NO_ZERO
    MOV AH, 02h
    MOV DL, '0'         ; zero de tete
    INT 21h
_FIX_NO_ZERO:
    MOV BX, AX          ; BX = partie decimale pour _PRINT_INT_RAW
    CALL _PRINT_INT_RAW
    POP DX
    POP CX
    POP BX
    POP AX
    RET
_PRINT_FIXED ENDP

;---- _READ_FIXED : lit un entier ou decimal depuis le clavier ----
;     retourne AX = valeur * 100 (virgule fixe)
;     ex: '3.14' => AX=314   '10' => AX=1000   '2.5' => AX=250   '-1.5' => AX=-150
_READ_FIXED PROC NEAR
    PUSH BX
    PUSH CX
    PUSH DX
    PUSH SI
    PUSH BP
    MOV BX, 0   ; partie entiere (valeur brute)
    MOV CX, 0   ; partie decimale (0..99)
    MOV SI, 0   ; SI=0 => mode entier, SI=1 => mode decimal
    MOV DX, 0   ; nombre de chiffres decimaux lus
    MOV BP, 0   ; BP=0 positif, BP=1 negatif
_RFIXED_CHAR:
    MOV AH, 01h
    INT 21h     ; AL = caractere saisi
    CMP AL, 0Dh ; Entree (CR) ?
    JE  _RFIXED_DONE
    CMP AL, '-'
    JNE _RFIXED_NOT_MINUS
    MOV BP, 1   ; marquer negatif
    JMP _RFIXED_CHAR
_RFIXED_NOT_MINUS:
    CMP AL, '.'
    JNE _RFIXED_NOT_DOT
    MOV SI, 1   ; activer le mode decimal
    JMP _RFIXED_CHAR
_RFIXED_NOT_DOT:
    CMP AL, '0'
    JL  _RFIXED_CHAR
    CMP AL, '9'
    JG  _RFIXED_CHAR
    SUB AL, '0' ; convertir ASCII => valeur 0..9
    CBW         ; AL => AX
    CMP SI, 1   ; mode decimal ?
    JE  _RFIXED_DEC_DIGIT
    PUSH AX
    MOV AX, BX
    MOV BX, 10
    MUL BX
    MOV BX, AX
    POP AX
    ADD BX, AX  ; BX = partie entiere accumulee
    JMP _RFIXED_CHAR
_RFIXED_DEC_DIGIT:
    CMP DX, 2   ; deja 2 chiffres decimaux lus ?
    JGE _RFIXED_CHAR ; ignorer les chiffres en trop
    INC DX
    CMP DX, 1   ; premier chiffre decimal ?
    JNE _RFIXED_DEC2
    MOV CX, AX
    PUSH BX
    MOV BX, 10
    MUL BX      ; AX = chiffre * 10 (ex: 5 => 50 = 0.50)
    MOV CX, AX  ; CX = dizaine
    POP BX
    JMP _RFIXED_CHAR
_RFIXED_DEC2:
    ADD CX, AX  ; CX = dizaine + unite => 0..99
    JMP _RFIXED_CHAR
_RFIXED_DONE:
    MOV AX, BX       ; AX = partie entiere brute
    MOV BX, 100
    MUL BX           ; AX = partie_entiere * SCALE
    ADD AX, CX       ; AX += partie decimale (0..99)
    CMP BP, 1
    JNE _RFIXED_POS
    NEG AX
_RFIXED_POS:
    POP BP
    POP SI
    POP DX
    POP CX
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
    MOV AX, 1000
    MOV x, AX

    ; [3] (:=, 5, , y)
    MOV AX, 500
    MOV y, AX

    ; [4] (:=, 2, , z)
    MOV AX, 200
    MOV z, AX

    ; [5] (:=, 2.500000, , a)
    MOV AX, 250
    MOV a, AX

    ; [6] (+, a, 3.141590, T0)
    MOV AX, a
    ADD AX, 314
    MOV _T0, AX

    ; [7] (+, T0, T0, T1)
    MOV AX, _T0
    MOV BX, _T0
    ADD AX, BX
    MOV _T1, AX

    ; [8] (:=, T1, , b)
    MOV AX, _T1
    MOV b, AX

    ; [9] (*, y, z, T2)
    MOV AX, y
    MOV BX, z
    CWD            ; etendre AX -> DX:AX pour IMUL signe
    IMUL BX        ; DX:AX = (a*SCALE)*(b*SCALE) = (a*b)*SCALE^2
    MOV BX, 100
    IDIV BX        ; AX = (a*b)*SCALE  (virgule fixe correct)
    MOV _T2, AX

    ; [10] (+, x, T2, T3)
    MOV AX, x
    MOV BX, _T2
    ADD AX, BX
    MOV _T3, AX

    ; [11] (:=, T3, , Tabint[0])
    MOV AX, _T3
    MOV SI, 0
    MOV Tabint[SI], AX

    ; [12] (+, b, 3.500000, T4)
    MOV AX, b
    ADD AX, 350
    MOV _T4, AX

    ; [13] (/, T4, 2.000000, T5)
    MOV AX, _T4
    MOV BX, 200
    MOV CX, BX     ; sauvegarder BX = (b*SCALE)
    MOV BX, 100     ; BX = SCALE
    IMUL BX        ; DX:AX = (a*SCALE)*SCALE = a*SCALE^2
    MOV BX, CX     ; restaurer BX = (b*SCALE)
    IDIV BX        ; AX = a*SCALE^2 / (b*SCALE) = (a/b)*SCALE
    MOV _T5, AX

    ; [14] (:=, T5, , Tabfloat[1])
    MOV AX, _T5
    MOV SI, 2
    MOV Tabfloat[SI], AX

    ; [15] (>, x, y, T6)
    MOV AX, x
    MOV BX, y
    CMP AX, BX
    JG _CMP15_V
    MOV _T6, 0
    JMP _CMP15_S
_CMP15_V:
    MOV _T6, 1
_CMP15_S:

    ; [16] (+, x, y, T7)
    MOV AX, x
    MOV BX, y
    ADD AX, BX
    MOV _T7, AX

    ; [17] (<, z, T7, T8)
    MOV AX, z
    MOV BX, _T7
    CMP AX, BX
    JL _CMP17_V
    MOV _T8, 0
    JMP _CMP17_S
_CMP17_V:
    MOV _T8, 1
_CMP17_S:

    ; [18] (AND, T6, T8, T9)
    MOV AX, _T6
    MOV BX, _T8
    AND AX, BX
    MOV _T9, AX

    ; [19] (==, y, 0, T10)
    MOV AX, y
    CMP AX, 0
    JE _CMP19_V
    MOV _T10, 0
    JMP _CMP19_S
_CMP19_V:
    MOV _T10, 1
_CMP19_S:

    ; [20] (NON, T10, , T11)
    MOV AX, _T10
    CMP AX, 0
    JE  _NON20_U
    MOV AX, 0
    JMP _NON20_F
_NON20_U:
    MOV AX, 1
_NON20_F:
    MOV _T11, AX

    ; [21] (OR, T9, T11, T12)
    MOV AX, _T9
    MOV BX, _T11
    OR AX, BX
    MOV _T12, AX

    ; [22] (BZ, T12, , 48)
    MOV AX, _T12
    CMP AX, 0
    JE  L48

    ; [23] (+, x, y, T13)
    MOV AX, x
    MOV BX, y
    ADD AX, BX
    MOV _T13, AX

    ; [24] (+, T13, z, T14)
    MOV AX, _T13
    MOV BX, z
    ADD AX, BX
    MOV _T14, AX

    ; [25] (:=, T14, , somme)
    MOV AX, _T14
    MOV somme, AX

    ; [26] (:=, 0, , i)
    MOV AX, 0
    MOV i, AX

L27:
    ; [27] (<=, i, 10, T15)
    MOV AX, i
    CMP AX, 1000
    JLE _CMP27_V
    MOV _T15, 0
    JMP _CMP27_S
_CMP27_V:
    MOV _T15, 1
_CMP27_S:

    ; [28] (BZ, T15, , 47)
    MOV AX, _T15
    CMP AX, 0
    JE  L47

    ; [29] (TAB, Tabint, i, T16)
    MOV AX, i
    CWD
    MOV BX, 100
    IDIV BX
    MOV SI, AX
    ADD SI, SI
    MOV AX, Tabint[SI]
    MOV _T16, AX

    ; [30] (+, T16, i, T17)
    MOV AX, _T16
    MOV BX, i
    ADD AX, BX
    MOV _T17, AX

    ; [31] (:=, T17, , Tabint[i])
    MOV AX, _T17
    PUSH AX
    MOV AX, i
    CWD
    MOV BX, 100
    IDIV BX
    MOV SI, AX
    ADD SI, SI
    POP AX
    MOV Tabint[SI], AX

    ; [32] (<, i, 5, T18)
    MOV AX, i
    CMP AX, 500
    JL _CMP32_V
    MOV _T18, 0
    JMP _CMP32_S
_CMP32_V:
    MOV _T18, 1
_CMP32_S:

    ; [33] (TAB, Tabint, i, T19)
    MOV AX, i
    CWD
    MOV BX, 100
    IDIV BX
    MOV SI, AX
    ADD SI, SI
    MOV AX, Tabint[SI]
    MOV _T19, AX

    ; [34] (>, T19, 10, T20)
    MOV AX, _T19
    CMP AX, 1000
    JG _CMP34_V
    MOV _T20, 0
    JMP _CMP34_S
_CMP34_V:
    MOV _T20, 1
_CMP34_S:

    ; [35] (AND, T18, T20, T21)
    MOV AX, _T18
    MOV BX, _T20
    AND AX, BX
    MOV _T21, AX

    ; [36] (BZ, T21, , 41)
    MOV AX, _T21
    CMP AX, 0
    JE  L41

    ; [37] (TAB, Tabint, i, T22)
    MOV AX, i
    CWD
    MOV BX, 100
    IDIV BX
    MOV SI, AX
    ADD SI, SI
    MOV AX, Tabint[SI]
    MOV _T22, AX

    ; [38] (*, T22, 1.500000, T23)
    MOV AX, _T22
    MOV BX, 150
    CWD            ; etendre AX -> DX:AX pour IMUL signe
    IMUL BX        ; DX:AX = (a*SCALE)*(b*SCALE) = (a*b)*SCALE^2
    MOV BX, 100
    IDIV BX        ; AX = (a*b)*SCALE  (virgule fixe correct)
    MOV _T23, AX

    ; [39] (:=, T23, , Tabfloat[i])
    MOV AX, _T23
    PUSH AX
    MOV AX, i
    CWD
    MOV BX, 100
    IDIV BX
    MOV SI, AX
    ADD SI, SI
    POP AX
    MOV Tabfloat[SI], AX

    ; [40] (BR, , , 44)
    JMP L44

L41:
    ; [41] (TAB, Tabint, i, T24)
    MOV AX, i
    CWD
    MOV BX, 100
    IDIV BX
    MOV SI, AX
    ADD SI, SI
    MOV AX, Tabint[SI]
    MOV _T24, AX

    ; [42] (/, T24, 2.000000, T25)
    MOV AX, _T24
    MOV BX, 200
    MOV CX, BX     ; sauvegarder BX = (b*SCALE)
    MOV BX, 100     ; BX = SCALE
    IMUL BX        ; DX:AX = (a*SCALE)*SCALE = a*SCALE^2
    MOV BX, CX     ; restaurer BX = (b*SCALE)
    IDIV BX        ; AX = a*SCALE^2 / (b*SCALE) = (a/b)*SCALE
    MOV _T25, AX

    ; [43] (:=, T25, , Tabfloat[i])
    MOV AX, _T25
    PUSH AX
    MOV AX, i
    CWD
    MOV BX, 100
    IDIV BX
    MOV SI, AX
    ADD SI, SI
    POP AX
    MOV Tabfloat[SI], AX

L44:
    ; [44] (+, i, 1, T26)
    MOV AX, i
    ADD AX, 100
    MOV _T26, AX

    ; [45] (:=, T26, , i)
    MOV AX, _T26
    MOV i, AX

    ; [46] (BR, , , 27)
    JMP L27

L47:
    ; [47] (BR, , , 49)
    JMP L49

L48:
    ; [48] (:=, 0, , somme)
    MOV AX, 0
    MOV somme, AX

L49:
    ; [49] (<=, x, 100, T27)
    MOV AX, x
    CMP AX, 10000
    JLE _CMP49_V
    MOV _T27, 0
    JMP _CMP49_S
_CMP49_V:
    MOV _T27, 1
_CMP49_S:

    ; [50] (!=, y, 0, T28)
    MOV AX, y
    CMP AX, 0
    JNE _CMP50_V
    MOV _T28, 0
    JMP _CMP50_S
_CMP50_V:
    MOV _T28, 1
_CMP50_S:

    ; [51] (<, z, 10, T29)
    MOV AX, z
    CMP AX, 1000
    JL _CMP51_V
    MOV _T29, 0
    JMP _CMP51_S
_CMP51_V:
    MOV _T29, 1
_CMP51_S:

    ; [52] (OR, T28, T29, T30)
    MOV AX, _T28
    MOV BX, _T29
    OR AX, BX
    MOV _T30, AX

    ; [53] (AND, T27, T30, T31)
    MOV AX, _T27
    MOV BX, _T30
    AND AX, BX
    MOV _T31, AX

    ; [54] (BZ, T31, , 71)
    MOV AX, _T31
    CMP AX, 0
    JE  L71

    ; [55] (+, x, 1, T32)
    MOV AX, x
    ADD AX, 100
    MOV _T32, AX

    ; [56] (:=, T32, , x)
    MOV AX, _T32
    MOV x, AX

L57:
    ; [57] (==, x, y, T33)
    MOV AX, x
    MOV BX, y
    CMP AX, BX
    JE _CMP57_V
    MOV _T33, 0
    JMP _CMP57_S
_CMP57_V:
    MOV _T33, 1
_CMP57_S:

    ; [58] (NON, T33, , T34)
    MOV AX, _T33
    CMP AX, 0
    JE  _NON58_U
    MOV AX, 0
    JMP _NON58_F
_NON58_U:
    MOV AX, 1
_NON58_F:
    MOV _T34, AX

    ; [59] (BZ, T34, , 70)
    MOV AX, _T34
    CMP AX, 0
    JE  L70

    ; [60] (+, y, 1, T35)
    MOV AX, y
    ADD AX, 100
    MOV _T35, AX

    ; [61] (:=, T35, , y)
    MOV AX, _T35
    MOV y, AX

    ; [62] (+, x, 1, T36)
    MOV AX, x
    ADD AX, 100
    MOV _T36, AX

    ; [63] (TAB, Tabint, 0, T37)
    MOV SI, 0
    MOV AX, Tabint[SI]
    MOV _T37, AX

    ; [64] (TAB, Tabint, 1, T38)
    MOV SI, 2
    MOV AX, Tabint[SI]
    MOV _T38, AX

    ; [65] (+, T37, T38, T39)
    MOV AX, _T37
    MOV BX, _T38
    ADD AX, BX
    MOV _T39, AX

    ; [66] (-, x, y, T40)
    MOV AX, x
    MOV BX, y
    SUB AX, BX
    MOV _T40, AX

    ; [67] (*, T39, T40, T41)
    MOV AX, _T39
    MOV BX, _T40
    CWD            ; etendre AX -> DX:AX pour IMUL signe
    IMUL BX        ; DX:AX = (a*SCALE)*(b*SCALE) = (a*b)*SCALE^2
    MOV BX, 100
    IDIV BX        ; AX = (a*b)*SCALE  (virgule fixe correct)
    MOV _T41, AX

    ; [68] (:=, T41, , Tabint[T36])
    MOV AX, _T41
    PUSH AX
    MOV AX, _T36
    CWD
    MOV BX, 100
    IDIV BX
    MOV SI, AX
    ADD SI, SI
    POP AX
    MOV Tabint[SI], AX

    ; [69] (BR, , , 57)
    JMP L57

L70:
    ; [70] (BR, , , 49)
    JMP L49

L71:
    ; [71] (:=, 1, , j)
    MOV AX, 100
    MOV j, AX

L72:
    ; [72] (<=, j, 20, T42)
    MOV AX, j
    CMP AX, 2000
    JLE _CMP72_V
    MOV _T42, 0
    JMP _CMP72_S
_CMP72_V:
    MOV _T42, 1
_CMP72_S:

    ; [73] (BZ, T42, , 86)
    MOV AX, _T42
    CMP AX, 0
    JE  L86

    ; [74] (-, j, 1, T43)
    MOV AX, j
    SUB AX, 100
    MOV _T43, AX

    ; [75] (TAB, Tabfloat, T43, T44)
    MOV AX, _T43
    CWD
    MOV BX, 100
    IDIV BX
    MOV SI, AX
    ADD SI, SI
    MOV AX, Tabfloat[SI]
    MOV _T44, AX

    ; [76] (TAB, Tabfloat, j, T45)
    MOV AX, j
    CWD
    MOV BX, 100
    IDIV BX
    MOV SI, AX
    ADD SI, SI
    MOV AX, Tabfloat[SI]
    MOV _T45, AX

    ; [77] (+, T44, T45, T46)
    MOV AX, _T44
    MOV BX, _T45
    ADD AX, BX
    MOV _T46, AX

    ; [78] (/, T46, 2.000000, T47)
    MOV AX, _T46
    MOV BX, 200
    MOV CX, BX     ; sauvegarder BX = (b*SCALE)
    MOV BX, 100     ; BX = SCALE
    IMUL BX        ; DX:AX = (a*SCALE)*SCALE = a*SCALE^2
    MOV BX, CX     ; restaurer BX = (b*SCALE)
    IDIV BX        ; AX = a*SCALE^2 / (b*SCALE) = (a/b)*SCALE
    MOV _T47, AX

    ; [79] (:=, T47, , Tabfloat[j])
    MOV AX, _T47
    PUSH AX
    MOV AX, j
    CWD
    MOV BX, 100
    IDIV BX
    MOV SI, AX
    ADD SI, SI
    POP AX
    MOV Tabfloat[SI], AX

    ; [80] (TAB, Tabfloat, j, T48)
    MOV AX, j
    CWD
    MOV BX, 100
    IDIV BX
    MOV SI, AX
    ADD SI, SI
    MOV AX, Tabfloat[SI]
    MOV _T48, AX

    ; [81] (+, moyenne, T48, T49)
    MOV AX, moyenne
    MOV BX, _T48
    ADD AX, BX
    MOV _T49, AX

    ; [82] (:=, T49, , moyenne)
    MOV AX, _T49
    MOV moyenne, AX

    ; [83] (+, j, 1, T50)
    MOV AX, j
    ADD AX, 100
    MOV _T50, AX

    ; [84] (:=, T50, , j)
    MOV AX, _T50
    MOV j, AX

    ; [85] (BR, , , 72)
    JMP L72

L86:
    ; [86] (/, moyenne, 20.000000, T51)
    MOV AX, moyenne
    MOV BX, 2000
    MOV CX, BX     ; sauvegarder BX = (b*SCALE)
    MOV BX, 100     ; BX = SCALE
    IMUL BX        ; DX:AX = (a*SCALE)*SCALE = a*SCALE^2
    MOV BX, CX     ; restaurer BX = (b*SCALE)
    IDIV BX        ; AX = a*SCALE^2 / (b*SCALE) = (a/b)*SCALE
    MOV _T51, AX

    ; [87] (:=, T51, , moyenne)
    MOV AX, _T51
    MOV moyenne, AX

    ; [88] (input, , , x)
    CALL _READ_FIXED   ; resultat dans AX (virgule fixe, AX = valeur * 100)
    MOV x, AX

    ; [89] (out, "Valeur finale de x: ", , )
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

    ; [90] (out, x, , )
    MOV AX, x
    CALL _PRINT_FIXED
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [91] (out, "Somme: ", , )
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

    ; [92] (out, somme, , )
    MOV AX, somme
    CALL _PRINT_FIXED
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

    ; [93] (out, "Moyenne: ", , )
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

    ; [94] (out, moyenne, , )
    MOV AX, moyenne
    CALL _PRINT_FIXED
    MOV AH, 02h
    MOV DL, 0Ah
    INT 21h

L95:
    MOV AH, 4Ch
    INT 21h

LECODE ENDS

END Debut
