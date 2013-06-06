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
 */

#include "DSTypes.h"
#include "DSVariable.h"

#ifndef __DS_GMA_SYSTEM__
#define __DS_GMA_SYSTEM__

#ifdef __cplusplus
__BEGIN_DECLS
#endif

#define M_DS_GMA_NULL              M_DS_NULL ": GMA System is NULL"

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, deallocation and initialization
#endif

extern DSGMASystem * DSGMASystemCopy(const DSGMASystem * gma);

extern void DSGMASystemFree(DSGMASystem * gma);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Factory functions
#endif

extern DSGMASystem * DSGMASystemByParsingStringList(const char * const string, const DSVariablePool * const Xd_a, ...);
extern DSGMASystem * DSGMASystemByParsingStrings(char * const * const strings, const DSVariablePool * const Xd_a, const DSUInteger numberOfEquations);
extern DSGMASystem * DSGMASystemByParsingStringsWithXi(const DSVariablePool * const Xd, const DSVariablePool * const Xi, char * const * const strings, const DSUInteger numberOfEquations);
#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Setter functions
#endif


#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Getter functions
#endif

extern const DSUInteger DSGMASystemNumberOfEquations(const DSGMASystem *gma);
extern DSExpression ** DSGMASystemEquations(const DSGMASystem *gma);

extern DSExpression * DSGMASystemPositiveTermsForEquations(const DSGMASystem *gma, const DSUInteger equation);
extern DSExpression * DSGMASystemNegativeTermsForEquations(const DSGMASystem *gma, const DSUInteger equation);

extern const DSMatrix *DSGMASystemAlpha(const DSGMASystem *gma);
extern const DSMatrix *DSGMASystemBeta(const DSGMASystem *gma);

extern const DSMatrixArray *DSGMASystemGd(const DSGMASystem *gma);
extern const DSMatrixArray *DSGMASystemGi(const DSGMASystem *gma);
extern const DSMatrixArray *DSGMASystemHd(const DSGMASystem *gma);
extern const DSMatrixArray *DSGMASystemHi(const DSGMASystem *gma);

extern const DSVariablePool *DSGMASystemXd(const DSGMASystem *gma);
extern const DSVariablePool *DSGMASystemXd_a(const DSGMASystem *gma);
extern const DSVariablePool *DSGMASystemXi(const DSGMASystem *gma);

extern const DSUInteger DSGMASystemNumberOfCases(const DSGMASystem *gma);
extern const DSUInteger * DSGMASystemSignature(const DSGMASystem *gma);



#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

extern void DSGMASystemPrint(const DSGMASystem * gma);
extern void DSGMASystemPrintEquations(const DSGMASystem *gma);

#ifdef __cplusplus
__END_DECLS
#endif

#endif
