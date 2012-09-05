%top{
/**
* \file DSExpressionTokenizerLex.c
* \brief Implementation file with functions for tokenizing matrices, generated
*        by flex.
*
* \details This file was generated directly by the flex program, and is the
* source code responsible for matrix tokenization.  This file was generated
* by flex, according to a specification written by Jason Lomnitz.  To generate
* this file, the following command must be executed:
* "flex -t DSExpressionGrammar.l > DSExpressionTokenizerLex.c".
*
* Copyright (C) 2011 Jason Lomnitz.\n\n
*
* This file is part of the Design Space Toolbox V2 (C Library).
*
* The Design Space Toolbox V2 is free software: you can redistribute it and/or 
* modify it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* The Design Space Toolbox V2 is distributed in the hope that it will be 
* useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with the Design Space Toolbox. If not, see 
* <http://www.gnu.org/licenses/>.
*
* \author Jason Lomnitz.
* \date 2011
*/
}

%option reentrant
%option extra-type="struct expression_token *"
%option prefix="DSExpressionFlex"

%{
#include "DSTypes.h"
#include "DSMemoryManager.h"
#include "DSExpression.h"
#include "DSExpressionTokenizer.h"

#define malloc(x)      DSSecureMalloc(x)
#define calloc(x, y)   DSSecureCalloc(x, y)
#define realloc(x, y)  DSSecureRealloc(x, y)

extern DSExpression * dsExpressionAllocWithOperator(const char op_code);
extern DSExpression * dsExpressionAllocWithConstant(const double value);
extern DSExpression * dsExpressionAllocWithVariableName(const char * name);

%}

%%
inf             {
        struct expression_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSExpressionTokenAlloc();
        token = token->next;
        DSExpressionTokenSetType(token, DS_EXPRESSION_TOKEN_DOUBLE);
        DSExpressionTokenSetDouble(token, INFINITY);
        yyset_extra(token, yyscanner);
}

[A-Za-z_\\]+[A-Za-z_\\0-9]* {
        struct expression_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSExpressionTokenAlloc();
        token = token->next;
        DSExpressionTokenSetType(token, DS_EXPRESSION_TOKEN_ID);
        DSExpressionTokenSetString(token, strdup(yytext));
        yyset_extra(token, yyscanner);
}

[0-9]+[\.[0-9]*]?E\-?[0-9]+ {
        double temp;
        struct expression_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        sscanf(yytext, "%lf", &temp);
        token->next = DSExpressionTokenAlloc();
        token = token->next;
        DSExpressionTokenSetType(token, DS_EXPRESSION_TOKEN_DOUBLE);
        DSExpressionTokenSetDouble(token, temp);
        yyset_extra(token, yyscanner);
}

[0-9]+[\.[0-9]*]?       {
        double temp;
        struct expression_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        sscanf(yytext, "%lf", &temp);
        token->next = DSExpressionTokenAlloc();
        token = token->next;
        DSExpressionTokenSetType(token, DS_EXPRESSION_TOKEN_DOUBLE);
        DSExpressionTokenSetDouble(token, temp);
        yyset_extra(token, yyscanner);
}

\+              {
        struct expression_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSExpressionTokenAlloc();
        token = token->next;
        DSExpressionTokenSetType(token, DS_EXPRESSION_TOKEN_PLUS);
        yyset_extra(token, yyscanner);
}

\-              {
        struct expression_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSExpressionTokenAlloc();
        token = token->next;
        DSExpressionTokenSetType(token, DS_EXPRESSION_TOKEN_MINUS);
        yyset_extra(token, yyscanner);
}

\*              {
        struct expression_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSExpressionTokenAlloc();
        token = token->next;
        DSExpressionTokenSetType(token, DS_EXPRESSION_TOKEN_TIMES);
        yyset_extra(token, yyscanner);
}

\/              {
        struct expression_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSExpressionTokenAlloc();
        token = token->next;
        DSExpressionTokenSetType(token, DS_EXPRESSION_TOKEN_DIVIDE);
        yyset_extra(token, yyscanner);
}

\^              {
        struct expression_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSExpressionTokenAlloc();
        token = token->next;
        DSExpressionTokenSetType(token, DS_EXPRESSION_TOKEN_POWER);
        yyset_extra(token, yyscanner);
}

\(              {
        struct expression_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSExpressionTokenAlloc();
        token = token->next;
        DSExpressionTokenSetType(token, DS_EXPRESSION_TOKEN_LPAREN);
        yyset_extra(token, yyscanner);
}

\)              {
        struct expression_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSExpressionTokenAlloc();
        token = token->next;
        DSExpressionTokenSetType(token, DS_EXPRESSION_TOKEN_RPAREN);
        yyset_extra(token, yyscanner);
}

.               {/* Do nothing. */}
%%

int yywrap(yyscan_t  yyscanner) {
return 1;
}

extern struct expression_token * DSExpressionTokenizeString(const char *string)
{
        struct expression_token * tokens = NULL;
        yyscan_t scanner;
        YY_BUFFER_STATE buffer;
        if (string == NULL) {
                DSError(M_DS_WRONG ": Input string is NULL", A_DS_ERROR);
                goto bail;
        }
        if (strlen(string) == 0) {
                DSError(M_DS_WRONG ": Input string is empty", A_DS_WARN);
                goto bail;
        }
        tokens = DSExpressionTokenAlloc();
        yylex_init_extra(tokens, &scanner);
        buffer = yy_scan_string(string, scanner);
        yylex (scanner);
        yy_delete_buffer(buffer, scanner);
        yylex_destroy ( scanner );
        bail:
        return tokens;
}