/**
 * \file DSvariableTokenizer.c
 * \brief Implementation file with functions for tokenizing with matrices.
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
 */

#include <stdio.h>
#include "DSVariableTokenizer.h"

extern struct variable_token * DSVariableTokenAlloc()
{
        struct variable_token *aToken = NULL;
        aToken = DSSecureCalloc(1, sizeof(struct variable_token));
        DSVariableTokenSetType(aToken, DS_VARIABLE_TOKEN_START);
        return aToken;
}

extern void DSVariableTokenFree(struct variable_token *root)
{
        struct variable_token *next = NULL;
        if (root == NULL) {
                DSError(M_DS_NULL ": token to free is NULL", A_DS_ERROR);
                goto bail;
        }
        while (root) {
                next = DSVariableTokenNext(root);
                if (DSVariableTokenType(root) == DS_VARIABLE_TOKEN_ID)
                        DSSecureFree(DSVariableTokenString(root));
                DSSecureFree(root);
                root = next;
        }
bail:
        return;
}

extern void DSVariableTokenSetString(struct variable_token *root, char *string)
{
        union v_token_data data;
        if (root == NULL) {
                DSError(M_DS_NULL ": Variable token is NULL", A_DS_ERROR);
                goto bail;
        }
        if (string == NULL) {
                DSError(M_DS_WRONG ": String is NULL", A_DS_ERROR);
                goto bail;
        }
        data.name = string;
        DSVariableTokenSetData(root, data);
bail:
        return;
}

extern void DSVariableTokenSetDouble(struct variable_token *root, double value)
{
        union v_token_data data;
        if (root == NULL) {
                DSError(M_DS_NULL ": Variable token is NULL", A_DS_ERROR);
                goto bail;
        }
        data.value = value;
        DSVariableTokenSetData(root, data);
bail:
        return;
}

extern char * DSVariableTokenString(struct variable_token *root)
{
        char *string = NULL;
        if (root == NULL) {
                DSError(M_DS_NULL ": Variable token is NULL", A_DS_ERROR);
                goto bail;
        }
        string = DSVariableTokenData(root).name;
bail:
        return string;
}

extern double DSVariableTokenDouble(struct variable_token *root)
{
        double value = NAN;
        if (root == NULL) {
                DSError(M_DS_NULL ": Variable token is NULL", A_DS_ERROR);
                goto bail;
        }
        value = DSVariableTokenData(root).value;
bail:
        return value;
}






