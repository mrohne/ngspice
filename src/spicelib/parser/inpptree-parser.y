%{

  #include <stdio.h>
  #include <stdlib.h>
  #include "inpptree-parser.h"

  extern int PTlex (YYSTYPE *lvalp, char **line);

  static void PTerror (char **line, struct INPparseNode **retval, void *ckt, char const *);

  #if defined (_MSC_VER)
  # define __func__ __FUNCTION__ /* __func__ is C99, but MSC can't */
  #endif


%}

%name-prefix="PT"
%output="inpptree-parser.c"

%defines

%pure-parser

%parse-param {char **line}
%lex-param   {char **line}

%parse-param {struct INPparseNode **retval}
%parse-param {void *ckt}

%union {
  double num;
  const char  *str;
  struct INPparseNode *pnode;
}

%token <num>  TOK_NUM
%token <str>  TOK_STR
%token        TOK_LE TOK_LT TOK_GE TOK_GT TOK_EQ TOK_NE

%type  <pnode>   exp nonempty_arglist

// Operator Precedence

%left   ','
%right  '?' ':'
%left   TOK_OR
%left   TOK_AND
%left   TOK_EQ TOK_NE
%left   TOK_LE TOK_LT TOK_GE TOK_GT
%left   '-' '+'
%left   '*' '/'
%left   '^'      /* exponentiation */
%left   NEG '!'  /* negation--unary minus, and boolean not */

%initial-action      /* initialize yylval */
{
    $$.num = 0.0;
};

%%

expression:
    exp { *retval = $1; }
  ;

exp:
    TOK_NUM                           { $$ = mknnode($1); }
  | TOK_STR                           { $$ = mksnode($1, ckt); }

  | exp '+' exp                       { $$ = mkbnode("+", $1, $3); }
  | exp '-' exp                       { $$ = mkbnode("-", $1, $3); }
  | exp '*' exp                       { $$ = mkbnode("*", $1, $3); }
  | exp '/' exp                       { $$ = mkbnode("/", $1, $3); }
  | exp '^' exp                       { $$ = mkbnode("^", $1, $3); }

  | '(' exp ')'                       { $$ = $2; }

  | '-' exp  %prec NEG                { $$ = mkfnode("-",$2); }

  | TOK_STR '(' nonempty_arglist ')'  { $$ = mkfnode($1, $3); }

  | exp '?' exp ':' exp               { $$ = mkfnode("ternary_fcn",
                                               mkbnode(",",
                                                 mkbnode(",", $1, $3),
                                                 $5)); }

  | exp TOK_EQ exp                    { $$ = mkfnode("eq0", mkbnode("-",$1,$3)); }
  | exp TOK_NE exp                    { $$ = mkfnode("ne0", mkbnode("-",$1,$3)); }
  | exp TOK_GT exp                    { $$ = mkfnode("gt0", mkbnode("-",$1,$3)); }
  | exp TOK_LT exp                    { $$ = mkfnode("lt0", mkbnode("-",$1,$3)); }
  | exp TOK_GE exp                    { $$ = mkfnode("ge0", mkbnode("-",$1,$3)); }
  | exp TOK_LE exp                    { $$ = mkfnode("le0", mkbnode("-",$1,$3)); }

  | exp TOK_OR exp                    { $$ = mkfnode("ne0",
                                               mkbnode("+",
                                                 mkfnode("ne0", $1),
                                                   mkfnode("ne0", $3))); }
  | exp TOK_AND exp                   { $$ = mkfnode("eq0",
                                               mkbnode("+",
                                                 mkfnode("eq0", $1),
                                                   mkfnode("eq0", $3))); }
  | '!' exp                           { $$ = mkfnode("eq0", $2); }

  ;

nonempty_arglist:
    exp
  | nonempty_arglist ',' exp     { $$ = mkbnode(",", $1, $3); }

%%


/* Called by yyparse on error.  */
static void
PTerror (char **line, struct INPparseNode **retval, void *ckt, char const *s)
{
  NG_IGNORE(line);
  NG_IGNORE(retval);
  NG_IGNORE(ckt);

  fprintf (stderr, "%s: %s\n", __func__, s);
}
