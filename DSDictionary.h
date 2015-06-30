/**
 * \file DSDictionary.h
 * \brief Header file with functions for dealing with dictionaries.
 *
 * \details The `DSDictionary` object is implemented using a binary tree.
 *          A binary tree was chosen due to memory limitations rather than 
 *          time to access limitations, which is not expected to be a 
 *          bottleneck.  The memory limitation arises from the expectation
 *          that these dictionaries typically hold small number of key-value
 *          pairs and a given dictionary may be duplicated many times for
 *          multiple models.
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
#include <stdlib.h>
#include "DSTypes.h"

#ifndef __DS_DICTIONARY__
#define __DS_DICTIONARY__

#define DSVariableAssignValue(x, y) DSVariableSetValue(x, y)
#define DSVariableReturnValue(x)    DSVariableValue(x)


/**
 *\addtogroup M_DS_Messages
 * Messages for DSVariable related errors are M_DS_VAR_NULL and M_DS_VAR_LOCKED.
 */
/*\{*/
#define M_DS_DICTIONARY_NULL               M_DS_NULL ": Dictionary is NULL"        //!< Error message indicating a NULL variable pool.
/*\}*/

#ifdef __cplusplus
__BEGIN_DECLS
#endif

#if defined(__APPLE__) && defined(__MACH__)
#pragma mark - Allocation and freeing
#endif

extern DSDictionary * DSDictionaryAlloc(void);
extern void DSDictionaryFree(DSDictionary * aDictionary);
extern void DSDictionaryFreeWithFunction(DSDictionary * aDictionary, void * freeFunction);

extern DSUInteger DSDictionaryCount(const DSDictionary *aDictionary);
extern void *DSDictionaryValueForName(const DSDictionary *dictionary, const char *name);
extern const char ** DSDictionaryNames(const DSDictionary *aDictionary);

extern void DSDictionaryAddValueWithName(DSDictionary *dictionary, const char * name, void *value);

#if defined(__APPLE__) && defined(__MACH__)
#pragma mark - Utility functions
#endif

extern void DSDictionaryPrint(const DSDictionary *dictionary);
extern void DSDictionaryPrintWithFunction(const DSDictionary *dictionary, const void * printFunction);

extern DSDictionary * DSDictionaryFromArray(void ** array, DSUInteger size);

#ifdef __cplusplus
__END_DECLS
#endif

#endif


