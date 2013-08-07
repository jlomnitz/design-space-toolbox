/**
 * \file DSSSystem.h
 * \brief Header file with functions for dealing with S-System.
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

#ifndef __DS_SSYSTEM__
#define __DS_SSYSTEM__

#ifdef __cplusplus
__BEGIN_DECLS
#endif

#define M_DS_SSYS_NULL                  M_DS_NULL ": S-System is NULL"

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, deallocation and initialization
#endif

extern void DSSSystemFree(DSSSystem * ssys);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Factory functions
#endif

__deprecated extern DSSSystem * DSSSystemFromGMAWithDominantTerms(const DSGMASystem * gma, const DSUInteger * termList);
extern DSSSystem * DSSSystemWithTermsFromGMA(const DSGMASystem * gma, const DSUInteger * termArray);
extern DSSSystem * DSSSystemByParsingStringList(char * const * const string, const DSVariablePool * const Xd_a, ...);
extern DSSSystem * DSSSystemByParsingStrings(char * const * const strings, const DSVariablePool * const Xd_a, const DSUInteger numberOfEquations);

//extern DSSSystem * DSSSystemByRemovingAlgebraicConstraints(const DSSSystem * originalSSystem, const DSVariablePool * algebraicVariables);
extern DSSSystem * DSSSystemByRemovingAlgebraicConstraints(const DSSSystem * originalSSystem);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Getter functions
#endif

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark S-System Properties
#endif

extern double DSSSystemSteadyStateFunction(const DSSSystem *ssys, const DSVariablePool *Xi0, const char * function);
extern DSMatrix * DSSSystemSteadyStateValues(const DSSSystem *ssys, const DSVariablePool *Xi0);
extern DSMatrix * DSSSystemSteadyStateFlux(const DSSSystem *ssys, const DSVariablePool *Xi0);
extern DSMatrix * DSSSystemRouthArray(const DSSSystem *ssys, const DSVariablePool *Xi0);
extern DSUInteger DSSSystemPositiveRoots(const DSSSystem *ssys, const DSVariablePool *Xi0);
extern DSUInteger DSSSystemRouthIndex(const DSSSystem *ssys, const DSVariablePool *Xi0);
extern DSUInteger DSSSystemCharacteristicEquationCoefficientIndex(const DSSSystem *ssys, const DSVariablePool *Xi0);


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

extern DSMatrix * DSSSystemAd(const DSSSystem * ssys);
extern DSMatrix * DSSSystemAi(const DSSSystem * ssys);
extern DSMatrix * DSSSystemB(const DSSSystem * ssys);
extern DSMatrix * DSSSystemA(const DSSSystem * ssys);
extern DSMatrix * DSSSystemG(const DSSSystem *ssys);
extern DSMatrix * DSSSystemH(const DSSSystem *ssys);

extern const DSVariablePool * DSSSystemXd(const DSSSystem * const ssys);
extern const DSVariablePool * DSSSystemXd_a(const DSSSystem * const ssys);
extern const DSVariablePool * DSSSystemXi(const DSSSystem * const ssys);

extern const bool DSSSystemHasSolution(const DSSSystem * ssys);
extern const bool DSSSystemIsSingular(const DSSSystem *ssys);


#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

extern void DSSSystemPrint(const DSSSystem * ssys);
extern void DSSSystemPrintEquations(const DSSSystem *ssys);
extern void DSSSystemPrintSolution(const DSSSystem *ssys);
extern void DSSSystemPrintLogarithmicSolution(const DSSSystem *ssys);

#ifdef __cplusplus
__END_DECLS
#endif

#endif
