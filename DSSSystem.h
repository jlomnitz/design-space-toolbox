/**
 * \file DSSSystem.h
 * \brief Header file with functions for dealing with S-System.
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

#include "DSTypes.h"
#include "DSDataSerialization.pb-c.h"

#ifndef __DS_SSYSTEM__
#define __DS_SSYSTEM__

#ifdef __cplusplus
__BEGIN_DECLS
#endif

#define M_DS_SSYS_NULL                  M_DS_NULL ": S-System is NULL"

#define DS_SSYSTEM_FLAG_SINGULAR          0x01
#define DS_SSYSTEM_FLAG_FREE_XD           0x02
#define DS_SSYSTEM_FLAG_FREE_XI           0x03


#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, deallocation and initialization
#endif

extern DSSSystem * DSSSystemCopy(const DSSSystem * ssys);
extern void DSSSystemFree(DSSSystem * ssys);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Factory functions
#endif

//__deprecated extern DSSSystem * DSSSystemFromGMAWithDominantTerms(const DSGMASystem * gma, const DSUInteger * termList);
extern DSSSystem * DSSSystemWithTermsFromGMA(const DSGMASystem * gma, const DSUInteger * termArray);
extern DSSSystem * DSSSystemByParsingStringList(char * const * const string, const DSVariablePool * const Xd_a, ...);
extern DSSSystem * DSSSystemByParsingStrings(char * const * const strings, const DSVariablePool * const Xd_a, const DSUInteger numberOfEquations);

//extern DSSSystem * DSSSystemByRemovingAlgebraicConstraints(const DSSSystem * originalSSystem, const DSVariablePool * algebraicVariables);
extern DSMatrixArray * DSSSystemSolvedAuxiliaryVariableMatrices(const DSSSystem * originalSSystem);
extern DSSSystem * DSSSystemByRemovingAlgebraicConstraints(const DSSSystem * originalSSystem);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Getter functions
#endif

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark S-System Properties
#endif

extern double DSSSystemSteadyStateFunction(const DSSSystem *ssys, const DSVariablePool *Xi0, const char * function);
extern DSMatrix * DSSSystemSteadyStateValues(const DSSSystem *ssys, const DSVariablePool *Xi0);
extern DSMatrix * DSSSystemAuxiliaryVariablesForSteadyState(const DSSSystem *ssys,
                                                            const DSVariablePool *Xdt0,
                                                            const DSVariablePool *Xi0);
extern DSMatrix * DSSSystemSteadyStateFluxForDependentVariables(const DSSSystem * ssys,
                                                                const DSVariablePool * Xd0,
                                                                const DSVariablePool * Xi0);
extern DSMatrix * DSSSystemSteadyStateFlux(const DSSSystem *ssys, const DSVariablePool *Xi0);
extern DSMatrix * DSSSystemRouthArrayForPoolTurnover(const DSSSystem *ssys, const DSMatrix * F, bool * hasImaginaryRoots);
extern DSMatrix * DSSSystemRouthArrayForSteadyState(const DSSSystem *ssys,
                                                    const DSVariablePool *Xd0,
                                                    const DSVariablePool *Xi0);
extern DSMatrix * DSSSystemRouthArray(const DSSSystem *ssys, const DSVariablePool *Xi0, bool * hasImaginaryRoots);
extern DSUInteger DSSSystemNumberOfPositiveRootsForRouthArray(const DSMatrix *routhArray);
extern DSUInteger DSSSystemPositiveRoots(const DSSSystem *ssys, const DSVariablePool *Xi0, bool * hasImaginaryRoots);
extern DSUInteger DSSSystemPositiveRootsForSteadyStateAndFlux(const DSSSystem *ssys,
                                                              const DSVariablePool *Xd0,
                                                              const DSVariablePool *Xi0,
                                                              const DSVariablePool *flux0);
extern DSUInteger DSSSystemPositiveRootsForSteadyState(const DSSSystem *ssys,
                                                       const DSVariablePool *Xd0,
                                                       const DSVariablePool *Xi0);
extern DSUInteger DSSSystemRouthIndex(const DSSSystem *ssys, const DSVariablePool *Xi0);
extern DSUInteger DSSSystemCharacteristicEquationCoefficientIndex(const DSSSystem *ssys, const DSVariablePool *Xi0);
extern DSUInteger DSSSystemCharacteristicEquationCoefficientsNumberSignChanges(const DSSSystem *ssys, const DSVariablePool *Xi0);


extern double DSSSystemLogarithmicGain(const DSSSystem *ssys, const char *XdName, const char *XiName);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Equation-related functions
#endif

extern const DSUInteger DSSSystemNumberOfEquations(const DSSSystem *ssys);
extern DSExpression ** DSSSystemEquations(const DSSSystem *ssys);
extern DSExpression ** DSSSystemSolution(const DSSSystem *ssys);
extern DSExpression ** DSSSystemLogarithmicSolution(const DSSSystem *ssys);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark S-System matrix functions
#endif

extern const DSMatrix * DSSSystemAlpha(const DSSSystem * ssys);
extern const DSMatrix * DSSSystemBeta(const DSSSystem * ssys);
extern const DSMatrix * DSSSystemGd(const DSSSystem * ssys);
extern const DSMatrix * DSSSystemGi(const DSSSystem * ssys);
extern const DSMatrix * DSSSystemHd(const DSSSystem * ssys);
extern const DSMatrix * DSSSystemHi(const DSSSystem * ssys);
extern const DSMatrix * DSSSystemM(const DSSSystem * ssys);

extern DSMatrix * DSSSystemM_a(const DSSSystem * ssys);

extern DSMatrix * DSSSystemAd(const DSSSystem * ssys);
extern DSMatrix * DSSSystemQd_a(const DSSSystem * ssys);
extern DSMatrix * DSSSystemQi_a(const DSSSystem * ssys);
extern DSMatrix * DSSSystemQB_a(const DSSSystem * ssys);
extern DSMatrix * DSSSystemAd_a(const DSSSystem * ssys);
extern DSMatrix * DSSSystemAd_t(const DSSSystem * ssys);

extern DSMatrix * DSSSystemAi(const DSSSystem * ssys);
extern DSMatrix * DSSSystemB(const DSSSystem * ssys);
extern DSMatrix * DSSSystemA(const DSSSystem * ssys);
extern DSMatrix * DSSSystemG(const DSSSystem *ssys);
extern DSMatrix * DSSSystemH(const DSSSystem *ssys);

extern const DSVariablePool * DSSSystemXd(const DSSSystem * const ssys);
extern const DSVariablePool * DSSSystemXd_a(const DSSSystem * const ssys);
extern const DSVariablePool * DSSSystemXd_t(const DSSSystem * const ssys);
extern const DSVariablePool * DSSSystemXi(const DSSSystem * const ssys);

extern const bool DSSSystemHasSolution(const DSSSystem * ssys);
extern bool DSSSystemIsSingular(const DSSSystem *ssys);

extern bool DSSSystemShouldFreeXd(const DSSSystem *ssys);
extern bool DSSSystemShouldFreeXi(const DSSSystem *ssys);


extern void DSSSystemSetIsSingular(DSSSystem *ssys, bool isSingular);
extern void DSSSystemSetShouldFreeXd(DSSSystem *ssys, bool shouldFreeXd);
extern void DSSSystemSetShouldFreeXi(DSSSystem *ssys, bool shouldFreeXi);


#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif




extern void DSSSystemRecalculateSolution(DSSSystem * ssys);
extern DSSSystem * DSSSystemWithQuasiSteadyStates(const DSSSystem * ssystem, DSUInteger numberOfVariables, const char ** variableNames);
extern void DSSSystemPrint(const DSSSystem * ssys);
extern void DSSSystemPrintEquations(const DSSSystem *ssys);
extern void DSSSystemPrintSolution(const DSSSystem *ssys);
extern void DSSSystemPrintLogarithmicSolution(const DSSSystem *ssys);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Data Serialization
#endif

extern DSSSystemMessage * DSSSystemEncode(const DSSSystem * ssys);
extern DSSSystem * DSSSystemFromSSystemMessage(const DSSSystemMessage * message);
extern DSSSystem * DSSSystemDecode(size_t length, const void * buffer);

#ifdef __cplusplus
__END_DECLS
#endif


#endif
