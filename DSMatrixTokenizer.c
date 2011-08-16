/**
 * \file DSMatrixTokenizer.c
 * \brief Implementation file with functions for tokenizing with matrices.
 *
 * \details 
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

#include <stdio.h>
#include "DSMatrixTokenizer.h"

extern struct matrix_token * DSMatrixTokenAlloc()
{
        struct matrix_token *aToken = NULL;
        aToken = DSSecureCalloc(1, sizeof(struct matrix_token));
        DSMatrixTokenSetType(aToken, DS_MATRIX_TOKEN_START);
        return aToken;
}

extern void DSMatrixTokenFree(struct matrix_token *root)
{
        struct matrix_token *next = NULL;
        if (root == NULL) {
                DSError(M_DS_NULL ": token to free is NULL", A_DS_ERROR);
                goto bail;
        }
        while (root) {
                next = DSMatrixTokenNext(root);
                DSSecureFree(root);
                root = next;
        }
bail:
        return;
}