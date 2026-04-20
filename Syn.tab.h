
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
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

/* Line 1676 of yacc.c  */
#line 28 "Syn.y"

    int    ival;
    float  fval;
    char*  sval;



/* Line 1676 of yacc.c  */
#line 112 "Syn.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


