/**
 * \file DSMatrixTokenizer.h
 * \brief Header file with functions for tokenizing matrices.
 *
 * \details This header file specifies the data structure relating to the
 * tokenization of an input string to be parsed as a matrix, as well as all
 * the functions necessary to tokenize it.  This file is a private file, and 
 * therefore its contents will be invisible to the public API. Therefore, it
 * is unnecessary to place the C++ compatability declerations.
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

#ifndef __DS_VARIABLE_TOKENIZER__
#define __DS_VARIABLE_TOKENIZER__

#include "DSTypes.h"
#include "DSErrors.h"
#include "DSMemoryManager.h"
#include "DSVariableGrammar.h"

#define DS_VARIABLE_TOKEN_START      0                 //!< Token indicating the start of a tokenization.
#define DS_VARIABLE_TOKEN_ID         TOKEN_IDENTIFIER  //!< Token indicating a variable identifier.
#define DS_VARIABLE_TOKEN_DOUBLE     TOKEN_VALUE       //!< Token indicating a numerical value.
#define DS_VARIABLE_TOKEN_SEPERATOR  TOKEN_SEPERATOR   //!< Token indicating a seperator.
#define DS_VARIABLE_TOKEN_ASSIGN     TOKEN_ASSIGN      //!< Token indicating assignation. 
#define DS_VARIABLE_TOKEN_QUOTE      TOKEN_QUOTE      //!< Token indicating assignation. 
#define DS_VARIABLE_TOKEN_OTHER      TOKEN_OTHER

#define DSVariableTokenNext(x)         ((x)->next)
#define DSVariableTokenData(x)         ((x)->data)
#define DSVariableTokenType(x)         ((x)->type)

#define DSVariableTokenSetNext(x, y)   ((x)->next = (y))
#define DSVariableTokenSetData(x, y)   ((x)->data = (y))
#define DSVariableTokenSetType(x, y)   ((x)->type = (y))

/**
 * \brief Union containing the alternative values a struct variable_token can
 *        take.
 *
 * \details The union can have either a string, used for the names of variables
 * when an identifier is found; and a double value used when a value is found.
 *
 * \see struct variable_token
 */
union v_token_data {
        char * name;
        double value;
};

/**
 * \brief A data structure representing a token used when parsing strings for
 * variable pools.
 *
 */
struct variable_token
{
        int type;
        union v_token_data data;
        struct variable_token *next;
};

extern struct variable_token * DSVariableTokenAlloc();
extern void DSVariableTokenFree(struct variable_token *root);

extern void DSVariableTokenSetString(struct variable_token *root, char *string);
extern void DSVariableTokenSetDouble(struct variable_token *root, double value);

extern char * DSVariableTokenString(struct variable_token *root);
extern double DSVariableTokenDouble(struct variable_token *root);

extern struct variable_token *DSVariablePoolTokenizeString(const char *string);

#endif
