/**
 * \file DSMatrixTokenizer.h
 * \brief Header file with functions for tokenizing matrices.
 *
 * \details This header file specifies the data structure relating to the
 * tokenization of an input string to be parsed as a matrix, as well as all
 * the functions necessary to tokenize it.  This file is a provate file, and 
 * therefore its contents will be invisible to the public API.  As such, it
 * is not necessary to place the C++ compatability decleration.
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

#ifndef __DS_MATRIX_TOKENIZER__
#define __DS_MATRIX_TOKENIZER__

#include "DSTypes.h"
#include "DSErrors.h"
#include "DSMemoryManager.h"

#define DS_MATRIX_TOKEN_START      0 //!< Token indicating the start of a tokenization.
#define DS_MATRIX_TOKEN_DOUBLE     1 //!< Token indicating a numerical value.
#define DS_MATRIX_TOKEN_NEWLINE    2 //!< Token indicating a newline, indicative of a new row.
#define DS_MATRIX_TOKEN_ERROR      3 //!< Token indicating an error during tokenization.

#define DSMatrixTokenNext(x)         ((x)->next)
#define DSMatrixTokenValue(x)        ((x)->value)
#define DSMatrixTokenType(x)         ((x)->token)
#define DSMatrixTokenRow(x)          ((x)->row)
#define DSMatrixTokenColumn(x)       ((x)->column)

#define DSMatrixTokenSetNext(x, y)   ((x)->next = (y))
#define DSMatrixTokenSetValue(x, y)  ((x)->value = (y))
#define DSMatrixTokenSetType(x, y)   ((x)->token = (y))
#define DSMatrixTokenSetRow(x, y)    ((x)->row = (y))
#define DSMatrixTokenSetColumn(x, y) ((x)->column = (y))

/**
 *
 */
struct matrix_token
{
        int token;
        double value;
        DSUInteger row;
        DSUInteger column;
        struct matrix_token *next;
};

extern struct matrix_token * DSMatrixTokenAlloc();
extern void DSMatrixTokenFree(struct matrix_token *root);

extern struct matrix_token *DSMatrixTokenizeString(const char *string);


#endif
