/**
 * \file DSTokenizer.h
 * \brief
 *
 * \details
 *
 * Copyright (C) 2011-2014 Jason Lomnitz.\n\n
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
 *
 * \todo Add options to register custom functions.
 */

#include <stdio.h>
#include "DSExpressionTokenizer.h"

extern struct expression_token * DSExpressionTokenAlloc()
{
        struct expression_token *aToken = NULL;
        aToken = DSSecureCalloc(1, sizeof(struct expression_token));
        DSExpressionTokenSetType(aToken, DS_EXPRESSION_TOKEN_START);
        return aToken;
}

extern void DSExpressionTokenFree(struct expression_token *root)
{
        struct expression_token *next = NULL;
        if (root == NULL) {
                DSError(M_DS_NULL ": token to free is NULL", A_DS_ERROR);
                goto bail;
        }
        while (root) {
                next = DSExpressionTokenNext(root);
                if (DSExpressionTokenType(root) == DS_EXPRESSION_TOKEN_ID)
                        DSSecureFree(DSExpressionTokenString(root));
                DSSecureFree(root);
                root = next;
        }
bail:
        return;
}

extern void DSExpressionTokenSetString(struct expression_token *root, char *string)
{
        if (root == NULL) {
                DSError(M_DS_NULL ": Variable token is NULL", A_DS_ERROR);
                goto bail;
        }
        if (string == NULL) {
                DSError(M_DS_WRONG ": String is NULL", A_DS_ERROR);
                goto bail;
        }
        root->data.name = string;
bail:
        return;
}

extern void DSExpressionTokenSetDouble(struct expression_token *root, double value)
{
        if (root == NULL) {
                DSError(M_DS_NULL ": Variable token is NULL", A_DS_ERROR);
                goto bail;
        }
        root->data.value = value;
bail:
        return;
}

extern char * DSExpressionTokenString(struct expression_token *root)
{
        char *string = NULL;
        if (root == NULL) {
                DSError(M_DS_NULL ": Variable token is NULL", A_DS_ERROR);
                goto bail;
        }
        string = DSExpressionTokenData(root).name;
bail:
        return string;
}

extern double DSExpressionTokenDouble(struct expression_token *root)
{
        double value = NAN;
        if (root == NULL) {
                DSError(M_DS_NULL ": Value token is NULL", A_DS_ERROR);
                goto bail;
        }
        value = DSExpressionTokenData(root).value;
bail:
        return value;
}
