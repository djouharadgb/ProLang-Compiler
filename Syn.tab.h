/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_SYN_TAB_H_INCLUDED
# define YY_YY_SYN_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    IDF = 258,                     /* IDF  */
    STRING = 259,                  /* STRING  */
    NUM_INT = 260,                 /* NUM_INT  */
    NUM_FLOAT = 261,               /* NUM_FLOAT  */
    BeginProject = 262,            /* BeginProject  */
    EndProject = 263,              /* EndProject  */
    Setup = 264,                   /* Setup  */
    Run = 265,                     /* Run  */
    DEFINE_MC = 266,               /* DEFINE_MC  */
    CONST_MC = 267,                /* CONST_MC  */
    INTEGER_MC = 268,              /* INTEGER_MC  */
    FLOAT_MC = 269,                /* FLOAT_MC  */
    IF_MC = 270,                   /* IF_MC  */
    THEN_MC = 271,                 /* THEN_MC  */
    ELSE_MC = 272,                 /* ELSE_MC  */
    ENDIF_MC = 273,                /* ENDIF_MC  */
    LOOP_MC = 274,                 /* LOOP_MC  */
    WHILE_MC = 275,                /* WHILE_MC  */
    ENDLOOP_MC = 276,              /* ENDLOOP_MC  */
    FOR_MC = 277,                  /* FOR_MC  */
    IN_MC = 278,                   /* IN_MC  */
    TO_MC = 279,                   /* TO_MC  */
    ENDFOR_MC = 280,               /* ENDFOR_MC  */
    INPUT_MC = 281,                /* INPUT_MC  */
    OUT_MC = 282,                  /* OUT_MC  */
    AND = 283,                     /* AND  */
    OR = 284,                      /* OR  */
    NON = 285,                     /* NON  */
    OP_ASSIGN = 286,               /* OP_ASSIGN  */
    OP_INIT = 287,                 /* OP_INIT  */
    OP_EQ = 288,                   /* OP_EQ  */
    OP_NE = 289,                   /* OP_NE  */
    OP_LT = 290,                   /* OP_LT  */
    OP_GT = 291,                   /* OP_GT  */
    OP_LE = 292,                   /* OP_LE  */
    OP_GE = 293,                   /* OP_GE  */
    OP_ADD = 294,                  /* OP_ADD  */
    OP_SUB = 295,                  /* OP_SUB  */
    OP_MUL = 296,                  /* OP_MUL  */
    OP_DIV = 297,                  /* OP_DIV  */
    SEP_SEMICOLON = 298,           /* SEP_SEMICOLON  */
    SEP_COLON = 299,               /* SEP_COLON  */
    SEP_COMMA = 300,               /* SEP_COMMA  */
    SEP_PIPE = 301,                /* SEP_PIPE  */
    SEP_LBRACKET = 302,            /* SEP_LBRACKET  */
    SEP_RBRACKET = 303,            /* SEP_RBRACKET  */
    SEP_LBRACE = 304,              /* SEP_LBRACE  */
    SEP_RBRACE = 305,              /* SEP_RBRACE  */
    SEP_LPAREN = 306,              /* SEP_LPAREN  */
    SEP_RPAREN = 307               /* SEP_RPAREN  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 52 "Syn.y"

    int    ival;
    float  fval;
    char*  sval;

#line 122 "Syn.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_SYN_TAB_H_INCLUDED  */
