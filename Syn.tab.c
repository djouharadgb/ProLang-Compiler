/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
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

#line 122 "Syn.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "Syn.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_IDF = 3,                        /* IDF  */
  YYSYMBOL_STRING = 4,                     /* STRING  */
  YYSYMBOL_NUM_INT = 5,                    /* NUM_INT  */
  YYSYMBOL_NUM_FLOAT = 6,                  /* NUM_FLOAT  */
  YYSYMBOL_BeginProject = 7,               /* BeginProject  */
  YYSYMBOL_EndProject = 8,                 /* EndProject  */
  YYSYMBOL_Setup = 9,                      /* Setup  */
  YYSYMBOL_Run = 10,                       /* Run  */
  YYSYMBOL_DEFINE_MC = 11,                 /* DEFINE_MC  */
  YYSYMBOL_CONST_MC = 12,                  /* CONST_MC  */
  YYSYMBOL_INTEGER_MC = 13,                /* INTEGER_MC  */
  YYSYMBOL_FLOAT_MC = 14,                  /* FLOAT_MC  */
  YYSYMBOL_IF_MC = 15,                     /* IF_MC  */
  YYSYMBOL_THEN_MC = 16,                   /* THEN_MC  */
  YYSYMBOL_ELSE_MC = 17,                   /* ELSE_MC  */
  YYSYMBOL_ENDIF_MC = 18,                  /* ENDIF_MC  */
  YYSYMBOL_LOOP_MC = 19,                   /* LOOP_MC  */
  YYSYMBOL_WHILE_MC = 20,                  /* WHILE_MC  */
  YYSYMBOL_ENDLOOP_MC = 21,                /* ENDLOOP_MC  */
  YYSYMBOL_FOR_MC = 22,                    /* FOR_MC  */
  YYSYMBOL_IN_MC = 23,                     /* IN_MC  */
  YYSYMBOL_TO_MC = 24,                     /* TO_MC  */
  YYSYMBOL_ENDFOR_MC = 25,                 /* ENDFOR_MC  */
  YYSYMBOL_INPUT_MC = 26,                  /* INPUT_MC  */
  YYSYMBOL_OUT_MC = 27,                    /* OUT_MC  */
  YYSYMBOL_AND = 28,                       /* AND  */
  YYSYMBOL_OR = 29,                        /* OR  */
  YYSYMBOL_NON = 30,                       /* NON  */
  YYSYMBOL_OP_ASSIGN = 31,                 /* OP_ASSIGN  */
  YYSYMBOL_OP_INIT = 32,                   /* OP_INIT  */
  YYSYMBOL_OP_EQ = 33,                     /* OP_EQ  */
  YYSYMBOL_OP_NE = 34,                     /* OP_NE  */
  YYSYMBOL_OP_LT = 35,                     /* OP_LT  */
  YYSYMBOL_OP_GT = 36,                     /* OP_GT  */
  YYSYMBOL_OP_LE = 37,                     /* OP_LE  */
  YYSYMBOL_OP_GE = 38,                     /* OP_GE  */
  YYSYMBOL_OP_ADD = 39,                    /* OP_ADD  */
  YYSYMBOL_OP_SUB = 40,                    /* OP_SUB  */
  YYSYMBOL_OP_MUL = 41,                    /* OP_MUL  */
  YYSYMBOL_OP_DIV = 42,                    /* OP_DIV  */
  YYSYMBOL_SEP_SEMICOLON = 43,             /* SEP_SEMICOLON  */
  YYSYMBOL_SEP_COLON = 44,                 /* SEP_COLON  */
  YYSYMBOL_SEP_COMMA = 45,                 /* SEP_COMMA  */
  YYSYMBOL_SEP_PIPE = 46,                  /* SEP_PIPE  */
  YYSYMBOL_SEP_LBRACKET = 47,              /* SEP_LBRACKET  */
  YYSYMBOL_SEP_RBRACKET = 48,              /* SEP_RBRACKET  */
  YYSYMBOL_SEP_LBRACE = 49,                /* SEP_LBRACE  */
  YYSYMBOL_SEP_RBRACE = 50,                /* SEP_RBRACE  */
  YYSYMBOL_SEP_LPAREN = 51,                /* SEP_LPAREN  */
  YYSYMBOL_SEP_RPAREN = 52,                /* SEP_RPAREN  */
  YYSYMBOL_YYACCEPT = 53,                  /* $accept  */
  YYSYMBOL_programme = 54,                 /* programme  */
  YYSYMBOL_partie_declaration = 55,        /* partie_declaration  */
  YYSYMBOL_liste_declarations = 56,        /* liste_declarations  */
  YYSYMBOL_declaration = 57,               /* declaration  */
  YYSYMBOL_decl_variable = 58,             /* decl_variable  */
  YYSYMBOL_suite_definition = 59,          /* suite_definition  */
  YYSYMBOL_decl_constante = 60,            /* decl_constante  */
  YYSYMBOL_liste_idf = 61,                 /* liste_idf  */
  YYSYMBOL_type = 62,                      /* type  */
  YYSYMBOL_valeur = 63,                    /* valeur  */
  YYSYMBOL_liste_instructions = 64,        /* liste_instructions  */
  YYSYMBOL_instruction = 65,               /* instruction  */
  YYSYMBOL_sem_checkpoint = 66,            /* sem_checkpoint  */
  YYSYMBOL_instruction_affectation = 67,   /* instruction_affectation  */
  YYSYMBOL_instruction_condition = 68,     /* instruction_condition  */
  YYSYMBOL_69_1 = 69,                      /* $@1  */
  YYSYMBOL_suite_if = 70,                  /* suite_if  */
  YYSYMBOL_71_2 = 71,                      /* $@2  */
  YYSYMBOL_instruction_loop_while = 72,    /* instruction_loop_while  */
  YYSYMBOL_73_3 = 73,                      /* $@3  */
  YYSYMBOL_74_4 = 74,                      /* $@4  */
  YYSYMBOL_instruction_for = 75,           /* instruction_for  */
  YYSYMBOL_76_5 = 76,                      /* $@5  */
  YYSYMBOL_instruction_input = 77,         /* instruction_input  */
  YYSYMBOL_instruction_output = 78,        /* instruction_output  */
  YYSYMBOL_liste_out = 79,                 /* liste_out  */
  YYSYMBOL_element_out = 80,               /* element_out  */
  YYSYMBOL_condition = 81,                 /* condition  */
  YYSYMBOL_expression = 82                 /* expression  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

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
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  194

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   307


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
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
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
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

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "IDF", "STRING",
  "NUM_INT", "NUM_FLOAT", "BeginProject", "EndProject", "Setup", "Run",
  "DEFINE_MC", "CONST_MC", "INTEGER_MC", "FLOAT_MC", "IF_MC", "THEN_MC",
  "ELSE_MC", "ENDIF_MC", "LOOP_MC", "WHILE_MC", "ENDLOOP_MC", "FOR_MC",
  "IN_MC", "TO_MC", "ENDFOR_MC", "INPUT_MC", "OUT_MC", "AND", "OR", "NON",
  "OP_ASSIGN", "OP_INIT", "OP_EQ", "OP_NE", "OP_LT", "OP_GT", "OP_LE",
  "OP_GE", "OP_ADD", "OP_SUB", "OP_MUL", "OP_DIV", "SEP_SEMICOLON",
  "SEP_COLON", "SEP_COMMA", "SEP_PIPE", "SEP_LBRACKET", "SEP_RBRACKET",
  "SEP_LBRACE", "SEP_RBRACE", "SEP_LPAREN", "SEP_RPAREN", "$accept",
  "programme", "partie_declaration", "liste_declarations", "declaration",
  "decl_variable", "suite_definition", "decl_constante", "liste_idf",
  "type", "valeur", "liste_instructions", "instruction", "sem_checkpoint",
  "instruction_affectation", "instruction_condition", "$@1", "suite_if",
  "$@2", "instruction_loop_while", "$@3", "$@4", "instruction_for", "$@5",
  "instruction_input", "instruction_output", "liste_out", "element_out",
  "condition", "expression", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-74)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-27)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
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

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
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

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -74,   -74,   -74,   223,   -74,   -74,   -74,   -74,   215,    62,
     201,   -46,   -74,   182,   -74,   -74,   -74,   -74,   -74,   -74,
     -74,   -74,   -74,   -74,   -74,   -74,   115,   -74,   -20,   -73
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     2,    11,    12,    13,    14,    30,    15,    18,    31,
      57,    45,    46,    74,    47,    48,   171,   182,   185,    49,
     120,   168,    50,   169,    51,    52,    90,    91,    81,    82
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
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

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
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

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
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

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
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


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
      if (yytable_value_is_error (yyn))
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* programme: BeginProject IDF SEP_SEMICOLON Setup SEP_COLON partie_declaration Run SEP_COLON SEP_LBRACE liste_instructions SEP_RBRACE EndProject SEP_SEMICOLON  */
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
        }
