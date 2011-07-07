/*
 * BSTVariable.h
 * BSTVariables
 *
 * Copyright (C) 2010 Jason Lomnitz
 *
 * This file is part of BSTVariables.
 *
 * BSTVariables is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BSTVariables is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with BSTVariables.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For any questions, suggestions or bug reporting please e-mail the chief
 * author, Jason G. Lomnitz <jlomn@ucdavis.edu>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef __deprecated
#define __deprecated
#endif

#ifndef INFINITY
#define INFINITY HUGE_VAL
#endif

/*********************************** WARNING **********************************
 * The following macros are in place for portability and consistency.
 * As the structure of the BSTVariable is subject to change, due to the nature of
 * early versions of the framework, using these macros will make the dependent
 * code less subject to errors.
 ******************************************************************************/

/**
 * @brief Macro to directly change the value of a variable.
 *
 * Macro for manipulating value of a BSTVariable.  As a measure to make the 
 * manipulation of a variable independent of the structure, this macro provides
 * a consistent and portable way of changing a variable value.  Direct access of
 * the variable value should be avoided.
 *
 * @see BSTVariable
 * @see BSTVariableReturnValue
 */
#define BSTVariableAssignValue(x, y) x->_value = y

/**
 * @brief Macro to directly access the value of a variable.
 *
 * Macro for retrieving the value of a BSTVariable.  As a measure to make the 
 * manipulation of a variable independent of the structure, this macro provides
 * a consistent and portable way of retrieving the value of a variable.  Direct 
 * access of the variable value should be avoided.
 * @see BSTVariable
 * @see BSTVariableAssignValue
 */
#define BSTVariableReturnValue(x)    x->_value

/**
 * @brief Basic variable structure containing name, value and NSString with
 *        special unicode characters for greek letters.
 *
 * Structure that carries variable information.  Internal to BSTVariables class
 * and should not be created and/or freed manually and beyond the context of
 * the BSTVariables class.
 * @see BSTVariables
 * @see BSTVariableAssignValue
 * @see BSTVariableReturnValue
 * @see newBSTVariable
 * @see freeBSTVariable
 *
 * @todo Create a retain count to keep track of variables, reducing risk
 *       of premature memory deallocation.
 */
typedef struct {
        char *name;             //!< Dynamically allocated name of the variable.
        NSString *variableName; //!< NSString containing the name of the variable.
        double _value;          //!< Value of the variable.
        NSUInteger retainCount; //!< Retain counter for memory management.
} BSTVariable;

/**
 * @brief Internal dictionary structure.
 *
 * Internal dictionary for fast variable querying.  The structure of the 
 * dictionary uses an alternative path, where each character is checked in order
 * at each position, if there is a match, the next position is consequently
 * checked.  The dictionary should never be manipulated manually, adding, 
 * retrieving and removing variables should be done through the accesory 
 * functions.
 *
 * @see BSTVariable
 * @see variableWithName
 * @see addToDictionary
 * @see freeVarDictionary
 */
struct _varDictionary
{
        char current;                //!< The current character in the dictionary.
        struct _varDictionary *alt;  //!< The alternative character in the dictionary.
        struct _varDictionary *next; //!< The next character in the dictionary.
        BSTVariable *variable;        //!< The variable stored. Only when current is '\\0'.
};

/***** Fuctions related to BSTVariable and struct _varDictionary *****/
BSTVariable *variableWithName(const char *name, struct _varDictionary *root);
BSTVariable * BSTVariableRetain(BSTVariable *aVariable);
void BSTVariableRelease(BSTVariable *aVariable);
struct _varDictionary *addToDictionary(BSTVariable *newVar, struct _varDictionary *root);
BSTVariable *newVariable(const char *name);
void freeBSTVariable(BSTVariable *var);
void freeVarDictionary(struct _varDictionary *root);
__deprecated void printVarDictionary(struct _varDictionary *root, int indent);
void printMembers(struct _varDictionary *root, char *buffer, int position);
