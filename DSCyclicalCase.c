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

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, deallocation and initialization
#endif


extern DSCyclicalCase * DSCyclicalCaseForCaseInDesignSpace(const DSDesignSpace * ds, const DSCase * aCase)
{
        DSCyclicalCase * aSubcase = NULL;
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
        aSubcase = DSSecureCalloc(sizeof(DSCyclicalCase), 1);
        aSubcase->internal = DSCyclicalCaseInternalForUnderdeterminedCase(aCase, ds);
        if (aSubcase->internal == NULL) {
                DSSecureFree(aSubcase);
                aSubcase = NULL;
                goto bail;
        }
        aSubcase->caseNumber = aCase->caseNumber;
        aSubcase->originalCase = DSCaseCopy(aCase);
bail:
        return aSubcase;
}

extern void DSCyclicalCaseFree(DSCyclicalCase * aSubcase)
{
        if (aSubcase == NULL) {
                DSError(M_DS_SUBCASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (aSubcase->internal != NULL)
                DSDesignSpaceFree(aSubcase->internal);
        if (aSubcase->originalCase != NULL)
                DSCaseFree(aSubcase->originalCase);
        DSSecureFree(aSubcase);
bail:
        return;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Getter =
#endif

extern const DSDesignSpace * DSCyclicalCaseInternalDesignSpace(const DSCyclicalCase * subcase)
{
        DSDesignSpace * ds = NULL;
        if (subcase == NULL) {
                DSError(M_DS_SUBCASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (subcase->internal == NULL)
                goto bail;
        ds = subcase->internal;
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
        return DSDesignSpaceNumberOfValidCases(DSCyclicalCaseInternalDesignSpace(cyclicalCase));
}

extern const DSUInteger DSCyclicalCaseNumberOfSubcases(const DSCyclicalCase * cyclicalCase)
{
        return DSDesignSpaceNumberOfCases(DSCyclicalCaseInternalDesignSpace(cyclicalCase));
}

extern DSCase * DSCyclicalCaseSubcaseWithCaseNumber(const DSCyclicalCase * cyclicalCase, const DSUInteger subcaseNumber)
{
        
        const DSDesignSpace *ds = NULL;
        DSCase * aSubcase = NULL;
        if (cyclicalCase == NULL) {
                DSError(M_DS_CASE_NULL ": Cyclical case is null", A_DS_ERROR);
                goto bail;
        }
        ds = DSCyclicalCaseInternalDesignSpace(cyclicalCase);
        aSubcase = DSDesignSpaceCaseWithCaseNumber(ds, subcaseNumber);
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
        numberOfValidCases = DSDesignSpaceNumberOfValidCases(aSubcase->internal);
        if (numberOfValidCases > 0)
                isValid = true;
bail:
        return isValid;
}

extern const bool DSCyclicalCaseIsValidAtPoint(const DSCyclicalCase *aSubcase, const DSVariablePool * variablesToFix);

extern const bool DSCyclicalCaseIsValidAtSlice(const DSCyclicalCase *aSubcase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds)
{
        bool isValid = false;
        DSDictionary * validCases = NULL;
        if (aSubcase == NULL) {
                DSError(M_DS_SUBCASE_NULL, A_DS_ERROR);
                goto bail;
        }
        validCases = DSDesignSpaceCalculateAllValidCasesForSlice(aSubcase->internal, lowerBounds, upperBounds);
        if (validCases == NULL)
                goto bail;
        if (DSDictionaryCount(validCases) != 0)
                isValid = true;
        DSDictionaryFreeWithFunction(validCases, DSCaseFree);
bail:
        return isValid;
}

extern DSDictionary * DSCyclicalCaseVerticesForSlice(const DSCyclicalCase *aSubcase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const DSUInteger numberOfVariables, const char ** variables);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

extern DSDictionary * DSCyclicalCaseCalculateAllValidSubcasesForSlice(const DSCyclicalCase * cyclicalCase,
                                                                      const DSVariablePool *lower,
                                                                      const DSVariablePool *upper)
{
        const DSDesignSpace * ds = DSCyclicalCaseInternalDesignSpace(cyclicalCase);
        DSDictionary * caseDictionary = NULL;
        DSUInteger i, numberValid = 0, numberValidSlice = 0;
        DSUInteger validCaseNumbers = 0;
        char nameString[100];
        DSCase * aCase = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        caseDictionary = DSDictionaryAlloc();
        numberValid = DSDesignSpaceNumberOfValidCases(ds);
        if (numberValid == 0)
                goto bail;
        for (i = 0; i < numberValid; i++) {
                validCaseNumbers = atoi(ds->validCases->names[i]);
                aCase = DSDesignSpaceCaseWithCaseNumber(ds, validCaseNumbers);
                sprintf(nameString, "%d_%d", cyclicalCase->caseNumber, validCaseNumbers);
                if (DSCaseIsValidAtSlice(aCase, lower, upper) == true)
                        DSDictionaryAddValueWithName(caseDictionary, nameString, aCase);
                else
                        DSCaseFree(aCase);
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
        DSUInteger i, numberValid = 0, numberValidSlice = 0;
        DSUInteger validCaseNumbers = 0;
        char nameString[100];
        DSCase * aCase = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        caseDictionary = DSDictionaryAlloc();
        numberValid = DSDesignSpaceNumberOfValidCases(ds);
        if (numberValid == 0)
                goto bail;
        for (i = 0; i < numberValid; i++) {
                validCaseNumbers = atoi(ds->validCases->names[i]);
                aCase = DSDesignSpaceCaseWithCaseNumber(ds, validCaseNumbers);
                sprintf(nameString, "%d_%d", cyclicalCase->caseNumber, validCaseNumbers);
                if (DSCaseIsValidAtSlice(aCase, lowerBounds, upperBounds) == true) {
                        vertices = DSCaseVerticesForSlice(aCase, lowerBounds, upperBounds, numberOfVariables, variables);
                        DSDictionaryAddValueWithName(caseDictionary, nameString, vertices);
                }
                DSCaseFree(aCase);
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
        DSUInteger i, numberValid = 0, numberValidSlice = 0;
        DSUInteger validCaseNumbers = 0;
        char nameString[100];
        DSCase * aCase = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        caseDictionary = DSDictionaryAlloc();
        numberValid = DSDesignSpaceNumberOfValidCases(ds);
        if (numberValid == 0)
                goto bail;
        for (i = 0; i < numberValid; i++) {
                validCaseNumbers = atoi(ds->validCases->names[i]);
                aCase = DSDesignSpaceCaseWithCaseNumber(ds, validCaseNumbers);
                sprintf(nameString, "%d_%d", cyclicalCase->caseNumber, validCaseNumbers);
                if (DSCaseIsValidAtSlice(aCase, lowerBounds, upperBounds) == true) {
                        vertices = DSCaseVerticesFor2DSlice(aCase, lowerBounds, upperBounds, xVariable, yVariable);
                        if (vertices != NULL)
                                DSDictionaryAddValueWithName(caseDictionary, nameString, vertices);
                }
                DSCaseFree(aCase);
        }
bail:
        return caseDictionary;
}

