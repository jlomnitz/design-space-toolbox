/**
 * \file DSCyclicalCase.c
 * \brief Implementation file with functions for dealing with subcases.
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

/**
 * \todo Cleanup this file!!
 */
#include <string.h>
#include <stdio.h>
#include "DSCyclicalCase.h"

extern DSDesignSpace * DSCyclicalCaseInternalForUnderdeterminedCase(const DSCase * aCase, const DSDesignSpace * original);
extern DSStack * DSCyclicalCaseDesignSpacesForUnderdeterminedCase(const DSCase * aCase, const DSDesignSpace * original);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, deallocation and initialization
#endif


extern DSCyclicalCase * DSCyclicalCaseForCaseInDesignSpace(const DSDesignSpace * ds, const DSCase * aCase)
{
        DSCyclicalCase * cyclicalCase = NULL;
        DSStack * subcases;
        DSUInteger i;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseIsValid(aCase) == true) {
                goto bail;
        }
        cyclicalCase = DSSecureCalloc(sizeof(DSCyclicalCase), 1);
        subcases = DSCyclicalCaseDesignSpacesForUnderdeterminedCase(aCase, ds);
        if (subcases == NULL) {
                DSSecureFree(cyclicalCase);
                cyclicalCase = NULL;
                goto bail;
        }
        if (DSStackCount(subcases) == 0) {
                DSSecureFree(cyclicalCase);
                DSStackFree(subcases);
                cyclicalCase = NULL;
                goto bail;
        }
        cyclicalCase->numberOfInternal = DSStackCount(subcases);
        cyclicalCase->internalDesignspaces = DSSecureCalloc(sizeof(DSDesignSpace *), cyclicalCase->numberOfInternal);
        for (i = 0; i < cyclicalCase->numberOfInternal; i++) {
                cyclicalCase->internalDesignspaces[i] = (DSDesignSpace *)DSStackObjectAtIndex(subcases, i);
        }
        cyclicalCase->caseNumber = aCase->caseNumber;
        cyclicalCase->originalCase = DSCaseCopy(aCase);
        DSStackFree(subcases);
bail:
        return cyclicalCase;
}