#line 1345 "Syn.tab.c"
    break;

  case 9: /* declaration: error SEP_SEMICOLON  */
#line 154 "Syn.y"
        {
            /* BUG 6 CORRIGE : liberer et reinitialiser la liste pending
               pour eviter que des noms fantomes contaminent la prochaine
               declaration valide. */
            int _i;
            for (_i = 0; _i < pending_count; _i++) free(pending_idfs[_i]);
            pending_count = 0;
            yyerrok;
        }
#line 1359 "Syn.tab.c"
    break;

  case 11: /* suite_definition: type SEP_SEMICOLON  */
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
        }
#line 1374 "Syn.tab.c"
    break;

  case 12: /* suite_definition: type OP_INIT valeur SEP_SEMICOLON  */
#line 182 "Syn.y"
        {
            int ok = 1;
            int _i;
            for (_i = 0; _i < pending_count; _i++) {
                if (!ts_inserer_variable(pending_idfs[_i], type_courant)) { ok = 0; free(pending_idfs[_i]); continue; }
                ts_marquer_init(pending_idfs[_i]);
                quadr(":=", (yyvsp[-1].sval), "", pending_idfs[_i]);
                ts_set_val(pending_idfs[_i], (yyvsp[-1].sval));
                free(pending_idfs[_i]);
            }
            pending_count = 0;
            if (ok) printf("Declaration de variable(s) avec initialisation correcte.\n");
        }
