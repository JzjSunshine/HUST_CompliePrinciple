/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

#ifndef YY_YY_PARSER_TAB_H_INCLUDED
# define YY_YY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    INT = 258,
    ID = 259,
    RELOP = 260,
    TYPE = 261,
    FLOAT = 262,
    CHAR = 263,
    STRING = 264,
    STRUCT = 265,
    LP = 266,
    RP = 267,
    LC = 268,
    RC = 269,
    SEMI = 270,
    COMMA = 271,
    LB = 272,
    RB = 273,
    DOT = 274,
    PLUS = 275,
    PLUS_ONE = 276,
    ONE_PLUS = 277,
    MINUS_ONE = 278,
    ONE_MINUS = 279,
    MINUS = 280,
    STAR = 281,
    DIV = 282,
    AND = 283,
    OR = 284,
    NOT = 285,
    IF = 286,
    ELSE = 287,
    WHILE = 288,
    RETURN = 289,
    FOR = 290,
    ASSIGNOP = 291,
    ASSIGNOP_MINUS = 292,
    ASSIGNOP_PLUS = 293,
    ASSIGNOP_DIV = 294,
    ASSIGNOP_STAR = 295,
    STRUCT_DEF = 296,
    STRUCT_DEC = 297,
    STRUCT_TAG = 298,
    BREAK_NODE = 299,
    CONTINUE_NODE = 300,
    FOR_DEC = 301,
    EXT_DEF_LIST = 302,
    EXT_VAR_DEF = 303,
    EXT_STRUCT_DEF = 304,
    FUNC_DEF = 305,
    FUNC_DEC = 306,
    EXT_DEC_LIST = 307,
    PARAM_LIST = 308,
    PARAM_DEC = 309,
    VAR_DEF = 310,
    DEC_LIST = 311,
    DEF_LIST = 312,
    COMP_STM = 313,
    STM_LIST = 314,
    EXP_STMT = 315,
    IF_THEN = 316,
    IF_THEN_ELSE = 317,
    FUNC_CALL = 318,
    ARGS = 319,
    FUNCTION = 320,
    PARAM = 321,
    ARG = 322,
    CALL = 323,
    LABEL = 324,
    GOTO = 325,
    JLT = 326,
    JLE = 327,
    JGT = 328,
    JGE = 329,
    EQ = 330,
    NEQ = 331,
    EXP_ELE = 332,
    EXP_ARRAY = 333,
    ARRAY_DEC = 334,
    BREAK = 335,
    CONTINUE = 336,
    UMINUS = 337,
    LOWER_THEN_ELSE = 338
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 17 "parser.y" /* yacc.c:1909  */

	int    type_int;
	float  type_float;
        char   type_char[3];
        char   type_string[31];
	char   type_id[32];
	struct ASTNode *ptr;

#line 147 "parser.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;
int yyparse (void);

#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */
