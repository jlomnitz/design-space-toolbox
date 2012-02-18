/**
 * \file DSVariable.h
 * \brief Header file with functions for dealing with variables.
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
#include <stdlib.h>
#include "DSTypes.h"

#ifndef __DS_VARIABLES__
#define __DS_VARIABLES__

#define DSVariableAssignValue(x, y) DSVariableSetValue(x, y)
#define DSVariableReturnValue(x)    DSVariableValue(x)

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
 * \brief Macro to set the value of a variable data structure.
 *
 * \details This macro provides a consistent way for changing the value of a
 * variable, despite the internal structure of the data type.  This macro is expanded to
 * a simple assignment.
 */
#define DSVariableSetValue(x, y)    ((x)->value = (y))

/**
 * \brief Macro to get the value of a variable data structure.
 *
 * \details This macro provides a consistent way for retrieving the value of a
 * variable, despite the internal structure of the data type.
 */
#define DSVariableValue(x)          (((x) != NULL) ? (x)->value : NAN)

/**
 * \brief Macro to get the value of a variable data structure.
 *
 * \details This macro provides a consistent way for retrieving the value of a
 * variable, despite the internal structure of the data type.
 */
#define DSVariableName(x)           ((x)->name)

/*\}*/

/**
 *\addtogroup M_DS_Messages
 * Messages for DSVariable related errors are M_DS_VAR_NULL and M_DS_VAR_LOCKED.
 */
/*\{*/
#define M_DS_VAR_NULL               M_DS_NULL ": Variable Pool is NULL"        //!< Error message indicating a NULL variable pool.
#define M_DS_VAR_LOCKED             " DSVariablePool: Insufficient priviliges" //!< Error message indicating insufficient priviliges to manipulate a variable pool.
/*\}*/

#ifdef __cplusplus
__BEGIN_DECLS
#endif

#if defined(__APPLE__) && defined(__MACH__)
#pragma mark - Symbol Variables
#endif

extern DSVariable *DSVariableAlloc(const char *name);
extern void DSVariableFree(DSVariable *var);
extern DSVariable * DSVariableRetain(DSVariable *aVariable);
extern void DSVariableRelease(DSVariable *aVariable);

#if defined(__APPLE__) && defined(__MACH__)
#pragma mark - Variable Pool
#endif

#define DSVariablePoolInternalDictionary(x)  ((x)->root)
#define DSVariablePoolVariableArray(x)       ((x)->variables)

#if defined(__APPLE__) && defined(__MACH__)
#pragma mark Allocation, Initialization and Freeing
#endif

extern DSVariablePool * DSVariablePoolAlloc(void);
extern DSVariablePool * DSVariablePoolCopy(const DSVariablePool * const pool);
extern void DSVariablePoolFree(DSVariablePool *pool);

#if defined(__APPLE__) && defined(__MACH__)
#pragma mark Factory functions
#endif

extern DSVariablePool * DSVariablePoolByParsingString(const char *string);

#if defined(__APPLE__) && defined(__MACH__)
#pragma mark Setter functions
#endif

extern void DSVariablePoolSetReadOnly(DSVariablePool * pool);
extern void DSVariablePoolSetReadWrite(DSVariablePool * pool);
extern void DSVariablePoolSetReadWriteAdd(DSVariablePool * pool);

extern void DSVariablePoolAddVariableWithName(DSVariablePool *pool, const char * name);
extern void DSVariablePoolAddVariable(DSVariablePool *pool, DSVariable *newVar);
extern void DSVariablePoolSetValueForVariableWithName(const DSVariablePool *pool, const char *name, const double value);

#if defined(__APPLE__) && defined(__MACH__)
#pragma mark Getter functions
#endif

extern DSUInteger DSVariablePoolNumberOfVariables(const DSVariablePool *pool);

extern bool DSVariablePoolIsReadOnly(const DSVariablePool *pool);
extern bool DSVariablePoolIsReadWrite(const DSVariablePool *pool);
extern bool DSVariablePoolIsReadWriteAdd(const DSVariablePool *pool);

extern bool DSVariablePoolHasVariableWithName(const DSVariablePool *pool, const char * const name);
extern DSVariable *DSVariablePoolVariableWithName(const DSVariablePool *pool, const char *name);
//extern double DSVariablePoolValueForVariable(con

extern const DSVariable ** DSVariablePoolAllVariables(const DSVariablePool *pool);
extern const char ** DSVariablePoolAllVariableNames(const DSVariablePool *pool);

extern DSUInteger DSVariablePoolIndexOfVariable(const DSVariablePool *pool, const DSVariable *var);
extern DSUInteger DSVariablePoolIndexOfVariableWithName(const DSVariablePool *pool, const char *name);

#if defined(__APPLE__) && defined(__MACH__)
#pragma mark Utility functions
#endif

extern void DSVariablePoolPrint(const DSVariablePool * const pool);
extern DSMatrix * DSVariablePoolValuesAsVector(const DSVariablePool *pool, const bool rowVector);

#ifdef __cplusplus
__END_DECLS
#endif

#endif