#line 1392 "Syn.tab.c"
    break;

  case 13: /* suite_definition: SEP_LBRACKET type SEP_SEMICOLON NUM_INT SEP_RBRACKET SEP_SEMICOLON  */
#line 196 "Syn.y"
        {
            if (pending_count != 1) {
                printf(RED "ERREUR semantique: un tableau ne peut avoir qu'un seul nom, ligne %d, col %d" RESET "\n",
                       nb_ligne, nb_col);
                semantic_errors++;
            } else if ((yyvsp[-2].ival) <= 0) {
                printf(RED "ERREUR semantique: taille de tableau invalide pour '%s', ligne %d, col %d" RESET "\n",
                       pending_idfs[0], nb_ligne, nb_col);
            } else {
                ts_inserer_tableau(pending_idfs[0], type_courant, (yyvsp[-2].ival));
                printf("Declaration de tableau correcte.\n");
            }
            int _i;
            for (_i = 0; _i < pending_count; _i++) free(pending_idfs[_i]);
            pending_count = 0;
        }
#line 1413 "Syn.tab.c"
    break;

  case 14: /* decl_constante: CONST_MC IDF SEP_COLON type OP_INIT valeur SEP_SEMICOLON  */
#line 216 "Syn.y"
        {
            if (ts_inserer_constante((yyvsp[-5].sval), type_courant)) {
                ts_marquer_init((yyvsp[-5].sval));
                quadr(":=", (yyvsp[-1].sval), "", (yyvsp[-5].sval));
                ts_set_val((yyvsp[-5].sval), (yyvsp[-1].sval));
                printf("Declaration de constante correcte.\n");
            }
        }
#line 1426 "Syn.tab.c"
    break;

  case 15: /* liste_idf: IDF  */
#line 228 "Syn.y"
        {
            if (pending_count < MAX_PENDING_IDF)
                pending_idfs[pending_count++] = strdup((yyvsp[0].sval));
        }
