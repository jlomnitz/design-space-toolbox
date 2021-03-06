/**
 * \file DSCase.h
 * \brief Header file with functions for dealing with cases in design space.
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

#ifndef __DS_CASE__
#define __DS_CASE__

#include "DSTypes.h"
#include "DSErrors.h"
#include "DSDataSerialization.pb-c.h"
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

#define DSCaseSSys(x)                ((x)->ssys)
#define DSCaseCd(x)                  ((x)->Cd)
#define DSCaseCi(x)                  ((x)->Ci)
#define DSCaseU(x)                   ((x)->U)
#define DSCaseDelta(x)               ((x)->delta)
#define DSCaseZeta(x)                ((x)->zeta)
#define DSCaseSig(x)                 ((x)->signature)
#define DSCaseNum(x)                 ((x)->caseNumber)
#define DSCaseId(x)                  ((x)->caseIdentifier)

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - DSCase Global behavior
#endif

extern void DSCaseSetEndianness(char endianness);
extern char DSCaseEndianness(void);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, deallocation and initialization
#endif

extern DSCase * DSCaseCopy(const DSCase *aCase);
extern void DSCaseFree(DSCase * aCase);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Factory functions
#endif

extern DSCase * DSCaseWithTermsFromGMA(const DSGMASystem * gma, const DSUInteger * termArray, const char * prefix);
extern DSCase * DSCaseWithTermsFromDesignSpace(const DSDesignSpace * ds, const DSUInteger * termArray, const char * prefix);


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

extern DSUInteger DSCaseNumber(const DSCase * aCase);
extern const char * DSCaseIdentifier(const DSCase * aCase);
extern const DSUInteger * DSCaseSignature(const DSCase * aCase);
extern const DSSSystem *DSCaseSSystem(const DSCase * aCase);

extern double DSCaseLogarithmicGain(const DSCase *aCase, const char *XdName, const char *XiName);

extern const DSVariablePool * DSCaseXd(const DSCase * aCase);
extern const DSVariablePool * DSCaseXd_a(const DSCase * aCase);
extern const DSVariablePool * DSCaseXi(const DSCase * aCase);
#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

extern void DSCaseRecalculateBoundaryMatrices(DSCase *aCase);
extern DSMatrix * DSCaseDoubleValueBoundariesAtPoint(const DSCase * aCase, const DSVariablePool * point);
extern void DSCaseAddConstraints(DSCase * aCase, const char ** strings, DSUInteger numberOfConstraints);

extern void DSCaseRemoveRedundantBoundaries(DSCase *aCase);


#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Linear programming functions - See DSCaseLinearProgramming.c
#endif

extern const bool DSCaseConditionsAreValid(const DSCase *aCase);

extern const bool DSCaseIsValid(const DSCase *aCase, const bool strict);
extern const bool DSCaseIsConsistent(const DSCase *aCase);
extern const bool DSCaseIsValidInStateSpace(const DSCase *aCase);
extern const bool DSCaseIsValidAtPoint(const DSCase *aCase, const DSVariablePool * variablesToFix);
extern const bool DSCaseIsConsistentAtPoint(const DSCase *aCase, const DSVariablePool * Xd_p, const DSVariablePool * Xi_p);
extern const bool DSCaseIsValidInStateSpaceAtPoint(const DSCase *aCase, const DSVariablePool * Xd_p, const DSVariablePool * Xi_p);
extern const bool DSCaseIsValidAtSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const bool strict);
extern const bool DSCaseIsConsistentAtSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const bool strict);
extern DSVertices * DSCaseVerticesForSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const DSUInteger numberOfVariables, const char ** variables);

extern DSVertices * DSCaseBoundingRangeForVariableWithConstraints(const DSCase *aCase, const char * variable, DSVariablePool * lowerBounds, DSVariablePool * upperBounds);
extern DSVertices * DSCaseBoundingRangeForVariable(const DSCase *aCase, const char * variable);
extern DSVertices * DSCaseVerticesFor1DSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable);
extern DSVertices * DSCaseVerticesFor2DSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable);

extern DSStack * DSCaseVertexEquationsFor2DSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable, const bool log_out);

extern DSMatrixArray * DSCaseFacesFor3DSliceAndConnectivity(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable, const char *zVariable);

extern DSMatrixArray * DSCaseVerticesFor3DSliceAndConnectivity(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable, const char *zVariable);
extern DSVertices * DSCaseVerticesFor3DSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable, const char *zVariable);
extern DSMatrixArray * DSCaseVerticesForNDSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds);

extern DSVariablePool * DSCaseConsistentParameterAndStateSet(const DSCase *aCase);
extern DSVariablePool * DSCaseValidParameterAndStateSet(const DSCase *aCase);
extern DSVariablePool * DSCaseValidParameterSet(const DSCase *aCase);
extern DSVariablePool * DSCaseValidParameterSetByOptimizingFunction(const DSCase *aCase, const char * function, const bool minimize);
extern DSVariablePool * DSCaseValidParameterSetAtSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds);
extern DSVariablePool * DSCaseValidParameterSetAtSliceByOptimizingFunction(const DSCase *aCase,
                                                                           const DSVariablePool * lowerBounds,
                                                                           const DSVariablePool *upperBounds,
                                                                           const char * function,
                                                                           const bool minimize);

extern DSMatrixArray * DSCaseParseOptimizationFunction(const DSCase * aCase, const char * string);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Intersection of cases
#endif

extern DSPseudoCase * DSPseudoCaseFromIntersectionOfCases(const DSUInteger numberOfCases, const DSCase ** cases);
extern DSPseudoCase * DSPseudoCaseFromIntersectionOfCasesExcludingSlice(const DSUInteger numberOfCases, const DSCase ** cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames);

extern const bool DSCaseIntersectionListIsValid(const DSUInteger numberOfCases, const DSCase *firstCase, ...);
extern const bool DSCaseIntersectionIsValid(const DSUInteger numberOfCases, const DSCase **cases);
extern const bool DSCaseIntersectionIsValidAtSlice(const DSUInteger numberOfCases, const DSCase **cases,  const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds);

extern DSVertices * DSCaseIntersectionVerticesForSlice(const DSUInteger numberOfCases, const DSCase **cases, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const DSUInteger numberOfVariables, const char ** variables);
extern DSMatrixArray * DSCaseIntersectionFacesFor3DSliceAndConnectivity(const DSUInteger numberOfCases, const DSCase **cases, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable, const char *zVariable);


extern const bool DSCaseIntersectionExceptSliceIsValid(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames);
extern const bool DSCaseIntersectionExceptSliceIsValidAtSlice(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames, const DSVariablePool * lowerBounds, const DSVariablePool * upperBounds);


extern DSVariablePool * DSCaseIntersectionExceptSliceValidParameterSet(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames);
extern DSVariablePool * DSCaseIntersectionExceptSliceValidParameterSetWithConstraints(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames, const char ** constraints, DSUInteger numberOfConstraints);

extern DSVariablePool * DSCaseIntersectionExceptSliceValidParameterSetAtSlice(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames, const DSVariablePool * lowerBounds, const DSVariablePool * upperBounds);


extern DSVariablePool * DSCaseIntersectionExceptSliceValidParameterSetByOptimizingFunction(const DSUInteger numberOfCases,
                                                                                           const DSCase **cases,
                                                                                           const DSUInteger numberOfExceptions,
                                                                                           const char ** exceptionVarNames,
                                                                                           const char * function,
                                                                                           bool minimize);
extern DSVariablePool * DSCaseIntersectionExceptSliceValidParameterSetWithConstraintsByOptimizingFunction(const DSUInteger numberOfCases,
                                                                                                          const DSCase **cases,
                                                                                                          const DSUInteger numberOfExceptions,
                                                                                                          const char ** exceptionVarNames,
                                                                                                          const char ** constraints,
                                                                                                          DSUInteger numberOfConstraints,
                                                                                                          const char * function, bool minimize);
extern DSVariablePool * DSCaseIntersectionExceptSliceValidParameterSetAtSliceByOptimizingFunction(const DSUInteger numberOfCases,
                                                                                                  const DSCase **cases,
                                                                                                  const DSUInteger numberOfExceptions,
                                                                                                  const char ** exceptionVarNames,
                                                                                                  const DSVariablePool * lowerBounds,
                                                                                                  const DSVariablePool * upperBounds,
                                                                                                  const char * function,
                                                                                                  bool minimize);

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

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Data Serialization
#endif

extern DSCaseMessage * DSCaseEncode(const DSCase * aCase);
extern DSCase * DSCaseFromCaseMessage(const DSCaseMessage * message);
extern DSCase * DSCaseDecode(size_t length, const void * buffer);

extern DSDesignSpace * DSCaseEigenSubspaces(const DSCase * aCase);

#ifdef __cplusplus
__END_DECLS
#endif


#endif
