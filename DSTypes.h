/**
 * \file DSTypes.h
 * \brief Header file with definitions for data types.
 *
 * \details 
 * This file specifies the design space standard data types. Contained here are
 * strictly the data type definitions, and specific macros to access data in
 * data structures.  Functions applying to these data types are contained
 * elsewhere, and the individual data structures should refer to these
 * files.
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
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>

#ifndef __DS_TYPES__
#define __DS_TYPES__

#ifndef __BEGIN_DECLS
#define __BEGIN_DECLS extern "C" {
#endif

#ifndef __END_DECLS
#define }
#endif

#ifndef __deprecated
#define __deprecated
#endif

#ifndef INFINITY
#define INFINITY HUGE_VAL
#endif

#ifdef __cplusplus
__BEGIN_DECLS
#endif


typedef int DSInteger;

typedef unsigned int DSUInteger;

/**
 * \brief Data type that is used to store errors.
 *
 * \details Type definition for the internal error handling, independent from
 * the error reporting mentioned in M_DS_Messages and A_DS_Actions.
 * The possible values this type definition has only symbolizes the
 * severity of the error.
 *
 * \see DSErrors.h
 *
 * \see M_DS_Messages
 * \see A_DS_Actions
 */
/*
typedef enum {
	DS_NOERROR = 0,  //!< The value for no  errors found.
	DS_WARN,         //!< The value for a warning found.
	DS_ERROR         //!< The value for a error being found.
} DSException;
*/
/**
 * \brief Basic variable structure containing name, value and NSString with
 *        special unicode characters for greek letters.
 *
 * Structure that carries variable information.  Internal to BSTVariables class
 * and should not be created and/or freed manually and beyond the context of
 * the BSTVariables class.
 *
 */
typedef struct {
        char *name;             //!< Dynamically allocated name of the variable.
        double value;           //!< Value of the variable.
        DSUInteger retainCount; //!< Retain counter for memory management.
} DSVariable;

/**
 * \brief Internal dictionary structure.
 *
 * Internal dictionary for fast variable querying.  The structure of the 
 * dictionary uses an alternative path, where each character is checked in order
 * at each position, if there is a match, the next position is consequently
 * checked.  The dictionary should never be manipulated manually, adding, 
 * retrieving and removing variables should be done through the accesory 
 * functions.
 *
 * \see DSVariable
 */
typedef struct _varDictionary
{
        char current;                //!< The current character in the dictionary.
        struct _varDictionary *alt;  //!< The alternative character in the dictionary.
        struct _varDictionary *next; //!< The next character in the dictionary.
        DSVariable *variable;        //!< The variable stored. Only when current is '\\0'.
} DSVariablePool;

/**
 * \brief Data type representing a matrix.
 *
 * \details This data type is the front end of the matric manipulation portion
 * of the design space toolbox.  Currently, the DST library uses the gsl library; 
 * however, it is designed to be used with different back-ends.  In particular, 
 * the CLAPACK package should be considered, as it will offer better performance.
 * Thus, the matrix API should be independent of implementation, and hence
 * a new matrix library could be used if chosen.
 *
 */
typedef struct {
        void *mat;          //!< The pointer to the internal representation of the matrix.
        DSUInteger rows;    //!< A DSUInteger specifying the number of rows in the matrix.
        DSUInteger columns; //!< A DSUInteger specifying the number of columns in the matrix.
} DSMatrix;

/**
 * \brief Data type representing an array of matrices.
 *
 * \details This data type is a utility data type that keeps track of arrays of
 * matrices.  This structure can also be used to represent three-dimensional 
 * matrices, as used in the internal representation of GMA's.
 */
typedef struct {
        DSUInteger numberOfMatrices; //!< A DSUInteger specifying the number of matrices in the array.
        DSMatrix **matrices;         //!< A pointer the the C-style array of matrices.
} DSMatrixArray;

/**
 * \brief Data type representing an S-System.
 *
 * This data structure is a standard representation of an S-System using 
 * matrix notation.  Here, the positive and negative terms are explicitly
 * represented according to the Gs and Hs.  Also, matrices are split up 
 * relating to either dependent and independent parameters.
 */
typedef struct {
        DSMatrix *Gd;
        DSMatrix *Gi;
        DSMatrix *Hd;
        DSMatrix *Hi;
        DSMatrix *M;
        DSMatrix *Ai;
} DSSSystem;

#ifdef __cplusplus
__END_DECLS
#endif

#endif

/******************************* Documentation *******************************/
/**
 *\defgroup DS_VARIABLE_ACCESSORY Macros to manipulate variables.
 *
 * \details The following macros are in place for portability and consistency.
 * As the structure of the BSTVariable is subject to change, due to the nature of
 * early versions of the framework, using these macros will make the dependent
 * code less subject to errors.
 */
/*\{*/
/**
 * \def DSVariableAssignValue
 * \brief Macro to directly change the value of a variable.
 *
 * Macro for manipulating value of a DSVariable.  As a measure to make the 
 * manipulation of a variable independent of the structure, this macro provides
 * a consistent and portable way of changing a variable value.  Direct access of
 * the variable value should be avoided.
 *
 * \see DSVariable
 * \see DSVariableReturnValue
 */

/**
 * \def DSVariableReturnValue
 * \brief Macro to directly access the value of a variable.
 *
 * Macro for retrieving the value of a DSVariable.  As a measure to make the 
 * manipulation of a variable independent of the structure, this macro provides
 * a consistent and portable way of retrieving the value of a variable.  Direct 
 * access of the variable value should be avoided.
 * \see DSVariable
 * \see DSVariableAssignValue
 */
/*\}*/


