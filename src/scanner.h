//
// Hook Programming Language
// scanner.h
//

#ifndef SCANNER_H
#define SCANNER_H

typedef enum
{
  TOKEN_EOF,
  TOKEN_SEMICOLON,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_STAR,
  TOKEN_SLASH,
  TOKEN_PERCENT,
  TOKEN_ECHO,
  TOKEN_INT
} token_type_t;

typedef struct
{
  token_type_t type;
  int line;
  int col;
  int length;
  char *chars;
} token_t;

typedef struct
{
  char *pos;
  int line;
  int col;
  token_t token;
} scanner_t;

void scanner_init(scanner_t *scan, char *chars);
void scanner_next_token(scanner_t *scan);

#endif
