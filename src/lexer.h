//
// lexer.h
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef LEXER_H
#define LEXER_H

#include "hook/string.h"

typedef enum
{
  TOKEN_KIND_EOF,          TOKEN_KIND_DOTDOT,     TOKEN_KIND_DOT,         TOKEN_KIND_COMMA,
  TOKEN_KIND_COLON,        TOKEN_KIND_SEMICOLON,  TOKEN_KIND_LPAREN,      TOKEN_KIND_RPAREN,
  TOKEN_KIND_LBRACKET,     TOKEN_KIND_RBRACKET,   TOKEN_KIND_LBRACE,      TOKEN_KIND_RBRACE,
  TOKEN_KIND_PIPEEQ,       TOKEN_KIND_PIPEPIPE,   TOKEN_KIND_PIPE,        TOKEN_KIND_CARETEQ,
  TOKEN_KIND_CARET,        TOKEN_KIND_AMPEQ,      TOKEN_KIND_AMPAMP,      TOKEN_KIND_AMP,
  TOKEN_KIND_ARROW,        TOKEN_KIND_EQEQ,       TOKEN_KIND_EQ,          TOKEN_KIND_BANGEQ,
  TOKEN_KIND_BANG,         TOKEN_KIND_GTEQ,       TOKEN_KIND_GTGTEQ,      TOKEN_KIND_GTGT,
  TOKEN_KIND_GT,           TOKEN_KIND_LTEQ,       TOKEN_KIND_LTLTEQ,      TOKEN_KIND_LTLT,
  TOKEN_KIND_LT,           TOKEN_KIND_PLUSEQ,     TOKEN_KIND_PLUSPLUS,    TOKEN_KIND_PLUS,
  TOKEN_KIND_DASHEQ,       TOKEN_KIND_DASHDASH,   TOKEN_KIND_DASH,        TOKEN_KIND_STAREQ,
  TOKEN_KIND_STAR,         TOKEN_KIND_SLASHEQ,    TOKEN_KIND_SLASH,       TOKEN_KIND_TILDESLASHEQ,
  TOKEN_KIND_TILDESLASH,   TOKEN_KIND_TILDE,      TOKEN_KIND_PERCENTEQ,   TOKEN_KIND_PERCENT,
  TOKEN_KIND_INT,          TOKEN_KIND_FLOAT,      TOKEN_KIND_STRING,      TOKEN_KIND_UNDERSCORE_KW,
  TOKEN_KIND_AS_KW,        TOKEN_KIND_BREAK_KW,   TOKEN_KIND_CONTINUE_KW, TOKEN_KIND_DEL_KW,
  TOKEN_KIND_DO_KW,        TOKEN_KIND_ELSE_KW,    TOKEN_KIND_FALSE_KW,    TOKEN_KIND_FN_KW,
  TOKEN_KIND_FOR_KW,       TOKEN_KIND_FOREACH_KW, TOKEN_KIND_FROM_KW,     TOKEN_KIND_IF_KW,
  TOKEN_KIND_IFBANG_KW,    TOKEN_KIND_IMPORT_KW,  TOKEN_KIND_IN_KW,       TOKEN_KIND_LET_KW,
  TOKEN_KIND_LOOP_KW,      TOKEN_KIND_MATCH_KW,   TOKEN_KIND_NIL_KW,      TOKEN_KIND_RETURN_KW,
  TOKEN_KIND_STRUCT_KW,    TOKEN_KIND_TRUE_KW,    TOKEN_KIND_VAR_KW,      TOKEN_KIND_WHILE_KW,
  TOKEN_KIND_WHILEBANG_KW, TOKEN_KIND_NAME
} TokenKind;

typedef struct
{
  TokenKind kind;
  int       line;
  int       col;
  int       length;
  char      *start;
} Token;

typedef struct
{
  HkString *file;
  HkString *source;
  char     *pos;
  int      line;
  int      col;
  Token    token;
} Lexer;

void lexer_init(Lexer *lex, HkString *file, HkString *source);
void lexer_deinit(Lexer *lex);
void lexer_next_token(Lexer *lex);

#endif // LEXER_H
