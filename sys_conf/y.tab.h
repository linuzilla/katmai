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

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    RW_FLAG_ON = 258,
    RW_FLAG_OFF = 259,
    RW_IF = 260,
    RW_ELSE = 261,
    RW_ELSE_IF = 262,
    IDENTIFIER = 263,
    DIGIT = 264,
    QSTRING = 265,
    FQSTRING = 266,
    RW_INCREASE = 267,
    RW_LOWER = 268,
    RW_NORMAL = 269,
    SQ_CHARACTER = 270,
    PERCENT = 271,
    RW_SPEICAL_STDRULE = 272,
    RW_SWITCH = 273,
    RW_CASE = 274,
    RW_BREAK = 275,
    RW_DEFAULT = 276,
    RW_PRINT = 277,
    RW_EXIT = 278
  };
#endif
/* Tokens.  */
#define RW_FLAG_ON 258
#define RW_FLAG_OFF 259
#define RW_IF 260
#define RW_ELSE 261
#define RW_ELSE_IF 262
#define IDENTIFIER 263
#define DIGIT 264
#define QSTRING 265
#define FQSTRING 266
#define RW_INCREASE 267
#define RW_LOWER 268
#define RW_NORMAL 269
#define SQ_CHARACTER 270
#define PERCENT 271
#define RW_SPEICAL_STDRULE 272
#define RW_SWITCH 273
#define RW_CASE 274
#define RW_BREAK 275
#define RW_DEFAULT 276
#define RW_PRINT 277
#define RW_EXIT 278

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
