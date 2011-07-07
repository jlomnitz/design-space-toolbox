%{
#include <stdio.h>
%}

%union {
        float number;
        char *string;
}

%token <string> IDENTIFIER
%token <number> CONSTANT
%start expression
%%

primary_expression
: IDENTIFIER                      {fprintf(stderr, "Identifier:%s (index i)\n", $1);}
| CONSTANT                        {fprintf(stderr, "Constant:%f\n", $1);}
| '(' additive_expression ')'     {fprintf(stderr, "(expression)\n");}
;


unary_expression
: primary_expression
| unary_operator unary_expression
;

unary_operator
: '+'
| '-' {fprintf(stderr, "neg\n");}
;

power_expression
: unary_expression
| power_expression '^' unary_expression {fprintf(stderr, "Power\n");}
;

multiplicative_expression
: power_expression
| multiplicative_expression '*' power_expression {fprintf(stderr, "Times\n");}
| multiplicative_expression '/' power_expression {fprintf(stderr, "Divide\n");}
;

additive_expression
: multiplicative_expression
| additive_expression '+' multiplicative_expression {fprintf(stderr, "Plus\n");}
| additive_expression '-' multiplicative_expression {fprintf(stderr, "Minus\n");}
;

expression
: '\n' {fprintf(stderr, "Done\n"); return 1;}
|additive_expression '\n' {fprintf(stderr, "Done\n"); return 1;}
;


%%

extern char yytext[];
extern int column;

yyerror(s)
char *s;
{
        fflush(stdout);
        printf("\n%*s\n%*s\n", column, "^", column, s);
}

main()
{
        char isDone = 0;
        char input[1000];
        while (!isDone) {
                yyparse();
                fflush(stdin);
                printf("Are you done?(Y/n) ");
                scanf("%s", input);
                if (strcmp(input, "Y") == 0) {
                        isDone = 1;
                } else {
                        scanf("%c", &isDone);
                        isDone = 0;
                }
        }
} 


