

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+


%{
#include <stdio.h>
#include "y.tab.h"

void count();
%}

%%

[a-zA-Z][a-zA-Z0-9_]*   { count(); yylval.string=strdup(yytext);return IDENTIFIER;}
{D}{D}*                 { count(); sscanf(yytext, "%f", &yylval.number);return CONSTANT;  }
{D}*"."{D}+({E})?	{ count(); sscanf(yytext, "%f", &yylval.number);return(CONSTANT); }
{D}+"."{D}*({E})?	{ count(); sscanf(yytext, "%f", &yylval.number);return(CONSTANT); }

"+"                     { count(); return('+');}
"-"                     { count(); return('-');}
"*"                     { count(); return('*');}
"/"                     { count(); return('/');}
"^"                     { count(); return('^');}
";"                     { count(); return(';');}
"("                     { count(); return('(');}
")"                     { count(); return(')');}
"\n"                    { count(); return('\n');}
[ \t\v\f]		{ count(); }

.                       {return (int) yytext[0];}

%%

yywrap()
{
return(1);
}


comment()
{
char c, c1;

loop:
while ((c = input()) != '*' && c != 0)
putchar(c);

if ((c1 = input()) != '/' && c != 0)
{
unput(c1);
goto loop;
}

if (c != 0)
putchar(c1);
}


int column = 0;

void count()
{
        int i;

        for (i = 0; yytext[i] != '\0'; i++)
        if (yytext[i] == '\n')
                column = 0;
        else if (yytext[i] == '\t')
                column += 8 - (column % 8);
        else
        column++;

        ECHO;
}


int check_type()
{
/*
* pseudo code --- this is what it should check
*
*	if (yytext == type_name)
*		return(TYPE_NAME);
*
*	return(IDENTIFIER);
*/

/*
*	it actually will only return IDENTIFIER
*/

        return(IDENTIFIER);
}