#line 1435 "Syn.tab.c"
    break;

  case 16: /* liste_idf: IDF SEP_PIPE liste_idf  */
#line 233 "Syn.y"
        {
            if (pending_count < MAX_PENDING_IDF)
                pending_idfs[pending_count++] = strdup((yyvsp[-2].sval));
        }
#line 1444 "Syn.tab.c"
    break;

  case 17: /* type: INTEGER_MC  */
#line 240 "Syn.y"
                   { type_courant = TYPE_INTEGER; }
#line 1450 "Syn.tab.c"
    break;

  case 18: /* type: FLOAT_MC  */
#line 241 "Syn.y"
                   { type_courant = TYPE_FLOAT;   }
#line 1456 "Syn.tab.c"
    break;

  case 19: /* valeur: NUM_INT  */
#line 246 "Syn.y"
        { (yyval.sval) = (char*)malloc(20); sprintf((yyval.sval), "%d", (yyvsp[0].ival)); }
#line 1462 "Syn.tab.c"
    break;

  case 20: /* valeur: NUM_FLOAT  */
#line 248 "Syn.y"
        { (yyval.sval) = (char*)malloc(20); sprintf((yyval.sval), "%f", (yyvsp[0].fval)); }
#line 1468 "Syn.tab.c"
    break;

  case 21: /* valeur: SEP_LPAREN OP_ADD NUM_INT SEP_RPAREN  */
#line 250 "Syn.y"
        { (yyval.sval) = (char*)malloc(20); sprintf((yyval.sval), "%d", (yyvsp[-1].ival)); }
#line 1474 "Syn.tab.c"
    break;

  case 22: /* valeur: SEP_LPAREN OP_SUB NUM_INT SEP_RPAREN  */
#line 252 "Syn.y"
        { (yyval.sval) = (char*)malloc(20); sprintf((yyval.sval), "%d", -(yyvsp[-1].ival)); }
#line 1480 "Syn.tab.c"
    break;

  case 23: /* valeur: SEP_LPAREN OP_ADD NUM_FLOAT SEP_RPAREN  */
#line 254 "Syn.y"
        { (yyval.sval) = (char*)malloc(20); sprintf((yyval.sval), "%f", (yyvsp[-1].fval)); }
#line 1486 "Syn.tab.c"
    break;

  case 24: /* valeur: SEP_LPAREN OP_SUB NUM_FLOAT SEP_RPAREN  */
#line 256 "Syn.y"
        { (yyval.sval) = (char*)malloc(20); sprintf((yyval.sval), "%f", -(yyvsp[-1].fval)); }
#line 1492 "Syn.tab.c"
    break;

  case 33: /* instruction: error SEP_SEMICOLON  */
#line 272 "Syn.y"
        { yyerrok; }
#line 1498 "Syn.tab.c"
    break;

  case 34: /* sem_checkpoint: %empty  */
#line 277 "Syn.y"
        { (yyval.ival) = semantic_errors; }
#line 1504 "Syn.tab.c"
    break;

  case 35: /* instruction_affectation: IDF OP_ASSIGN sem_checkpoint expression SEP_SEMICOLON  */
#line 282 "Syn.y"
        {
            int expr_has_error = (semantic_errors > (yyvsp[-2].ival));

            if (!ts_est_declare((yyvsp[-4].sval))) {
                printf(RED "ERREUR semantique: variable '%s' non declaree, "
                       "ligne %d, col %d" RESET "\n", (yyvsp[-4].sval), nb_ligne, nb_col);
                semantic_errors++;
            } else if (ts_est_constante((yyvsp[-4].sval))) {
                printf(RED "ERREUR semantique: modification de la constante '%s', "
                       "ligne %d, col %d" RESET "\n", (yyvsp[-4].sval), nb_ligne, nb_col);
                semantic_errors++;
            } else if (expr_has_error) {
                /* expression invalide : pas d'affectation */
            } else {
                const char *type_dest = ts_get_type((yyvsp[-4].sval));
                const char *type_src  = ts_get_type((yyvsp[-1].sval));
                if (type_dest != NULL && type_src != NULL
                    && strcmp(type_dest, type_src) != 0) {
                    printf(RED "ERREUR semantique: incompatibilite de types pour "
                           "'%s' (%s) <-- expression (%s), ligne %d, col %d"
                           RESET "\n",
                           (yyvsp[-4].sval), type_dest, type_src, nb_ligne, nb_col);
                    semantic_errors++;
                } else {
                    quadr(":=", (yyvsp[-1].sval), "", (yyvsp[-4].sval));
                    ts_marquer_init((yyvsp[-4].sval));
                    printf("Affectation correcte.\n");
                }
            }
        }
