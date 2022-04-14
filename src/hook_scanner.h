//
// Hook Programming Language
// hook_scanner.h
//

#ifndef HOOK_SCANNER_H
#define HOOK_SCANNER_H

#include "hook_string.h"

#define TOKEN_EOF          0x00
#define TOKEN_DOT          0x01
#define TOKEN_DOTDOT       0x02
#define TOKEN_COMMA        0x03
#define TOKEN_COLON        0x04
#define TOKEN_SEMICOLON    0x05
#define TOKEN_LPAREN       0x06
#define TOKEN_RPAREN       0x07
#define TOKEN_LBRACKET     0x08
#define TOKEN_RBRACKET     0x09
#define TOKEN_LBRACE       0x0a
#define TOKEN_RBRACE       0x0b
#define TOKEN_PIPEPIPE     0x0c
#define TOKEN_AMPAMP       0x0d
#define TOKEN_ARROW        0x0e
#define TOKEN_EQ           0x0f
#define TOKEN_EQEQ         0x10
#define TOKEN_BANG         0x11
#define TOKEN_BANGEQ       0x12
#define TOKEN_GT           0x13
#define TOKEN_GTEQ         0x14
#define TOKEN_LT           0x15
#define TOKEN_LTEQ         0x16
#define TOKEN_PLUS         0x17
#define TOKEN_PLUSPLUS     0x18
#define TOKEN_PLUSEQ       0x19
#define TOKEN_MINUS        0x1a
#define TOKEN_MINUSMINUS   0x1b
#define TOKEN_MINUSEQ      0x1c
#define TOKEN_STAR         0x1d
#define TOKEN_STAREQ       0x1e
#define TOKEN_SLASH        0x1f
#define TOKEN_SLASHEQ      0x20
#define TOKEN_TILDESLASH   0x21
#define TOKEN_TILDESLASHEQ 0x22
#define TOKEN_PERCENT      0x23
#define TOKEN_PERCENTEQ    0x24
#define TOKEN_INT          0x25
#define TOKEN_FLOAT        0x26
#define TOKEN_STRING       0x27
#define TOKEN_UNDERSCORE   0x28
#define TOKEN_AS           0x29
#define TOKEN_BREAK        0x2a
#define TOKEN_CONTINUE     0x2b
#define TOKEN_DEL          0x2c
#define TOKEN_DO           0x2d
#define TOKEN_ELSE         0x2e
#define TOKEN_FALSE        0x2f
#define TOKEN_FN           0x30
#define TOKEN_FOR          0x31
#define TOKEN_FROM         0x32
#define TOKEN_IF           0x33
#define TOKEN_LOOP         0x34
#define TOKEN_MATCH        0x35
#define TOKEN_MUT          0x36
#define TOKEN_NIL          0x37
#define TOKEN_RETURN       0x38
#define TOKEN_STRUCT       0x39
#define TOKEN_TRUE         0x3a
#define TOKEN_USE          0x3b
#define TOKEN_VAL          0x3c
#define TOKEN_WHILE        0x3d
#define TOKEN_NAME         0x3e

typedef struct
{
  int32_t type;
  int32_t line;
  int32_t col;
  int32_t length;
  char *start;
} token_t;

typedef struct
{
  hk_string_t *file;
  hk_string_t *source;
  char *pos;
  int32_t line;
  int32_t col;
  token_t token;
} scanner_t;

void scanner_init(scanner_t *scan, hk_string_t *file, hk_string_t *source);
void scanner_free(scanner_t *scan);
void scanner_next_token(scanner_t *scan);

#endif // HOOK_SCANNER_H
