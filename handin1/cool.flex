/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>
#include <string>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */

static std::string str_buf;

%}

/*
 * Define names for regular expressions here.
 */

DARROW          =>
CLASS           class
ELSE            else
FI              fi
IF              if
IN              in
INHERITS        inherits
LET             let
LOOP            loop
POOL            pool
THEN            then
WHILE           while
CASE            case
ESAC            esac
OF              of
NEW             new
ISVOID          isvoid
ASSIGN          <-
NOT             not
LE              <=


%x COMMENT

%x STRING
%x STRING_ESCAPE

%%


 /*
  *  Nested comments
  */

--.*$ {}

\(\* {
    BEGIN(COMMENT);
}


<COMMENT>\*\) {
    BEGIN(INITIAL);
}

\*\) {
    cool_yylval.error_msg = "Unmatched *)";
    return (ERROR);
}


<COMMENT><<EOF>> {
    BEGIN(INITIAL);
    cool_yylval.error_msg = "EOF in comment";
    return (ERROR);
}

<COMMENT>\n {
    curr_lineno++;
}

<COMMENT>. {}

 /*
  *  The multiple-character operators.
  */
{DARROW}		{ return (DARROW); }
{CLASS}     { return (CLASS); }
{ELSE}      { return (ELSE); }
{FI}        { return (FI); }
{IF}        { return (IF); }
{IN}        { return (IN); }
{INHERITS}  { return (INHERITS); }
{LET}       { return (LET); }
{LOOP}      { return (LOOP); }
{POOL}      { return (POOL); }
{THEN}      { return (THEN); }
{WHILE}     { return (WHILE); }
{CASE}      { return (CASE); }
{ESAC}      { return (ESAC); }
{OF}        { return (OF); }
{NEW}       { return (NEW); }
{ISVOID}    { return (ISVOID); }
{ASSIGN}    { return (ASSIGN); }
{NOT}       { return (NOT); }
{LE}        { return (LE); }


 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */

t[Rr][Uu][Ee] {
    cool_yylval.boolean = true;
    return (BOOL_CONST);
}

f[Aa][Ll][Ss][Ee] {
    cool_yylval.boolean = false;
    return (BOOL_CONST);
}

 /* Identifiers */
[A-Z_][A-Za-z0-9_]*  {
    cool_yylval.symbol = idtable.add_string(yytext, yyleng);
    return (TYPEID);
}
[a-z_][A-Za-z0-9_]*  {
    cool_yylval.symbol = idtable.add_string(yytext, yyleng);
    return (OBJECTID);
}

 /* Constants */
[0-9]+ {
    cool_yylval.symbol = inttable.add_string(yytext, yyleng);
    return (INT_CONST);
}

 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */

\" {
    str_buf.clear();
    BEGIN(STRING);
}

<STRING>[^\"\\]*\" {
    str_buf.insert(str_buf.end(), yytext, yytext + yyleng - 1);
    cool_yylval.symbol = stringtable.add_string(&str_buf[0], str_buf.size());
    BEGIN(INITIAL);
    return (STR_CONST);
}

<STRING>[^\"\\]*\\ {
    str_buf.insert(str_buf.end(), yytext, yytext + yyleng - 1);
    BEGIN(STRING_ESCAPE);
}

<STRING_ESCAPE>n {
    str_buf.push_back('\n');
    BEGIN(STRING);
}

<STRING_ESCAPE>t {
    str_buf.push_back('\t');
    BEGIN(STRING);
}

<STRING_ESCAPE>b {
    str_buf.push_back('\b');
    BEGIN(STRING);
}

<STRING_ESCAPE>f {
    str_buf.push_back('\f');
    BEGIN(STRING);
}

<STRING_ESCAPE>. {
    str_buf.push_back(yytext[0]);
    BEGIN(STRING);
}

<STRING_ESCAPE>\n {
    str_buf.push_back('\n');
    ++curr_lineno;
    BEGIN(STRING);
}

<STRING>[^\"\\]*$ {
    str_buf.insert(str_buf.end(), yytext, yytext + yyleng);
    cool_yylval.error_msg = "String Constant without ending";
    BEGIN(INITIAL);
    ++curr_lineno;
    return (ERROR);
}

<STRING,STRING_ESCAPE><<EOF>> {
    cool_yylval.error_msg = "String Constant Definition meets EOF";
    BEGIN(INITIAL);
    return (ERROR);
} 

[ \t\f\r\v]  {}

\n { 
  ++curr_lineno; 
}

 /* Invalid Chars */
[\[\]\'>\\] {
  cool_yylval.error_msg = yytext;
  return (ERROR);
}

 /* Any Other Chars */
. { return yytext[0]; }

%%
