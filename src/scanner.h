//
// Hook Programming Language
// scanner.h
//

#ifndef SCANNER_H
#define SCANNER_H

#include "string.h"

typedef enum
{
  TOKEN_EOF,
  TOKEN_DOT,
  TOKEN_COMMA,
  TOKEN_COLON,
  TOKEN_SEMICOLON,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LBRACKET,
  TOKEN_RBRACKET,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_PIPEPIPE,
  TOKEN_AMPAMP,
  TOKEN_EQ,
  TOKEN_EQEQ,
  TOKEN_BANG,
  TOKEN_BANGEQ,
  TOKEN_GT,
  TOKEN_GTEQ,
  TOKEN_LT,
  TOKEN_LTEQ,
  TOKEN_PLUS,
  TOKEN_PLUSPLUS,
  TOKEN_PLUSEQ,
  TOKEN_MINUS,
  TOKEN_MINUSMINUS,
  TOKEN_MINUSEQ,
  TOKEN_STAR,
  TOKEN_STAREQ,
  TOKEN_SLASH,
  TOKEN_SLASHEQ,
  TOKEN_PERCENT,
  TOKEN_PERCENTEQ,
  TOKEN_INT,
  TOKEN_FLOAT,
  TOKEN_STRING,
  TOKEN_BREAK,
  TOKEN_CONST,
  TOKEN_CONTINUE,
  TOKEN_DELETE,
  TOKEN_DO,
  TOKEN_ELSE,
  TOKEN_FALSE,
  TOKEN_FN,
  TOKEN_FOR,
  TOKEN_IF,
  TOKEN_LET,
  TOKEN_LOOP,
  TOKEN_NULL,
  TOKEN_RETURN,
  TOKEN_STRUCT,
  TOKEN_TRUE,
  TOKEN_WHILE,
  TOKEN_NAME
} token_type_t;

typedef struct
{
  token_type_t type;
  int line;
  int col;
  int length;
  char *start;
} token_t;

typedef struct
{
  string_t *file;
  string_t *source;
  char *pos;
  int line;
  int col;
  token_t token;
} scanner_t;

void scanner_init(scanner_t *scan, string_t *file, string_t *source);
void scanner_free(scanner_t *scan);
void scanner_next_token(scanner_t *scan);

#endif
