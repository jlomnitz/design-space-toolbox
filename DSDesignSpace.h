/**
 * \file DSDesignSpace.h
 * \brief Header file with functions for dealing with Design Spaces
 *
 * \details 
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

#ifndef __DS_DESIGN_SPACE__
#define __DS_DESIGN_SPACE__

#include "DSTypes.h"
#include "DSErrors.h"
#include "DSDataSerialization.pb-c.h"

#ifdef __cplusplus
__BEGIN_DECLS
#endif

#define M_DS_DESIGN_SPACE_NULL              M_DS_NULL ": Design Space is NULL"

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, deallocation and initialization
#endif

DSDesignSpace * DSDesignSpaceAlloc(void);
void DSDesignSpaceFree(DSDesignSpace * ds);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Factory -
#endif

extern DSDesignSpace * DSDesignSpaceByParsingStringList(const char * string, const DSVariablePool * const Xd_a, ...);
extern DSDesignSpace * DSDesignSpaceByParsingStrings(char * const * const strings, const DSVariablePool * const Xd_a, const DSUInteger numberOfEquations);
extern DSDesignSpace * DSDesignSpaceByParsingStringsWithXi(char * const * const strings, const DSVariablePool * const Xd_a, const DSVariablePool * const Xi, const DSUInteger numberOfEquations);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Setters -
#endif

extern void DSDesignSpaceSetGMA(DSDesignSpace * ds, DSGMASystem *gma);
extern void DSDesignSpaceAddConditions(DSDesignSpace *ds, const DSMatrix * Cd, const DSMatrix * Ci, const DSMatrix * delta);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Getters -
#endif

extern const DSVariablePool * DSDesignSpaceXi(const DSDesignSpace *ds);

extern const DSUInteger DSDesignSpaceNumberOfEquations(const DSDesignSpace *ds);
extern DSExpression ** DSDesignSpaceEquations(const DSDesignSpace *ds);
extern const DSUInteger * DSDesignSpaceSignature(const DSDesignSpace *ds);
extern char * DSDesignSpaceSignatureToString(const DSDesignSpace *ds);

extern const DSUInteger DSDesignSpaceNumberOfValidCases(const DSDesignSpace *ds);
extern const DSUInteger DSDesignSpaceNumberOfCases(const DSDesignSpace *ds);

extern DSCase * DSDesignSpaceCaseWithCaseNumber(const DSDesignSpace * ds, const DSUInteger caseNumber);
extern DSCase * DSDesignSpaceCaseWithCaseIdentifier(const DSDesignSpace * ds, const char * identifer);
extern DSCase * DSDesignSpaceCaseWithCaseSignature(const DSDesignSpace * ds, const DSUInteger * signature);
//extern DSCase * DSDesignSpaceCaseWithCaseSignatureList(const DSDesignSpace *ds, const DSUInteger firstTerm, ...);

extern const bool DSDesignSpaceCaseWithCaseNumberIsValid(const DSDesignSpace *ds, const DSUInteger caseNumber);
extern const bool DSDesignSpaceCaseWithCaseSignatureIsValid(const DSDesignSpace *ds, const DSUInteger * signature);
//extern const bool DSDesignSpaceCaseWithCaseSignatureListIsValid(const DSDesignSpace *ds, const DSUInteger firstTerm, ...);

extern const DSGMASystem * DSDesignSpaceGMASystem(const DSDesignSpace * ds);
extern const DSDictionary * DSDesignSpaceCyclicalCaseDictionary(const DSDesignSpace *ds);
//extern DSDictionary * DSDesignSpaceCycleDictionaryForSignature(const DSDesignSpace * ds, const DSUInteger * signature);

extern const char * DSDesignSpaceCasePrefix(const DSDesignSpace * ds);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Utility -
#endif

extern void * DSDesignSpaceTermListForAllStrings(const char ** strings, const DSUInteger numberOfEquations);
extern void DSDesignSpaceAddConstraints(DSDesignSpace * ds, const char ** strings, DSUInteger numberOfConstraints);
extern void DSDesignSpacePrint(const DSDesignSpace * ds);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Case and Case validity
#endif

extern DSUInteger * DSDesignSpaceCaseNumbersWithPrefix(const DSDesignSpace * ds, const DSUInteger sizeOfPrefix, const DSUInteger *prefix, DSUInteger * numberOfCases)
;

extern DSCase ** DSDesignSpaceCalculateCases(DSDesignSpace *ds, const DSUInteger numberOfCase, DSUInteger *cases);
extern DSCase ** DSDesignSpaceCalculateValidCasesByPrunning(DSDesignSpace *ds);
extern DSCase ** DSDesignSpaceCalculateAllValidCases(DSDesignSpace *ds);
extern DSDictionary * DSDesignSpaceCalculateAllValidCasesForSliceByResolvingCyclicalCases(DSDesignSpace *ds, const DSVariablePool * lower, const DSVariablePool * upper);
extern DSDictionary * DSDesignSpaceCalculateAllValidCasesByResolvingCyclicalCases(DSDesignSpace *ds);
extern DSDictionary * DSDesignSpaceCalculateAllValidCasesForSlice(DSDesignSpace *ds, const DSVariablePool *lower, const DSVariablePool *upper);
extern DSDictionary * DSDesignSpaceCalculateAllValidCasesForSliceNonStrict(DSDesignSpace *ds, const DSVariablePool *lower, const DSVariablePool *upper);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Cyclical Cases and Cyclical Case validity
#endif

extern DSUInteger DSDesignSpaceNumberOfCyclicalCases(const DSDesignSpace * ds);
extern const DSCyclicalCase * DSDesignSpaceCyclicalCaseWithCaseNumber(const DSDesignSpace *ds, DSUInteger caseNumber);
extern const DSCyclicalCase * DSDesignSpaceCyclicalCaseWithCaseIdentifier(const DSDesignSpace * ds, const char * identifer);
extern void DSDesignSpaceCalculateCyclicalCase(DSDesignSpace *ds, DSCase * aCase);
extern void DSDesignSpaceCalculateCyclicalCases(DSDesignSpace *ds);

extern void DSDesignSpaceCalculateValidityOfCases(DSDesignSpace *ds);
extern DSDictionary * DSDesignSpaceCalculateValidityOfCaseSet(DSDesignSpace *ds, DSUInteger numberOfCases, DSCase ** cases);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Data Serialization
#endif


extern DSDesignSpaceMessage * DSDesignSpaceEncode(const DSDesignSpace * aCase);
extern DSDesignSpace * DSDesignSpaceFromDesignSpaceMessage(const DSDesignSpaceMessage * message);
extern DSDesignSpace * DSDesignSpaceDecode(size_t length, const void * buffer);

#ifdef __cplusplus
__END_DECLS
#endif

#endif
