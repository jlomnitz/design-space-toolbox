//
//  DSCase.h
//  DesignSpaceToolboxV2
//
//  Created by Jason Lomnitz on 8/18/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef __DS_CASE__
#define __DS_CASE__

#include "DSTypes.h"
#include "DSErrors.h"

/**
 *\addtogroup M_DS_Messages
 * Messages for DSCase related errors is M_DS_CASE_NULL.
 */
/*\{*/
#define M_DS_CASE_NULL                  M_DS_NULL ": Case is NULL"
/*\}*/

#ifdef __cplusplus
__BEGIN_DECLS
#endif

#define DS_CASE_NUMBER_BIG_ENDIAN    0
#define DS_CASE_NUMBER_SMALL_ENDIAN  1

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - DSCase Global behavior
#endif

extern void DSCaseSetEndianness(char endianness);
extern char DSCaseEndianness(void);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, deallocation and initialization
#endif

extern void DSCaseFree(DSCase * ssys);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Factory functions
#endif

extern DSCase * DSCaseWithTermsFromGMA(const DSGMASystem * gma, const DSUInteger * termArray);
extern DSCase * DSCaseWithTermsFromDesignSpace(const DSDesignSpace * ds, const DSUInteger * termArray);


#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Getter functions
#endif

extern const bool DSCaseHasSolution(const DSCase *aCase);

extern const DSUInteger DSCaseNumberOfEquations(const DSCase *aCase);

extern DSExpression ** DSCaseEquations(const DSCase *aCase);
extern DSExpression ** DSCaseSolution(const DSCase *aCase);
extern DSExpression ** DSCaseLogarithmicSolution(const DSCase * aCase);

extern const DSUInteger DSCaseNumberOfConditions(const DSCase *aCase);
extern DSExpression ** DSCaseConditions(const DSCase *aCase);
extern DSExpression ** DSCaseLogarithmicConditions(const DSCase *aCase);

extern const DSUInteger DSCaseNumberOfBoundaries(const DSCase *aCase);
extern DSExpression ** DSCaseBoundaries(const DSCase *aCase);
extern DSExpression ** DSCaseLogarithmicBoundaries(const DSCase *aCase);


extern const DSSSystem *DSCaseSSystem(const DSCase * aCase);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Linear programming functions - See DSCaseLinearProgramming.c
#endif


extern const bool DSCaseIsValid(const DSCase *aCase);
extern const bool DSCaseIsValidAtPoint(const DSCase *aCase, const DSVariablePool * variablesToFix);
extern const bool DSCaseIsValidAtSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds);

extern DSVertices * DSCaseVerticesForSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const DSUInteger numberOfVariables, const char ** variables);
extern DSVertices * DSCaseVerticesFor2DSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable);


#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Intersection of cases
#endif

extern const bool DSCaseIntersectionListIsValid(const DSUInteger numberOfCases, const DSCase *firstCase, ...);
extern const bool DSCaseIntersectionIsValid(const DSUInteger numberOfCases, const DSCase **cases);
extern const bool DSCaseIntersectionIsValidAtSlice(const DSUInteger numberOfCases, const DSCase **cases,  const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds);

extern DSVertices * DSCaseIntersectionVerticesForSlice(const DSUInteger numberOfCases, const DSCase **cases, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const DSUInteger numberOfVariables, const char ** variables);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Case signature
#endif

extern const DSUInteger DSCaseNumberForSignature(const DSUInteger * signature, const DSGMASystem * gma);
extern DSUInteger * DSCaseSignatureForCaseNumber(const DSUInteger caseNumber, const DSGMASystem * gma);

extern char * DSCaseSignatureToString(const DSCase *aCase);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Printing functions
#endif

extern void DSCasePrint(const DSCase *aCase);
extern void DSCasePrintEquations(const DSCase *aCase);
extern void DSCasePrintSolution(const DSCase *aCase);
extern void DSCasePrintLogarithmicSolution(const DSCase *aCase);
extern void DSCasePrintConditions(const DSCase *aCase);
extern void DSCasePrintLogarithmicConditions(const DSCase *aCase);
extern void DSCasePrintBoundaries(const DSCase *aCase);
extern void DSCasePrintLogarithmicBoundaries(const DSCase *aCase);

#ifdef __cplusplus
__END_DECLS
#endif


#endif
