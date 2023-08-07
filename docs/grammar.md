
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
      <td><code>mut</code></td>
      <td><code>nil</code></td>
    </tr>
    <tr>
      <td><code>return</code></td>
      <td><code>struct</code></td>
      <td><code>true</code></td>
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

```
chunk                ::= statement* EOF

statement            ::= import_statement
                       | variable_declaration ';'
                       | assign_call ';'
                       | struct_declaration
                       | function_declaration
                       | delete_statement
                       | if_statement
                       | match_statement
                       | loop_statement
                       | while_statement
                       | for_statement
                       | break_statement
                       | return_statement
                       | block

import_statement     ::= 'import' name ( 'as' name )? ';'
                       | 'import' STRING 'as' name ';'
                       | 'import' '{' name ( ',' name )* '}' 'from' ( name | STRING ) ';'

variable_declaration ::= 'let' name '=' expression
                       | 'mut' name ( '=' expression )?
                       | ( 'let' | 'mut' ) '[' '_' | name ( ',' '_' | name )* ']' '=' expression
                       | ( 'let' | 'mut' ) '{' name ( ',' name )* '}' '=' expression

assign_call          ::= name subscript* assign_op expression
                       | name subscript* ( '++' | '--' )
                       | name subscript* '[' ']' '=' expression
                       | name subscript* subscript '=' expression
                       | name ( subscript | call )* call

struct_declaration   ::= 'struct' name '{' ( string | name ( ',' string | name )* )? '}'

function_declaration ::= 'fn' name '(' ( 'mut'? name ( ',' 'mut'? name )* )? ')' ( '=>' expression | block )

delete_statement     ::= 'del' name subscript* '[' expression ']' ';'

if_statement         ::= ( 'if' | 'if!' ) '(' ( variable_declaration ';' )? expression ')' statement ( 'else' statement )?

match_statement      ::= 'match' '(' ( variable_declaration ';' )? expression ')' '{' ( expression '=>' statement )+ ( '_' '=>' statement )? '}'

loop_statement       ::= 'loop' statement

while_statement      ::= ( 'while' | 'while!' ) '(' expression ')' statement
                       | 'do' statement ( 'while' | 'while!' ) '(' expression ')' ';'

for_statement        ::= 'for' '(' variable_declaration | assign_call? ';' expression? ';' assign_call? ')' statement
                       | 'foreach' '(' name 'in' expression ')' statement

break_statement      ::= ( 'break' | 'continue' ) ';'

return_statement     ::= 'return' expression? ';'

block                ::= '{' stmt* '}'

assign_op            ::= '=' | '|=' | '^=' | '&=' | '<<=' | '>>=' 
                       | '+=' | '-=' | '*=' | '/=' | '~/=' | '%='

subscript            ::= '[' expression ']' | '.' name

call                 ::= '(' ( expression ( ',' expression )* )? ')'

expression           ::= and_expression ( '||' and_expression )*

and_expression       ::= equal_expression ( '&&' equal_expression )*

equal_expression     ::= comp_expression ( ( '==' | '!=' ) comp_expression )*

comp_expression      ::= bor_expression ( ( '>' | '>=' | '<' | '<=' ) bor_expression )*

bor_expression       ::= bxor_expression ( '|' bxor_expression )*

bxor_expression      ::= band_expression ( '^' band_expression )*

band_expression      ::= shift_expression ( '&' shift_expression )*

shift_expression     ::= range_expression ( ( '<<' | '>>' ) range_expression )*

range_expression     ::= add_expression ( '..' add_expression )?

add_expression       ::= mul_expression ( ( '+' | '-' ) mul_expression )*

mul_expression       ::= unary_expression ( ( '*' | '/' | '~/' | '%' ) unary_expression )*

unary_expression     ::= ( '-' | '!' | '~' ) unary_expression | primary_expression

primary_expression   ::= literal
                       | array_constructor
                       | struct_constructor
                       | anonymous_struct
                       | anonymous_function
                       | if_expression
                       | match_expression
                       | subscript_call
                       | group_expression

literal              ::= 'nil' | 'false' | 'true' | number | string

array_constructor    ::= '[' ( expression ( ',' expression )* )? ']'

struct_constructor   ::= '{' ( string | name ':' expression ( ',' string | name ':' expression )* )? '}'

anonymous_struct     ::= 'struct' '{' ( string | name ( ',' string | name )* )? '}'

anonymous_function   ::= '|' ( 'mut'? name ( ',' 'mut'? name )* )? '|' ( '=>' expression | block )
                       | '||' ( '=>' expression | block )

if_expression        ::= ( 'if' | 'if!' ) '(' expression ')' expression 'else' expression

match_expression     ::= 'match' '(' expression ')' '{' expression '=>' expression ( ',' expression '=>' expression )*
                         ',' '_' '=>' expression '}'

subscript_call       ::= name ( subscript | call )* ( '{' ( expression ( ',' expression )* )? '}' )?

group_expression     ::= '(' expression ')'
```
