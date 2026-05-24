
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "Syn.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RED   "\033[1;31m"
#define RESET "\033[0m"
#include "ts.h"
#include "quad.h"
#include "opt.h"
#include "asm8086.h"

int  yylex(void);
int  yyerror(char *msg);

extern int nb_ligne;
extern int nb_col;
extern int semantic_errors;
TypeVar type_courant;

/* Pending IDF list */
#define MAX_PENDING_IDF 100
static char *pending_idfs[MAX_PENDING_IDF];
static int pending_count = 0;

/* Contexte des boucles while imbriquees */
#define MAX_WHILE_NEST 100
static int while_start_stack[MAX_WHILE_NEST];
static int while_bz_stack[MAX_WHILE_NEST];
static int while_top = -1;

/*
 * Contexte des boucles for imbriquees.
 *
 * BUG 3 CORRIGE : l'ancienne implementation cherchait le BZ non patche
 * en remontant le tableau a reculons, ce qui est fragile (un BZ non
 * patche issu d'un if imbrique pouvait etre intercepte).  On utilise
 * maintenant un vrai empilement comme pour while, avec deux informations
 * par niveau :
 *   for_cond_stack[top] = indice du quad DEBUT DE CONDITION (quad <=)
 *                         -> cible du BR de rebouclage
 *   for_bz_stack[top]   = indice du quad BZ (a patcher a la fin)
 *
 * Cela garantit que chaque endfor patche exactement son propre BZ,
 * independamment des structures de controle imbriquees.
 */
#define MAX_FOR_NEST 100
static int for_cond_stack[MAX_FOR_NEST]; /* indice du quad condition */
static int for_bz_stack[MAX_FOR_NEST];   /* indice du quad BZ        */
static int for_top = -1;


/* Line 189 of yacc.c  */
#line 125 "Syn.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDF = 258,
     STRING = 259,
     NUM_INT = 260,
     NUM_FLOAT = 261,
     BeginProject = 262,
     EndProject = 263,
     Setup = 264,
     Run = 265,
     DEFINE_MC = 266,
     CONST_MC = 267,
     INTEGER_MC = 268,
     FLOAT_MC = 269,
     IF_MC = 270,
     THEN_MC = 271,
     ELSE_MC = 272,
     ENDIF_MC = 273,
     LOOP_MC = 274,
     WHILE_MC = 275,
     ENDLOOP_MC = 276,
     FOR_MC = 277,
     IN_MC = 278,
     TO_MC = 279,
     ENDFOR_MC = 280,
     INPUT_MC = 281,
     OUT_MC = 282,
     AND = 283,
     OR = 284,
     NON = 285,
     OP_ASSIGN = 286,
     OP_INIT = 287,
     OP_EQ = 288,
     OP_NE = 289,
     OP_LT = 290,
     OP_GT = 291,
     OP_LE = 292,
     OP_GE = 293,
     OP_ADD = 294,
     OP_SUB = 295,
     OP_MUL = 296,
     OP_DIV = 297,
     SEP_SEMICOLON = 298,
     SEP_COLON = 299,
     SEP_COMMA = 300,
     SEP_PIPE = 301,
     SEP_LBRACKET = 302,
     SEP_RBRACKET = 303,
     SEP_LBRACE = 304,
     SEP_RBRACE = 305,
     SEP_LPAREN = 306,
     SEP_RPAREN = 307
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 52 "Syn.y"

    int    ival;
    float  fval;
    char*  sval;



/* Line 214 of yacc.c  */
#line 221 "Syn.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 233 "Syn.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   243

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  53
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  30
/* YYNRULES -- Number of rules.  */
#define YYNRULES  75
/* YYNRULES -- Number of states.  */
#define YYNSTATES  194

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   307

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,    17,    19,    20,    23,    25,    27,    29,
      32,    37,    40,    45,    52,    60,    62,    66,    68,    70,
      72,    74,    79,    84,    89,    94,    97,    98,   100,   102,
     104,   106,   108,   110,   113,   114,   120,   129,   130,   142,
     143,   151,   154,   155,   156,   169,   170,   183,   189,   195,
     197,   201,   203,   205,   207,   209,   213,   217,   221,   225,
     229,   233,   237,   241,   246,   250,   254,   258,   262,   266,
     270,   275,   280,   282,   287,   289
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      54,     0,    -1,     7,     3,    43,     9,    44,    55,    10,
      44,    49,    64,    50,     8,    43,    -1,    56,    -1,    -1,
      57,    56,    -1,    57,    -1,    58,    -1,    60,    -1,     1,
      43,    -1,    11,    61,    44,    59,    -1,    62,    43,    -1,
      62,    32,    63,    43,    -1,    47,    62,    43,     5,    48,
      43,    -1,    12,     3,    44,    62,    32,    63,    43,    -1,
       3,    -1,     3,    46,    61,    -1,    13,    -1,    14,    -1,
       5,    -1,     6,    -1,    51,    39,     5,    52,    -1,    51,
      40,     5,    52,    -1,    51,    39,     6,    52,    -1,    51,
      40,     6,    52,    -1,    65,    64,    -1,    -1,    67,    -1,
      68,    -1,    72,    -1,    75,    -1,    77,    -1,    78,    -1,
       1,    43,    -1,    -1,     3,    31,    66,    82,    43,    -1,
       3,    47,    66,    82,    48,    31,    82,    43,    -1,    -1,
      15,    51,    81,    52,    16,    44,    49,    69,    64,    50,
      70,    -1,    -1,    17,    71,    49,    64,    50,    18,    43,
      -1,    18,    43,    -1,    -1,    -1,    19,    20,    51,    73,
      81,    52,    74,    49,    64,    50,    21,    43,    -1,    -1,
      22,     3,    23,    82,    24,    82,    76,    49,    64,    50,
      25,    43,    -1,    26,    51,     3,    52,    43,    -1,    27,
      51,    79,    52,    43,    -1,    80,    -1,    80,    45,    79,
      -1,     4,    -1,     3,    -1,     5,    -1,     6,    -1,    82,
      33,    82,    -1,    82,    34,    82,    -1,    82,    35,    82,
      -1,    82,    36,    82,    -1,    82,    37,    82,    -1,    82,
      38,    82,    -1,    81,    28,    81,    -1,    81,    29,    81,
      -1,    30,    51,    81,    52,    -1,    51,    81,    52,    -1,
      82,    39,    82,    -1,    82,    40,    82,    -1,    82,    41,
      82,    -1,    82,    42,    82,    -1,    51,    82,    52,    -1,
      51,    39,    82,    52,    -1,    51,    40,    82,    52,    -1,
       3,    -1,     3,    47,    82,    48,    -1,     5,    -1,     6,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   112,   112,   141,   142,   146,   147,   151,   152,   153,
     166,   170,   181,   195,   215,   227,   232,   240,   241,   245,
     247,   249,   251,   253,   255,   260,   261,   265,   266,   267,
     268,   269,   270,   271,   277,   281,   312,   350,   348,   359,
     358,   387,   404,   412,   403,   453,   452,   507,   522,   527,
     528,   532,   533,   548,   553,   561,   563,   565,   567,   569,
     571,   573,   575,   577,   579,   584,   586,   588,   590,   605,
     607,   609,   611,   626,   636,   641
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDF", "STRING", "NUM_INT", "NUM_FLOAT",
  "BeginProject", "EndProject", "Setup", "Run", "DEFINE_MC", "CONST_MC",
  "INTEGER_MC", "FLOAT_MC", "IF_MC", "THEN_MC", "ELSE_MC", "ENDIF_MC",
  "LOOP_MC", "WHILE_MC", "ENDLOOP_MC", "FOR_MC", "IN_MC", "TO_MC",
  "ENDFOR_MC", "INPUT_MC", "OUT_MC", "AND", "OR", "NON", "OP_ASSIGN",
  "OP_INIT", "OP_EQ", "OP_NE", "OP_LT", "OP_GT", "OP_LE", "OP_GE",
  "OP_ADD", "OP_SUB", "OP_MUL", "OP_DIV", "SEP_SEMICOLON", "SEP_COLON",
  "SEP_COMMA", "SEP_PIPE", "SEP_LBRACKET", "SEP_RBRACKET", "SEP_LBRACE",
  "SEP_RBRACE", "SEP_LPAREN", "SEP_RPAREN", "$accept", "programme",
  "partie_declaration", "liste_declarations", "declaration",
  "decl_variable", "suite_definition", "decl_constante", "liste_idf",
  "type", "valeur", "liste_instructions", "instruction", "sem_checkpoint",
  "instruction_affectation", "instruction_condition", "$@1", "suite_if",
  "$@2", "instruction_loop_while", "$@3", "$@4", "instruction_for", "$@5",
  "instruction_input", "instruction_output", "liste_out", "element_out",
  "condition", "expression", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    53,    54,    55,    55,    56,    56,    57,    57,    57,
      58,    59,    59,    59,    60,    61,    61,    62,    62,    63,
      63,    63,    63,    63,    63,    64,    64,    65,    65,    65,
      65,    65,    65,    65,    66,    67,    67,    69,    68,    71,
      70,    70,    73,    74,    72,    76,    75,    77,    78,    79,
      79,    80,    80,    80,    80,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    82
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,    13,     1,     0,     2,     1,     1,     1,     2,
       4,     2,     4,     6,     7,     1,     3,     1,     1,     1,
       1,     4,     4,     4,     4,     2,     0,     1,     1,     1,
       1,     1,     1,     2,     0,     5,     8,     0,    11,     0,
       7,     2,     0,     0,    12,     0,    12,     5,     5,     1,
       3,     1,     1,     1,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     4,     3,     3,     3,     3,     3,     3,
       4,     4,     1,     4,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     1,     0,     0,     0,     0,     0,
       0,     0,     3,     0,     7,     8,     9,    15,     0,     0,
       0,     5,     0,     0,     0,     0,    16,    17,    18,     0,
      10,     0,     0,     0,     0,     0,    11,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    27,    28,    29,
      30,    31,    32,     0,    19,    20,     0,     0,     0,    33,
      34,    34,     0,     0,     0,     0,     0,     0,    25,     0,
       0,     0,    12,    14,     0,     0,    72,    74,    75,     0,
       0,     0,     0,    42,     0,     0,    52,    51,    53,    54,
       0,    49,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     2,    13,    21,    23,    22,
      24,     0,    35,     0,     0,     0,     0,     0,    64,    69,
      61,    62,     0,    55,    56,    57,    58,    59,    60,    65,
      66,    67,    68,     0,     0,    47,    48,    50,     0,    73,
      63,    70,    71,     0,    43,    45,     0,    37,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      39,     0,    38,     0,     0,     0,    41,    44,    46,     0,
       0,     0,     0,    40
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,    11,    12,    13,    14,    30,    15,    18,    31,
      57,    45,    46,    74,    47,    48,   171,   182,   185,    49,
     120,   168,    50,   169,    51,    52,    90,    91,    81,    82
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -74
static const yytype_int16 yypact[] =
{
       5,    16,    54,    15,   -74,    71,    45,    67,    53,   117,
     121,   118,   -74,   129,   -74,   -74,   -74,    99,    98,   102,
     103,   -74,   117,    51,    -8,   122,   -74,   -74,   -74,    -8,
     -74,   -23,   135,     7,   126,     8,   -74,     8,   127,   -15,
     140,   172,   190,   143,   144,   146,     7,   -74,   -74,   -74,
     -74,   -74,   -74,   192,   -74,   -74,    -4,   155,   156,   -74,
     -74,   -74,    18,   149,   178,   199,   180,   195,   -74,   157,
      56,    87,   -74,   -74,    50,    50,   159,   -74,   -74,   153,
      44,   -25,   116,   -74,    50,   158,   -74,   -74,   -74,   -74,
     160,   162,   165,   166,   161,   163,   164,   167,    12,   134,
     120,    50,    18,    50,    50,    38,    69,    18,    18,   198,
      50,    50,    50,    50,    50,    50,    50,    50,    50,    50,
      18,    73,   168,   174,   180,   -74,   -74,   -74,   -74,   -74,
     -74,    77,   -74,   187,   124,    42,    92,    96,   -74,   -74,
     -74,   -74,   176,   148,   148,   148,   148,   148,   148,    31,
      31,   -74,   -74,    47,    50,   -74,   -74,   -74,    50,   -74,
     -74,   -74,   -74,   173,   -74,   148,   139,   -74,   175,   177,
     -74,     7,     7,     7,   171,   179,   181,   105,   202,   200,
     -74,   184,   -74,   185,   189,   186,   -74,   -74,   -74,     7,
     183,   212,   191,   -74
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -74,   -74,   -74,   223,   -74,   -74,   -74,   -74,   215,    62,
     201,   -46,   -74,   182,   -74,   -74,   -74,   -74,   -74,   -74,
     -74,   -74,   -74,   -74,   -74,   -74,   115,   -74,   -20,   -73
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -27
static const yytype_int16 yytable[] =
{
      68,    99,   100,   107,   108,    27,    28,   106,    38,    35,
      39,   121,     1,    54,    55,    76,    60,    77,    78,     3,
      36,    76,    40,    77,    78,   131,    41,   109,   134,    42,
     136,   137,    61,    43,    44,    70,    71,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,    76,    79,    77,
      78,   103,   104,    76,     4,    77,    78,   -26,     5,    56,
     105,    94,    95,    98,    27,    28,   107,   108,     8,    80,
     107,   108,   118,   119,    79,   107,   108,    -4,     9,    10,
       6,   165,   135,   103,   104,   166,    32,   140,   141,     7,
     138,    34,    96,    97,   160,    80,    16,   154,    29,   164,
     153,    98,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   116,   117,   118,   119,   116,   117,   118,   119,
      17,   139,   180,   181,    19,   174,   175,   176,    20,   139,
       8,   116,   117,   118,   119,   116,   117,   118,   119,    -6,
       9,    10,    23,   190,   161,    22,    24,    25,   162,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   116,
     117,   118,   119,   116,   117,   118,   119,    37,   133,    53,
      59,    33,   159,   116,   117,   118,   119,   132,   116,   117,
     118,   119,   170,    86,    87,    88,    89,   116,   117,   118,
     119,    62,    63,    64,    65,    66,    67,    69,    72,    73,
      83,    84,    85,    92,   102,    93,   101,   124,   125,   126,
     122,   155,   123,   127,   142,   128,   129,   156,   158,   130,
     163,   177,   167,   183,   172,   184,   173,   186,   187,   178,
     192,   179,   188,   191,   193,   189,    21,    26,    58,   157,
       0,     0,     0,    75
};

static const yytype_int16 yycheck[] =
{
      46,    74,    75,    28,    29,    13,    14,    80,     1,    32,
       3,    84,     7,     5,     6,     3,    31,     5,     6,     3,
      43,     3,    15,     5,     6,    98,    19,    52,   101,    22,
     103,   104,    47,    26,    27,    39,    40,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,     3,    30,     5,
       6,    39,    40,     3,     0,     5,     6,    50,    43,    51,
      80,     5,     6,    51,    13,    14,    28,    29,     1,    51,
      28,    29,    41,    42,    30,    28,    29,    10,    11,    12,
       9,   154,   102,    39,    40,   158,    24,   107,   108,    44,
      52,    29,     5,     6,    52,    51,    43,    24,    47,    52,
     120,    51,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    39,    40,    41,    42,    39,    40,    41,    42,
       3,    52,    17,    18,     3,   171,   172,   173,    10,    52,
       1,    39,    40,    41,    42,    39,    40,    41,    42,    10,
      11,    12,    44,   189,    52,    46,    44,    44,    52,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    39,
      40,    41,    42,    39,    40,    41,    42,    32,    48,    43,
      43,    49,    48,    39,    40,    41,    42,    43,    39,    40,
      41,    42,    43,     3,     4,     5,     6,    39,    40,    41,
      42,    51,    20,     3,    51,    51,    50,     5,    43,    43,
      51,    23,     3,     8,    51,    48,    47,    45,    43,    43,
      52,    43,    52,    52,    16,    52,    52,    43,    31,    52,
      44,    50,    49,    21,    49,    25,    49,    43,    43,    50,
      18,    50,    43,    50,    43,    49,    13,    22,    37,   124,
      -1,    -1,    -1,    61
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     7,    54,     3,     0,    43,     9,    44,     1,    11,
      12,    55,    56,    57,    58,    60,    43,     3,    61,     3,
      10,    56,    46,    44,    44,    44,    61,    13,    14,    47,
      59,    62,    62,    49,    62,    32,    43,    32,     1,     3,
      15,    19,    22,    26,    27,    64,    65,    67,    68,    72,
      75,    77,    78,    43,     5,     6,    51,    63,    63,    43,
      31,    47,    51,    20,     3,    51,    51,    50,    64,     5,
      39,    40,    43,    43,    66,    66,     3,     5,     6,    30,
      51,    81,    82,    51,    23,     3,     3,     4,     5,     6,
      79,    80,     8,    48,     5,     6,     5,     6,    51,    82,
      82,    47,    51,    39,    40,    81,    82,    28,    29,    52,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      73,    82,    52,    52,    45,    43,    43,    52,    52,    52,
      52,    82,    43,    48,    82,    81,    82,    82,    52,    52,
      81,    81,    16,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    81,    24,    43,    43,    79,    31,    48,
      52,    52,    52,    44,    52,    82,    82,    49,    74,    76,
      43,    69,    49,    49,    64,    64,    64,    50,    50,    50,
      17,    18,    70,    21,    25,    71,    43,    43,    43,    49,
      64,    50,    18,    43
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 118 "Syn.y"
    {
            if (semantic_errors == 0)
                printf("\nAnalyse syntaxique et semantique correcte.\n");
            else
                printf(RED "\n%d erreur(s) semantique(s) detectee(s) - analyse terminee avec erreurs." RESET "\n", semantic_errors);

            ts_afficher();
            printf("\n=== Code intermediaire AVANT optimisation ===\n");
            afficher_qdr();
            optimiser_quadruplets();
            printf("\n=== Code intermediaire APRES optimisation ===\n");
            afficher_qdr();
            if (semantic_errors == 0) {
                generer_asm("sortie.asm");
            } else {
                printf("Generation assembleur ignoree (erreurs semantiques).\n");
            }

            YYACCEPT;
        ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 154 "Syn.y"
    {
            /* BUG 6 CORRIGE : liberer et reinitialiser la liste pending
               pour eviter que des noms fantomes contaminent la prochaine
               declaration valide. */
            int _i;
            for (_i = 0; _i < pending_count; _i++) free(pending_idfs[_i]);
            pending_count = 0;
            yyerrok;
        ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 171 "Syn.y"
    {
            int ok = 1;
            int _i;
            for (_i = 0; _i < pending_count; _i++) {
                if (!ts_inserer_variable(pending_idfs[_i], type_courant)) ok = 0;
                free(pending_idfs[_i]);
            }
            pending_count = 0;
            if (ok) printf("Declaration de variable(s) correcte.\n");
        ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 182 "Syn.y"
    {
            int ok = 1;
            int _i;
            for (_i = 0; _i < pending_count; _i++) {
                if (!ts_inserer_variable(pending_idfs[_i], type_courant)) { ok = 0; free(pending_idfs[_i]); continue; }
                ts_marquer_init(pending_idfs[_i]);
                quadr(":=", (yyvsp[(3) - (4)].sval), "", pending_idfs[_i]);
                ts_set_val(pending_idfs[_i], (yyvsp[(3) - (4)].sval));
                free(pending_idfs[_i]);
            }
            pending_count = 0;
            if (ok) printf("Declaration de variable(s) avec initialisation correcte.\n");
        ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 196 "Syn.y"
    {
            if (pending_count != 1) {
                printf(RED "ERREUR semantique: un tableau ne peut avoir qu'un seul nom, ligne %d, col %d" RESET "\n",
                       nb_ligne, nb_col);
                semantic_errors++;
            } else if ((yyvsp[(4) - (6)].ival) <= 0) {
                printf(RED "ERREUR semantique: taille de tableau invalide pour '%s', ligne %d, col %d" RESET "\n",
                       pending_idfs[0], nb_ligne, nb_col);
            } else {
                ts_inserer_tableau(pending_idfs[0], type_courant, (yyvsp[(4) - (6)].ival));
                printf("Declaration de tableau correcte.\n");
            }
            int _i;
            for (_i = 0; _i < pending_count; _i++) free(pending_idfs[_i]);
            pending_count = 0;
        ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 216 "Syn.y"
    {
            if (ts_inserer_constante((yyvsp[(2) - (7)].sval), type_courant)) {
                ts_marquer_init((yyvsp[(2) - (7)].sval));
                quadr(":=", (yyvsp[(6) - (7)].sval), "", (yyvsp[(2) - (7)].sval));
                ts_set_val((yyvsp[(2) - (7)].sval), (yyvsp[(6) - (7)].sval));
                printf("Declaration de constante correcte.\n");
            }
        ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 228 "Syn.y"
    {
            if (pending_count < MAX_PENDING_IDF)
                pending_idfs[pending_count++] = strdup((yyvsp[(1) - (1)].sval));
        ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 233 "Syn.y"
    {
            if (pending_count < MAX_PENDING_IDF)
                pending_idfs[pending_count++] = strdup((yyvsp[(1) - (3)].sval));
        ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 240 "Syn.y"
    { type_courant = TYPE_INTEGER; ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 241 "Syn.y"
    { type_courant = TYPE_FLOAT;   ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 246 "Syn.y"
    { (yyval.sval) = (char*)malloc(20); sprintf((yyval.sval), "%d", (yyvsp[(1) - (1)].ival)); ;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 248 "Syn.y"
    { (yyval.sval) = (char*)malloc(20); sprintf((yyval.sval), "%f", (yyvsp[(1) - (1)].fval)); ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 250 "Syn.y"
    { (yyval.sval) = (char*)malloc(20); sprintf((yyval.sval), "%d", (yyvsp[(3) - (4)].ival)); ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 252 "Syn.y"
    { (yyval.sval) = (char*)malloc(20); sprintf((yyval.sval), "%d", -(yyvsp[(3) - (4)].ival)); ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 254 "Syn.y"
    { (yyval.sval) = (char*)malloc(20); sprintf((yyval.sval), "%f", (yyvsp[(3) - (4)].fval)); ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 256 "Syn.y"
    { (yyval.sval) = (char*)malloc(20); sprintf((yyval.sval), "%f", -(yyvsp[(3) - (4)].fval)); ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 272 "Syn.y"
    { yyerrok; ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 277 "Syn.y"
    { (yyval.ival) = semantic_errors; ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 282 "Syn.y"
    {
            int expr_has_error = (semantic_errors > (yyvsp[(3) - (5)].ival));

            if (!ts_est_declare((yyvsp[(1) - (5)].sval))) {
                printf(RED "ERREUR semantique: variable '%s' non declaree, "
                       "ligne %d, col %d" RESET "\n", (yyvsp[(1) - (5)].sval), nb_ligne, nb_col);
                semantic_errors++;
            } else if (ts_est_constante((yyvsp[(1) - (5)].sval))) {
                printf(RED "ERREUR semantique: modification de la constante '%s', "
                       "ligne %d, col %d" RESET "\n", (yyvsp[(1) - (5)].sval), nb_ligne, nb_col);
                semantic_errors++;
            } else if (expr_has_error) {
                /* expression invalide : pas d'affectation */
            } else {
                const char *type_dest = ts_get_type((yyvsp[(1) - (5)].sval));
                const char *type_src  = ts_get_type((yyvsp[(4) - (5)].sval));
                if (type_dest != NULL && type_src != NULL
                    && strcmp(type_dest, type_src) != 0) {
                    printf(RED "ERREUR semantique: incompatibilite de types pour "
                           "'%s' (%s) <-- expression (%s), ligne %d, col %d"
                           RESET "\n",
                           (yyvsp[(1) - (5)].sval), type_dest, type_src, nb_ligne, nb_col);
                    semantic_errors++;
                } else {
                    quadr(":=", (yyvsp[(4) - (5)].sval), "", (yyvsp[(1) - (5)].sval));
                    ts_marquer_init((yyvsp[(1) - (5)].sval));
                    printf("Affectation correcte.\n");
                }
            }
        ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 313 "Syn.y"
    {
            int expr_has_error = (semantic_errors > (yyvsp[(3) - (8)].ival));

            if (!ts_est_declare((yyvsp[(1) - (8)].sval))) {
                printf(RED "ERREUR semantique: tableau '%s' non declare, "
                       "ligne %d, col %d" RESET "\n", (yyvsp[(1) - (8)].sval), nb_ligne, nb_col);
                semantic_errors++;
            } else if (expr_has_error) {
                /* index ou valeur invalide */
            } else {
                int idx_val = atoi((yyvsp[(4) - (8)].sval));
                int taille  = ts_get_taille((yyvsp[(1) - (8)].sval));

                if (idx_val < 0) {
                    printf(RED "ERREUR semantique: index negatif (%d) pour le "
                           "tableau '%s', ligne %d, col %d" RESET "\n",
                           idx_val, (yyvsp[(1) - (8)].sval), nb_ligne, nb_col);
                    semantic_errors++;
                } else if (taille > 0 && idx_val >= taille) {
                    printf(RED "ERREUR semantique: index (%d) hors limites pour "
                           "le tableau '%s' (taille %d), ligne %d, col %d"
                           RESET "\n",
                           idx_val, (yyvsp[(1) - (8)].sval), taille, nb_ligne, nb_col);
                    semantic_errors++;
                } else {
                    char dest[100];
                    sprintf(dest, "%s[%s]", (yyvsp[(1) - (8)].sval), (yyvsp[(4) - (8)].sval));
                    quadr(":=", (yyvsp[(7) - (8)].sval), "", dest);
                    printf("Affectation tableau correcte.\n");
                }
            }
        ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 350 "Syn.y"
    {
            quadr("BZ", (yyvsp[(3) - (7)].sval), "", "");
        ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 359 "Syn.y"
    {
            quadr("BR", "", "", "");
            /* patcher le BZ vers le debut du else */
            char idx_str[20];
            sprintf(idx_str, "%d", qc);
            int _i;
            for (_i = qc - 2; _i >= 0; _i--) {
                if (strcmp(quad[_i].oper, "BZ") == 0 && strlen(quad[_i].res) == 0) {
                    updateQuad(_i, 3, idx_str);
                    break;
                }
            }
        ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 374 "Syn.y"
    {
            /* patcher le BR vers apres le else */
            char idx_str[20];
            sprintf(idx_str, "%d", qc);
            int _i;
            for (_i = qc - 1; _i >= 0; _i--) {
                if (strcmp(quad[_i].oper, "BR") == 0 && strlen(quad[_i].res) == 0) {
                    updateQuad(_i, 3, idx_str);
                    break;
                }
            }
            printf("Instruction if-else correcte.\n");
        ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 388 "Syn.y"
    {
            char idx_str[20];
            sprintf(idx_str, "%d", qc);
            int _i;
            for (_i = qc - 1; _i >= 0; _i--) {
                if (strcmp(quad[_i].oper, "BZ") == 0 && strlen(quad[_i].res) == 0) {
                    updateQuad(_i, 3, idx_str);
                    break;
                }
            }
            printf("Instruction if correcte.\n");
        ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 404 "Syn.y"
    {
            if (while_top < MAX_WHILE_NEST - 1) {
                while_top++;
                while_start_stack[while_top] = qc;
                while_bz_stack[while_top] = -1;
            }
      ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 412 "Syn.y"
    {
            quadr("BZ", (yyvsp[(5) - (6)].sval), "", "");
            if (while_top >= 0)
                while_bz_stack[while_top] = qc - 1;
        ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 419 "Syn.y"
    {
            if (while_top >= 0) {
                char debut_str[20];
                sprintf(debut_str, "%d", while_start_stack[while_top]);
                quadr("BR", "", "", debut_str);

                char fin_str[20];
                sprintf(fin_str, "%d", qc);
                if (while_bz_stack[while_top] >= 0)
                    updateQuad(while_bz_stack[while_top], 3, fin_str);

                while_top--;
            }
            printf("Boucle while correcte.\n");
        ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 453 "Syn.y"
    {
            if (!ts_est_declare((yyvsp[(2) - (6)].sval))) {
                printf(RED "ERREUR semantique: variable '%s' non declaree, ligne %d, col %d" RESET "\n",
                       (yyvsp[(2) - (6)].sval), nb_ligne, nb_col);
                semantic_errors++;
            }

            /* Initialiser : var <- debut */
            quadr(":=", (yyvsp[(4) - (6)].sval), "", (yyvsp[(2) - (6)].sval));
            ts_marquer_init((yyvsp[(2) - (6)].sval));

            /* Empiler le contexte for AVANT d'emettre la condition */
            if (for_top < MAX_FOR_NEST - 1) {
                for_top++;
                for_cond_stack[for_top] = qc;   /* indice du prochain quad = debut condition */
                for_bz_stack[for_top]   = -1;
            }

            /* Condition : var <= fin */
            char *tmp = strdup(nouveau_temp());
            quadr("<=", (yyvsp[(2) - (6)].sval), (yyvsp[(6) - (6)].sval), tmp);

            /* BZ de sortie (non patche pour l'instant) */
            quadr("BZ", tmp, "", "");
            if (for_top >= 0)
                for_bz_stack[for_top] = qc - 1;  /* indice du BZ qu'on vient d'emettre */
        ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 482 "Syn.y"
    {
            if (for_top >= 0) {
                /* Incrementer : var <- var + 1 */
                char *tmp = strdup(nouveau_temp());
                quadr("+", (yyvsp[(2) - (12)].sval), "1", tmp);
                quadr(":=", tmp, "", (yyvsp[(2) - (12)].sval));

                /* BR vers le debut REEL de la condition (enregistre sur la pile) */
                char cond_str[20];
                sprintf(cond_str, "%d", for_cond_stack[for_top]);
                quadr("BR", "", "", cond_str);

                /* Patcher le BZ de CE for vers apres le BR (sortie boucle) */
                char fin_str[20];
                sprintf(fin_str, "%d", qc);
                if (for_bz_stack[for_top] >= 0)
                    updateQuad(for_bz_stack[for_top], 3, fin_str);

                for_top--;
            }
            printf("Boucle for correcte.\n");
        ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 508 "Syn.y"
    {
            if (!ts_est_declare((yyvsp[(3) - (5)].sval))) {
                printf(RED "ERREUR semantique: variable '%s' non declaree, ligne %d, col %d" RESET "\n",
                       (yyvsp[(3) - (5)].sval), nb_ligne, nb_col);
                semantic_errors++;
            } else {
                quadr("input", "", "", (yyvsp[(3) - (5)].sval));
                ts_marquer_init((yyvsp[(3) - (5)].sval));
                printf("Instruction input correcte.\n");
            }
        ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 523 "Syn.y"
    { printf("Instruction out correcte.\n"); ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 532 "Syn.y"
    { quadr("out", (yyvsp[(1) - (1)].sval), "", ""); ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 534 "Syn.y"
    {
            if (!ts_est_declare((yyvsp[(1) - (1)].sval))) {
                printf(RED "ERREUR semantique: variable '%s' non declaree, "
                       "ligne %d, col %d" RESET "\n", (yyvsp[(1) - (1)].sval), nb_ligne, nb_col);
                semantic_errors++;
            } else if (!ts_est_initialise((yyvsp[(1) - (1)].sval))) {
                printf(RED "ERREUR semantique: variable '%s' utilisee sans "
                       "initialisation, ligne %d, col %d" RESET "\n",
                       (yyvsp[(1) - (1)].sval), nb_ligne, nb_col);
                semantic_errors++;
            } else {
                quadr("out", (yyvsp[(1) - (1)].sval), "", "");
            }
        ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 549 "Syn.y"
    {
            char s[20]; sprintf(s, "%d", (yyvsp[(1) - (1)].ival));
            quadr("out", s, "", "");
        ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 554 "Syn.y"
    {
            char s[20]; sprintf(s, "%f", (yyvsp[(1) - (1)].fval));
            quadr("out", s, "", "");
        ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 562 "Syn.y"
    { (yyval.sval) = strdup(nouveau_temp()); quadr("==", (yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].sval), (yyval.sval)); ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 564 "Syn.y"
    { (yyval.sval) = strdup(nouveau_temp()); quadr("!=", (yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].sval), (yyval.sval)); ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 566 "Syn.y"
    { (yyval.sval) = strdup(nouveau_temp()); quadr("<",  (yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].sval), (yyval.sval)); ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 568 "Syn.y"
    { (yyval.sval) = strdup(nouveau_temp()); quadr(">",  (yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].sval), (yyval.sval)); ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 570 "Syn.y"
    { (yyval.sval) = strdup(nouveau_temp()); quadr("<=", (yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].sval), (yyval.sval)); ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 572 "Syn.y"
    { (yyval.sval) = strdup(nouveau_temp()); quadr(">=", (yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].sval), (yyval.sval)); ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 574 "Syn.y"
    { (yyval.sval) = strdup(nouveau_temp()); quadr("AND", (yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].sval), (yyval.sval)); ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 576 "Syn.y"
    { (yyval.sval) = strdup(nouveau_temp()); quadr("OR",  (yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].sval), (yyval.sval)); ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 578 "Syn.y"
    { (yyval.sval) = strdup(nouveau_temp()); quadr("NON", (yyvsp[(3) - (4)].sval), "",  (yyval.sval)); ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 580 "Syn.y"
    { (yyval.sval) = (yyvsp[(2) - (3)].sval); ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 585 "Syn.y"
    { (yyval.sval) = strdup(nouveau_temp()); quadr("+", (yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].sval), (yyval.sval)); ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 587 "Syn.y"
    { (yyval.sval) = strdup(nouveau_temp()); quadr("-", (yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].sval), (yyval.sval)); ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 589 "Syn.y"
    { (yyval.sval) = strdup(nouveau_temp()); quadr("*", (yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].sval), (yyval.sval)); ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 591 "Syn.y"
    {
            int div_zero = (strcmp((yyvsp[(3) - (3)].sval), "0") == 0 || strcmp((yyvsp[(3) - (3)].sval), "0.0") == 0);
            if (!div_zero) {
                const char *v = ts_get_val((yyvsp[(3) - (3)].sval));
                if (v && (strcmp(v, "0") == 0 || strcmp(v, "0.0") == 0))
                    div_zero = 1;
            }
            if (div_zero) {
                printf(RED "ERREUR semantique: division par zero (diviseur '%s' vaut 0), ligne %d, col %d" RESET "\n",
                       (yyvsp[(3) - (3)].sval), nb_ligne, nb_col);
                semantic_errors++;
            }
            (yyval.sval) = strdup(nouveau_temp()); quadr("/", (yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].sval), (yyval.sval));
        ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 606 "Syn.y"
    { (yyval.sval) = (yyvsp[(2) - (3)].sval); ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 608 "Syn.y"
    { (yyval.sval) = (yyvsp[(3) - (4)].sval); ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 610 "Syn.y"
    { (yyval.sval) = strdup(nouveau_temp()); quadr("NEG", (yyvsp[(3) - (4)].sval), "", (yyval.sval)); ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 612 "Syn.y"
    {
            if (!ts_est_declare((yyvsp[(1) - (1)].sval))) {
                printf(RED "ERREUR semantique: variable '%s' non declaree, ligne %d, col %d" RESET "\n",
                       (yyvsp[(1) - (1)].sval), nb_ligne, nb_col);
                semantic_errors++;
            } else if (!ts_est_initialise((yyvsp[(1) - (1)].sval))) {
                /* BUG 5 CORRIGE : verifier l'initialisation aussi dans les expressions
                   arithmetiques, pas uniquement dans out(). */
                printf(RED "ERREUR semantique: variable '%s' utilisee sans initialisation, ligne %d, col %d" RESET "\n",
                       (yyvsp[(1) - (1)].sval), nb_ligne, nb_col);
                semantic_errors++;
            }
            (yyval.sval) = (yyvsp[(1) - (1)].sval);
        ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 627 "Syn.y"
    {
            if (!ts_est_declare((yyvsp[(1) - (4)].sval))) {
                printf(RED "ERREUR semantique: tableau '%s' non declare, ligne %d, col %d" RESET "\n",
                       (yyvsp[(1) - (4)].sval), nb_ligne, nb_col);
                semantic_errors++;
            }
            (yyval.sval) = strdup(nouveau_temp());
            quadr("TAB", (yyvsp[(1) - (4)].sval), (yyvsp[(3) - (4)].sval), (yyval.sval));
        ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 637 "Syn.y"
    {
            (yyval.sval) = (char*)malloc(20);
            sprintf((yyval.sval), "%d", (yyvsp[(1) - (1)].ival));
        ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 642 "Syn.y"
    {
            (yyval.sval) = (char*)malloc(20);
            sprintf((yyval.sval), "%f", (yyvsp[(1) - (1)].fval));
        ;}
    break;



/* Line 1455 of yacc.c  */
#line 2305 "Syn.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 648 "Syn.y"


int main()
{
    ts_initialiser();
    yyparse();
    ts_liberer();
    return 0;
}

int yywrap()
{
    return 1;
}

int yyerror(char *msg)
{
    printf(RED "ERREUR syntaxique: %s, ligne %d, col %d" RESET "\n", msg, nb_ligne, nb_col);
    return 1;
}