#line 1539 "Syn.tab.c"
    break;

  case 36: /* instruction_affectation: IDF SEP_LBRACKET sem_checkpoint expression SEP_RBRACKET OP_ASSIGN expression SEP_SEMICOLON  */
#line 313 "Syn.y"
        {
            int expr_has_error = (semantic_errors > (yyvsp[-5].ival));

            if (!ts_est_declare((yyvsp[-7].sval))) {
                printf(RED "ERREUR semantique: tableau '%s' non declare, "
                       "ligne %d, col %d" RESET "\n", (yyvsp[-7].sval), nb_ligne, nb_col);
                semantic_errors++;
            } else if (expr_has_error) {
                /* index ou valeur invalide */
            } else {
                int idx_val = atoi((yyvsp[-4].sval));
                int taille  = ts_get_taille((yyvsp[-7].sval));

                if (idx_val < 0) {
                    printf(RED "ERREUR semantique: index negatif (%d) pour le "
                           "tableau '%s', ligne %d, col %d" RESET "\n",
                           idx_val, (yyvsp[-7].sval), nb_ligne, nb_col);
                    semantic_errors++;
                } else if (taille > 0 && idx_val >= taille) {
                    printf(RED "ERREUR semantique: index (%d) hors limites pour "
                           "le tableau '%s' (taille %d), ligne %d, col %d"
                           RESET "\n",
                           idx_val, (yyvsp[-7].sval), taille, nb_ligne, nb_col);
                    semantic_errors++;
                } else {
                    char dest[100];
                    sprintf(dest, "%s[%s]", (yyvsp[-7].sval), (yyvsp[-4].sval));
                    quadr(":=", (yyvsp[-1].sval), "", dest);
                    printf("Affectation tableau correcte.\n");
                }
            }
        }
#line 1576 "Syn.tab.c"
    break;

  case 37: /* $@1: %empty  */
#line 350 "Syn.y"
        {
            quadr("BZ", (yyvsp[-4].sval), "", "");
        }
#line 1584 "Syn.tab.c"
    break;

  case 39: /* $@2: %empty  */
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
        }
#line 1602 "Syn.tab.c"
    break;

  case 40: /* suite_if: ELSE_MC $@2 SEP_LBRACE liste_instructions SEP_RBRACE ENDIF_MC SEP_SEMICOLON  */
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
        }
#line 1620 "Syn.tab.c"
    break;

  case 41: /* suite_if: ENDIF_MC SEP_SEMICOLON  */
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
        }
#line 1637 "Syn.tab.c"
    break;

  case 42: /* $@3: %empty  */
#line 404 "Syn.y"
      {
            if (while_top < MAX_WHILE_NEST - 1) {
                while_top++;
                while_start_stack[while_top] = qc;
                while_bz_stack[while_top] = -1;
            }
      }
#line 1649 "Syn.tab.c"
    break;

  case 43: /* $@4: %empty  */
#line 412 "Syn.y"
        {
            quadr("BZ", (yyvsp[-1].sval), "", "");
            if (while_top >= 0)
                while_bz_stack[while_top] = qc - 1;
        }
#line 1659 "Syn.tab.c"
    break;

  case 44: /* instruction_loop_while: LOOP_MC WHILE_MC SEP_LPAREN $@3 condition SEP_RPAREN $@4 SEP_LBRACE liste_instructions SEP_RBRACE ENDLOOP_MC SEP_SEMICOLON  */
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
        }
