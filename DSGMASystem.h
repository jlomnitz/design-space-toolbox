/**
 * \file DSGMASystem.h
 * \brief Header file with functions for dealing with GMA Systems.
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

#include "DSTypes.h"
#include "DSVariable.h"

#ifndef __DS_GMA_SYSTEM__
#define __DS_GMA_SYSTEM__

#ifdef __cplusplus
__BEGIN_DECLS
#endif

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, deallocation and initialization
#endif

void DSGMASystemFree(DSGMASystem * gma);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Factory methods
#endif

extern DSGMASystem * DSGMASystemByParsingStringList(const DSVariablePool * const Xd, const char * const string, ...);
extern DSGMASystem * DSGMASystemByParsingStrings(const DSVariablePool * const Xd, char * const * const strings, const DSUInteger numberOfEquations);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Setter functions
#endif


#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Getter functions
#endif

const DSUInteger DSGMASystemNumberOfEquations(const DSGMASystem *gma);
char ** DSGMASystemEquations(const DSGMASystem *gma);

const DSMatrix *DSGMASystemAlpha(const DSGMASystem *gma);
const DSMatrix *DSGMASystemBeta(const DSGMASystem *gma);

const DSMatrixArray *DSGMASystemGd(const DSGMASystem *gma);
const DSMatrixArray *DSGMASystemGi(const DSGMASystem *gma);
const DSMatrixArray *DSGMASystemHd(const DSGMASystem *gma);
const DSMatrixArray *DSGMASystemHi(const DSGMASystem *gma);

const DSVariablePool *DSGMASystemXd(const DSGMASystem *gma);
const DSVariablePool *DSGMASystemXi(const DSGMASystem *gma);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - S-System functions
#endif

DSSSystem * DSGMASystemSSystemWithSignature(const DSGMASystem *gma, const char *signature);


extern void DSGMASystemPrint(const DSGMASystem * gma);

#ifdef __cplusplus
__END_DECLS
#endif

#endif
