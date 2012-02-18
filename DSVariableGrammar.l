%top{
/**
 * \file DSVariableTokenizerLex.c
 * \brief Implementation file with functions for tokenizing matrices, generated
 *        by flex.
 *
 * \details This file was generated directly by the flex program, and is the
 * source code responsible for matrix tokenization.  This file was generated
 * by flex, according to a specification written by Jason Lomnitz.  To generate
 * this file, the following command must be executed:
 * "flex -t DSVariableGrammar.l > DSVariableTokenizerLex.c".
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
%option extra-type="struct variable_token *"
%option prefix="DSVariableFlex"

%{
#include "DSTypes.h"
#include "DSMemoryManager.h"
#include "DSVariable.h"
#include "DSVariableTokenizer.h"
        
#define malloc(x)      DSSecureMalloc(x)
#define calloc(x, y)   DSSecureCalloc(x, y)
#define realloc(x, y)  DSSecureRealloc(x, y)
        
%}

%%
\"inf\"             {
        struct variable_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSVariableTokenAlloc();
        token = token->next;
        DSVariableTokenSetType(token, DS_VARIABLE_TOKEN_DOUBLE);
        DSVariableTokenSetDouble(token, INFINITY);
        yyset_extra(token, yyscanner);
}

inf             {
        struct variable_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSVariableTokenAlloc();
        token = token->next;
        DSVariableTokenSetType(token, DS_VARIABLE_TOKEN_DOUBLE);
        DSVariableTokenSetDouble(token, INFINITY);
        yyset_extra(token, yyscanner);
}

\"\-inf\"             {
        struct variable_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSVariableTokenAlloc();
        token = token->next;
        DSVariableTokenSetType(token, DS_VARIABLE_TOKEN_DOUBLE);
        DSVariableTokenSetDouble(token, -INFINITY);
        yyset_extra(token, yyscanner);
}

\-inf             {
        struct variable_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSVariableTokenAlloc();
        token = token->next;
        DSVariableTokenSetType(token, DS_VARIABLE_TOKEN_DOUBLE);
        DSVariableTokenSetDouble(token, -INFINITY);
        yyset_extra(token, yyscanner);
}

\"      {
struct variable_token *token = yyget_extra(yyscanner);
if (token == NULL) {
DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
}
token->next = DSVariableTokenAlloc();
token = token->next;
DSVariableTokenSetType(token, DS_VARIABLE_TOKEN_QUOTE);
DSVariableTokenSetDouble(token, 0.0);
yyset_extra(token, yyscanner);
}

[A-Za-z_\\]+[A-Za-z_\\0-9]* {
        struct variable_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSVariableTokenAlloc();
        token = token->next;
        DSVariableTokenSetType(token, DS_VARIABLE_TOKEN_ID);
        DSVariableTokenSetString(token, strdup(yytext));
        yyset_extra(token, yyscanner);
}


\-?[0-9]+[\.[0-9]*]?[eE][\-]?[0-9]+ {
        double temp;
        struct variable_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        sscanf(yytext, "%lf", &temp);
        token->next = DSVariableTokenAlloc();
        token = token->next;
        DSVariableTokenSetType(token, DS_VARIABLE_TOKEN_DOUBLE);
        DSVariableTokenSetDouble(token, temp);
        yyset_extra(token, yyscanner);
}

\-?[0-9]+[\.[0-9]*]?  {
        double temp;
        struct variable_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        sscanf(yytext, "%lf", &temp);
        token->next = DSVariableTokenAlloc();
        token = token->next;
        DSVariableTokenSetType(token, DS_VARIABLE_TOKEN_DOUBLE);
        DSVariableTokenSetDouble(token, temp);
        yyset_extra(token, yyscanner);
}

\,               {
        struct variable_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSVariableTokenAlloc();
        token = token->next;
        DSVariableTokenSetType(token, DS_VARIABLE_TOKEN_SEPERATOR);
        DSVariableTokenSetDouble(token, 0.0);
        yyset_extra(token, yyscanner);
}

\:               {
        struct variable_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSVariableTokenAlloc();
        token = token->next;
        DSVariableTokenSetType(token, DS_VARIABLE_TOKEN_ASSIGN);
        DSVariableTokenSetDouble(token, 0.0);
        yyset_extra(token, yyscanner);
}

\n              {/* Do nothing. */}

\t              {/* Do nothing. */}

.               {
        struct variable_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSVariableTokenAlloc();
        token = token->next;
        DSVariableTokenSetType(token, DS_VARIABLE_TOKEN_OTHER);
        DSVariableTokenSetDouble(token, 0.0);
        yyset_extra(token, yyscanner);
}
%%

int yywrap(yyscan_t  yyscanner) {
        return 1;
}

extern struct variable_token * DSVariablePoolTokenizeString(const char *string)
{
        struct variable_token * tokens = NULL;
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
        tokens = DSVariableTokenAlloc();
        yylex_init_extra(tokens, &scanner);
        buffer = yy_scan_string(string, scanner);
        yylex (scanner);
        yy_delete_buffer(buffer, scanner);
        yylex_destroy ( scanner );
        bail:
        return tokens;
}