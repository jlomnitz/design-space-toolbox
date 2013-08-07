/**
 * \file DSSubcase.c
 * \brief Implementation file with functions for dealing with subcases.
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

/**
 * \todo Cleanup this file!!
 */
#include <string.h>
#include <stdio.h>
#include "DSSubcase.h"

extern DSDesignSpace * DSSubcaseInternalForUnderdeterminedCase(const DSCase * aCase, const DSDesignSpace * original);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, deallocation and initialization
#endif


extern DSSubcase * DSSubcaseForCaseInDesignSpace(const DSDesignSpace * ds, const DSCase * aCase)
{
        DSSubcase * aSubcase = NULL;
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
        aSubcase = DSSecureCalloc(sizeof(DSSubcase), 1);
        aSubcase->internal = DSSubcaseInternalForUnderdeterminedCase(aCase, ds);
        aSubcase->caseNumber = aCase->caseNumber;
        aSubcase->originalCase = DSCaseCopy(aCase);
bail:
        return aSubcase;
}

extern void DSSubcaseFree(DSSubcase * aSubcase)
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
#pragma mark - Getter functions
#endif

extern const DSDesignSpace * DSSubcaseInternalDesignSpace(const DSSubcase * subcase)
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

extern const DSCase * DSSubcaseOriginalCase(const DSSubcase * subcase)
{
        DSCase * aCase = NULL;
        if (subcase == NULL) {
                DSError(M_DS_SUBCASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (subcase->originalCase == NULL)
                goto bail;
        aCase = subcase->originalCase;
bail:
        return aCase;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Linear programming functions
#endif

extern const bool DSSubcaseIsValid(const DSSubcase *aSubcase)
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

extern const bool DSSubcaseIsValidAtPoint(const DSSubcase *aSubcase, const DSVariablePool * variablesToFix);

extern const bool DSSubcaseIsValidAtSlice(const DSSubcase *aSubcase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds)
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

extern DSDictionary * DSSubcaseVerticesForSlice(const DSSubcase *aSubcase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const DSUInteger numberOfVariables, const char ** variables);

extern DSDictionary * DSSubcaseVerticesFor2DSlice(const DSSubcase *aSubcase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable)
{
        DSDictionary *vertices = NULL;
        DSDictionary *validCases = NULL;
        DSUInteger i, count;
        DSVertices * vertex;
        DSCase * aCase;
        const char ** names = NULL;
        if (aSubcase == NULL) {
                DSError(M_DS_SUBCASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (lowerBounds == NULL || upperBounds == NULL) {
                DSError(M_DS_VAR_NULL, A_DS_ERROR);
                goto bail;
        }
        if (xVariable == NULL || yVariable == NULL) {
                DSError(M_DS_NULL ": Variable name(s) is null", A_DS_ERROR);
                goto bail;
        }
        validCases = DSDesignSpaceCalculateAllValidCasesForSlice(aSubcase->internal, lowerBounds, upperBounds);
        if (validCases == NULL)
                goto bail;
        count = DSDictionaryCount(validCases);
        names = DSDictionaryNames(validCases);
        validCases = DSDictionaryAlloc();
        for (i = 0; i < count; i++) {
                aCase = DSDictionaryValueForName(validCases, names[i]);
                vertex = DSCaseVerticesFor2DSlice(aCase, lowerBounds, upperBounds, xVariable, yVariable);
                if (vertex != NULL) {
                        DSDictionaryAddValueWithName(vertices, names[i], vertex);
                }
        }
        DSDictionaryFreeWithFunction(validCases, DSCaseFree);
bail:
        return vertices;
}

/*
DSCaseCd(aCase) = DSMatrixCalloc(numberOfConditions, numberOfXd);
DSCaseCi(aCase) = DSMatrixCalloc(numberOfConditions, numberOfXi);
DSCaseDelta(aCase) = DSMatrixCalloc(numberOfConditions, 1);
for (i = 0, l = 0; i < 2*numberOfEquations; i++) {
        if (i % 2 == 0) {
                a =  DSGMASystemAlpha;
                kd = DSGMASystemGd;
                ki = DSGMASystemGi;
        } else {
                a =  DSGMASystemBeta;
                kd = DSGMASystemHd;
                ki = DSGMASystemHi;
        }
        for (j = 0; j < DSGMASystemSignature(gma)[i]; j++) {
                if (j == termArray[i]-1)
                        continue;
                value = log10(DSMatrixDoubleValue(a(gma), i/2, termArray[i]-1)
                              /DSMatrixDoubleValue(a(gma), i/2, j));
                DSMatrixSetDoubleValue(DSCaseDelta(aCase), l, 0, value);
                for (k = 0; k < numberOfXd; k++) {
                        value = DSMatrixArrayDoubleWithIndices(kd(gma), i/2, termArray[i]-1, k);
                        value -= DSMatrixArrayDoubleWithIndices(kd(gma), i/2, j, k);
                        DSMatrixSetDoubleValue(DSCaseCd(aCase), l, k, value);
                }
                for (k = 0; k < numberOfXi; k++) {
                        value = DSMatrixArrayDoubleWithIndices(ki(gma), i/2, termArray[i]-1, k);
                        value -= DSMatrixArrayDoubleWithIndices(ki(gma), i/2, j, k);
                        DSMatrixSetDoubleValue(DSCaseCi(aCase), l, k, value);
                }
                l++;
        }
}
*/
//static DSDesignSpace * dsSubcaseCreateUniqueSystemSubcase(const DSCase *aCase, const DSGMASystem * modifiedGMA, const DSMatrix  * problematicEquations, const DSExpression ** augmentedEquations)
//{
//        DSDesignSpace * ds = NULL;
//        DSUInteger i, j;
//        DSUInteger * equationIndex = NULL;
//        char **equations;
//        DSExpression **caseEquations;
//        DSExpression *newEquations;
//        if (aCase == NULL) {
//                DSError(M_DS_CASE_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        if (modifiedGMA == NULL) {
//                DSError(M_DS_GMA_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        if (problematicEquations == NULL) {
//                DSError(M_DS_MAT_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        if (augmentedEquations == NULL) {
//                DSError(M_DS_NULL ": Augmented equations not found", A_DS_ERROR);
//                goto bail;
//        }
//        equationIndex = DSSecureMalloc(sizeof(DSUInteger)*DSMatrixColumns(problematicEquations));
//        for (i = 0; i < DSMatrixColumns(problematicEquations); i++) {
//                equationIndex[i] = DSMatrixRows(problematicEquations);
//                for (j = 0; j < DSMatrixRows(problematicEquations); j++) {
//                        if (DSMatrixDoubleValue(problematicEquations, j, i) == 0.0)
//                                continue;
//                        equationIndex[i] = j;
//                        break;
//                }
//        }
//        caseEquations = DSCaseEquations(aCase);
//        equations = DSSecureCalloc(sizeof(char *), DSCaseNumberOfEquations(aCase));
//        for (i = 0; i < DSCaseNumberOfEquations(aCase); i++) {
//                equations[i] = DSExpressionAsString(caseEquations[i]);
//                for (j = 0; j < DSMatrixColumns(problematicEquations); j++) {
//                        if (i != equationIndex[j])
//                                continue;
//                        newEquations = DSExpressionCopy(augmentedEquations[j]);
//                        DSSecureFree(equations[i]);
//                        equations[i] = DSExpressionAsString(newEquations);
//                        DSExpressionFree(newEquations);
//                }
//                //                printf("%i: %s\n", i, equations[i]);
//        }
//        ds = DSDesignSpaceByParsingStringsWithXi(DSGMASystemXd(modifiedGMA), DSGMASystemXi(modifiedGMA), equations, DSCaseNumberOfEquations(aCase));
//        for (i = 0; i < DSCaseNumberOfEquations(aCase); i++) {
//                DSSecureFree(equations[i]);
//                DSExpressionFree(caseEquations[i]);
//        }
//        DSSecureFree(equations);
//        DSSecureFree(caseEquations);
//bail:
//        if (equationIndex != NULL)
//                DSSecureFree(equationIndex);
//        return ds;
//}
//
//
//
//static DSDesignSpace * dsSubcaseInternalDesignSpaceForUnderdeterminedCase(const DSCase * aCase, const DSDesignSpace * original)
//{
//        DSDesignSpace *subcases = NULL;
//        DSGMASystem * temp = NULL;
//        DSMatrix * problematicEquations = NULL;
//        DSMatrixArray * problematicTerms = NULL;
//        DSMatrixArray * coefficientArray = NULL;
//        DSUInteger i, j, k, l;
//        DSExpression **augmentedEquations;
//        double value;
//        if (aCase == NULL) {
//                DSError(M_DS_CASE_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        if (original == NULL) {
//                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        if (DSCaseNumberOfEquations(aCase) != DSDesignSpaceNumberOfEquations(original)) {
//                DSError(M_DS_WRONG ": Number of equation in design space must match number of equations in case", A_DS_ERROR);
//                goto bail;
//        }
//        problematicEquations = DSSubcaseProblematicEquations(aCase);
//        if (problematicEquations == NULL)
//                goto bail;
//        problematicTerms = DSSubcaseProblematicTerms(aCase, problematicEquations);
//        if (problematicTerms == NULL)
//                goto bail;
//        coefficientArray = DSSubcaseCoefficientsOfInterest(aCase, problematicTerms);
//        if (coefficientArray == NULL)
//                goto bail;
//        if (DSMatrixArrayNumberOfMatrices(problematicTerms) != DSMatrixArrayNumberOfMatrices(coefficientArray))
//                goto bail;
//        temp = DSGMASystemCopy(original->gma);
//        augmentedEquations = DSSecureCalloc(sizeof(DSExpression *), DSMatrixColumns(problematicEquations));
//        for (i = 0; i < DSMatrixColumns(problematicEquations); i++) {
//                l = 0;
//                for (j = 0; j < DSMatrixRows(problematicEquations); j++) {
//                        if (DSMatrixDoubleValue(problematicEquations, j, i) == 0)
//                                continue;
//                        value = DSMatrixArrayDoubleWithIndices(coefficientArray, i, l, 0);
//                        for (k = 0; k < DSMatrixColumns(DSGMASystemAlpha(temp)); k++) {
//                                if (k+1 == aCase->signature[2*j])
//                                        DSMatrixSetDoubleValue((DSMatrix *)DSGMASystemAlpha(temp), j, k, 0.0f);
//                                else
//                                        DSMatrixSetDoubleValue((DSMatrix *)DSGMASystemAlpha(temp), j, k,
//                                                               DSMatrixDoubleValue(DSGMASystemAlpha(temp), j, k)*value);
//                                if (k+1 == aCase->signature[2*j+1])
//                                        DSMatrixSetDoubleValue((DSMatrix *)DSGMASystemBeta(temp), j, k, 0.0f);
//                                else
//                                        DSMatrixSetDoubleValue((DSMatrix *)DSGMASystemBeta(temp), j, k,
//                                                               DSMatrixDoubleValue(DSGMASystemBeta(temp), j, k)*value);
//                        }
//                        l++;
//                        augmentedEquations[i] = DSExpressionAddExpressions(augmentedEquations[i], DSGMASystemPositiveTermsForEquations(temp, j));
//                        augmentedEquations[i] = DSExpressionAddExpressions(augmentedEquations[i], DSGMASystemNegativeTermsForEquations(temp, j));
//                }
//        }
//        
//        subcases = dsSubcaseCreateUniqueSystemSubcase(aCase, temp, problematicEquations, (const DSExpression **)augmentedEquations);
//        if (subcases != NULL) {
//                DSDesignSpaceAddConditions(subcases, aCase->Cd, aCase->Ci, aCase->delta);
//        }
//        
//        for (i = 0; i < DSMatrixColumns(problematicEquations); i++)
//                DSExpressionFree(augmentedEquations[i]);
//        DSSecureFree(augmentedEquations);
//        DSGMASystemFree(temp);
//bail:
//        if (problematicEquations != NULL)
//                DSMatrixFree(problematicEquations);
//        if (problematicTerms != NULL)
//                DSMatrixArrayFree(problematicTerms);
//        if (coefficientArray != NULL)
//                DSMatrixArrayFree(coefficientArray);
//        return subcases;
//}
//
//
//
//extern void DSSubcaseDesignSpaceForUnderdeterminedCase(const DSCase * aCase, const DSDesignSpace * original)
//{
//        DSDesignSpace *subcases = NULL;
//        DSGMASystem * temp = NULL;
//        DSMatrix * problematicEquations = NULL;
//        DSMatrixArray * problematicTerms = NULL;
//        DSMatrixArray * coefficientArray = NULL;
//        DSUInteger i, j, k, l;
//        DSExpression **augmentedEquations;
//        DSStack * stack = NULL; 
//        char *string = NULL;
//        double value;
//        if (aCase == NULL) {
//                DSError(M_DS_CASE_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        if (original == NULL) {
//                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        if (DSCaseNumberOfEquations(aCase) != DSDesignSpaceNumberOfEquations(original)) {
//                DSError(M_DS_WRONG ": Number of equation in design space must match number of equations in case", A_DS_ERROR);
//                goto bail;
//        }
//        problematicEquations = DSSubcaseProblematicEquations(aCase);
//        if (problematicEquations == NULL)
//                goto bail;
//        problematicTerms = DSSubcaseProblematicTerms(aCase, problematicEquations);
//        if (problematicTerms == NULL)
//                goto bail;
//        coefficientArray = DSSubcaseCoefficientsOfInterest(aCase, problematicTerms);
//        if (coefficientArray == NULL)
//                goto bail;
//        if (DSMatrixArrayNumberOfMatrices(problematicTerms) != DSMatrixArrayNumberOfMatrices(coefficientArray))
//                goto bail;
//        temp = DSGMASystemCopy(original->gma);
//        augmentedEquations = DSSecureCalloc(sizeof(DSExpression *), DSMatrixColumns(problematicEquations));
//        for (i = 0; i < DSMatrixColumns(problematicEquations); i++) {
//                l = 0;
//                for (j = 0; j < DSMatrixRows(problematicEquations); j++) {
//                        if (DSMatrixDoubleValue(problematicEquations, j, i) == 0)
//                                continue;
//                        for (k = 0; k < DSMatrixColumns(DSGMASystemAlpha(temp)); k++) {
//                                value = DSMatrixArrayDoubleWithIndices(coefficientArray, i, l, 0);
//                                if (k+1 == aCase->signature[2*j])
//                                        value = 0.0;
//                                DSMatrixSetDoubleValue((DSMatrix *)DSGMASystemAlpha(temp), j, k, DSMatrixDoubleValue(DSGMASystemAlpha(temp), j, k)*value);
//                        }
//                        for (k = 0; k < DSMatrixColumns(DSGMASystemBeta(temp)); k++) {
//                                value = DSMatrixArrayDoubleWithIndices(coefficientArray, i, l, 0);
//                                if (k+1 == aCase->signature[2*j+1])
//                                        value = 0.0;
//                                DSMatrixSetDoubleValue((DSMatrix *)DSGMASystemBeta(temp), j, k, DSMatrixDoubleValue(DSGMASystemBeta(temp), j, k)*value);
//                        }
//                        l++;
//                        augmentedEquations[i] = DSExpressionAddExpressions(augmentedEquations[i], DSGMASystemPositiveTermsForEquations(temp, j));
//                        augmentedEquations[i] = DSExpressionAddExpressions(augmentedEquations[i], DSGMASystemNegativeTermsForEquations(temp, j));
//                }
//        }
//        
//        subcases = dsSubcaseCreateUniqueSystemSubcase(aCase, temp, problematicEquations, (const DSExpression **)augmentedEquations);
//        if (subcases != NULL) {
//                DSDesignSpaceAddConditions(subcases, aCase->Cd, aCase->Ci, aCase->delta);
//                stack = DSStackAlloc();
//                DSStackPush(stack, subcases);
//                string = DSSecureCalloc(sizeof(char), 100);
//                sprintf(string, "%d", aCase->caseNumber);
//                if (DSDictionaryValueForName(original->subcases, string) == NULL)
//                        DSDictionaryAddValueWithName(original->subcases, string, stack);
//                else
//                        DSStackFreeWithFunction(stack, DSDesignSpaceFree);
//                DSSecureFree(string);
//        }
//        
//        for (i = 0; i < DSMatrixColumns(problematicEquations); i++)
//                DSExpressionFree(augmentedEquations[i]);
//        DSSecureFree(augmentedEquations);
//        DSGMASystemFree(temp);
//bail:
//        if (problematicEquations != NULL)
//                DSMatrixFree(problematicEquations);
//        if (problematicTerms != NULL)
//                DSMatrixArrayFree(problematicTerms);
//        if (coefficientArray != NULL)
//                DSMatrixArrayFree(coefficientArray);
//        return;
//}
