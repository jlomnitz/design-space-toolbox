/* To generate the appropriate source file, the following command must be
 * called within the project directory:
 * flex -t DSMatrixGrammar.l > DSMatrixTokenizerLex.c
 */
%option reentrant
%option extra-type="struct matrix_token *"
%option prefix="DSMatrixFlex"
%top{
/**
 * \file DSMatrixTokenizerLex.c
 * \brief Implementation file with functions for tokenizing matrices, generated
 *        by flex.
 *
 * \details This file was generated directly by the flex program, and is the
 * source code responsible for matrix tokenization.  This file was generated
 * by flex, according to a specification written by Jason Lomnitz.  To generate
 * this file, the following command must be executed:
 * "flex -t DSMatrixGrammar.l > DSMatrixTokenizerLex.c".
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
%{
#include "DSTypes.h"
#include "DSMemoryManager.h"
#include "DSMatrix.h"
#include "DSMatrixTokenizer.h"

#define malloc(x)      DSSecureMalloc(x)
#define calloc(x, y)   DSSecureCalloc(x, y)
#define realloc(x, y)  DSSecureRealloc(x, y)

%}

%%
\-?[0-9]+          {
        struct matrix_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSSecureCalloc(sizeof(struct matrix_token), 1);
        if (DSMatrixTokenType(token) == DS_MATRIX_TOKEN_START)
                token->next->row = token->row+1;
        else
                token->next->row = token->row;
        token->next->column = token->column+1;
        token = token->next;
        token->token = DS_MATRIX_TOKEN_DOUBLE;
        sscanf(yytext, "%lf", &(token->value));
        yyset_extra(token, yyscanner);
}

\-?[0-9]+\.[0-9]*  {
        struct matrix_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSSecureCalloc(sizeof(struct matrix_token), 1);
        if (DSMatrixTokenType(token) == DS_MATRIX_TOKEN_START)
                token->next->row = token->row+1;
        else
                token->next->row = token->row;
        token->next->column = token->column+1;
        token = token->next;
        token->token = DS_MATRIX_TOKEN_DOUBLE;
        sscanf(yytext, "%lf", &(token->value));
        yyset_extra(token, yyscanner);
}

\-?[0-9]+[\.[0-9]*]?E[\-]?[0-9]+ {
        struct matrix_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSSecureCalloc(sizeof(struct matrix_token), 1);
        if (DSMatrixTokenType(token) == DS_MATRIX_TOKEN_START)
        token->next->row = token->row+1;
        else
        token->next->row = token->row;
        token->next->column = token->column+1;
        token = token->next;
        token->token = DS_MATRIX_TOKEN_DOUBLE;
        sscanf(yytext, "%lf", &(token->value));
        yyset_extra(token, yyscanner);
}

\n+            {
        struct matrix_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        if (DSMatrixTokenType(token) == DS_MATRIX_TOKEN_DOUBLE) {
                token->next = DSSecureCalloc(sizeof(struct matrix_token), 1);
                token->next->row = token->row+1;
                token->next->column = 0;
                token = token->next;
                token->token = DS_MATRIX_TOKEN_NEWLINE;
                token->value = 0.0;
                yyset_extra(token, yyscanner);
        }
}

\,               {/* Do Nothing. */}
\ +             {/* Do Nothing. */}
\t+             {/* Do Nothing. */}

.               {
        struct matrix_token *token = yyget_extra(yyscanner);
        if (token == NULL) {
                DSError(M_DS_NULL ": tokenizer is NULL", A_DS_FATAL);
        }
        token->next = DSSecureCalloc(sizeof(struct matrix_token), 1);
        token = token->next;
        token->token = DS_MATRIX_TOKEN_ERROR;
        token->value = 0.0;
        yyset_extra(token, yyscanner);
        return -1;
}
%%

int yywrap(yyscan_t  yyscanner) {
        return 1;
}

extern struct matrix_token *DSMatrixTokenizeString(const char *string)
{
        struct matrix_token * tokens = NULL;
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
        tokens = DSMatrixTokenAlloc();
        yylex_init_extra(tokens, &scanner);
        buffer = yy_scan_string(string, scanner);
        yylex (scanner);
        yy_delete_buffer(buffer, scanner);
        yylex_destroy ( scanner );
bail:
        return tokens;
}