extern void DSCyclicalCaseFree(DSCyclicalCase * aSubcase)
{
        DSUInteger i;
        if (aSubcase == NULL) {
                DSError(M_DS_SUBCASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (aSubcase->numberOfInternal > 0) {
                for (i = 0; i < aSubcase->numberOfInternal; i++) {
                        DSDesignSpaceFree(aSubcase->internalDesignspaces[i]);
                }
                DSSecureFree(aSubcase->internalDesignspaces);
        }
        if (aSubcase->originalCase != NULL)
                DSCaseFree(aSubcase->originalCase);
        DSSecureFree(aSubcase);
bail:
        return;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Getter =
#endif


extern const DSVariablePool * DSCyclicalCaseXd(const DSCyclicalCase * cyclicalCase)
{
        DSVariablePool * Xd = NULL;
        if (cyclicalCase == NULL) {
                DSError(M_DS_SUBCASE_NULL, A_DS_ERROR);
                goto bail;
        }
        Xd = DSSSystemXd(DSCaseSSystem(DSCyclicalCaseOriginalCase(cyclicalCase)));
bail:
        return Xd;
}

extern const DSVariablePool *  DSCyclicalCaseXi(const DSCyclicalCase * cyclicalCase)
{
        DSVariablePool * Xi = NULL;
        if (cyclicalCase == NULL) {
                DSError(M_DS_SUBCASE_NULL, A_DS_ERROR);
                goto bail;
        }
        Xi = DSSSystemXi(DSCaseSSystem(DSCyclicalCaseOriginalCase(cyclicalCase)));
bail:
        return Xi;
}

extern const DSDesignSpace * DSCyclicalCaseInternalDesignSpace(const DSCyclicalCase * subcase)
{
        DSDesignSpace * ds = NULL;
        if (subcase == NULL) {
                DSError(M_DS_SUBCASE_NULL, A_DS_ERROR);
                goto bail;
        }
//        if (subcase->internal == NULL)
//                goto bail;
//        ds = subcase->internal;
bail:
        return ds;
}

extern const DSCase * DSCyclicalCaseOriginalCase(const DSCyclicalCase * cyclicalCase)
{
        DSCase * aCase = NULL;
        if (cyclicalCase == NULL) {
                DSError(M_DS_SUBCASE_NULL, A_DS_ERROR);
                goto bail;
        }
        aCase = cyclicalCase->originalCase;
bail:
        return aCase;
}

extern const DSUInteger DSCyclicalCaseNumberOfValidSubcases(const DSCyclicalCase *cyclicalCase)
{
        DSDesignSpace * ds;
        DSUInteger i, numberOfValidSubcases = 0;
        if (cyclicalCase == NULL) {
                DSError(M_DS_CASE_NULL ": Cyclical case is null", A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < cyclicalCase->numberOfInternal; i++) {
                ds = cyclicalCase->internalDesignspaces[i];
                if (ds == NULL) {
                        DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                        goto bail;
                }
                numberOfValidSubcases += DSDesignSpaceNumberOfValidCases(ds);
        }
bail:
        return numberOfValidSubcases;
}

extern const DSUInteger DSCyclicalCaseNumberOfSubcases(const DSCyclicalCase * cyclicalCase)
{
        DSUInteger i, numberOfCases = 0;
        if (cyclicalCase == NULL) {
                DSError(M_DS_CASE_NULL ": Cyclical case is null", A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < cyclicalCase->numberOfInternal; i++) {
                numberOfCases += DSDesignSpaceNumberOfCases(cyclicalCase->internalDesignspaces[i]);
        }
bail:
        return numberOfCases;
}

extern DSCase * DSCyclicalCaseSubcaseWithCaseNumber(const DSCyclicalCase * cyclicalCase, const DSUInteger subcaseNumber)
{
        DSCase * aSubcase = NULL;
        DSUInteger i, numberInDesignSpace;
        DSUInteger caseNumber = subcaseNumber;
        if (cyclicalCase == NULL) {
                DSError(M_DS_CASE_NULL ": Cyclical case is null", A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < cyclicalCase->numberOfInternal; i++) {
                numberInDesignSpace = DSDesignSpaceNumberOfCases(cyclicalCase->internalDesignspaces[i]);
                if (caseNumber > numberInDesignSpace) {
                        caseNumber -= numberInDesignSpace;
                        continue;
                }
                aSubcase = DSDesignSpaceCaseWithCaseNumber(cyclicalCase->internalDesignspaces[i], caseNumber);
                break;
        }
bail:
        return aSubcase;
}

extern const DSCyclicalCase * DSCyclicalCaseCyclicalSubcaseWithCaseNumber(const DSCyclicalCase * cyclicalCase, const DSUInteger subcaseNumber)
{
        const DSCyclicalCase * aSubcase = NULL;
        DSUInteger i, numberInDesignSpace;
        DSUInteger caseNumber = subcaseNumber;
        if (cyclicalCase == NULL) {
                DSError(M_DS_CASE_NULL ": Cyclical case is null", A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < cyclicalCase->numberOfInternal; i++) {
                numberInDesignSpace = DSDesignSpaceNumberOfCases(cyclicalCase->internalDesignspaces[i]);
                if (caseNumber > numberInDesignSpace) {
                        caseNumber -= numberInDesignSpace;
                        continue;
                }
                aSubcase = DSDesignSpaceCyclicalCaseWithCaseNumber(cyclicalCase->internalDesignspaces[i], caseNumber);
                break;
        }
bail:
        return aSubcase;
}

extern const DSUInteger DSCyclicalCaseNumberOfEquations(const DSCyclicalCase *cyclicalCase)
{
        return DSCaseNumberOfEquations(DSCyclicalCaseOriginalCase(cyclicalCase));
}

extern DSExpression ** DSCyclicalCaseEquations(const DSCyclicalCase *cyclicalCase)
{
        return DSCaseEquations(DSCyclicalCaseOriginalCase(cyclicalCase));
}

extern const DSUInteger DSCyclicalCaseNumberOfConditions(const DSCyclicalCase *cyclicalCase)
{
        return DSCaseNumberOfConditions(DSCyclicalCaseOriginalCase(cyclicalCase));
}

extern DSExpression ** DSCyclicalCaseConditions(const DSCyclicalCase *cyclicalCase)
{
        return DSCaseConditions(DSCyclicalCaseOriginalCase(cyclicalCase));
}

extern DSExpression ** DSCyclicalCaseLogarithmicConditions(const DSCyclicalCase *cyclicalCase)
{
        return DSCaseLogarithmicConditions(DSCyclicalCaseOriginalCase(cyclicalCase));
}

extern const DSUInteger DSCyclicalCaseNumberOfBoundaries(const DSCyclicalCase *cyclicalCase)
{
        return DSCaseNumberOfBoundaries(DSCyclicalCaseOriginalCase(cyclicalCase));
}

extern DSExpression ** DSCyclicalCaseBoundaries(const DSCyclicalCase *cyclicalCase)
{
        return DSCaseBoundaries(DSCyclicalCaseOriginalCase(cyclicalCase));
}

extern DSExpression ** DSCyclicalCaseLogarithmicBoundaries(const DSCyclicalCase *cyclicalCase)
{
        return DSCaseLogarithmicBoundaries(DSCyclicalCaseOriginalCase(cyclicalCase));
}

extern DSUInteger DSCyclicalCaseNumber(const DSCyclicalCase *cyclicalCase)
{
        return DSCaseNumber(DSCyclicalCaseOriginalCase(cyclicalCase));
}

extern const DSUInteger * DSCyclicalCaseSignature(const DSCyclicalCase *cyclicalCase)
{
        return DSCaseSignature(DSCyclicalCaseOriginalCase(cyclicalCase));
}

extern char * DSCyclicalCaseSignatureToString(const DSCyclicalCase *cyclicalCase)
{
        return DSCaseSignatureToString(DSCyclicalCaseOriginalCase(cyclicalCase));
}

extern const DSSSystem *DSCyclicalCaseSSystem(const DSCyclicalCase *cyclicalCase)
{
        return DSCaseSSystem(DSCyclicalCaseOriginalCase(cyclicalCase));
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Linear programming functions
#endif

extern const bool DSCyclicalCaseIsValid(const DSCyclicalCase *aSubcase)
{
        bool isValid = false;
        DSUInteger numberOfValidCases = 0;
        if (aSubcase == NULL) {
                DSError(M_DS_SUBCASE_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfValidCases = DSCyclicalCaseNumberOfValidSubcases(aSubcase);
        if (numberOfValidCases > 0)
                isValid = true;
bail:
        return isValid;
}


extern const bool DSCyclicalCaseIsValidAtPoint(const DSCyclicalCase *aSubcase, const DSVariablePool * variablesToFix);

extern const bool DSCyclicalCaseIsValidAtSlice(const DSCyclicalCase *cyclicalCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds)
{
        bool isValid = false;
        DSCase * aCase;
        DSUInteger i, j, validCaseNumbers, numberValid;
        DSDesignSpace * ds;
        DSDictionary * validCases;
        if (cyclicalCase == NULL) {
                DSError(M_DS_SUBCASE_NULL, A_DS_ERROR);
                goto bail;
        }
        for (j = 0; j < cyclicalCase->numberOfInternal && isValid == false; j++) {
                ds = cyclicalCase->internalDesignspaces[j];
                if (ds == NULL) {
                        DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                        goto bail;
                }
                numberValid = DSDesignSpaceNumberOfValidCases(ds);
                if (numberValid == 0)
                        continue;
                validCases = DSDesignSpaceCalculateAllValidCasesForSlice(ds, lowerBounds, upperBounds);
                if (validCases == NULL)
                        continue;
                if (DSDictionaryCount(validCases) != 0)
                        isValid = true;
                DSDictionaryFreeWithFunction(validCases, DSCaseFree);
        }
bail:
        return isValid;
}

extern DSDictionary * DSCyclicalCaseVerticesForSlice(const DSCyclicalCase *aSubcase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const DSUInteger numberOfVariables, const char ** variables);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

extern DSDictionary * DSCyclicalCaseCalculateAllValidSubcasesByResolvingCyclicalCases(DSCyclicalCase *cyclicalCase)
{
        DSDictionary * caseDictionary = NULL, *subcaseDictionary;
        DSUInteger i, j, numberValidSubcases = 0;
        DSUInteger validCaseNumbers = 0;
        char nameString[100],  * subcaseString = NULL;
        const char ** subcaseNames;
        DSCase * aCase = NULL;
        DSCyclicalCase * cyclicalSubcase;
        if (cyclicalCase == NULL) {
                DSError(M_DS_CASE_NULL ": Cyclical Case is Null", A_DS_ERROR);
                goto bail;
        }
        caseDictionary = DSDictionaryAlloc();
        for (i = 0; i < DSCyclicalCaseNumberOfSubcases(cyclicalCase); i++) {
                validCaseNumbers = i+1;
                sprintf(nameString, "%d", validCaseNumbers);
                aCase = DSCyclicalCaseSubcaseWithCaseNumber(cyclicalCase, validCaseNumbers);
                cyclicalSubcase = (DSCyclicalCase *)DSCyclicalCaseCyclicalSubcaseWithCaseNumber(cyclicalCase, validCaseNumbers);
                if (cyclicalSubcase != NULL) {
                        subcaseDictionary = DSCyclicalCaseCalculateAllValidSubcasesByResolvingCyclicalCases(cyclicalSubcase);
                        numberValidSubcases = DSDictionaryCount(subcaseDictionary);
                        subcaseNames = DSDictionaryNames(subcaseDictionary);
                        for (j = 0; j < numberValidSubcases; j++) {
                                asprintf(&subcaseString, "%s_%s", nameString, subcaseNames[j]);
                                DSDictionaryAddValueWithName(caseDictionary, subcaseString, DSDictionaryValueForName(subcaseDictionary, subcaseNames[j]));
                        }
                        DSDictionaryFree(subcaseDictionary);
                } else if (DSCaseIsValid(aCase) == true) {
                        DSDictionaryAddValueWithName(caseDictionary, nameString, aCase);
                } else {
                        DSCaseFree(aCase);
                }
        }
bail:
        if (subcaseString != NULL)
                DSSecureFree(subcaseString);
        return caseDictionary;
}


extern DSDictionary * DSCyclicalCaseCalculateAllValidSubcases(const DSCyclicalCase * cyclicalCase)
{
        const DSDesignSpace * ds;
        DSDictionary * caseDictionary = NULL;
        DSUInteger i, j, numberValid = 0, numberValidSlice = 0;
        DSUInteger validCaseNumbers = 0;
        char nameString[100];
        DSCase * aCase = NULL;
        if (cyclicalCase == NULL) {
                DSError(M_DS_CASE_NULL ": Cyclical Case is Null", A_DS_ERROR);
                goto bail;
        }
        numberValidSlice = 0;
        caseDictionary = DSDictionaryAlloc();
        for (j = 0; j < cyclicalCase->numberOfInternal; j++) {
                ds = cyclicalCase->internalDesignspaces[j];
                if (ds == NULL) {
                        DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                        goto bail;
                }
                numberValid = DSDesignSpaceNumberOfValidCases(ds);
                if (numberValid == 0) {
                        numberValidSlice += DSDesignSpaceNumberOfCases(ds);
                        continue;
                }
                for (i = 0; i < numberValid; i++) {
                        validCaseNumbers = atoi(ds->validCases->names[i]);
                        aCase = DSDesignSpaceCaseWithCaseNumber(ds, validCaseNumbers);
                        sprintf(nameString, "%d_%d", cyclicalCase->caseNumber, numberValidSlice+validCaseNumbers);
                        DSDictionaryAddValueWithName(caseDictionary, nameString, aCase);
                }
                numberValidSlice += DSDesignSpaceNumberOfCases(ds);
        }
bail:
        return caseDictionary;
}

extern DSDictionary * DSCyclicalCaseCalculateAllValidSubcasesForSlice(const DSCyclicalCase * cyclicalCase,
                                                                      const DSVariablePool *lower,
                                                                      const DSVariablePool *upper)
{
        const DSDesignSpace * ds;
        DSDictionary * caseDictionary = NULL;
        DSUInteger i, j, numberValid = 0, numberValidSlice = 0;
        DSUInteger validCaseNumbers = 0;
        char nameString[100];
        DSCase * aCase = NULL;
        if (cyclicalCase == NULL) {
                DSError(M_DS_CASE_NULL ": Cyclical Case is Null", A_DS_ERROR);
                goto bail;
        }
        numberValidSlice = 0;
        caseDictionary = DSDictionaryAlloc();
        for (j = 0; j < cyclicalCase->numberOfInternal; j++) {
                ds = cyclicalCase->internalDesignspaces[j];
                if (ds == NULL) {
                        DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                        goto bail;
                }
                numberValid = DSDesignSpaceNumberOfValidCases(ds);
                if (numberValid == 0) {
                        numberValidSlice += DSDesignSpaceNumberOfCases(ds);
                        continue;
                }
                for (i = 0; i < numberValid; i++) {
                        validCaseNumbers = atoi(ds->validCases->names[i]);
                        aCase = DSDesignSpaceCaseWithCaseNumber(ds, validCaseNumbers);
                        sprintf(nameString, "%d_%d", cyclicalCase->caseNumber, numberValidSlice+validCaseNumbers);
                        if (DSCaseIsValidAtSlice(aCase, lower, upper) == true)
                                DSDictionaryAddValueWithName(caseDictionary, nameString, aCase);
                        else
                                DSCaseFree(aCase);
                }
                numberValidSlice += DSDesignSpaceNumberOfCases(ds);
        }
bail:
        return caseDictionary;
}

extern DSDictionary * DSCyclicalCaseVerticesForSlice(const DSCyclicalCase *cyclicalCase,
                                                     const DSVariablePool * lowerBounds,
                                                     const DSVariablePool *upperBounds,
                                                     const DSUInteger numberOfVariables,
                                                     const char ** variables)
{
        const DSDesignSpace * ds = DSCyclicalCaseInternalDesignSpace(cyclicalCase);
        DSDictionary * caseDictionary = NULL;
        DSVertices * vertices = NULL;
        DSUInteger i, j, numberValid = 0, numberValidSlice = 0;
        DSUInteger validCaseNumbers = 0;
        char nameString[100];
        DSCase * aCase = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        numberValidSlice = 0;
        caseDictionary = DSDictionaryAlloc();
        for (j = 0; j < cyclicalCase->numberOfInternal; j++) {
                ds = cyclicalCase->internalDesignspaces[j];
                if (ds == NULL) {
                        DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                        goto bail;
                }
                numberValid = DSDesignSpaceNumberOfValidCases(ds);
                if (numberValid == 0) {
                        numberValidSlice += DSDesignSpaceNumberOfCases(ds);
                        continue;
                }
                for (i = 0; i < numberValid; i++) {
                        validCaseNumbers = atoi(ds->validCases->names[i]);
                        aCase = DSDesignSpaceCaseWithCaseNumber(ds, validCaseNumbers);
                        sprintf(nameString, "%d_%d", cyclicalCase->caseNumber, numberValidSlice+validCaseNumbers);
                        if (DSCaseIsValidAtSlice(aCase, lowerBounds, upperBounds) == true) {
                                vertices = DSCaseVerticesForSlice(aCase, lowerBounds, upperBounds, numberOfVariables, variables);
                                DSDictionaryAddValueWithName(caseDictionary, nameString, vertices);
                        }
                        DSCaseFree(aCase);
                }
                numberValidSlice += DSDesignSpaceNumberOfCases(ds);
        }
bail:
        return caseDictionary;
}

extern DSDictionary * DSCyclicalCaseVerticesFor2DSlice(const DSCyclicalCase *cyclicalCase,
                                                       const DSVariablePool * lowerBounds,
                                                       const DSVariablePool *upperBounds,
                                                       const char * xVariable,
                                                       const char *yVariable)
{
        const DSDesignSpace * ds = DSCyclicalCaseInternalDesignSpace(cyclicalCase);
        DSDictionary * caseDictionary = NULL;
        DSVertices * vertices = NULL;
        DSUInteger i, j, numberValid = 0, numberValidSlice = 0;
        DSUInteger validCaseNumbers = 0;
        char nameString[100];
        DSCase * aCase = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        numberValidSlice = 0;
        caseDictionary = DSDictionaryAlloc();
        for (j = 0; j < cyclicalCase->numberOfInternal; j++) {
                ds = cyclicalCase->internalDesignspaces[j];
                if (ds == NULL) {
                        DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                        goto bail;
                }
                numberValid = DSDesignSpaceNumberOfValidCases(ds);
                if (numberValid == 0) {
                        numberValidSlice += DSDesignSpaceNumberOfCases(ds);
                        continue;
                }
                for (i = 0; i < numberValid; i++) {
                        validCaseNumbers = atoi(ds->validCases->names[i]);
                        aCase = DSDesignSpaceCaseWithCaseNumber(ds, validCaseNumbers);
                        sprintf(nameString, "%d_%d", cyclicalCase->caseNumber, numberValidSlice+validCaseNumbers);
                        if (DSCaseIsValidAtSlice(aCase, lowerBounds, upperBounds) == true) {
                                vertices = DSCaseVerticesFor2DSlice(aCase, lowerBounds, upperBounds, xVariable, yVariable);
                                if (vertices != NULL)
                                        DSDictionaryAddValueWithName(caseDictionary, nameString, vertices);
                        }
                        DSCaseFree(aCase);
                }
                numberValidSlice += DSDesignSpaceNumberOfCases(ds);
        }
bail:
        return caseDictionary;
}

