/**
 * \file DSSubcase.h
 * \brief Header file with functions for dealing with subcases.
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
#include "DSErrors.h"
#include "DSMemoryManager.h"
#include "DSExpression.h"
#include "DSMatrix.h"
#include "DSMatrixArray.h"
#include "DSSSystem.h"
#include "DSCase.h"
#include "DSGMASystem.h"
#include "DSDesignSpace.h"
#include "DSStack.h"

#ifndef __DS_SUBCASE__
#define __DS_SUBCASE__

/**
 *\addtogroup M_DS_Messages
 * Messages for DSCase related errors is M_DS_CASE_NULL.
 */
/*\{*/
#define M_DS_SUBCASE_NULL                  M_DS_NULL ": Subcase is NULL"
/*\}*/

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, deallocation and initialization
#endif

extern DSSubcase * DSSubcaseForCaseInDesignSpace(const DSDesignSpace * ds, const DSCase * aCase);
extern void DSSubcaseFree(DSSubcase * aSubcase);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Getter functions
#endif

extern const DSDesignSpace * DSSubcaseInternalDesignSpace(const DSSubcase * subcase);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Linear programming functions
#endif

extern const bool DSSubcaseIsValid(const DSSubcase *aSubcase);
extern const bool DSSubcaseIsValidAtPoint(const DSSubcase *aSubcase, const DSVariablePool * variablesToFix);
extern const bool DSSubcaseIsValidAtSlice(const DSSubcase *aSubcase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds);

extern DSDictionary * DSSubcaseVerticesForSlice(const DSSubcase *aSubcase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const DSUInteger numberOfVariables, const char ** variables);
extern DSDictionary * DSSubcaseVerticesFor2DSlice(const DSSubcase *aSubcase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Subcase calculation functions
#endif

extern DSMatrix * DSSubcaseProblematicEquations(const DSCase * aCase);
extern DSMatrixArray * DSSubcaseProblematicTerms(const DSCase *aCase, const DSMatrix * dependentEquations);
extern DSMatrixArray * DSSubcaseCoefficientsOfInterest(const DSCase * aCase, const DSMatrixArray * problematicTerms);

extern void DSSubcaseDesignSpaceForUnderdeterminedCase(const DSCase * aCase, const DSDesignSpace * original);

#endif
