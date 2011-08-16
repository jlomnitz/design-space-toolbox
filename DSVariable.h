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
 * \todo Remove printVarDictionary and add a DSVariablePoolPrint function.
 */

#include <stdio.h>
#include <stdlib.h>
#include "DSTypes.h"

#ifndef __DS_VARIABLES__
#define __DS_VARIABLES__

#define DSVariableAssignValue(x, y) DSVariableSetValue(x, y)
#define DSVariableReturnValue(x)    DSVariableValue(x)

#define DSVariableSetValue(x, y)    (x)->value = (y)
#define DSVariableValue(x)          (x)->value
#define DSVariableName(x)           (x)->name

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

#define DSVariablePoolNumberOfVariables(x)   ((x)->numberOfVariables)
#define DSVariablePoolInternalDictionary(x)  ((x)->root)
#define DSVariablePoolVariableArray(x)       ((x)->variables)

extern DSVariablePool * DSVariablePoolAlloc(void);
extern DSVariablePool * DSVariablePoolCopy(const DSVariablePool * const pool);
extern void DSVariablePoolAddVariableWithName(DSVariablePool *pool, const char * name);
extern void DSVariablePoolAddVariable(DSVariablePool *pool, DSVariable *newVar);
extern bool DSVariablePoolHasVariableWithName(const DSVariablePool *pool, const char * const name);
extern DSVariable *DSVariablePoolVariableWithName(const DSVariablePool *pool, const char *name);
extern void DSVariablePoolFree(DSVariablePool *pool);

extern void DSVariablePoolSetValueForVariableWithName(DSVariablePool *pool, const char *name, const double value);
extern const DSVariable ** DSVariablePoolAllVariables(const DSVariablePool *pool);
extern const char ** DSVariablePoolAllVariableNames(const DSVariablePool *pool);
extern DSUInteger DSVariablePoolIndexOfVariable(const DSVariablePool *pool, const DSVariable *var);
extern DSUInteger DSVariablePoolIndexOfVariableWithName(const DSVariablePool *pool, const char *name);

extern DSVariablePool * DSVariablePoolByParsingString(const char *string);
extern void DSVariablePoolPrint(const DSVariablePool * const pool);

#ifdef __cplusplus
__END_DECLS
#endif

#endif