#line 1679 "Syn.tab.c"
    break;

  case 45: /* $@5: %empty  */
#line 453 "Syn.y"
        {
            if (!ts_est_declare((yyvsp[-4].sval))) {
                printf(RED "ERREUR semantique: variable '%s' non declaree, ligne %d, col %d" RESET "\n",
                       (yyvsp[-4].sval), nb_ligne, nb_col);
                semantic_errors++;
            }

            /* Initialiser : var <- debut */
            quadr(":=", (yyvsp[-2].sval), "", (yyvsp[-4].sval));
            ts_marquer_init((yyvsp[-4].sval));

            /* Empiler le contexte for AVANT d'emettre la condition */
            if (for_top < MAX_FOR_NEST - 1) {
                for_top++;
                for_cond_stack[for_top] = qc;   /* indice du prochain quad = debut condition */
                for_bz_stack[for_top]   = -1;
            }

            /* Condition : var <= fin */
            char *tmp = strdup(nouveau_temp());
            quadr("<=", (yyvsp[-4].sval), (yyvsp[0].sval), tmp);

            /* BZ de sortie (non patche pour l'instant) */
            quadr("BZ", tmp, "", "");
            if (for_top >= 0)
                for_bz_stack[for_top] = qc - 1;  /* indice du BZ qu'on vient d'emettre */
        }
#line 1711 "Syn.tab.c"
    break;

  case 46: /* instruction_for: FOR_MC IDF IN_MC expression TO_MC expression $@5 SEP_LBRACE liste_instructions SEP_RBRACE ENDFOR_MC SEP_SEMICOLON  */
