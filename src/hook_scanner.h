//
// Hook Programming Language
// hook_scanner.h
//

#ifndef HOOK_SCANNER_H
#define HOOK_SCANNER_H

#include "hook_string.h"

typedef enum
{
  TOKEN_EOF,
  TOKEN_DOT,
  TOKEN_DOTDOT,
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
  TOKEN_ARROW,
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
  TOKEN_NUMBER,
  TOKEN_STRING,
  TOKEN_UNDERSCORE,
  TOKEN_AS,
  TOKEN_BREAK,
  TOKEN_CONTINUE,
  TOKEN_DEL,
  TOKEN_DO,
  TOKEN_ELSE,
  TOKEN_FALSE,
  TOKEN_FN,
  TOKEN_FOR,
  TOKEN_IF,
  TOKEN_IN,
  TOKEN_LOOP,
  TOKEN_MATCH,
  TOKEN_MUT,
  TOKEN_NIL,
  TOKEN_RETURN,
  TOKEN_STRUCT,
  TOKEN_TRUE,
  TOKEN_USE,
  TOKEN_VAL,
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
  hk_string_t *file;
  hk_string_t *source;
  char *pos;
  int line;
  int col;
  token_t token;
} scanner_t;

void scanner_init(scanner_t *scan, hk_string_t *file, hk_string_t *source);
void scanner_free(scanner_t *scan);
void scanner_next_token(scanner_t *scan);

#endif // HOOK_SCANNER_H
