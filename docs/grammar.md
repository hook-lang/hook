
# Hook's grammar

The purpose of this document is to provide a comprehensive guide to the lexical and syntactic grammar of Hook, using the [EBNF](https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form) (Extended Backus-Naur Form) notation to describe both grammars. By following this guide, you will gain a thorough understanding of the rules and conventions that govern the Hook language.

## Lexical grammar

The lexical grammar in Hook defines the set of valid tokens, which are the building blocks of the language. This includes literals, names, keywords and other elements that make up the Hook lexicon.

### Literal numbers

Hook supports two types of literal numbers: integers and floating-point numbers. Both types of numbers are represented in the same way, with the only difference being that integers do not have a fractional part. Below you can find the EBNF grammar:

```
number        ::= ( '0' | nonzero_digit ) digit* fraction? exponent?
nonzero_digit ::= digit - '0'
digit         ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
fraction      ::= '.' digit+
exponent      ::= ( 'e' | 'E' ) ( '+' | '-' )? digit+
```

### Literal strings

In Hook, literal strings are sequences of characters enclosed in double quotes. They can contain any character except newline, carriage return, double quote, and backslash. The strings are encoded using UTF-8, which means they can include any Unicode code point.

To include special characters within strings, Hook provides escape sequences that are sequences of backslash followed by a character. The supported escape sequences include:

| Escape sequence | Resulting character  |
|-----------------|----------------------|
| `\"`            | Double quote         |
| `\\`            | Backslash            |
| `\b`            | Backspace            |
| `\f`            | Form feed            |
| `\n`            | Newline              |
| `\r`            | Carriage return      |
| `\t`            | Tab                  |

The following EBNF grammar describes the structure of string literals:

```
string ::= '"' ( char | escape )* '"'
char   ::= CODE_POINT - '\n' - '\r' - '"' - '\'
escape ::= '\' ( '"' | '\' | 'b' | 'f' | 'n' | 'r' | 't' )
```

Here, `CODE_POINT` refers to any Unicode code point.

### Names

Names, also known as identifiers, are sequences of ASCII characters used to name variables, functions, and other entities in Hook. To define a name in Hook, you can use any combination of letters, digits, and underscores. However, a name must start with a letter or an underscore. Hook follows the same rules for naming variables as the C programming language.

The following EBNF grammar describes the structure of names:

```
name ::= ( LETTER | '_' ) ( LETTER | digit | '_' )*
```

Here, `LETTER` refers to any uppercase or lowercase letter in the ASCII character set, while `digit` was defined earlier in this document.

### Keywords

Keywords are reserved words that have a special meaning and cannot be used as names for variables, functions, or other entities. The following table lists all the keywords in Hook:

<table>
  <tbody>
    <tr>
      <td><code>as</code></td>
      <td><code>break</code></td>
      <td><code>continue</code></td>
      <td><code>del</code></td>
      <td><code>do</code></td>
    </tr>
    <tr>
      <td><code>else</code></td>
      <td><code>false</code></td>
      <td><code>fn</code></td>
      <td><code>for</code></td>
      <td><code>foreach</code></td>
    </tr>
    <tr>
      <td><code>from</code></td>
      <td><code>if</code></td>
      <td><code>if!</code></td>
      <td><code>import</code></td>
      <td><code>in</code></td>
    </tr>
    <tr>
      <td><code>let</code></td>
      <td><code>loop</code></td>
      <td><code>match</code></td>
      <td><code>nil</code></td>
      <td><code>return</code></td>
    </tr>
    <tr>
      <td><code>struct</code></td>
      <td><code>true</code></td>
      <td><code>var</code></td>
      <td><code>while</code></td>
      <td><code>while!</code></td>
    </tr>
  </tbody>
</table>

### Other tokens

Besides literals, names, and keywords, Hook uses several other tokens consisting of one or two ASCII characters. These tokens have special meanings in the language.

<table>
  <tbody>
    <tr>
      <td><code>..</code></td>
      <td><code>..</code></td>
      <td><code>,</code></td>
      <td><code>:</code></td>
      <td><code>;</code></td>
      <td><code>(</code></td>
      <td><code>)</code></td>
    </tr>
    <tr>
      <td><code>[</code></td>
      <td><code>]</code></td>
      <td><code>{</code></td>
      <td><code>}</code></td>
      <td><code>|=</code></td>
      <td><code>||</code></td>
      <td><code>|</code></td>
    </tr>
    <tr>
      <td><code>^=</code></td>
      <td><code>^</code></td>
      <td><code>&=</code></td>
      <td><code>&&</code></td>
      <td><code>&</code></td>
      <td><code>=></code></td>
      <td><code>==</code></td>
    </tr>
    <tr>
      <td><code>=</code></td>
      <td><code>!=</code></td>
      <td><code>!</code></td>
      <td><code>>=</code></td>
      <td><code>>>=</code></td>
      <td><code>>></code></td>
      <td><code>></code></td>
    </tr>
    <tr>
      <td><code><=</code></td>
      <td><code><<=</code></td>
      <td><code><<</code></td>
      <td><code><</code></td>
      <td><code>+=</code></td>
      <td><code>++</code></td>
      <td><code>+</code></td>
    </tr>
    <tr>
      <td><code>-=</code></td>
      <td><code>--</code></td>
      <td><code>-</code></td>
      <td><code>*=</code></td>
      <td><code>*</code></td>
      <td><code>/=</code></td>
      <td><code>/</code></td>
    </tr>
    <tr>
      <td><code>~/=</code></td>
      <td><code>~/</code></td>
      <td><code>~</code></td>
      <td><code>%=</code></td>
      <td><code>%</code></td>
      <td><code>_</code></td>
      <td></td>
    </tr>
  </tbody>