#line 482 "Syn.y"
        {
            if (for_top >= 0) {
                /* Incrementer : var <- var + 1 */
                char *tmp = strdup(nouveau_temp());
                quadr("+", (yyvsp[-10].sval), "1", tmp);
                quadr(":=", tmp, "", (yyvsp[-10].sval));

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
        }
#line 1738 "Syn.tab.c"
    break;

  case 47: /* instruction_input: INPUT_MC SEP_LPAREN IDF SEP_RPAREN SEP_SEMICOLON  */
#line 508 "Syn.y"
        {
            if (!ts_est_declare((yyvsp[-2].sval))) {
                printf(RED "ERREUR semantique: variable '%s' non declaree, ligne %d, col %d" RESET "\n",
                       (yyvsp[-2].sval), nb_ligne, nb_col);
                semantic_errors++;
            } else {
                quadr("input", "", "", (yyvsp[-2].sval));
                ts_marquer_init((yyvsp[-2].sval));
                printf("Instruction input correcte.\n");
            }
        }
#line 1754 "Syn.tab.c"
    break;

  case 48: /* instruction_output: OUT_MC SEP_LPAREN liste_out SEP_RPAREN SEP_SEMICOLON  */
#line 523 "Syn.y"
        { printf("Instruction out correcte.\n"); }
#line 1760 "Syn.tab.c"
    break;

  case 51: /* element_out: STRING  */
#line 532 "Syn.y"
                 { quadr("out", (yyvsp[0].sval), "", ""); }
#line 1766 "Syn.tab.c"
    break;

  case 52: /* element_out: IDF  */
#line 534 "Syn.y"
        {
            if (!ts_est_declare((yyvsp[0].sval))) {
                printf(RED "ERREUR semantique: variable '%s' non declaree, "
                       "ligne %d, col %d" RESET "\n", (yyvsp[0].sval), nb_ligne, nb_col);
                semantic_errors++;
            } else if (!ts_est_initialise((yyvsp[0].sval))) {
                printf(RED "ERREUR semantique: variable '%s' utilisee sans "
                       "initialisation, ligne %d, col %d" RESET "\n",
                       (yyvsp[0].sval), nb_ligne, nb_col);
                semantic_errors++;
            } else {
                quadr("out", (yyvsp[0].sval), "", "");
            }
        }
#line 1785 "Syn.tab.c"
    break;

  case 53: /* element_out: NUM_INT  */
#line 549 "Syn.y"
        {
            char s[20]; sprintf(s, "%d", (yyvsp[0].ival));
            quadr("out", s, "", "");
        }
#line 1794 "Syn.tab.c"
    break;

  case 54: /* element_out: NUM_FLOAT  */
#line 554 "Syn.y"
        {
            char s[20]; sprintf(s, "%f", (yyvsp[0].fval));
            quadr("out", s, "", "");
        }
#line 1803 "Syn.tab.c"
    break;

  case 55: /* condition: expression OP_EQ expression  */
#line 562 "Syn.y"
        { (yyval.sval) = strdup(nouveau_temp()); quadr("==", (yyvsp[-2].sval), (yyvsp[0].sval), (yyval.sval)); }
#line 1809 "Syn.tab.c"
    break;

  case 56: /* condition: expression OP_NE expression  */
#line 564 "Syn.y"
        { (yyval.sval) = strdup(nouveau_temp()); quadr("!=", (yyvsp[-2].sval), (yyvsp[0].sval), (yyval.sval)); }
#line 1815 "Syn.tab.c"
    break;

  case 57: /* condition: expression OP_LT expression  */
#line 566 "Syn.y"
        { (yyval.sval) = strdup(nouveau_temp()); quadr("<",  (yyvsp[-2].sval), (yyvsp[0].sval), (yyval.sval)); }
#line 1821 "Syn.tab.c"
    break;

  case 58: /* condition: expression OP_GT expression  */
#line 568 "Syn.y"
        { (yyval.sval) = strdup(nouveau_temp()); quadr(">",  (yyvsp[-2].sval), (yyvsp[0].sval), (yyval.sval)); }
#line 1827 "Syn.tab.c"
    break;

  case 59: /* condition: expression OP_LE expression  */
#line 570 "Syn.y"
        { (yyval.sval) = strdup(nouveau_temp()); quadr("<=", (yyvsp[-2].sval), (yyvsp[0].sval), (yyval.sval)); }
#line 1833 "Syn.tab.c"
    break;

  case 60: /* condition: expression OP_GE expression  */
#line 572 "Syn.y"
        { (yyval.sval) = strdup(nouveau_temp()); quadr(">=", (yyvsp[-2].sval), (yyvsp[0].sval), (yyval.sval)); }
#line 1839 "Syn.tab.c"
    break;

  case 61: /* condition: condition AND condition  */
#line 574 "Syn.y"
        { (yyval.sval) = strdup(nouveau_temp()); quadr("AND", (yyvsp[-2].sval), (yyvsp[0].sval), (yyval.sval)); }
#line 1845 "Syn.tab.c"
    break;

  case 62: /* condition: condition OR condition  */
#line 576 "Syn.y"
        { (yyval.sval) = strdup(nouveau_temp()); quadr("OR",  (yyvsp[-2].sval), (yyvsp[0].sval), (yyval.sval)); }
#line 1851 "Syn.tab.c"
    break;

  case 63: /* condition: NON SEP_LPAREN condition SEP_RPAREN  */
#line 578 "Syn.y"
        { (yyval.sval) = strdup(nouveau_temp()); quadr("NON", (yyvsp[-1].sval), "",  (yyval.sval)); }
#line 1857 "Syn.tab.c"
    break;

  case 64: /* condition: SEP_LPAREN condition SEP_RPAREN  */
#line 580 "Syn.y"
        { (yyval.sval) = (yyvsp[-1].sval); }
#line 1863 "Syn.tab.c"
    break;

  case 65: /* expression: expression OP_ADD expression  */
#line 585 "Syn.y"
        { (yyval.sval) = strdup(nouveau_temp()); quadr("+", (yyvsp[-2].sval), (yyvsp[0].sval), (yyval.sval)); }
#line 1869 "Syn.tab.c"
    break;

  case 66: /* expression: expression OP_SUB expression  */
#line 587 "Syn.y"
        { (yyval.sval) = strdup(nouveau_temp()); quadr("-", (yyvsp[-2].sval), (yyvsp[0].sval), (yyval.sval)); }
#line 1875 "Syn.tab.c"
    break;

  case 67: /* expression: expression OP_MUL expression  */
#line 589 "Syn.y"
        { (yyval.sval) = strdup(nouveau_temp()); quadr("*", (yyvsp[-2].sval), (yyvsp[0].sval), (yyval.sval)); }
#line 1881 "Syn.tab.c"
    break;

  case 68: /* expression: expression OP_DIV expression  */
#line 591 "Syn.y"
        {
            int div_zero = (strcmp((yyvsp[0].sval), "0") == 0 || strcmp((yyvsp[0].sval), "0.0") == 0);
            if (!div_zero) {
                const char *v = ts_get_val((yyvsp[0].sval));
                if (v && (strcmp(v, "0") == 0 || strcmp(v, "0.0") == 0))
                    div_zero = 1;
            }
            if (div_zero) {
                printf(RED "ERREUR semantique: division par zero (diviseur '%s' vaut 0), ligne %d, col %d" RESET "\n",
                       (yyvsp[0].sval), nb_ligne, nb_col);
                semantic_errors++;
            }
            (yyval.sval) = strdup(nouveau_temp()); quadr("/", (yyvsp[-2].sval), (yyvsp[0].sval), (yyval.sval));
        }
#line 1900 "Syn.tab.c"
    break;

  case 69: /* expression: SEP_LPAREN expression SEP_RPAREN  */
#line 606 "Syn.y"
        { (yyval.sval) = (yyvsp[-1].sval); }
#line 1906 "Syn.tab.c"
    break;

  case 70: /* expression: SEP_LPAREN OP_ADD expression SEP_RPAREN  */
#line 608 "Syn.y"
        { (yyval.sval) = (yyvsp[-1].sval); }
#line 1912 "Syn.tab.c"
    break;

  case 71: /* expression: SEP_LPAREN OP_SUB expression SEP_RPAREN  */
#line 610 "Syn.y"
        { (yyval.sval) = strdup(nouveau_temp()); quadr("NEG", (yyvsp[-1].sval), "", (yyval.sval)); }
#line 1918 "Syn.tab.c"
    break;

  case 72: /* expression: IDF  */
#line 612 "Syn.y"
        {
            if (!ts_est_declare((yyvsp[0].sval))) {
                printf(RED "ERREUR semantique: variable '%s' non declaree, ligne %d, col %d" RESET "\n",
                       (yyvsp[0].sval), nb_ligne, nb_col);
                semantic_errors++;
            } else if (!ts_est_initialise((yyvsp[0].sval))) {
                /* BUG 5 CORRIGE : verifier l'initialisation aussi dans les expressions
                   arithmetiques, pas uniquement dans out(). */
                printf(RED "ERREUR semantique: variable '%s' utilisee sans initialisation, ligne %d, col %d" RESET "\n",
                       (yyvsp[0].sval), nb_ligne, nb_col);
                semantic_errors++;
            }
            (yyval.sval) = (yyvsp[0].sval);
        }
#line 1937 "Syn.tab.c"
    break;

  case 73: /* expression: IDF SEP_LBRACKET expression SEP_RBRACKET  */
#line 627 "Syn.y"
        {
            if (!ts_est_declare((yyvsp[-3].sval))) {
                printf(RED "ERREUR semantique: tableau '%s' non declare, ligne %d, col %d" RESET "\n",
                       (yyvsp[-3].sval), nb_ligne, nb_col);
                semantic_errors++;
            }
            (yyval.sval) = strdup(nouveau_temp());
            quadr("TAB", (yyvsp[-3].sval), (yyvsp[-1].sval), (yyval.sval));
        }
#line 1951 "Syn.tab.c"
    break;

  case 74: /* expression: NUM_INT  */
#line 637 "Syn.y"
        {
            (yyval.sval) = (char*)malloc(20);
            sprintf((yyval.sval), "%d", (yyvsp[0].ival));
        }
#line 1960 "Syn.tab.c"
    break;

  case 75: /* expression: NUM_FLOAT  */
#line 642 "Syn.y"
        {
            (yyval.sval) = (char*)malloc(20);
            sprintf((yyval.sval), "%f", (yyvsp[0].fval));
        }
#line 1969 "Syn.tab.c"
    break;


#line 1973 "Syn.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

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
