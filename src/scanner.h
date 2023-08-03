//
// The Hook Programming Language
// scanner.h
//

#ifndef SCANNER_H
#define SCANNER_H

#include <hook/string.h>

typedef enum
{
  TOKEN_EOF,    TOKEN_DOTDOT,       TOKEN_DOT,        TOKEN_COMMA,  TOKEN_COLON,     TOKEN_SEMICOLON, TOKEN_LPAREN,
  TOKEN_RPAREN, TOKEN_LBRACKET,     TOKEN_RBRACKET,   TOKEN_LBRACE, TOKEN_RBRACE,    TOKEN_PIPEEQ,    TOKEN_PIPEPIPE,
  TOKEN_PIPE,   TOKEN_CARETEQ,      TOKEN_CARET,      TOKEN_AMPEQ,  TOKEN_AMPAMP,    TOKEN_AMP,       TOKEN_ARROW,
  TOKEN_EQEQ,   TOKEN_EQ,           TOKEN_BANGEQ,     TOKEN_BANG,   TOKEN_GTEQ,      TOKEN_GTGTEQ,    TOKEN_GTGT,
  TOKEN_GT,     TOKEN_LTEQ,         TOKEN_LTLTEQ,     TOKEN_LTLT,   TOKEN_LT,        TOKEN_PLUSEQ,    TOKEN_PLUSPLUS,
  TOKEN_PLUS,   TOKEN_DASHEQ,       TOKEN_DASHDASH,   TOKEN_DASH,   TOKEN_STAREQ,    TOKEN_STAR,      TOKEN_SLASHEQ,
  TOKEN_SLASH,  TOKEN_TILDESLASHEQ, TOKEN_TILDESLASH, TOKEN_TILDE,  TOKEN_PERCENTEQ, TOKEN_PERCENT,   TOKEN_INT,
  TOKEN_FLOAT,  TOKEN_STRING,       TOKEN_UNDERSCORE, TOKEN_AS,     TOKEN_BREAK,     TOKEN_CONTINUE,  TOKEN_DEL,
  TOKEN_DO,     TOKEN_ELSE,         TOKEN_FALSE,      TOKEN_FN,     TOKEN_FOR,       TOKEN_FOREACH,   TOKEN_FROM,
  TOKEN_IF,     TOKEN_IFBANG,       TOKEN_IMPORT,     TOKEN_IN,     TOKEN_LET,       TOKEN_LOOP,      TOKEN_MATCH,
  TOKEN_MUT,    TOKEN_NIL,          TOKEN_RETURN,     TOKEN_STRUCT, TOKEN_TRUE,      TOKEN_WHILE,     TOKEN_WHILEBANG,
  TOKEN_NAME
} TokenType;

typedef struct
{
  TokenType type;
  int line;
  int col;
  int length;
  char *start;
} Token;

typedef struct
{
  HkString *file;
  HkString *source;
  char *pos;
  int line;
  int col;
  Token token;
} Scanner;

void scanner_init(Scanner *scan, HkString *file, HkString *source);
void scanner_deinit(Scanner *scan);
void scanner_next_token(Scanner *scan);

#endif // SCANNER_H