</table>

In addition, Hook uses a special token to indicate the end of a file. This token is represented by the `'\0'` character.

## Syntatic grammar

The complete syntactic grammar of Hook is defined by the following EBNF grammar:

```ebnf
chunk                ::= stmt* EOF

stmt                 ::= import_stmt
                       | var_decl ';'
                       | assign_call ';'
                       | struct_decl
                       | fn_decl
                       | del_stmt
                       | if_stmt
                       | match_stmt
                       | loop_stmt
                       | while_stmt
                       | for_stmt
                       | break_stmt
                       | return_stmt
                       | block

import_stmt          ::= 'import' NAME ( 'as' NAME )? ';'
                       | 'import' STRING 'as' NAME ';'
                       | 'import' '{' NAME ( ',' NAME )* '}' 'from' ( NAME | STRING ) ';'

var_decl             ::= 'let' NAME '=' expr
                       | 'var' NAME ( '=' expr )?
                       | ( 'let' | 'var' ) '[' '_' | NAME ( ',' '_' | NAME )* ']' '=' expr
                       | ( 'let' | 'var' ) '{' NAME ( ',' NAME )* '}' '=' expr

assign_call          ::= NAME subsc* assign_op expr
                       | NAME subsc* ( '++' | '--' )
                       | NAME subsc* '[' ']' '=' expr
                       | NAME subsc* subsc '=' expr
                       | NAME ( subsc | call )* call

struct_decl          ::= 'struct' NAME '{' ( string | NAME ( ',' string | NAME )* )? '}'

fn_decl              ::= 'fn' NAME '(' ( NAME ( ',' NAME )* )? ')' ( '=>' expr ";" | block )

del_stmt             ::= 'del' NAME subsc* '[' expr ']' ';'

if_stmt              ::= ( 'if' | 'if!' ) '(' ( var_decl ';' )? expr ')'
                         stmt ( 'else' stmt )?

match_stmt           ::= 'match' '(' ( var_decl ';' )? expr ')'
                         '{' ( expr '=>' stmt )+ ( '_' '=>' stmt )? '}'

loop_stmt            ::= 'loop' stmt

while_stmt           ::= ( 'while' | 'while!' ) '(' expr ')' stmt
                       | 'do' stmt ( 'while' | 'while!' ) '(' expr ')' ';'

for_stmt             ::= 'for' '(' ( var_decl | assign_call )? ';' expr?
                         ';' assign_call? ')' stmt
                       | 'foreach' '(' NAME 'in' expr ')' stmt

break_stmt           ::= ( 'break' | 'continue' ) ';'

return_stmt          ::= 'return' expr? ';'

block                ::= '{' stmt* '}'

assign_op            ::= '=' | '|=' | '^=' | '&=' | '<<=' | '>>=' 
                       | '+=' | '-=' | '*=' | '/=' | '~/=' | '%='

subsc                ::= '[' expr ']' | '.' NAME

call                 ::= '(' ( expr ( ',' expr )* )? ')'

expr                 ::= and_expr ( '||' and_expr )*

and_expr             ::= equal_expr ( '&&' equal_expr )*

equal_expr           ::= comp_expr ( ( '==' | '!=' ) comp_expr )*

comp_expr            ::= bor_expr ( ( '>' | '>=' | '<' | '<=' ) bor_expr )*

bor_expr             ::= bxor_expr ( '|' bxor_expr )*

bxor_expr            ::= band_expr ( '^' band_expr )*

band_expr            ::= shift_expr ( '&' shift_expr )*

shift_expr           ::= range_expr ( ( '<<' | '>>' ) range_expr )*

range_expr           ::= add_expr ( '..' add_expr )?

add_expr             ::= mul_expr ( ( '+' | '-' ) mul_expr )*

mul_expr             ::= unary_expr ( ( '*' | '/' | '~/' | '%' ) unary_expr )*

unary_expr           ::= ( '-' | '!' | '~' ) unary_expr | primary_expr

primary_expr         ::= literal
                       | array_constructor
                       | struct_constructor
                       | anonymous_struct
                       | anonymous_fn
                       | if_expr
                       | match_expr
                       | subsc_call
                       | group_expr

literal              ::= 'nil' | 'false' | 'true' | number | string

array_constructor    ::= '[' ( expr ( ',' expr )* )? ']'

struct_constructor   ::= '{' ( string | NAME ':' expr ( ',' string | NAME ':' expr )* )? '}'

anonymous_struct     ::= 'struct' '{' ( string | NAME ( ',' string | NAME )* )? '}'

anonymous_fn         ::= '|' ( NAME ( ',' NAME )* )? '|' ( '=>' expr | block )
                       | '||' ( '=>' expr | block )

if_expr              ::= ( 'if' | 'if!' ) '(' expr ')' expr 'else' expr

match_expr           ::= 'match' '(' expr ')' '{' expr '=>' expr ( ',' expr '=>' expr )*
                         ',' '_' '=>' expr '}'

subsc_call           ::= NAME ( subsc | call )* ( '{' ( expr ( ',' expr )* )? '}' )?

group_expr           ::= '(' expr ')'
```
