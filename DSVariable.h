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

#define DSVariableSetValue(x, y)    (x)->value = (y)
#define DSVariableValue(x)          (x)->value
#define DSVariableName(x)           (x)->name

#ifdef __cplusplus
__BEGIN_DECLS
#endif

#if defined(__APPLE__) && defined(__MACH__)
#pragma mark - Symbol Variables
#endif
/***** DSVariable functionality *****/
extern DSVariable *DSVariableAlloc(const char *name);
extern void DSVariableFree(DSVariable *var);
extern DSVariable * DSVariableRetain(DSVariable *aVariable);
extern void DSVariableRelease(DSVariable *aVariable);

__deprecated extern DSVariable *DSNewVariable(const char *name);

#if defined(__APPLE__) && defined(__MACH__)
#pragma mark - Variable Pool
#endif
/***** DSVariablePool *****/
extern DSVariablePool *DSVariablePoolAddVariableWithName(const char * name, DSVariablePool *root);
extern DSVariablePool *DSVariablePoolAddVariable(DSVariable *newVar, DSVariablePool *root);
extern DSVariable *DSVariablePoolVariableWithName(const char *name, DSVariablePool *root);
extern void DSVariablePoolFree(DSVariablePool *root);

__deprecated extern DSVariable *DSVariableWithName(const char *name, DSVariablePool *root);
#ifdef __cplusplus
__END_DECLS
#endif

#endif


