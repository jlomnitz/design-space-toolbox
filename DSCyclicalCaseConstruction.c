/**
 * \file DSCyclicalCaseConstruction.h
 * \brief
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
 *
 * \todo Add options to register custom functions.
 */

#include <stdio.h>
#include <string.h>
#include "DSCyclicalCase.h"
#include "DSMatrixArray.h"


extern void DSCyclicalCaseDesignSpaceCalculateSubCyclicalCases(DSDesignSpace *ds, DSDesignSpace * modifierDesignSpace, const DSUInteger * modifierSignature);
static DSUInteger dsCyclicalCaseNumberOfAugmentedSystems(const DSDesignSpace * original,
                                                         const DSMatrix * problematicEquations);
#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Cyclical calculation functions -
#endif


extern DSMatrix * dsSubcaseProblematicEquations(const DSCase * aCase)
{
        DSMatrix *problematic = NULL;
        bool isUnderdetermined = false;
        DSMatrix *nullspace = NULL, *A = NULL;
        DSUInteger i, j;
        double firstValue = NAN, current;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseHasSolution(aCase) == true) {
                goto bail;
        }
        A = DSSSystemA(aCase->ssys);
        if (DSMatrixRows(A) > DSMatrixColumns(A)) {
                printf("%i,%i\n", DSMatrixRows(A), DSMatrixColumns(A));
                DSMatrixPrint(A);
        }
        nullspace = DSMatrixLeftNullspace(A);
        if (nullspace == NULL)
                goto bail;
        problematic = DSMatrixIdenticalRows(nullspace);
//        isUnderdetermined = true;
//        problematic = DSMatrixCalloc(DSMatrixRows(nullspace), DSMatrixColumns(nullspace));
//        for (i = 0; i < DSMatrixColumns(nullspace); i++) {
//                firstValue = NAN;
//                for (j = 0; j < DSMatrixRows(nullspace); j++) {
//                        current = DSMatrixDoubleValue(nullspace, j, i);
//                        if (fabs(current) < 1E-14)
//                                continue;
//                        DSMatrixSetDoubleValue(problematic, j, i, 1.0);
//                        if (isnan(firstValue) == true) {
//                                firstValue = current;
//                        } else if (fabs(current - firstValue) >= 1E-14) {
//                                isUnderdetermined = false;
//                                break;
//                        }
//                }
//                if (j != DSMatrixRows(nullspace))
//                        break;
//        }
//        
//        if (isUnderdetermined == false) {
//                DSMatrixFree(problematic);
//                problematic = NULL;
//        } else {
//                printf("------ %i -------\n", DSCaseNumber(aCase));
//                DSMatrixPrint(problematic);
//                printf("\n");
//                DSMatrixPrint(nullspace);
//                printf("\n");
//                DSMatrixPrint(DSMatrixIdenticalRows(nullspace));
//                printf("=================\n");
//        }
        DSMatrixFree(nullspace);
bail:
        if (A != NULL)
                DSMatrixFree(A);
        return problematic;
}

extern DSMatrixArray * dsSubcaseProblematicTerms(const DSCase *aCase, const DSMatrix * dependentEquations)
{
        DSMatrixArray *dependentTerms = NULL;
        DSMatrix *G, *H, *g, *h, *termMatrix, *nullspace, *coefficients;
        DSUInteger i, j,k, *dependent, numDependent;
        double value;
        double sign;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseHasSolution(aCase) == true) {
                goto bail;
        }
        if (dependentEquations == NULL) {
                DSError(M_DS_MAT_NULL ": Array of dependent equations must not be null", A_DS_ERROR);
                goto bail;
        }
        dependentTerms = DSMatrixArrayAlloc();
        dependent = DSSecureCalloc(sizeof(DSUInteger), DSSSystemNumberOfEquations(aCase->ssys));
        G = DSSSystemG(aCase->ssys);
        H = DSSSystemH(aCase->ssys);
        for (i = 0; i < DSMatrixColumns(dependentEquations); i++) {
                numDependent = 0;
                sign = 1.0;
                for (j = 0; j < DSMatrixRows(dependentEquations); j++) {
                        if (DSMatrixDoubleValue(dependentEquations, j, i) == 1.0) {
                                dependent[numDependent++] = j;
                        }
                }
                if (numDependent == 1) {
//                        DSMatrixPrint(termMatrix);
                        DSMatrixArrayFree(dependentTerms);
                        dependentTerms = NULL;
                        break;
                }
                g = DSMatrixSubMatrixIncludingRows(G, numDependent, dependent);
                h = DSMatrixSubMatrixIncludingRows(H, numDependent, dependent);
                termMatrix = DSMatrixAppendMatrices(g, h, false);
                DSMatrixFree(g);
                DSMatrixFree(h);
                nullspace = DSMatrixIdenticalRows(termMatrix);
//                nullspace = DSMatrixLeftNullspace(termMatrix);
                if (nullspace == NULL) {
//                        DSMatrixPrint(termMatrix);
                        DSMatrixArrayFree(dependentTerms);
                        dependentTerms = NULL;
                        break;
                }
                coefficients = DSMatrixCalloc(numDependent, DSMatrixColumns(nullspace));
                if (numDependent == 0)
                        break;
                for (j = 0; j < DSMatrixRows(nullspace); j++) {
                        for (k = 0; k < DSMatrixColumns(nullspace); k++) {
                                value = DSMatrixDoubleValue(nullspace, j, k);
                                if (fabs(value) <= 1E-14) {
                                        DSMatrixSetDoubleValue(nullspace, j, k, 0.0);
                                        continue;
                                }
                                DSMatrixSetDoubleValue(nullspace, j, k, copysign(1.0, value));
                                if ((j / numDependent) == 0)
                                        DSMatrixSetDoubleValue(coefficients, j % numDependent, k, DSMatrixDoubleValue(DSSSystemAlpha(aCase->ssys), dependent[j % numDependent], 0));
                                else
                                        DSMatrixSetDoubleValue(coefficients, j % numDependent,
                                                               k,
                                                               DSMatrixDoubleValue(coefficients, j%numDependent, k)-DSMatrixDoubleValue(DSSSystemBeta(aCase->ssys), dependent[j % numDependent], 0));
                                sign *= -1.0;
                        }
                }
                DSMatrixFree(termMatrix);
                DSMatrixFree(nullspace);
                DSMatrixArrayAddMatrix(dependentTerms, coefficients);
        }
        DSMatrixFree(G);
        DSMatrixFree(H);
        DSSecureFree(dependent);
bail:
        return dependentTerms;
}

extern DSMatrixArray * dsSubcaseCoefficientsOfInterestALT(const DSCase * aCase, const DSMatrix * problematicEquations, const DSMatrixArray * problematicTerms)
{
        DSMatrixArray * coefficientArray = NULL;
        DSMatrix *problematic = NULL, *coefficients;
        DSUInteger i, j, k, * inCycle, cycleLength;
        double min, value;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseHasSolution(aCase) == true) {
                goto bail;
        }
        if (problematicTerms == NULL) {
                DSError(M_DS_MAT_NULL ": Matrix of problematic equations is null", A_DS_ERROR);
                goto bail;
        }
        if (problematicTerms == NULL) {
                DSError(M_DS_MAT_NULL ": Array of problematic terms is null", A_DS_ERROR);
                goto bail;
        }
        coefficientArray = DSMatrixArrayAlloc();
        inCycle = DSSecureCalloc(sizeof(DSUInteger), DSMatrixRows(problematicEquations));
        for (i = 0; i < DSMatrixArrayNumberOfMatrices(problematicTerms); i++) {
                for (j = 0, cycleLength = 0; j < DSMatrixRows(problematicEquations); j++){
                        if (DSMatrixDoubleValue(problematicEquations, j, i) == 0.0f)
                                continue;
                        inCycle[cycleLength++] = j;
                }
//                if (DSMatrixRows(DSMatrixArrayMatrix(problematicTerms, i)) > DSMatrixColumns(DSMatrixArrayMatrix(problematicTerms, i))) {
//                        printf("QRD:\n");
//                        DSMatrixQRD(DSMatrixTranspose(DSMatrixArrayMatrix(problematicTerms, i)));
//                        printf("\n");
//                        continue;
//                }
                problematic = DSMatrixIdenticalRows(DSMatrixArrayMatrix(problematicTerms, i));//DSMatrixLeftNullspace(DSMatrixArrayMatrix(problematicTerms, i));
//                if (temp == NULL) {
//                        DSMatrixFree(temp);
//                        continue;
//                }
                DSMatrixRoundToSignificantFigures(problematic, 14);
                for (k = 0; k < DSMatrixColumns(problematic); k++) {
                        min = INFINITY;
                        for (j = 0; j < DSMatrixRows(problematic); j++) {
                                value = DSMatrixDoubleValue(problematic, j, k);
                                if (fabs(value) == 0)
                                        continue;
                                min = ((fabs(value) <= fabs(min)) ? value : min);
                        }
                        for (j = 0; j < DSMatrixRows(problematic); j++) {
                                value = DSMatrixDoubleValue(problematic, j, k);
                                if (value == 0)
                                        continue;
                                DSMatrixSetDoubleValue(problematic, j, k, value/min);
                        }
                }
                coefficients = DSMatrixCalloc(DSMatrixRows(problematic), 1);
                for (j = 0; j < DSMatrixRows(problematic); j++) {
                        value = fabs(DSMatrixDoubleValue(coefficients, j, 0));
                        for (k = 0; k < DSMatrixColumns(problematic); k++) {
                                value += fabs(DSMatrixDoubleValue(problematic, j, k));
                        }
                        if (value == 0) {
                                DSMatrixFree(coefficients);
                                coefficients = NULL;
                                break;
                        }
                        DSMatrixSetDoubleValue(coefficients, j, 0, value);
                }
                DSMatrixFree(problematic);
                if (coefficients != NULL)
                        DSMatrixArrayAddMatrix(coefficientArray, coefficients);
        }
bail:
        return coefficientArray;
}

extern DSMatrixArray * dsSubcaseCoefficientsOfInterest2(const DSCase * aCase, const DSMatrixArray * problematicTerms)
{
        DSMatrixArray * coefficientArray = NULL;
        DSMatrix *problematic = NULL, *coefficients;
        DSUInteger i, j, k;
        double min, value;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseHasSolution(aCase) == true) {
                goto bail;
        }
        if (problematicTerms == NULL) {
                DSError(M_DS_MAT_NULL ": Array of problematic terms is null", A_DS_ERROR);
                goto bail;
        }
        
        coefficientArray = DSMatrixArrayAlloc();
        for (i = 0; i < DSMatrixArrayNumberOfMatrices(problematicTerms); i++) {
                //                if (DSMatrixRows(DSMatrixArrayMatrix(problematicTerms, i)) > DSMatrixColumns(DSMatrixArrayMatrix(problematicTerms, i))) {
                //                        DSMatrixPrint(DSMatrixArrayMatrix(problematicTerms, i));
                ////                        continue;
                //                }
                problematic = DSMatrixLeftNullspace(DSMatrixArrayMatrix(problematicTerms, i));
                if (problematic == NULL)
                        continue;
                DSMatrixRoundToSignificantFigures(problematic, 14);
                for (k = 0; k < DSMatrixColumns(problematic); k++) {
                        min = INFINITY;
                        for (j = 0; j < DSMatrixRows(problematic); j++) {
                                value = DSMatrixDoubleValue(problematic, j, k);
                                if (fabs(value) == 0)
                                        continue;
                                min = ((fabs(value) <= fabs(min)) ? value : min);
                        }
                        for (j = 0; j < DSMatrixRows(problematic); j++) {
                                value = DSMatrixDoubleValue(problematic, j, k);
                                if (value == 0)
                                        continue;
                                DSMatrixSetDoubleValue(problematic, j, k, value/min);
                        }
                }
                coefficients = DSMatrixCalloc(DSMatrixRows(problematic), 1);
                for (j = 0; j < DSMatrixRows(problematic); j++) {
                        value = fabs(DSMatrixDoubleValue(coefficients, j, 0));
                        for (k = 0; k < DSMatrixColumns(problematic); k++) {
                                value += fabs(DSMatrixDoubleValue(problematic, j, k));
                        }
                        if (value == 0) {
                                DSMatrixFree(coefficients);
                                coefficients = NULL;
                                break;
                        }
                        DSMatrixSetDoubleValue(coefficients, j, 0, value);
                }
                DSMatrixFree(problematic);
                if (coefficients != NULL)
                        DSMatrixArrayAddMatrix(coefficientArray, coefficients);
        }
bail:
        return coefficientArray;
}

extern DSMatrixArray * dsSubcaseCoefficientsOfInterest(const DSCase * aCase, const DSMatrixArray * problematicTerms)
{
        DSMatrixArray * coefficientArray = NULL;
        DSMatrix *nullspace, *problematic = NULL, *coefficients;
        DSUInteger i, j, k;
        double min, value;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseHasSolution(aCase) == true) {
                goto bail;
        }
        if (problematicTerms == NULL) {
                DSError(M_DS_MAT_NULL ": Array of problematic terms is null", A_DS_ERROR);
                goto bail;
        }

        coefficientArray = DSMatrixArrayAlloc();
        for (i = 0; i < DSMatrixArrayNumberOfMatrices(problematicTerms); i++) {
//                if (DSMatrixRows(DSMatrixArrayMatrix(problematicTerms, i)) > DSMatrixColumns(DSMatrixArrayMatrix(problematicTerms, i))) {
//                        DSMatrixPrint(DSMatrixArrayMatrix(problematicTerms, i));
////                        continue;
//                }
                problematic = DSMatrixLeftNullspace(DSMatrixArrayMatrix(problematicTerms, i));
//                if (nullspace == NULL)
//                        continue;
//                DSMatrixPrint(nullspace);
//                problematic = DSMatrixIdenticalRows(nullspace);
//                DSMatrixFree(nullspace);
//                printf("\n");
//                DSMatrixPrint(problematic);
                if (problematic == NULL)
                        continue;
                DSMatrixRoundToSignificantFigures(problematic, 14);
                for (k = 0; k < DSMatrixColumns(problematic); k++) {
                        min = INFINITY;
                        for (j = 0; j < DSMatrixRows(problematic); j++) {
                                value = DSMatrixDoubleValue(problematic, j, k);
                                if (fabs(value) == 0)
                                        continue;
                                min = ((fabs(value) <= fabs(min)) ? value : min);
                        }
                        for (j = 0; j < DSMatrixRows(problematic); j++) {
                                value = DSMatrixDoubleValue(problematic, j, k);
                                if (value == 0)
                                        continue;
                                DSMatrixSetDoubleValue(problematic, j, k, value/min);
                        }
                }
                coefficients = DSMatrixCalloc(DSMatrixRows(problematic), 1);
                for (j = 0; j < DSMatrixRows(problematic); j++) {
                        value = fabs(DSMatrixDoubleValue(coefficients, j, 0));
                        for (k = 0; k < DSMatrixColumns(problematic); k++) {
                                value += fabs(DSMatrixDoubleValue(problematic, j, k));
                        }
                        if (value == 0) {
                                DSMatrixFree(coefficients);
                                coefficients = NULL;
                                break;
                        }
                        DSMatrixSetDoubleValue(coefficients, j, 0, value);
                }
                DSMatrixFree(problematic);
                if (coefficients != NULL)
                        DSMatrixArrayAddMatrix(coefficientArray, coefficients);
        }
bail:
        return coefficientArray;
}

static DSDesignSpace * dsSubcaseCreateUniqueSystemSubcase(const DSCase *aCase, const DSGMASystem * modifiedGMA, const DSMatrix  * problematicEquations, const DSExpression ** augmentedEquations)
{
        DSDesignSpace * ds = NULL;
        DSUInteger i, j;
        DSUInteger * equationIndex = NULL;
        char **equations, *temp_rhs, *temp_lhs;
        DSExpression **caseEquations;
        DSExpression  *eqLHS;
        DSVariablePool * Xda = NULL;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (modifiedGMA == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (problematicEquations == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (augmentedEquations == NULL) {
                DSError(M_DS_NULL ": Augmented equations not found", A_DS_ERROR);
                goto bail;
        }
        caseEquations = DSCaseEquations(aCase);
        equations = DSSecureCalloc(sizeof(char *), DSCaseNumberOfEquations(aCase));
        Xda = DSVariablePoolAlloc();
        for (i = 0; i < DSVariablePoolNumberOfVariables(DSGMASystemXd_a(modifiedGMA)); i++)
                DSVariablePoolAddVariableWithName(Xda, DSVariableName(DSVariablePoolVariableAtIndex(DSGMASystemXd_a(modifiedGMA), i)));
        equationIndex = DSSecureMalloc(sizeof(DSUInteger)*DSMatrixColumns(problematicEquations));
        for (i = 0; i < DSCaseNumberOfEquations(aCase); i++)
                equations[i] = DSExpressionAsString(caseEquations[i]);
        for (i = 0; i < DSMatrixColumns(problematicEquations); i++) {
                for (j = 0; j < DSMatrixRows(problematicEquations); j++) {
                        if (DSMatrixDoubleValue(problematicEquations, j, i) == 0.0)
                                continue;
                        equationIndex[i] = j;
                        break;
                }
        }
        for (i = 0; i < DSCaseNumberOfEquations(aCase); i++) {
                for (j = 0; j < DSMatrixColumns(problematicEquations); j++) {
                        if (i != equationIndex[j])
                                continue;
                        DSSecureFree(equations[i]);
                        eqLHS = DSExpressionEquationLHSExpression(caseEquations[i]);
                        temp_lhs = DSExpressionAsString(eqLHS);
                        temp_rhs = DSExpressionAsString(augmentedEquations[j]);
                        DSExpressionFree(eqLHS);
                        equations[i] = DSSecureCalloc(sizeof(char),strlen(temp_rhs) + strlen(temp_lhs)+4);
                        equations[i] = strcpy(equations[i], temp_lhs);
                        equations[i] = strcat(equations[i], " = ");
                        equations[i] = strcat(equations[i], temp_rhs);
                        DSSecureFree(temp_rhs);
                        DSSecureFree(temp_lhs);
                }
        }
        ds = DSDesignSpaceByParsingStringsWithXi(equations, Xda, DSGMASystemXi(modifiedGMA), DSCaseNumberOfEquations(aCase));
        for (i = 0; i < DSCaseNumberOfEquations(aCase); i++) {
                DSSecureFree(equations[i]);
                DSExpressionFree(caseEquations[i]);
        }
        DSSecureFree(equations);
        DSSecureFree(caseEquations);
        DSVariablePoolFree(Xda);
bail:
        if (equationIndex != NULL)
                DSSecureFree(equationIndex);
        return ds;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Multiple augmented systems to obtain appropriate dynamics -
#endif

static void dsAddConstraintsForSubdominantDecays(DSDesignSpace * subcase, const DSCase * aCase, const DSDesignSpace * original, const DSMatrix * problematicEquations, const DSMatrixArray * coefficientArray, const DSUInteger * subdominantDecays, const DSUInteger * subdominantDecayTerms)
{
        const DSGMASystem * gma = NULL;
        DSUInteger i, j, k, m, l, index = 0;
        DSUInteger numberOfConditions = 0, numberOfXd, numberOfXi;
        DSMatrix * Cd, *Ci, *delta;
        double subCoefficient = 0, coefficient, value;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (original == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (problematicEquations == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (subdominantDecays == NULL) {
                DSError(M_DS_NULL ": Array indicating the subdominant decay equation is NULL", A_DS_ERROR);
                goto bail;
        }
        if (subdominantDecayTerms == NULL) {
                DSError(M_DS_NULL ": Array indicating the subdominant decay terms is NULL", A_DS_ERROR);
                goto bail;
        }
        gma = DSDesignSpaceGMASystem(original);
        numberOfXd = DSVariablePoolNumberOfVariables(DSGMASystemXd(gma));
        numberOfXi = DSVariablePoolNumberOfVariables(DSGMASystemXi(gma));
        for (i = 0; i < DSMatrixColumns(problematicEquations); i++) {
                for (j = 0; j < DSMatrixRows(problematicEquations); j++) {
                        if ((DSUInteger)DSMatrixDoubleValue(problematicEquations, j, i) == 0)
                                continue;
                        if (subdominantDecays[i] == j) {
                                numberOfConditions += (DSDesignSpaceSignature(original)[j*2+1]-2);
                                continue;
                        }
                        numberOfConditions += (DSDesignSpaceSignature(original)[j*2+1]-1);
                }
        }
        if (numberOfConditions == 0) {
                goto bail;
        }
        Cd = DSMatrixAlloc(numberOfConditions, DSVariablePoolNumberOfVariables(DSGMASystemXd(gma)));
        Ci = DSMatrixAlloc(numberOfConditions, DSVariablePoolNumberOfVariables(DSGMASystemXi(gma)));
        delta =DSMatrixAlloc(numberOfConditions, 1);
        index = 0;

        for (i = 0; i < DSMatrixColumns(problematicEquations); i++) {
                k = 0;
                for (j = 0; j < DSMatrixRows(problematicEquations); j++) {
                        if (DSMatrixDoubleValue(problematicEquations, j, i) == 0)
                                continue;
                        if (j == subdominantDecays[i]) {
                                subCoefficient = DSMatrixArrayDoubleWithIndices(coefficientArray, i, k, 0);
                        }
                        k++;
                }
                l = 0;
                for (j = 0; j < DSMatrixRows(problematicEquations); j++) {
                        if ((DSUInteger)DSMatrixDoubleValue(problematicEquations, j, i) == 0)
                                continue;
                        coefficient = DSMatrixArrayDoubleWithIndices(coefficientArray, i, l, 0);
                        for (k = 0; k < DSDesignSpaceSignature(original)[j*2+1];  k++) {
                                if (k+1 == DSCaseSignature(aCase)[j*2+1])
                                        continue;
                                if (k == subdominantDecayTerms[i] && j == subdominantDecays[i])
                                        continue;
                                value = log10((subCoefficient/coefficient)*DSMatrixDoubleValue(DSGMASystemBeta(gma), subdominantDecays[i], subdominantDecayTerms[i])
                                              /DSMatrixDoubleValue(DSGMASystemBeta(gma), j, k));
                                DSMatrixSetDoubleValue(delta, index, 0, value);
                                for (m = 0; m < numberOfXd; m++) {
                                        value = DSMatrixArrayDoubleWithIndices(DSGMASystemHd(gma), subdominantDecays[i], subdominantDecayTerms[i], m);
                                        value -= DSMatrixArrayDoubleWithIndices(DSGMASystemHd(gma), j, k, m);
                                        DSMatrixSetDoubleValue(Cd, index, m, value);
                                }
                                for (m = 0; m < numberOfXi; m++) {
                                        value = DSMatrixArrayDoubleWithIndices(DSGMASystemHi(gma), subdominantDecays[i], subdominantDecayTerms[i], m);
                                        value -= DSMatrixArrayDoubleWithIndices(DSGMASystemHi(gma), j, k, m);
                                        DSMatrixSetDoubleValue(Ci, index, m, value);
                                }
                                index++;
                        }
                        l++;
                }
        }
        DSDesignSpaceAddConditions(subcase, Cd, Ci, delta);
        DSMatrixFree(Cd);
        DSMatrixFree(Ci);
        DSMatrixFree(delta);
bail:
        return;
}

static DSDesignSpace * dsCyclicalCaseCreateUniqueAugmentedSystem(const DSCase *aCase, const DSGMASystem * modifiedGMA, const DSMatrix  * problematicEquations, const DSExpression ** augmentedEquations, const DSUInteger * subdominantDecays)
{
        DSDesignSpace * ds = NULL;
        DSUInteger i, j;
        char **equations, *temp_rhs, *temp_lhs;
        DSExpression **caseEquations;
        DSExpression  *eqLHS;
        DSVariablePool * Xda = NULL;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (modifiedGMA == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (problematicEquations == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (augmentedEquations == NULL) {
                DSError(M_DS_NULL ": Augmented equations not found", A_DS_ERROR);
                goto bail;
        }
        caseEquations = DSCaseEquations(aCase);
        equations = DSSecureCalloc(sizeof(char *), DSCaseNumberOfEquations(aCase));
        Xda = DSVariablePoolAlloc();
        for (i = 0; i < DSVariablePoolNumberOfVariables(DSGMASystemXd_a(modifiedGMA)); i++)
                DSVariablePoolAddVariableWithName(Xda, DSVariableName(DSVariablePoolVariableAtIndex(DSGMASystemXd_a(modifiedGMA), i)));
        for (i = 0; i < DSCaseNumberOfEquations(aCase); i++) {
                equations[i] = DSExpressionAsString(caseEquations[i]);
        }
        for (j = 0; j < DSMatrixColumns(problematicEquations); j++) {
                i = subdominantDecays[j];
                DSSecureFree(equations[i]);
                eqLHS = DSExpressionEquationLHSExpression(caseEquations[i]);
                temp_lhs = DSExpressionAsString(eqLHS);
                temp_rhs = DSExpressionAsString(augmentedEquations[j]);
                equations[i] = NULL;
                asprintf(&equations[i], "%s = %s", temp_lhs, temp_rhs);
                DSExpressionFree(eqLHS);
                DSSecureFree(temp_rhs);
                DSSecureFree(temp_lhs);
        }
        ds = DSDesignSpaceByParsingStringsWithXi(equations, Xda, DSGMASystemXi(modifiedGMA), DSCaseNumberOfEquations(aCase));
        for (i = 0; i < DSCaseNumberOfEquations(aCase); i++) {
                DSSecureFree(equations[i]);
                DSExpressionFree(caseEquations[i]);
        }
        DSVariablePoolFree(Xda);
        DSSecureFree(equations);
        DSSecureFree(caseEquations);
bail:
        return ds;
}

static DSDesignSpace * dsCyclicalCaseAugmentedSystemForSubdominantDecays(const DSCase * aCase,
                                                                         const DSDesignSpace * original,
                                                                         DSMatrix * problematicEquations,
                                                                         const DSMatrixArray * problematicTerms,
                                                                         const DSMatrixArray * coefficientArray,
                                                                         const DSUInteger *subdominantDecaySpecies,
                                                                         const DSUInteger *subdominantDecayTerm)
{
        DSDesignSpace * augmentedSystem = NULL, *modifierDesignSpace = NULL;
        DSGMASystem * gma = NULL;
        DSExpression ** augmentedEquations = NULL, ** dsEquations, ** caseEquations;
        DSUInteger i, j, k, l,*signature;
        double subCoefficient, value;
        char ** subcaseEquations;
        DSUInteger positiveTerms;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (original == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (problematicEquations == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (problematicTerms == NULL) {
                DSError(M_DS_MAT_NULL ": Problematic term matrix array is NULL", A_DS_ERROR);
                goto bail;
        }
        if (coefficientArray == NULL) {
                DSError(M_DS_MAT_NULL ": Coefficient matrix array is NULL", A_DS_ERROR);
                goto bail;
        }
        if (subdominantDecaySpecies == NULL) {
                DSError(M_DS_NULL ": Array indicating the subdominant decay equation is NULL", A_DS_ERROR);
                goto bail;
        }
        gma = DSGMASystemCopy(DSDesignSpaceGMASystem(original));
        augmentedEquations = DSSecureCalloc(sizeof(DSExpression *), DSMatrixColumns(problematicEquations));
        subcaseEquations =  DSSecureCalloc(sizeof(char *), DSGMASystemNumberOfEquations(gma));
        signature = DSSecureMalloc(sizeof(DSUInteger)*DSGMASystemNumberOfEquations(gma)*2);
        for (i = 0; i < DSMatrixColumns(problematicEquations); i++) {
                positiveTerms = 0;
                l = 0;
                for (j = 0; j < DSMatrixRows(problematicEquations); j++) {
                        if (DSMatrixDoubleValue(problematicEquations, j, i) == 0)
                                continue;
                        positiveTerms += DSDesignSpaceSignature(original)[2*j]-1;
                        if (j == subdominantDecaySpecies[i]) {
                                subCoefficient = DSMatrixArrayDoubleWithIndices(coefficientArray, i, l, 0);
                        }
                        l++;
                }
                l = 0;
                for (j = 0; j < DSMatrixRows(problematicEquations); j++) {
                        if (DSMatrixDoubleValue(problematicEquations, j, i) == 0)
                                continue;
                        value = DSMatrixArrayDoubleWithIndices(coefficientArray, i, l, 0);
                        for (k = 0; k < DSMatrixColumns(DSGMASystemAlpha(gma)); k++) {
                                if (k+1 == aCase->signature[2*j]) {
                                        DSMatrixSetDoubleValue((DSMatrix *)DSGMASystemAlpha(gma), j, k, 0.0f);
                                } else {
                                        DSMatrixSetDoubleValue((DSMatrix *)DSGMASystemAlpha(gma), j, k,
                                                               DSMatrixDoubleValue(DSGMASystemAlpha(gma), j, k)*value/subCoefficient);
                                }

                        }
                        l++;
                        augmentedEquations[i] = DSExpressionAddExpressions(augmentedEquations[i], DSGMASystemPositiveTermsForEquations(gma, j));
                }
                if (positiveTerms == 0)
                        goto bail;
                j = subdominantDecaySpecies[i];
                for (k = 0; k < DSMatrixColumns(DSGMASystemBeta(gma)); k++) {
                        if (k+1 == aCase->signature[2*j+1]) {
                                DSMatrixSetDoubleValue((DSMatrix *)DSGMASystemBeta(gma), j, k, 0.0f);
                        } else {
                                DSMatrixSetDoubleValue((DSMatrix *)DSGMASystemBeta(gma), j, k,
                                                       DSMatrixDoubleValue(DSGMASystemBeta(gma), j, k));
                        }
                }
                augmentedEquations[i] = DSExpressionAddExpressions(augmentedEquations[i], DSGMASystemNegativeTermsForEquations(gma, j));
        }
        augmentedSystem = dsCyclicalCaseCreateUniqueAugmentedSystem(aCase,
                                                                    gma,
                                                                    problematicEquations,
                                                                    (const DSExpression **)augmentedEquations,
                                                                    subdominantDecaySpecies);
        DSDesignSpaceAddConditions(augmentedSystem, DSCaseCd(aCase), DSCaseCi(aCase), DSCaseDelta(aCase));
        dsAddConstraintsForSubdominantDecays(augmentedSystem,
                                             aCase,
                                             original,
                                             problematicEquations,
                                             coefficientArray,
                                             subdominantDecaySpecies,
                                             subdominantDecayTerm);
        DSDesignSpaceSetSerial(augmentedSystem, true);
        DSDesignSpaceSetCyclical(augmentedSystem, true);
        dsEquations = DSDesignSpaceEquations(original);
        for (i = 0; i < DSGMASystemNumberOfEquations(gma); i++) {
                subcaseEquations[i] = DSExpressionAsString(dsEquations[i]);
                signature[2*i] = DSCaseSignature(aCase)[2*i];
                signature[2*i+1] = DSCaseSignature(aCase)[2*i+1];
                DSExpressionFree(dsEquations[i]);
        }
        DSSecureFree(dsEquations);
        dsEquations = DSDesignSpaceEquations(augmentedSystem);
        caseEquations = DSCaseEquations(aCase);
        for (i = 0; i < DSMatrixColumns(problematicEquations); i++) {
                for (j = 0; j < DSMatrixRows(problematicEquations); j++) {
                        if (DSMatrixDoubleValue(problematicEquations, j, i) == 0)
                                continue;
                        signature[2*j] = 0;
                        signature[2*j+1] = 0;
                        DSSecureFree(subcaseEquations[j]);
                        subcaseEquations[j] = DSExpressionAsString(caseEquations[j]);
                }
                j = subdominantDecaySpecies[i];
                DSSecureFree(subcaseEquations[j]);
                subcaseEquations[j] = DSExpressionAsString(dsEquations[j]);
        }
        for (i = 0; i < DSDesignSpaceNumberOfEquations(augmentedSystem); i++) {
                DSExpressionFree(dsEquations[i]);
                DSExpressionFree(caseEquations[i]);
        }
        DSSecureFree(dsEquations);
        DSSecureFree(caseEquations);
        modifierDesignSpace = DSDesignSpaceByParsingStringsWithXi(subcaseEquations,
                                                                  DSGMASystemXd_a(DSDesignSpaceGMASystem(original)),
                                                                  DSGMASystemXi(DSDesignSpaceGMASystem(original)),
                                                                  DSDesignSpaceNumberOfEquations(original));
        modifierDesignSpace->Cd = DSMatrixCopy(augmentedSystem->Cd);
        modifierDesignSpace->Ci = DSMatrixCopy(augmentedSystem->Ci);
        modifierDesignSpace->delta = DSMatrixCopy(augmentedSystem->delta);
        DSCyclicalCaseDesignSpaceCalculateSubCyclicalCases(augmentedSystem, modifierDesignSpace, signature);
        for (i = 0; i < DSDesignSpaceNumberOfEquations(original); i++) {
                DSSecureFree(subcaseEquations[i]);
        }
        DSSecureFree(subcaseEquations);
        DSSecureFree(signature);
bail:
        if (augmentedEquations != NULL) {
                for (i = 0; i < DSMatrixColumns(problematicEquations); i++) {
                        if (augmentedEquations[i] != NULL)
                                DSExpressionFree(augmentedEquations[i]);
                }
                DSSecureFree(augmentedEquations);
        }
        if (gma != NULL) {
                DSGMASystemFree(gma);
        }
        if (modifierDesignSpace != NULL) {
                DSDesignSpaceFree(modifierDesignSpace);
        }
        return augmentedSystem;
}

extern void DSCyclicalCaseDesignSpaceCalculateSubCyclicalCase(DSDesignSpace *ds, DSCase * aCase, const DSDesignSpace * modifierDS)
{
        DSUInteger caseNumber;
        char * string = NULL;
        DSCyclicalCase * aSubcase = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        string = DSSecureCalloc(sizeof(char), 100);
        caseNumber = DSCaseNumber(aCase);
        sprintf(string, "%d", caseNumber);
        if (DSDictionaryValueForName(DSDesignSpaceCyclicalCaseDictionary(ds), string) == NULL) {
                aSubcase = DSCyclicalCaseForCaseInDesignSpace(modifierDS, aCase);
                if (aSubcase != NULL) {
                        DSDictionaryAddValueWithName((DSDictionary *)DSDesignSpaceCyclicalCaseDictionary(ds), string, aSubcase);
                }
        }
        if (string != NULL)
                DSSecureFree(string);
bail:
        return;
}

extern void DSCyclicalCaseDesignSpaceCalculateSubCyclicalCases(DSDesignSpace *ds, DSDesignSpace * modifierDesignSpace, const DSUInteger * modifierSignature)
{
        DSUInteger i, j, numberOfCases;
        DSCase * aCase = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (modifierDesignSpace == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (modifierSignature == NULL) {
                DSError(M_DS_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfCases = DSDesignSpaceNumberOfCases(ds);
        if (numberOfCases == 0) {
                goto bail;
        }
        for (i = 0; i < numberOfCases; i++) {
                aCase = DSDesignSpaceCaseWithCaseNumber(ds, i+1);
                for (j = 0; j < DSCaseNumberOfEquations(aCase); j++) {
                        if (modifierSignature[2*j] != 0)
                                aCase->signature[2*j] = modifierSignature[2*j];
                        if (modifierSignature[2*j+1] != 0)
                                aCase->signature[2*j+1] = modifierSignature[2*j+1];
                }
                DSCyclicalCaseDesignSpaceCalculateSubCyclicalCase(ds, aCase, modifierDesignSpace);
                DSCaseFree(aCase);
        }
bail:
        return;
}

static DSUInteger dsCyclicalCaseNumberOfAugmentedSystems(const DSDesignSpace * original,
                                                         const DSMatrix * problematicEquations)
{
        DSUInteger i, j, max = 0, count;
        if (original == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (problematicEquations == NULL) {
                DSError(M_DS_MAT_NULL ": Matrix of problematic equations is NULL", A_DS_ERROR);
                goto bail;
        }
        max = 1;
        for (i = 0; i < DSMatrixColumns(problematicEquations); i++) {
                count = 0;
                for (j = 0; j < DSMatrixRows(problematicEquations); j++) {
                        if (DSMatrixDoubleValue(problematicEquations, j, i) == 0.0f)
                                continue;
                        count += (DSDesignSpaceSignature(original)[j*2+1] > 1);
                }
                max *= count;
        }
bail:
        return max;
}

static DSStack * dsCyclicalCaseCreateAugmentedSystems(const DSCase * aCase,
                                                      const DSDesignSpace * original,
                                                      DSMatrix * problematicEquations,
                                                      const DSMatrixArray * problematicTerms,
                                                      const DSMatrixArray * coefficientArray)
{
        DSStack * augmentedSystemsStack = NULL;
        DSUInteger i, j, k, current, index, max, *numberOfequations, numberOfTerms;
        DSUInteger * decayEquations = NULL;
        DSUInteger * decayTerms;
        DSDesignSpace * subcase;
        DSDictionary * validity;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (original == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseNumberOfEquations(aCase) != DSDesignSpaceNumberOfEquations(original)) {
                DSError(M_DS_WRONG ": Number of equation in design space must match number of equations in case", A_DS_ERROR);
                goto bail;
        }
        if (problematicEquations == NULL)
                goto bail;
        if (problematicTerms == NULL)
                goto bail;
        if (coefficientArray == NULL)
                goto bail;
        if (DSMatrixArrayNumberOfMatrices(problematicTerms) != DSMatrixArrayNumberOfMatrices(coefficientArray))
                goto bail;
        decayEquations = DSSecureCalloc(sizeof(DSUInteger), DSMatrixColumns(problematicEquations));
        decayTerms = DSSecureCalloc(sizeof(DSUInteger), DSMatrixColumns(problematicEquations));
        numberOfequations = DSSecureCalloc(sizeof(DSUInteger), DSMatrixColumns(problematicEquations));
        max = dsCyclicalCaseNumberOfAugmentedSystems(original, problematicEquations);
        augmentedSystemsStack = DSStackAlloc();
        for (i = 0; i < DSMatrixColumns(problematicEquations); i++) {
                for (j = 0; j < DSMatrixRows(problematicEquations); j++) {
                        if ((DSUInteger)DSMatrixDoubleValue(problematicEquations, j, i) == 0)
                                continue;
                        numberOfequations[i] += (DSDesignSpaceSignature(original)[j*2+1] > 1);
                }
        }
        for (i = 0; i < max; i++) {
                current = i;
                numberOfTerms = 0;
                for (j = 0; j < DSMatrixColumns(problematicEquations); j++) {
                        decayEquations[j] = current % numberOfequations[j];
                        index = 0;
                        for (k = 0; k < DSMatrixRows(problematicEquations); k++) {
                                if ((DSUInteger)DSMatrixDoubleValue(problematicEquations, k, j) == 0)
                                        continue;
                                if (DSDesignSpaceSignature(original)[k*2+1] > 1) {
                                        if (index == decayEquations[j])
                                                break;
                                        else
                                                index++;
                                }
                        }
                        decayEquations[j] = k;
                        numberOfTerms += DSDesignSpaceSignature(original)[k*2+1];
                        current = current / numberOfequations[j];
                }
                for (j = 0; j < numberOfTerms; j++) {
                        index = j;
                        for (k = 0; k < DSMatrixColumns(problematicEquations); k++) {
                                decayTerms[k] = index % DSDesignSpaceSignature(original)[decayEquations[k]*2+1];
                                if (DSCaseSignature(aCase)[decayEquations[k]*2+1] == decayTerms[k]+1)
                                        break;
                                index = index / DSDesignSpaceSignature(original)[decayEquations[k]*2+1];
                        }
                        if (k != DSMatrixColumns(problematicEquations))
                                continue;
                        subcase = dsCyclicalCaseAugmentedSystemForSubdominantDecays(aCase,
                                                                                    original,
                                                                                    problematicEquations,
                                                                                    problematicTerms,
                                                                                    coefficientArray,
                                                                                    decayEquations,
                                                                                    decayTerms);
                        validity = DSDesignSpaceCalculateAllValidCasesByResolvingCyclicalCases(subcase);
                        if (validity == NULL) {
                                DSDesignSpaceFree(subcase);
                        } else {
                                DSStackPush(augmentedSystemsStack, subcase);
                        }
                        DSDictionaryFree(validity);
                }
        }
        DSSecureFree(decayEquations);
        DSSecureFree(decayTerms);
        DSSecureFree(numberOfequations);
bail:
        return augmentedSystemsStack;
}


static DSMatrix * dsCyclicalCaseExpandLcMatrix(const DSVariablePool * Xd,
                                               const DSMatrix * Lc,
                                               const DSVariablePool * yc)
{
        DSUInteger i, j, index;
        DSMatrix * newLc = NULL;
        if (Xd == NULL || yc == NULL) {
                DSError(M_DS_VAR_NULL, A_DS_ERROR);
                goto bail;
        }
        if (Lc == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        newLc = DSMatrixCalloc(DSMatrixRows(Lc), DSVariablePoolNumberOfVariables(Xd));
        for (i = 0; i < DSMatrixRows(Lc); i++) {
                for (j = 0; j < DSMatrixColumns(Lc); j++) {
                        index = DSVariablePoolIndexOfVariableWithName(Xd, DSVariableName(DSVariablePoolVariableAtIndex(yc, j)));
                        DSMatrixSetDoubleValue(newLc, i, index, DSMatrixDoubleValue(Lc, i, j));
                }
        }
bail:
        return newLc;
}


static DSUInteger dsCyclicalCasePrimaryCycleVariableIndices(const DSCase * aCase,
                                                            DSMatrix * problematicEquations,
                                                            DSUInteger ** primaryVariables)
{
        DSUInteger numberOfCycles = 0, * cycleIndices;
        DSUInteger i, j, k, index;
        DSMatrix * Ad, *temp, *nullspace;
        double value, matrixValue;
        DSUInteger max;
        if (primaryVariables == NULL) {
                DSError(M_DS_NULL ": Pointer to hold primary cycle variable indices cannot be null", A_DS_ERROR);
                goto bail;
        }
        *primaryVariables = NULL;
        if (problematicEquations == NULL) {
                goto bail;
        }
        numberOfCycles = DSMatrixColumns(problematicEquations);
        *primaryVariables = DSSecureCalloc(sizeof(DSUInteger), numberOfCycles);
        cycleIndices = DSSecureCalloc(sizeof(DSUInteger), DSMatrixRows(problematicEquations));
        Ad = DSSSystemAd(DSCaseSSys(aCase));
        for (i = 0; i < numberOfCycles; i++) {
                k = 0;
                for (j = 0; j < DSMatrixRows(problematicEquations); j++) {
                        if (DSMatrixDoubleValue(problematicEquations, j, i) == 0)
                                continue;
                        cycleIndices[k++] = j;
                }
                if (k == 0) {
                        DSSecureFree(*primaryVariables);
                        *primaryVariables = NULL;
                        break;
                }
                if (k == 1) {
                        (*primaryVariables)[i] = cycleIndices[0];
                        continue;
                }
                temp = DSMatrixSubMatrixIncludingRowsAndColumns(Ad, k, k, cycleIndices, cycleIndices);
                nullspace = DSMatrixRightNullspace(temp);
                if (nullspace == NULL) {
                        max = 0;
                } else {
                        value = NAN;
                        max = 0;
                        index = 0;
                        for (j = 0; j < DSMatrixRows(nullspace); j++) {
                                matrixValue = fabs(DSMatrixDoubleValue(nullspace, j, 0));
                                if (matrixValue < 1e-14)
                                        continue;
                                if (isnan(value)) {
                                        max = j;
                                        value = matrixValue;
                                        continue;
                                        
                                }
                                if (matrixValue < value) {
                                        value = matrixValue;
                                        max = j;
                                }
                                index++;
                                if (index >= k) {
                                        break;
                                }
                        }
                }
                (*primaryVariables)[i] = cycleIndices[max];
                if (nullspace != NULL)
                        DSMatrixFree(nullspace);
                DSMatrixFree(temp);
                
        }
        DSSecureFree(cycleIndices);
        DSMatrixFree(Ad);

bail:
        return numberOfCycles;
}

static DSUInteger dsCyclicalCaseAllSecondaryCycleVariables(const DSMatrix * problematicEquations,
                                                           const DSMatrixArray * coefficientArray,
                                                           const DSUInteger numberOfCycles,
                                                           const DSUInteger * primaryVariables,
                                                           DSUInteger ** cycleIndices,
                                                           double ** coefficients)
{
        DSUInteger numberSecondaryVariables = 0;
        DSUInteger i, j, k, index, * coefficientIndices = NULL;
        if (cycleIndices == NULL) {
                DSError(M_DS_NULL ": Pointer to hold primary cycle variable indices cannot be null", A_DS_ERROR);
                goto bail;
        }
        *cycleIndices = NULL;
        *coefficients = NULL;
        if (problematicEquations == NULL) {
                goto bail;
        }
        coefficientIndices = DSSecureCalloc(sizeof(DSUInteger), DSMatrixArrayNumberOfMatrices(coefficientArray));
        for (i = 0; i < DSMatrixColumns(problematicEquations); i++) {
                k = 0;
                for (j = 0; j < DSMatrixRows(problematicEquations); j++) {
                        if (DSMatrixDoubleValue(problematicEquations, j, i) == 0)
                                continue;
                        if (primaryVariables[i] == j) {
                                coefficientIndices[i] = k;
                                continue;
                        }
                        numberSecondaryVariables++;
                        k++;
                }
        }
        if (numberSecondaryVariables == 0) {
                goto bail;
        }
        *cycleIndices = DSSecureCalloc(sizeof(DSUInteger), numberSecondaryVariables);
        *coefficients = DSSecureCalloc(sizeof(double), numberSecondaryVariables);
        index = 0;
        for (i = 0; i < DSMatrixColumns(problematicEquations); i++) {
                k = 0;
                for (j = 0; j < DSMatrixRows(problematicEquations); j++) {
                        if (DSMatrixDoubleValue(problematicEquations, j, i) == 0)
                                continue;
                        if (primaryVariables[i] == j)
                                continue;
                        (*coefficients)[index] = DSMatrixArrayDoubleWithIndices(coefficientArray, i, k++, 0)/DSMatrixArrayDoubleWithIndices(coefficientArray, i, coefficientIndices[i], 0);
                        (*cycleIndices)[index++] = j;
                }
        }
bail:
        if (coefficientIndices != NULL)
                DSSecureFree(coefficientIndices);
        return numberSecondaryVariables;
}

static DSUInteger dsCyclicalCaseSecondaryCycleVariableIndicesForCycle(DSMatrix * problematicEquations,
                                                                      DSUInteger cycleNumber,
                                                                      DSUInteger * primaryVariables,
                                                                      DSUInteger ** cycleIndices)
{
        DSUInteger numberOfCycleVariables = 0;
        DSUInteger i, j;
        if (cycleIndices == NULL) {
                DSError(M_DS_NULL ": Pointer to hold primary cycle variable indices cannot be null", A_DS_ERROR);
                goto bail;
        }
        *cycleIndices = NULL;
        if (problematicEquations == NULL) {
                goto bail;
        }
        for (i = 0; i < DSMatrixRows(problematicEquations); i++) {
                if (DSMatrixDoubleValue(problematicEquations, i, cycleNumber) == 0)
                        continue;
                numberOfCycleVariables++;
        }
        if (numberOfCycleVariables == 0) {
                goto bail;
        }
        numberOfCycleVariables -= 1;
        if (numberOfCycleVariables == 0) {
                goto bail;
        }
        *cycleIndices = DSSecureCalloc(sizeof(DSUInteger), numberOfCycleVariables);
        j = 0;
        for (i = 0; i < DSMatrixRows(problematicEquations); i++) {
                if (DSMatrixDoubleValue(problematicEquations, i, cycleNumber) == 0)
                        continue;
                if (i != primaryVariables[cycleNumber]) {
                        (*cycleIndices)[j] = i;
                        j++;
                }
                if (j == numberOfCycleVariables)
                        break;
        }
bail:
        return numberOfCycleVariables;
}

static void dsCyclicalCasePartitionSolutionMatrices(const DSCase * aCase,
                                                    const DSUInteger numberOfSecondaryVariables,
                                                    const DSUInteger * secondaryVariables,
                                                    DSMatrix ** ADn,
                                                    DSMatrix ** ADc,
                                                    DSMatrix ** AIn,
                                                    DSMatrix ** Bn,
                                                    DSVariablePool ** yn,
                                                    DSVariablePool ** yc)
{
        DSMatrix * tempMatrix, *Ad, *Ai, *B;
        const DSSSystem * ssystem;
        DSUInteger i;
        char * name;
        if (ADn == NULL || ADc == NULL || AIn == NULL || Bn == NULL) {
                DSError(M_DS_NULL ": Matrix pointers to hold partitioned matrices cannot be null", A_DS_ERROR);
                goto bail;
        }
        *ADn = NULL;
        *ADc = NULL;
        *AIn = NULL;
        *Bn = NULL;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (secondaryVariables == NULL) {
                goto bail;
        }
        ssystem = DSCaseSSys(aCase);
        Ad = DSSSystemAd(ssystem);
        Ai = DSSSystemAi(ssystem);
        B = DSSSystemB(ssystem);
        tempMatrix = DSMatrixSubMatrixIncludingRows(Ad, numberOfSecondaryVariables, secondaryVariables);
        *ADc = DSMatrixSubMatrixExcludingColumns(tempMatrix, numberOfSecondaryVariables, secondaryVariables);
        *ADn = DSMatrixSubMatrixIncludingColumns(tempMatrix, numberOfSecondaryVariables, secondaryVariables);
        DSMatrixFree(tempMatrix);
        *AIn = DSMatrixSubMatrixIncludingRows(Ai, numberOfSecondaryVariables, secondaryVariables);
        *Bn = DSMatrixSubMatrixIncludingRows(B, numberOfSecondaryVariables, secondaryVariables);
        *yn = DSVariablePoolAlloc();
        *yc = DSVariablePoolAlloc();
        DSMatrixFree(Ad);
        DSMatrixFree(Ai);
        DSMatrixFree(B);
        for (i = 0; i < numberOfSecondaryVariables; i++) {
                name = DSVariableName(DSVariablePoolVariableAtIndex(DSSSystemXd(ssystem), secondaryVariables[i]));
                DSVariablePoolAddVariableWithName(*yn, name);
        }
        for (i = 0; i < DSVariablePoolNumberOfVariables(DSSSystemXd(ssystem)); i++) {
                name = DSVariableName(DSVariablePoolVariableAtIndex(DSSSystemXd(ssystem), i));
                if (DSVariablePoolHasVariableWithName(*yn, name) == false) {
                        DSVariablePoolAddVariableWithName(*yc, name);
                }
        }
bail:
        return;
}

static void dsCyclicalCaseSolutionOfPartitionedMatrices(const DSCase * aCase,
                                                        const DSUInteger numberOfSecondaryVariables,
                                                        const DSUInteger * secondaryVariables,
                                                        DSMatrix ** LI,
                                                        DSMatrix **Lc,
                                                        DSMatrix **MBn,
                                                        DSVariablePool ** yn,
                                                        DSVariablePool ** yc)
{
        DSMatrix *ADn = NULL, * AIn = NULL, * ADc = NULL, * Bn = NULL, * Mn = NULL;
        if (LI == NULL || Lc == NULL || MBn == NULL) {
                DSError(M_DS_NULL ": Matrix pointers to hold partitioned matrices cannot be null", A_DS_ERROR);
                goto bail;
        }
        *LI = NULL;
        *Lc = NULL;
        *MBn = NULL;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (secondaryVariables == NULL) {
                goto bail;
        }
        dsCyclicalCasePartitionSolutionMatrices(aCase, numberOfSecondaryVariables, secondaryVariables, &ADn, &ADc, &AIn, &Bn, yn, yc);
        if (ADn == NULL || ADc == NULL || AIn == NULL || Bn == NULL) {
                goto bail;
        }
        Mn = DSMatrixInverse(ADn);
        if (Mn == NULL) {
                goto bail;
        }
        *LI = DSMatrixByMultiplyingMatrix(Mn, AIn);
        *Lc = DSMatrixByMultiplyingMatrix(Mn, ADc);
        *MBn = DSMatrixByMultiplyingMatrix(Mn, Bn);
        DSMatrixMultiplyByScalar(*LI, 1.);
        DSMatrixMultiplyByScalar(*Lc, 1.);
bail:
        if (ADn != NULL)
                DSMatrixFree(ADn);
        if (ADc != NULL)
                DSMatrixFree(ADc);
        if (AIn != NULL)
                DSMatrixFree(AIn);
        if (Bn != NULL)
                DSMatrixFree(Bn);
        if (Mn != NULL)
                DSMatrixFree(Mn);
        return;
}

static char * dsCyclicalCaseEquationForFlux(const DSCase * aCase,
                                            const DSDesignSpace * original,
                                            const DSMatrixArray * coefficientArray,
                                            const DSUInteger variableIndex,
                                            const DSUInteger fluxIndex,
                                            const bool positiveFlux,
                                            const DSUInteger cycleNumber,
                                            const DSUInteger primaryVariable,
                                            const DSUInteger numberSecondaryVariables,
                                            const DSUInteger * secondaryCycleVariables,
                                            const DSMatrix * LI,
                                            const DSMatrix * Lc,
                                            const DSMatrix * MBn,
                                            const DSVariablePool * yc)
{
        char * fluxEquationString = NULL;
        DSExpression * fluxEquation = NULL;
        const DSMatrix * A;
        DSMatrix *Kd, *LKi, *LKd, *Kc, *Ks, *Ki, *temp;
        const DSGMASystem * gma = NULL;
        DSUInteger i, j, index;
        double denominator = 0, numerator = 0;
        DSExpression * (*termFunction)(const DSGMASystem *, const DSUInteger, DSUInteger);
        // Checks...
        gma = DSDesignSpaceGMASystem(original);
        denominator = DSMatrixArrayDoubleWithIndices(coefficientArray, cycleNumber, 0, 0);
        if (variableIndex == primaryVariable) {
                numerator = denominator;
        } else {
                for (i = 0; i < numberSecondaryVariables; i++) {
                        if (secondaryCycleVariables[i] == variableIndex) {
                                numerator = DSMatrixArrayDoubleWithIndices(coefficientArray, cycleNumber, i+1, 0);
                        }
                }
        }
        if (numerator == 0) {
                goto bail;
        }
        if (positiveFlux == true) {
                A = DSGMASystemAlpha(gma);
                Kd = DSMatrixArrayMatrix(DSGMASystemGd(gma), variableIndex);
                Kd = DSMatrixSubMatrixIncludingRowList(Kd, 1, fluxIndex);
                Ki = DSMatrixArrayMatrix(DSGMASystemGi(gma), variableIndex);
                Ki = DSMatrixSubMatrixIncludingRowList(Ki, 1, fluxIndex);
                termFunction = DSGMASystemPositiveTermForEquations;
        } else {
                A = DSGMASystemBeta(gma);
                Kd = DSMatrixArrayMatrix(DSGMASystemGd(gma), variableIndex);
                Kd = DSMatrixSubMatrixIncludingRowList(Kd, 1, fluxIndex);
                Ki = DSMatrixArrayMatrix(DSGMASystemGi(gma), variableIndex);
                Ki = DSMatrixSubMatrixIncludingRowList(Ki, 1, fluxIndex);
                termFunction = DSGMASystemNegativeTermForEquations;
        }
        if (numberSecondaryVariables > 0) {
                Ks = DSMatrixSubMatrixIncludingColumns(Kd, numberSecondaryVariables, secondaryCycleVariables);
                LKi = DSMatrixByMultiplyingMatrix(Ks, LI);
                Kc = DSMatrixCalloc(DSMatrixRows(Kd), DSMatrixColumns(Kd));
                temp = dsCyclicalCaseExpandLcMatrix(DSCaseXd(aCase), Lc, yc);
                for (i = 0; i < DSMatrixRows(Kc); i++) {
                        for (j = 0; j < DSMatrixColumns(Lc); j++) {
                                index = DSVariablePoolIndexOfVariableWithName(DSCaseXd(aCase), DSVariableName(DSVariablePoolVariableAtIndex(yc, j)));
                                DSMatrixSetDoubleValue(Kc, i, index, DSMatrixDoubleValue(Kd, i, index));
                        }
                }
                LKd = DSMatrixByMultiplyingMatrix(Ks, temp);
                DSMatrixFree(temp);
                temp = DSMatrixByMultiplyingMatrix(Ks, MBn);
                DSMatrixAddByMatrix(LKd, Kc);
                DSMatrixAddByMatrix(Ki, LKi);
                DSMatrixSetDoubleValue(temp, 0, 0, DSMatrixDoubleValue(temp, 0, 0)+log10(DSMatrixDoubleValue(A, variableIndex, fluxIndex)));
                fluxEquation = DSExpressionFromPowerlawInMatrixForm(0, LKd, DSCaseXd(aCase), Ki, DSCaseXi(aCase), temp);
                fluxEquationString = DSSecureCalloc(sizeof(char), 1000);
                asprintf(&fluxEquationString, "%i*%s*%lf", (positiveFlux ? 1 : -1), DSExpressionAsString(fluxEquation), numerator/denominator);
                DSExpressionFree(fluxEquation);
        }// else {
                fluxEquation = termFunction(gma, variableIndex, fluxIndex);
                fluxEquationString = DSSecureCalloc(sizeof(char), 1000);
                asprintf(&fluxEquationString, "%s*%lf", DSExpressionAsString(fluxEquation), numerator/denominator);
                DSExpressionFree(fluxEquation);
//        }
bail:
        return fluxEquationString;
}

static DSExpression ** dsCyclicalCaseEquationsForCycle(const DSCase * aCase,
                                                       const DSDesignSpace * original,
                                                       const DSMatrixArray * coefficientArray,
                                                       const DSUInteger cycleNumber,
                                                       const DSUInteger primaryCycleVariable,
                                                       const DSUInteger numberSecondaryVariables,
                                                       const DSUInteger * secondaryCycleVariables,
                                                       const DSMatrix * LI,
                                                       const DSMatrix * Lc,
                                                       const DSMatrix * MBn,
                                                       const DSVariablePool * yn,
                                                       const DSVariablePool * yc)
{
        DSUInteger i, j, index, numberOfX, pcount = 0, ncount = 0;
        const DSSSystem * ssys;
        char * string = NULL, *name, *flux;
        double value;
        DSExpression ** cycleEquations = NULL;
        
//        DSMatrix * LI;
//        DSMatrix * Lc;
//        DSMatrix * MBn;
//        DSVariablePool * yn;
//        DSVariablePool * yc;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (original == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseNumberOfEquations(aCase) != DSDesignSpaceNumberOfEquations(original)) {
                DSError(M_DS_WRONG ": Number of equation in design space must match number of equations in case", A_DS_ERROR);
                goto bail;
        }
        if (secondaryCycleVariables == NULL && numberSecondaryVariables > 0) {
                DSError(M_DS_NULL ": Array of secondary cycle variables is null", A_DS_ERROR);
                goto bail;
        }
        if (numberSecondaryVariables > 0) {
//                dsCyclicalCaseSolutionOfPartitionedMatrices(aCase, numberSecondaryVariables, secondaryCycleVariables, &LI, &Lc, &MBn, &yn, &yc);
                if (LI == NULL || Lc == NULL || MBn == NULL) {
                        goto bail;
                }
        }
        ssys = DSCaseSSystem(aCase);
        string = DSSecureCalloc(sizeof(char *), 1000);
        // asprintf introduces memory error....
        asprintf(&string, "%s. = ", DSVariableName(DSVariablePoolVariableAtIndex(DSSSystemXd(ssys), primaryCycleVariable)));
        for (i = 0; i < numberSecondaryVariables+1; i++) {
                if (i == 0) {
                        index = primaryCycleVariable;
                } else {
                        index = secondaryCycleVariables[i-1];
                }
                for (j = 1; j <= DSDesignSpaceSignature(original)[2*index]; j++) {
                        if (j == DSCaseSignature(aCase)[2*index]) {
                                continue;
                        }
                        flux = dsCyclicalCaseEquationForFlux(aCase, original,
                                                             coefficientArray,
                                                             index,
                                                             j-1,
                                                             true,
                                                             cycleNumber,
                                                             primaryCycleVariable,
                                                             numberSecondaryVariables,
                                                             secondaryCycleVariables,
                                                             LI, Lc, MBn, yc);
                        if (flux != NULL) {
                                asprintf(&string, "%s + %s", string, flux);
                                DSSecureFree(flux);
                                pcount++;
                        }
                }
                for (j = 1; j <= DSDesignSpaceSignature(original)[2*index+1]; j++) {
                        if (j == DSCaseSignature(aCase)[2*index+1]) {
                                continue;
                        }
                        flux = dsCyclicalCaseEquationForFlux(aCase, original,
                                                             coefficientArray,
                                                             index,
                                                             j-1,
                                                             false,
                                                             cycleNumber,
                                                             primaryCycleVariable,
                                                             numberSecondaryVariables,
                                                             secondaryCycleVariables,
                                                             LI, Lc, MBn, yc);
                        if (flux != NULL) {
                                asprintf(&string, "%s + %s", string, flux);
                                DSSecureFree(flux);
                                ncount++;
                        }
                }
        }
        printf("%s\n", string);
        if (pcount == 0 || ncount == 0) {
                goto bail;
        }
        cycleEquations = DSSecureCalloc(sizeof(DSExpression *), numberSecondaryVariables+1);
        cycleEquations[0] = DSExpressionByParsingString(string);
        for (i = 0; i < numberSecondaryVariables; i++) {
                name = DSVariableName(DSVariablePoolVariableAtIndex(DSSSystemXd(ssys), secondaryCycleVariables[i]));
                index = DSVariablePoolIndexOfVariableWithName(yn, name);
                asprintf(&string, "%s = 10^%lf", name, DSMatrixDoubleValue(MBn, index, 0));
                numberOfX = DSVariablePoolNumberOfVariables(DSSSystemXi(ssys));
                for (j = 0; j < numberOfX; j++) {
                        name = DSVariableName(DSVariablePoolVariableAtIndex(DSSSystemXi(ssys), j));
                        value = -DSMatrixDoubleValue(LI, index, j);
                        if (value == 0.0)
                                continue;
                        if (value == 1.0)
                                asprintf(&string, "%s*%s", string, name);
                        else
                                asprintf(&string, "%s*%s^%lf", string, name, value);
                }
                numberOfX = DSVariablePoolNumberOfVariables(yc);
                for (j = 0; j < numberOfX; j++) {
                        name = DSVariableName(DSVariablePoolVariableAtIndex(yc, j));
                        value = -DSMatrixDoubleValue(Lc, index, j);
                        if (value == 0.0)
                                continue;
                        if (value == 1.0)
                                asprintf(&string, "%s*%s", string, name);
                        else
                                asprintf(&string, "%s*%s^%lf", string, name, value);
                }
                cycleEquations[i+1] = DSExpressionByParsingString(string);
        }

bail:
        if (string != NULL)
                DSSecureFree(string);
        return cycleEquations;
}

//static const char * dsExtensionDataFluxVariableInOriginalDesignSpace(const DSCase * aCase,
//                                                                     const DSDesignSpace * original,
//                                                                     const DSUInteger equationIndex,
//                                                                     const DSUInteger negativeTerm,
//                                                                     DSExpression ** fluxEquation)
//{
//        char * name = NULL;
//        DSUInteger i, j, fluxVariable;
//        DSCycleExtensionData * extensionData = NULL;
//        if (aCase == NULL) {
//                DSError(M_DS_CASE_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        if (original == NULL) {
//                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        if (equationIndex >= DSDesignSpaceNumberOfEquations(original)) {
//                DSError(M_DS_WRONG ": Equation index out of bounds", A_DS_ERROR);
//                goto bail;
//        }
//        if (negativeTerm >= DSDesignSpaceSignature(original)[2*equationIndex+1]) {
//                DSError(M_DS_WRONG ": Negative term index out of bounds", A_DS_ERROR);
//                goto bail;
//        }
//        extensionData = original->extensionData;
//        if (extensionData == NULL)
//                goto bail;
//        for (i = 0; i < extensionData->numberCycles; i++) {
//                if (extensionData->cycleVariables[i] != equationIndex)
//                        continue;
//                fluxVariable = extensionData->fluxIndex[i][negativeTerm];
//                name = DSVariableName(DSVariablePoolVariableAtIndex(DSGMASystemXd(DSDesignSpaceGMASystem(original)), fluxVariable));
//                *fluxEquation = extensionData->fluxEquations[i][negativeTerm];
//        }
//bail:
//        return name;
//}

//static void dsExtensionDataForCycle(const DSCase * aCase,
//                                    const DSDesignSpace * original,
//                                    DSCycleExtensionData * extensionData,
//                                    const DSUInteger cycleNumber,
//                                    const DSUInteger primaryCycleVariable,
//                                    const DSUInteger numberSecondaryVariables,
//                                    const DSUInteger * secondaryCycleVariables)
//{
//        DSUInteger i, j, k, index, numberOfFluxes;
//        DSExpression * fluxEquation;
//        const DSUInteger * signature;
//        const char * name;
//        if (extensionData == NULL) {
//                DSError(M_DS_NULL ": Extension data container is NULL", A_DS_ERROR);
//        }
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
//        if (secondaryCycleVariables == NULL && numberSecondaryVariables > 0) {
//                DSError(M_DS_NULL ": Array of secondary cycle variables is null", A_DS_ERROR);
//                goto bail;
//        }
//        numberOfFluxes = 0;
//        signature =DSDesignSpaceSignature(original);
//        for (i = 0; i < numberSecondaryVariables+1; i++) {
//                if (i == 0)
//                        index = primaryCycleVariable;
//                else
//                        index = secondaryCycleVariables[i-1];
//                numberOfFluxes += signature[2*index+1]-1;
//        }
//        extensionData->cycleVariables[cycleNumber] = primaryCycleVariable;
//        extensionData->fluxEquations[cycleNumber] = DSSecureCalloc(sizeof(DSExpression *), numberOfFluxes);
//        extensionData->fluxIndex[cycleNumber] = DSSecureCalloc(sizeof(DSExpression *), numberOfFluxes);
//        extensionData->numberOfFluxes[cycleNumber] = numberOfFluxes;
//        for (i = 0, k = 0; i < numberSecondaryVariables+1; i++) {
//                if (i == 0)
//                        index = primaryCycleVariable;
//                else
//                        index = secondaryCycleVariables[i-1];
//                for (j = 0; j < signature[2*index+1]; j++) {
//                        if (j+1 == DSCaseSignature(aCase)[2*index+1])
//                                continue;
//                        name = dsExtensionDataFluxVariableInOriginalDesignSpace(aCase, original, index, j, &fluxEquation);
//                        if (name != NULL) {
//                                extensionData->fluxIndex[cycleNumber][k] = DSVariablePoolIndexOfVariableWithName(DSCaseXd(aCase), name);
//                                fluxEquation = DSExpressionCopy(fluxEquation);
//                        } else {
//                                extensionData->fluxIndex[cycleNumber][k] = index;
//                                fluxEquation = DSGMASystemNegativeTermForEquations(DSDesignSpaceGMASystem(original),
//                                                                                   index,
//                                                                                   j);
//                                fluxEquation = DSExpressionMultiplyExpressionByConstant(fluxEquation, -1.);
//                        }
//                        extensionData->fluxEquations[cycleNumber][k] = fluxEquation;
//                        k++;
//                }
//        }
//bail:
//        return;
//}

static void dsCyclicalCaseEquilibriumEquationForVariableALT(DSUInteger index,
                                                         char ** systemEquations,
                                                         const DSCase * aCase,
                                                         const DSDesignSpace * original,
                                                         const DSMatrix * LI,
                                                         const DSMatrix * Lc,
                                                         const DSMatrix * Mb,
                                                         const DSVariablePool * yn,
                                                         const DSVariablePool * yc)
{
        DSExpression *fluxEquation;
        char * string, *name;
        DSUInteger i;
        DSMatrix * C;
        if (original == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (LI == NULL || Lc == NULL || Mb == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (yn == NULL || yc == NULL) {
                DSError(M_DS_VAR_NULL, A_DS_ERROR);
                goto bail;
        }
        C = DSMatrixCopy(Mb);
        for (i = 0; i < DSMatrixRows(C); i++) {
                DSMatrixSetDoubleValue(C, i, 0, pow(10, DSMatrixDoubleValue(C, i, 0)));
        }
        name = DSVariableName(DSVariablePoolVariableAtIndex(DSCaseXd(aCase), index));
        i = DSVariablePoolIndexOfVariableWithName(yn, name);
        Lc = DSMatrixCopy(Lc);
        LI = DSMatrixCopy(LI);
        DSMatrixMultiplyByScalar((DSMatrix *)Lc, -1.0f);
        DSMatrixMultiplyByScalar((DSMatrix *)LI, -1.0f);
        fluxEquation = DSExpressionFromPowerlawInMatrixForm(i, Lc, DSCaseXd(aCase), LI, DSCaseXi(aCase), C);
        string = DSExpressionAsString(fluxEquation);
        DSExpressionFree(fluxEquation);
        DSSecureFree(systemEquations[index]);
        asprintf(&(systemEquations[index]), "%s = %s", name, string);
        DSSecureFree(string);
        DSMatrixFree((DSMatrix *)Lc);
        DSMatrixFree((DSMatrix *)LI);
        DSMatrixFree((DSMatrix *)C);
bail:
        return;
}

static void dsCyclicalCaseEquilibriumEquationForVariable(DSUInteger index,
                                                         char ** systemEquations,
                                                         const DSCase * aCase,
                                                         const DSDesignSpace * original,
                                                         const DSMatrix * LI,
                                                         const DSMatrix * Lc,
                                                         const DSMatrix * Mb,
                                                         const DSVariablePool * yn,
                                                         const DSVariablePool * yc)
{
        DSExpression *fluxEquation;
        char * string, *name;
        DSUInteger i;
        DSMatrix * C;
        if (original == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (LI == NULL || Lc == NULL || Mb == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (yn == NULL || yc == NULL) {
                DSError(M_DS_VAR_NULL, A_DS_ERROR);
                goto bail;
        }
        C = DSMatrixCopy(Mb);
        for (i = 0; i < DSMatrixRows(C); i++) {
                DSMatrixSetDoubleValue(C, i, 0, pow(10, DSMatrixDoubleValue(C, i, 0)));
        }
        name = DSVariableName(DSVariablePoolVariableAtIndex(DSCaseXd(aCase), index));
        i = DSVariablePoolIndexOfVariableWithName(yn, name);
        Lc = DSMatrixCopy(Lc);
        LI = DSMatrixCopy(LI);
        DSMatrixMultiplyByScalar((DSMatrix *)Lc, -1.0f);
        DSMatrixMultiplyByScalar((DSMatrix *)LI, -1.0f);
        fluxEquation = DSExpressionFromPowerlawInMatrixForm(i, Lc, yc, LI, DSCaseXi(aCase), C);
        string = DSExpressionAsString(fluxEquation);
        DSExpressionFree(fluxEquation);
        DSSecureFree(systemEquations[index]);
        asprintf(&(systemEquations[index]), "%s = %s", name, string);
        DSSecureFree(string);
        DSMatrixFree((DSMatrix *)Lc);
        DSMatrixFree((DSMatrix *)LI);
        DSMatrixFree((DSMatrix *)C);
bail:
        return;
}

static void dsCyclicalCaseAugmentedEquationsForCycleALT(char ** systemEquations,
                                                     const DSCase * aCase,
                                                     const DSDesignSpace * original,
                                                     const DSMatrix * problematicMatrix,
                                                     const DSMatrixArray * coefficientArray,
                                                     const DSUInteger cycleNumber,
                                                     const DSUInteger primaryCycleVariable,
                                                     const DSUInteger numberSecondaryVariables,
                                                     const DSUInteger * secondaryVariables,
                                                     const DSMatrix * LI,
                                                     const DSMatrix * Lc,
                                                     const DSMatrix * Mb,
                                                     const DSVariablePool * yn,
                                                     const DSVariablePool * yc)
{
        DSExpression *fluxEquation;
        char * string, *name;
        DSUInteger i, j, k, l, index, eta;
        const DSUInteger *signature;
        DSMatrix * C, *Kd, *Ki;
        DSMatrix * Ks, *Kn, *LKi, *LKd, *temp;
        const DSGMASystem * gma;
        double denominator = 0, numerator = 0;
        
        if (original == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (problematicMatrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        l = 0;
        for (i = 0; i < DSDesignSpaceNumberOfEquations(original); i++) {
                if (DSMatrixDoubleValue(problematicMatrix, i, cycleNumber) == 0.0f) {
                        continue;
                }
                if (primaryCycleVariable == i) {
                        denominator = DSMatrixArrayDoubleWithIndices(coefficientArray, cycleNumber, l, 0);
                        break;
                }
                l++;
        }
        signature = DSDesignSpaceSignature(original);
        gma = DSDesignSpaceGMASystem(original);
        string = DSSecureCalloc(sizeof(char), 1000);
        DSSecureFree(systemEquations[primaryCycleVariable]);
        systemEquations[primaryCycleVariable] = string;
        name = DSVariableName(DSVariablePoolVariableAtIndex(DSGMASystemXd(gma), primaryCycleVariable));
        sprintf(string, "%s. = ", name);
        if (string != systemEquations[primaryCycleVariable]) {
                DSSecureFree(systemEquations[primaryCycleVariable]);
                systemEquations[primaryCycleVariable] = string;
        }
        l = 0;
        for (i = 0; i < 2*DSDesignSpaceNumberOfEquations(original); i++) {
                if (DSMatrixDoubleValue(problematicMatrix, i/2, cycleNumber) == 0.0f) {
                        continue;
                }
                index = i/2;
                if (i % 2 == 0) {
                        if (l >= DSMatrixRows(DSMatrixArrayMatrix(coefficientArray, cycleNumber))) // This Check was added to avoid an error that was appearing.
                                break;
                        C = DSMatrixSubMatrixIncludingRowList(DSGMASystemAlpha(gma), 1, i/2);
                        Kd = DSMatrixCopy(DSMatrixArrayMatrix(DSGMASystemGd(gma), i/2));
                        Ki = DSMatrixCopy(DSMatrixArrayMatrix(DSGMASystemGi(gma), i/2));
                        if (index != primaryCycleVariable) {
                                numerator = DSMatrixArrayDoubleWithIndices(coefficientArray, cycleNumber, l++, 0);
                                dsCyclicalCaseEquilibriumEquationForVariableALT(index, systemEquations, aCase, original, LI, Lc, Mb, yn, yc);
                        } else {
                                numerator = denominator;
                                l++;
                        }
                } else {
                        C = DSMatrixSubMatrixIncludingRowList(DSGMASystemBeta(gma), 1, i/2);
                        Kd = DSMatrixCopy(DSMatrixArrayMatrix(DSGMASystemHd(gma), i/2));
                        Ki = DSMatrixCopy(DSMatrixArrayMatrix(DSGMASystemHi(gma), i/2));
                        
                }
                temp = DSMatrixTranspose(C);
                DSMatrixFree(C);
                C = temp;
                if (numberSecondaryVariables > 0) {
                        Ks = DSMatrixSubMatrixIncludingColumns(Kd, numberSecondaryVariables, secondaryVariables);
                        Kn = DSMatrixCopy(Kd);
                        for (j = 0; j < DSMatrixRows(Kd); j++) {
                                for (k = 0; k < numberSecondaryVariables; k++) {
                                        index = secondaryVariables[k];
                                        DSMatrixSetDoubleValue(Kn, j, index, 0.0);
                                }
                        }
                        LKi = DSMatrixByMultiplyingMatrix(Ks, LI);
                        LKd = DSMatrixByMultiplyingMatrix(Ks, Lc);
                        DSMatrixMultiplyByScalar(LKd, -1.);
                        DSMatrixMultiplyByScalar(LKi, -1.);
                        DSMatrixAddByMatrix(LKd, Kn);
                        DSMatrixFree(Kd);
                        Kd = LKd;
                        DSMatrixAddByMatrix(Ki, LKi);
                        temp = DSMatrixByMultiplyingMatrix(Ks, Mb);
                        for (j = 0; j < DSMatrixRows(temp); j++) {
                                DSMatrixSetDoubleValue(temp, j, 0, numerator/denominator*DSMatrixDoubleValue(C, j, 0)*pow(10, DSMatrixDoubleValue(temp, j, 0)));
                        }
                        DSMatrixFree(C);
                        DSMatrixFree(LKi);
                        C = temp;
                        DSMatrixFree(Kn);
                        DSMatrixFree(Ks);
                }
                for (j = 0; j < signature[i]; j++) {
                        if (j + 1 == DSCaseSignature(aCase)[i])
                                continue;
                        fluxEquation = DSExpressionFromPowerlawInMatrixForm(j, Kd, DSGMASystemXd(gma), Ki, DSGMASystemXi(gma), C);
                        name = DSExpressionAsString(fluxEquation);
                        if (i % 2 == 0) {
                                sprintf(string, "%s + %s", systemEquations[primaryCycleVariable], name);
                        } else {
                                sprintf(string, "%s - %s", systemEquations[primaryCycleVariable], name);
                        }
                        if (string != systemEquations[primaryCycleVariable]) {
                                DSSecureFree(systemEquations[primaryCycleVariable]);
                                systemEquations[primaryCycleVariable] = string;
                        }
                        DSExpressionFree(fluxEquation);
//                        if (DSCaseNumber(aCase) == 126 && DSDesignSpaceCyclical(original) == false) {
//                                printf("%i,%i: %s\n", i, j, name);
//                        }
                        DSSecureFree(name);
                }
                DSMatrixFree(Kd);
                DSMatrixFree(Ki);
                DSMatrixFree(C);
        }
        
bail:
        return;
}


static void dsCyclicalCaseAugmentedEquationsForCycle(char ** systemEquations,
                                                     const DSCase * aCase,
                                                     const DSDesignSpace * original,
                                                     const DSMatrix * problematicMatrix,
                                                     const DSMatrixArray * coefficientArray,
                                                     const DSUInteger cycleNumber,
                                                     const DSUInteger primaryCycleVariable,
                                                     const DSUInteger numberSecondaryVariables,
                                                     const DSUInteger * secondaryVariables,
                                                     const DSMatrix * LI,
                                                     const DSMatrix * Lc,
                                                     const DSMatrix * Mb,
                                                     const DSVariablePool * yn,
                                                     const DSVariablePool * yc)
{
        DSExpression *fluxEquation;
        char * string, *name;
        DSUInteger i, j, k, l, index;
        const DSUInteger *signature;
        DSMatrix * C, *Kd, *Ki;
        DSMatrix * Ks, *Kn, *LKi, *LKd, *temp;
        const DSGMASystem * gma;
        double denominator = 0, numerator = 0;

        if (original == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (problematicMatrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        l = 0;
        for (i = 0; i < DSDesignSpaceNumberOfEquations(original); i++) {
                if (DSMatrixDoubleValue(problematicMatrix, i, cycleNumber) == 0.0f) {
                        continue;
                }
                if (primaryCycleVariable == i) {
                        denominator = DSMatrixArrayDoubleWithIndices(coefficientArray, cycleNumber, l, 0);
                        break;
                }
                l++;
        }
        signature = DSDesignSpaceSignature(original);
        gma = DSDesignSpaceGMASystem(original);
        string = DSSecureCalloc(sizeof(char), 1000);
        DSSecureFree(systemEquations[primaryCycleVariable]);
        systemEquations[primaryCycleVariable] = string;
        name = DSVariableName(DSVariablePoolVariableAtIndex(DSGMASystemXd(gma), primaryCycleVariable));
        asprintf(&string, "%s. = ", name);
        if (string != systemEquations[primaryCycleVariable]) {
                DSSecureFree(systemEquations[primaryCycleVariable]);
                systemEquations[primaryCycleVariable] = string;
        }
        l = 0;
        for (i = 0; i < 2*DSDesignSpaceNumberOfEquations(original); i++) {
                if (DSMatrixDoubleValue(problematicMatrix, i/2, cycleNumber) == 0.0f) {
                        continue;
                }
                index = i/2;
                if (i % 2 == 0) {
                        C = DSMatrixSubMatrixIncludingRowList(DSGMASystemAlpha(gma), 1, i/2);
                        Kd = DSMatrixCopy(DSMatrixArrayMatrix(DSGMASystemGd(gma), i/2));
                        Ki = DSMatrixCopy(DSMatrixArrayMatrix(DSGMASystemGi(gma), i/2));
                        if (index != primaryCycleVariable) {
                                numerator = DSMatrixArrayDoubleWithIndices(coefficientArray, cycleNumber, l++, 0);
                                dsCyclicalCaseEquilibriumEquationForVariable(index, systemEquations, aCase, original, LI, Lc, Mb, yn, yc);
                        } else {
                                numerator = denominator;
                                l++;
                        }
                } else {
                        C = DSMatrixSubMatrixIncludingRowList(DSGMASystemBeta(gma), 1, i/2);
                        Kd = DSMatrixCopy(DSMatrixArrayMatrix(DSGMASystemHd(gma), i/2));
                        Ki = DSMatrixCopy(DSMatrixArrayMatrix(DSGMASystemHi(gma), i/2));
                        
                }
                temp = DSMatrixTranspose(C);
                DSMatrixFree(C);
                C = temp;
                if (numberSecondaryVariables > 0) {
                        Ks = DSMatrixSubMatrixIncludingColumns(Kd, numberSecondaryVariables, secondaryVariables);
                        Kn = DSMatrixCalloc(DSMatrixRows(Kd), DSMatrixColumns(Kd));
                        for (j = 0; j < DSMatrixRows(Kd); j++) {
                                for (k = 0; k < DSMatrixColumns(Lc); k++) {
                                        index = DSVariablePoolIndexOfVariableWithName(DSGMASystemXd(gma),
                                                                                      DSVariableName(DSVariablePoolVariableAtIndex(yc, k)));
                                        DSMatrixSetDoubleValue(Kn, j, index, DSMatrixDoubleValue(Kd, j, index));
                                }
                        }
                        LKi = DSMatrixByMultiplyingMatrix(Ks, LI);
                        temp = dsCyclicalCaseExpandLcMatrix(DSGMASystemXd(gma), Lc, yc);
                        LKd = DSMatrixByMultiplyingMatrix(Ks, temp);
                        DSMatrixMultiplyByScalar(LKd, -1.);
                        DSMatrixMultiplyByScalar(LKi, -1.);
                        DSMatrixFree(temp);
                        DSMatrixAddByMatrix(LKd, Kn);
                        DSMatrixFree(Kd);
                        Kd = LKd;
                        DSMatrixAddByMatrix(Ki, LKi);
                        temp = DSMatrixByMultiplyingMatrix(Ks, Mb);
                        for (j = 0; j < DSMatrixRows(temp); j++) {
                                DSMatrixSetDoubleValue(temp, j, 0, numerator/denominator*DSMatrixDoubleValue(C, j, 0)*pow(10, DSMatrixDoubleValue(temp, j, 0)));
                        }
                        DSMatrixFree(C);
                        DSMatrixFree(LKi);
                        C = temp;
                        DSMatrixFree(Kn);
                        DSMatrixFree(Ks);
                }
                for (j = 0; j < signature[i]; j++) {
                        if (j + 1 == DSCaseSignature(aCase)[i])
                                continue;
                        fluxEquation = DSExpressionFromPowerlawInMatrixForm(j, Kd, DSGMASystemXd(gma), Ki, DSGMASystemXi(gma), C);
                        name = DSExpressionAsString(fluxEquation);
                        if (i % 2 == 0) {
                                asprintf(&string, "%s + %s", systemEquations[primaryCycleVariable], name);
                        } else {
                                asprintf(&string, "%s - %s", systemEquations[primaryCycleVariable], name);
                        }
                        if (string != systemEquations[primaryCycleVariable]) {
                                DSSecureFree(systemEquations[primaryCycleVariable]);
                                systemEquations[primaryCycleVariable] = string;
                        }
                        DSExpressionFree(fluxEquation);
//                        if (DSCaseNumber(aCase) == 126 && DSDesignSpaceCyclical(original) == false) {
//                                printf("%i,%i: %s\n", i, j, name);
//                        }
                        DSSecureFree(name);
                }
                DSMatrixFree(Kd);
                DSMatrixFree(Ki);
                DSMatrixFree(C);
        }
        
bail:
        return;
}


static char ** dsCyclicalCaseOriginalCaseEquationsWithEquilibriumConstraints(const DSCase * aCase,
                                                                             const DSUInteger numberSecondaryVariables,
                                                                             const DSUInteger * secondaryVariables,
                                                                             const DSMatrix * LI,
                                                                             const DSMatrix * Lc,
                                                                             const DSMatrix * Mb,
                                                                             const DSVariablePool * yn,
                                                                             const DSVariablePool * yc)
{
        DSExpression ** equations, *fluxEquation;
        char ** systemEquations = NULL, * string, *name;
        DSUInteger i, j, k, index;
        DSMatrix * C, *Kd, *Ki;
        DSMatrix * Ks, *Kn, *LKi, *LKd, *temp;
        const DSSSystem * ssystem;
        if (numberSecondaryVariables == 0) {
                systemEquations = DSSecureCalloc(sizeof(char *), DSCaseNumberOfEquations(aCase));
                equations = DSCaseEquations(aCase);
                for (i = 0; i < DSCaseNumberOfEquations(aCase); i++) {
                        systemEquations[i] = DSExpressionAsString(equations[i]);
                        DSExpressionFree(equations[i]);
                }
                DSSecureFree(equations);
                goto bail;
        }
        if (LI == NULL || Lc == NULL || Mb == NULL) {
                goto bail;
        }
        if (yn == NULL || yc == NULL) {
                goto bail;
        }
        systemEquations = DSSecureCalloc(sizeof(char *), DSCaseNumberOfEquations(aCase));
        ssystem = DSCaseSSys(aCase);
        equations = DSCaseEquations(aCase);
        for (i = 0; i < 2*DSCaseNumberOfEquations(aCase); i++) {
                name = DSVariableName(DSVariablePoolVariableAtIndex(DSSSystemXd(ssystem), i/2));
                if (DSVariablePoolHasVariableWithName(DSSSystemXd_t(ssystem), name) == false) {
                        if (i % 2 == 0) {
                                string = DSSecureCalloc(sizeof(char), 1000);
                                systemEquations[i/2] = DSExpressionAsString(equations[i/2]);
                        }
                        continue;
                }
                if (i % 2 == 0) {
                        string = DSSecureCalloc(sizeof(char), 1000);
                        systemEquations[i/2] = string;
                        asprintf(&string, "%s. = ", name);
                        if (string != systemEquations[i/2]) {
                                DSSecureFree(systemEquations[i/2]);
                                systemEquations[i/2] = string;
                        }
                        C = DSMatrixSubMatrixIncludingRowList(DSSSystemAlpha(ssystem), 1, i/2);
                        Kd = DSMatrixSubMatrixIncludingRowList(DSSSystemGd(ssystem), 1, i/2);
                        Ki = DSMatrixSubMatrixIncludingRowList(DSSSystemGi(ssystem), 1, i/2);
                } else {
                        C = DSMatrixSubMatrixIncludingRowList(DSSSystemBeta(ssystem), 1, i/2);
                        Kd = DSMatrixSubMatrixIncludingRowList(DSSSystemHd(ssystem), 1, i/2);
                        Ki = DSMatrixSubMatrixIncludingRowList(DSSSystemHi(ssystem), 1, i/2);
                }
                if (numberSecondaryVariables > 0) {
                        Ks = DSMatrixSubMatrixIncludingColumns(Kd, numberSecondaryVariables, secondaryVariables);
                        Kn = DSMatrixCalloc(DSMatrixRows(Kd), DSMatrixColumns(Kd));
                        for (j = 0; j < DSMatrixRows(Kd); j++) {
                                for (k = 0; k < DSMatrixColumns(Lc); k++) {
                                        index = DSVariablePoolIndexOfVariableWithName(DSSSystemXd(ssystem),
                                                                                      DSVariableName(DSVariablePoolVariableAtIndex(yc, k)));
                                        DSMatrixSetDoubleValue(Kn, j, index, DSMatrixDoubleValue(Kd, j, index));
                                }
                        }
                        temp = dsCyclicalCaseExpandLcMatrix(DSSSystemXd(ssystem), Lc, yc);
                        LKi = DSMatrixByMultiplyingMatrix(Ks, LI);
                        LKd = DSMatrixByMultiplyingMatrix(Ks, temp);
                        DSMatrixMultiplyByScalar(LKd, -1.);
                        DSMatrixMultiplyByScalar(LKi, -1.);
                        DSMatrixFree(temp);
                        DSMatrixAddByMatrix(LKd, Kn);
                        DSMatrixAddByMatrix(Ki, LKi);
                        temp = DSMatrixByMultiplyingMatrix(Ks, Mb);
                        for (j = 0; j < DSMatrixRows(temp); j++) {
                                DSMatrixSetDoubleValue(C, j, 0, DSMatrixDoubleValue(C, j, 0)*pow(10, DSMatrixDoubleValue(temp, j, 0)));
                        }
                        DSMatrixFree(Kd);
                        Kd = LKd;
                        DSMatrixFree(temp);
                        DSMatrixFree(LKi);
                        DSMatrixFree(Kn);
                        DSMatrixFree(Ks);
                }
                fluxEquation = DSExpressionFromPowerlawInMatrixForm(0, Kd, DSSSystemXd(ssystem), Ki, DSSSystemXi(ssystem), C);
                name = DSExpressionAsString(fluxEquation);
                if (i % 2 == 0) {
                        asprintf(&string, "%s + %s", systemEquations[i/2], name);
                } else {
                        asprintf(&string, "%s - %s", systemEquations[i/2], name);
                }
                if (string != systemEquations[i/2]) {
                        DSSecureFree(systemEquations[i/2]);
                        systemEquations[i/2] = string;
                }
//                DSExpressionPrint(fluxEquation);
                DSExpressionFree(fluxEquation);
                DSMatrixFree(Kd);
                DSMatrixFree(Ki);
                DSMatrixFree(C);
        }
        for (i = 0; i < DSCaseNumberOfEquations(aCase); i++) {
                DSExpressionFree(equations[i]);
        }
        DSSecureFree(equations);
        
bail:
        return systemEquations;
}

static char ** dsCyclicalCaseOriginalEquationsWithEquilibriumConstraintsALT(const DSCase * aCase,
                                                                         const DSDesignSpace * original,
                                                                         const DSUInteger numberSecondaryVariables,
                                                                         const DSUInteger * secondaryVariables,
                                                                         const double * coefficientMultipliers,
                                                                         const DSMatrix * LI,
                                                                         const DSMatrix * Lc,
                                                                         const DSMatrix * Mb,
                                                                         const DSVariablePool * yn,
                                                                         const DSVariablePool * yc)
{
        DSExpression ** equations, **caseEquations, *fluxEquation;
        char ** systemEquations = NULL, * string, *name;
        DSUInteger i, j, k, index;
        const DSUInteger *signature;
        DSMatrix * C, *Kd, *Ki;
        DSMatrix * Ks, *Kn, *LKi, *LKd, *temp;
        const DSGMASystem * gma;
        if (original == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        gma = DSDesignSpaceGMASystem(original);
        if (numberSecondaryVariables == 0) {
                systemEquations = DSSecureCalloc(sizeof(char *), DSDesignSpaceNumberOfEquations(original));
                equations = DSDesignSpaceEquations(original);
                caseEquations = DSCaseEquations(aCase);
                for (i = 0; i < DSDesignSpaceNumberOfEquations(original); i++) {
                        name = DSVariableName(DSVariablePoolVariableAtIndex(DSGMASystemXd(gma), i));
                        if (DSVariablePoolHasVariableWithName(DSGMASystemXd_t(gma), name) == true) {
                                systemEquations[i] = DSExpressionAsString(equations[i]);
                        } else {
                                systemEquations[i] = DSExpressionAsString(caseEquations[i]);
                        }
                        DSExpressionFree(equations[i]);
                        DSExpressionFree(caseEquations[i]);
                }
                DSSecureFree(equations);
                DSSecureFree(caseEquations);
                goto bail;
        }
        if (LI == NULL || Lc == NULL || Mb == NULL) {
                goto bail;
        }
        if (yn == NULL || yc == NULL) {
                goto bail;
        }
        systemEquations = DSSecureCalloc(sizeof(char *), DSDesignSpaceNumberOfEquations(original));
        signature = DSDesignSpaceSignature(original);
        equations = DSDesignSpaceEquations(original);
        caseEquations = DSCaseEquations(aCase);
        for (i = 0; i < 2*DSDesignSpaceNumberOfEquations(original); i++) {
                name = DSVariableName(DSVariablePoolVariableAtIndex(DSGMASystemXd(gma), i/2));
                if (DSVariablePoolHasVariableWithName(DSGMASystemXd_t(gma), name) == false) {
                        if (i % 2 == 0) {
                                //                                string = DSSecureCalloc(sizeof(char), 1000);
                                systemEquations[i/2] = DSExpressionAsString(caseEquations[i/2]);//DSExpressionAsString(caseEquations[i/2]);
                        }
                        continue;
                }
                if (i % 2 == 0) {
                        string = DSSecureCalloc(sizeof(char), 1000);
                        systemEquations[i/2] = string;
                        sprintf(string, "%s. = ", name);
                        if (string != systemEquations[i/2]) {
                                DSSecureFree(systemEquations[i/2]);
                                systemEquations[i/2] = string;
                        }
                        C = DSMatrixSubMatrixIncludingRowList(DSGMASystemAlpha(gma), 1, i/2);
                        Kd = DSMatrixCopy(DSMatrixArrayMatrix(DSGMASystemGd(gma), i/2));
                        Ki = DSMatrixCopy(DSMatrixArrayMatrix(DSGMASystemGi(gma), i/2));
                } else {
                        C = DSMatrixSubMatrixIncludingRowList(DSGMASystemBeta(gma), 1, i/2);
                        Kd = DSMatrixCopy(DSMatrixArrayMatrix(DSGMASystemHd(gma), i/2));
                        Ki = DSMatrixCopy(DSMatrixArrayMatrix(DSGMASystemHi(gma), i/2));
                }
                temp = DSMatrixTranspose(C);
                DSMatrixFree(C);
                C = temp;
                if (numberSecondaryVariables > 0) {
                        Ks = DSMatrixSubMatrixIncludingColumns(Kd, numberSecondaryVariables, secondaryVariables);
                        Kn = DSMatrixCopy(Kd);
                        for (j = 0; j < DSMatrixRows(Kd); j++) {
                                for (k = 0; k < numberSecondaryVariables; k++) {
                                        index = secondaryVariables[k];
                                        DSMatrixSetDoubleValue(Kn, j, index, 0.0);
                                }
                        }
                        LKi = DSMatrixByMultiplyingMatrix(Ks, LI);
                        LKd = DSMatrixByMultiplyingMatrix(Ks, Lc);
                        DSMatrixMultiplyByScalar(LKd, -1.);
                        DSMatrixMultiplyByScalar(LKi, -1.);
                        DSMatrixAddByMatrix(LKd, Kn);
                        DSMatrixAddByMatrix(Ki, LKi);
                        temp = DSMatrixByMultiplyingMatrix(Ks, Mb);
                        for (j = 0; j < DSMatrixRows(temp); j++) {
                                DSMatrixSetDoubleValue(C, j, 0, DSMatrixDoubleValue(C, j, 0)*pow(10, DSMatrixDoubleValue(temp, j, 0)));
                        }
                        DSMatrixFree(Kd);
                        Kd = LKd;
                        DSMatrixFree(temp);
                        DSMatrixFree(LKi);
                        DSMatrixFree(Kn);
                        DSMatrixFree(Ks);
                }
                for (j = 0; j < numberSecondaryVariables; j++) {
                        if (i/2 == secondaryVariables[j]) {
                                DSMatrixMultiplyByScalar(C, coefficientMultipliers[j]);
                                break;
                        }
                }
                for (j = 0; j < signature[i]; j++) {
                        fluxEquation = DSExpressionFromPowerlawInMatrixForm(j, Kd, DSGMASystemXd(gma), Ki, DSGMASystemXi(gma), C);
                        name = DSExpressionAsString(fluxEquation);
                        if (i % 2 == 0) {
                                sprintf(string, "%s + %s", systemEquations[i/2], name);
                        } else {
                                sprintf(string, "%s - %s", systemEquations[i/2], name);
                        }
                        if (string != systemEquations[i/2]) {
                                DSSecureFree(systemEquations[i/2]);
                                systemEquations[i/2] = string;
                        }
                        DSExpressionFree(fluxEquation);
                        DSSecureFree(name);
                }
                DSMatrixFree(Kd);
                DSMatrixFree(Ki);
                DSMatrixFree(C);
        }
        for (i = 0; i < DSDesignSpaceNumberOfEquations(original); i++) {
//                if (DSCaseNumber(aCase) == 126 && DSDesignSpaceCyclical(original) == false) {
//                        printf("%s\n", systemEquations[i]);
//                }
                DSExpressionFree(equations[i]);
                DSExpressionFree(caseEquations[i]);
        }
        DSSecureFree(equations);
        DSSecureFree(caseEquations);
bail:
        return systemEquations;
}


static char ** dsCyclicalCaseOriginalEquationsWithEquilibriumConstraints(const DSCase * aCase,
                                                                         const DSDesignSpace * original,
                                                                         const DSUInteger numberSecondaryVariables,
                                                                         const DSUInteger * secondaryVariables,
                                                                         const double * coefficientMultipliers,
                                                                         const DSMatrix * LI,
                                                                         const DSMatrix * Lc,
                                                                         const DSMatrix * Mb,
                                                                         const DSVariablePool * yn,
                                                                         const DSVariablePool * yc)
{
        DSExpression ** equations, **caseEquations, *fluxEquation;
        char ** systemEquations = NULL, * string, *name;
        DSUInteger i, j, k, index;
        const DSUInteger *signature;
        DSMatrix * C, *Kd, *Ki;
        DSMatrix * Ks, *Kn, *LKi, *LKd, *temp;
        const DSGMASystem * gma;
        if (original == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        gma = DSDesignSpaceGMASystem(original);
        if (numberSecondaryVariables == 0) {
                systemEquations = DSSecureCalloc(sizeof(char *), DSDesignSpaceNumberOfEquations(original));
                equations = DSDesignSpaceEquations(original);
                caseEquations = DSCaseEquations(aCase);
                for (i = 0; i < DSDesignSpaceNumberOfEquations(original); i++) {
                        name = DSVariableName(DSVariablePoolVariableAtIndex(DSGMASystemXd(gma), i));
                        if (DSVariablePoolHasVariableWithName(DSGMASystemXd_t(gma), name) == true) {
                                systemEquations[i] = DSExpressionAsString(equations[i]);
                        } else {
                                systemEquations[i] = DSExpressionAsString(caseEquations[i]);
                        }
                        DSExpressionFree(equations[i]);
                        DSExpressionFree(caseEquations[i]);
                }
                DSSecureFree(equations);
                DSSecureFree(caseEquations);
                goto bail;
        }
        if (LI == NULL || Lc == NULL || Mb == NULL) {
                goto bail;
        }
        if (yn == NULL || yc == NULL) {
                goto bail;
        }
        systemEquations = DSSecureCalloc(sizeof(char *), DSDesignSpaceNumberOfEquations(original));
        signature = DSDesignSpaceSignature(original);
        equations = DSDesignSpaceEquations(original);
        caseEquations = DSCaseEquations(aCase);
        for (i = 0; i < 2*DSDesignSpaceNumberOfEquations(original); i++) {
                name = DSVariableName(DSVariablePoolVariableAtIndex(DSGMASystemXd(gma), i/2));
                if (DSVariablePoolHasVariableWithName(DSGMASystemXd_t(gma), name) == false) {
                        if (i % 2 == 0) {
//                                string = DSSecureCalloc(sizeof(char), 1000);
                                systemEquations[i/2] = DSExpressionAsString(caseEquations[i/2]);//DSExpressionAsString(caseEquations[i/2]);
                        }
                        continue;
                }
                if (i % 2 == 0) {
                        string = DSSecureCalloc(sizeof(char), 1000);
                        systemEquations[i/2] = string;
                        asprintf(&string, "%s. = ", name);
                        if (string != systemEquations[i/2]) {
                                DSSecureFree(systemEquations[i/2]);
                                systemEquations[i/2] = string;
                        }
                        C = DSMatrixSubMatrixIncludingRowList(DSGMASystemAlpha(gma), 1, i/2);
                        Kd = DSMatrixCopy(DSMatrixArrayMatrix(DSGMASystemGd(gma), i/2));
                        Ki = DSMatrixCopy(DSMatrixArrayMatrix(DSGMASystemGi(gma), i/2));
                } else {
                        C = DSMatrixSubMatrixIncludingRowList(DSGMASystemBeta(gma), 1, i/2);
                        Kd = DSMatrixCopy(DSMatrixArrayMatrix(DSGMASystemHd(gma), i/2));
                        Ki = DSMatrixCopy(DSMatrixArrayMatrix(DSGMASystemHi(gma), i/2));
                }
                temp = DSMatrixTranspose(C);
                DSMatrixFree(C);
                C = temp;
                if (numberSecondaryVariables > 0) {
                        Ks = DSMatrixSubMatrixIncludingColumns(Kd, numberSecondaryVariables, secondaryVariables);
                        Kn = DSMatrixCalloc(DSMatrixRows(Kd), DSMatrixColumns(Kd));
                        for (j = 0; j < DSMatrixRows(Kd); j++) {
                                for (k = 0; k < DSMatrixColumns(Lc); k++) {
                                        index = DSVariablePoolIndexOfVariableWithName(DSGMASystemXd(gma),
                                                                                      DSVariableName(DSVariablePoolVariableAtIndex(yc, k)));
                                        DSMatrixSetDoubleValue(Kn, j, index, DSMatrixDoubleValue(Kd, j, index));
                                }
                        }
                        temp = dsCyclicalCaseExpandLcMatrix(DSGMASystemXd(gma), Lc, yc);
                        LKi = DSMatrixByMultiplyingMatrix(Ks, LI);
                        LKd = DSMatrixByMultiplyingMatrix(Ks, temp);
                        DSMatrixMultiplyByScalar(LKd, -1.);
                        DSMatrixMultiplyByScalar(LKi, -1.);
                        DSMatrixFree(temp);
                        DSMatrixAddByMatrix(LKd, Kn);
                        DSMatrixAddByMatrix(Ki, LKi);
                        temp = DSMatrixByMultiplyingMatrix(Ks, Mb);
                        for (j = 0; j < DSMatrixRows(temp); j++) {
                                DSMatrixSetDoubleValue(C, j, 0, DSMatrixDoubleValue(C, j, 0)*pow(10, DSMatrixDoubleValue(temp, j, 0)));
                        }
                        DSMatrixFree(Kd);
                        Kd = LKd;
                        DSMatrixFree(temp);
                        DSMatrixFree(LKi);
                        DSMatrixFree(Kn);
                        DSMatrixFree(Ks);
                }
                for (j = 0; j < numberSecondaryVariables; j++) {
                        if (i/2 == secondaryVariables[j]) {
                                DSMatrixMultiplyByScalar(C, coefficientMultipliers[j]);
                                break;
                        }
                }
                for (j = 0; j < signature[i]; j++) {
                        fluxEquation = DSExpressionFromPowerlawInMatrixForm(j, Kd, DSGMASystemXd(gma), Ki, DSGMASystemXi(gma), C);
                        name = DSExpressionAsString(fluxEquation);
                        if (i % 2 == 0) {
                                asprintf(&string, "%s + %s", systemEquations[i/2], name);
                        } else {
                                asprintf(&string, "%s - %s", systemEquations[i/2], name);
                        }
                        if (string != systemEquations[i/2]) {
                                DSSecureFree(systemEquations[i/2]);
                                systemEquations[i/2] = string;
                        }
                        DSExpressionFree(fluxEquation);
                        DSSecureFree(name);
                }
                DSMatrixFree(Kd);
                DSMatrixFree(Ki);
                DSMatrixFree(C);
        }
        for (i = 0; i < DSDesignSpaceNumberOfEquations(original); i++) {
//                if (DSCaseNumber(aCase) == 126 && DSDesignSpaceCyclical(original) == false) {
//                        printf("%s\n", systemEquations[i]);
//                }
                DSExpressionFree(equations[i]);
                DSExpressionFree(caseEquations[i]);
        }
        DSSecureFree(equations);
        DSSecureFree(caseEquations);
bail:
        return systemEquations;
}


static char ** dsCyclicalCaseEquationsSplitVariables(const DSCase * aCase,
                                                     const DSDesignSpace * original,
                                                     DSMatrix * problematicEquations,
                                                     const DSMatrixArray * coefficientArray,
                                                     DSCycleExtensionData * extensionData)
{
        DSMatrix * Mb = NULL, *LI = NULL, *Lc = NULL;
        DSMatrix * TLI = NULL, * TLc = NULL, * TMb = NULL;
        DSVariablePool * yn = NULL, *yc = NULL;
        char *name, ** systemEquations = NULL;
        const DSVariablePool *Xd;
        bool error=false;
        DSUInteger i, j, k, index, numberOfCycles, numberSecondaryVariables, *primaryVariables = NULL, *secondaryVariables = NULL;
        DSUInteger numberAllSecondaryVariables, * allSecondaryVariables = NULL;
        double  * coefficientMultipliers = NULL;
        //        DSCycleExtensionData * newExtensionData;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (original == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseNumberOfEquations(aCase) != DSDesignSpaceNumberOfEquations(original)) {
                DSError(M_DS_WRONG ": Number of equation in design space must match number of equations in case", A_DS_ERROR);
                goto bail;
        }
        if (problematicEquations == NULL)
                goto bail;
        if (coefficientArray == NULL)
                goto bail;
        numberOfCycles = dsCyclicalCasePrimaryCycleVariableIndices(aCase, problematicEquations, &primaryVariables);
        numberAllSecondaryVariables = dsCyclicalCaseAllSecondaryCycleVariables(problematicEquations,
                                                                               coefficientArray, numberOfCycles,
                                                                               primaryVariables,
                                                                               &allSecondaryVariables, &coefficientMultipliers);
        if (primaryVariables == NULL) {
                goto bail;
        }
        
        Mb = DSMatrixCalloc(numberAllSecondaryVariables, 1);
        LI = DSMatrixCalloc(numberAllSecondaryVariables, DSVariablePoolNumberOfVariables(DSCaseXi(aCase)));
        Lc = DSMatrixCalloc(numberAllSecondaryVariables, DSVariablePoolNumberOfVariables(DSCaseXd(aCase)));
        for (i = 0; i < numberOfCycles; i++) {
                numberSecondaryVariables = dsCyclicalCaseSecondaryCycleVariableIndicesForCycle(problematicEquations, i, primaryVariables, &secondaryVariables);
                dsCyclicalCaseSolutionOfPartitionedMatrices(aCase, numberSecondaryVariables, secondaryVariables, &TLI, &TLc, &TMb, &yn, &yc);
                if (TLI == NULL) {
                        DSSecureFree(secondaryVariables);
                        goto bail;
                }
                for (j = 0; j < DSMatrixRows(TLI); j++){
                        for (k = 0; k < numberAllSecondaryVariables; k++) {
                                if (secondaryVariables[j] == allSecondaryVariables[k]) {
                                        index = k;
                                        break;
                                }
                        }
                        DSMatrixSetDoubleValue(Mb, index, 0, DSMatrixDoubleValue(TMb, j, 0));
                        for (k = 0; k < DSMatrixColumns(TLI); k++) {
                                DSMatrixSetDoubleValue(LI, index, k, DSMatrixDoubleValue(TLI, j, k));
                        }
                        for (k = 0; k < DSMatrixColumns(TLc); k++) {
                                name = DSVariableName(DSVariablePoolVariableAtIndex(yc, k));
                                DSMatrixSetDoubleValue(Lc, index, DSVariablePoolIndexOfVariableWithName(DSCaseXd(aCase), name),
                                                       DSMatrixDoubleValue(TLc, j, k));
                        }
                }
                DSMatrixFree(TLc);
                DSMatrixFree(TLI);
                DSMatrixFree(TMb);
                DSVariablePoolFree(yc);
                DSVariablePoolFree(yn);
                DSSecureFree(secondaryVariables);
        }
        yn = DSVariablePoolAlloc();
        yc = DSVariablePoolAlloc();
        for (i = 0; i < numberAllSecondaryVariables; i++) {
                DSVariablePoolAddVariableWithName(yn, DSVariableName(DSVariablePoolVariableAtIndex(DSCaseXd(aCase), allSecondaryVariables[i])));
        }
        for (i = 0; i < DSVariablePoolNumberOfVariables(DSCaseXd(aCase)); i++) {
                name = DSVariableName(DSVariablePoolVariableAtIndex(DSCaseXd(aCase), i));
                if (DSVariablePoolHasVariableWithName(yn, name) == false) {
                        DSVariablePoolAddVariableWithName(yc, name);
                }
        }
        
//        goto bail;
//        if (secondaryVariables == NULL && numberSecondaryVariables > 0) {
//                goto bail;
//        }
//        if (numberSecondaryVariables > 0) {
//                dsCyclicalCaseSolutionOfPartitionedMatrices(aCase, numberSecondaryVariables, secondaryVariables, &LI, &Lc, &Mb, &yn, &yc);
//                if (DSCaseNumber(aCase) == 49) {
//                        printf("49 LI:\n");
//                        DSMatrixPrint(LI);
//                }
//        }
        //        systemEquations = dsCyclicalCaseOriginalEquationsWithEquilibriumConstraints(aCase, original, numberSecondaryVariables, secondaryVariables, LI, Lc, Mb, yn, yc);
        //        systemEquations = dsCyclicalCaseOriginalCaseEquationsWithEquilibriumConstraints(aCase, numberSecondaryVariables, secondaryVariables, LI, Lc, Mb, yn, yc);
        //        extensionData = DSSecureCalloc(sizeof(extensionData), 1);
        systemEquations = dsCyclicalCaseOriginalEquationsWithEquilibriumConstraintsALT(aCase, original, numberAllSecondaryVariables, allSecondaryVariables, coefficientMultipliers, LI, Lc, Mb, yn, yc);
        Xd = DSGMASystemXd(DSDesignSpaceGMASystem(original));
        //        extensionData->numberCycles = numberOfCycles;
        //        extensionData->cycleVariables = DSSecureCalloc(sizeof(DSUInteger), numberOfCycles);
        //        extensionData->fluxEquations =  DSSecureCalloc(sizeof(DSExpression **), numberOfCycles);
        //        extensionData->fluxIndex =  DSSecureCalloc(sizeof(DSUInteger *), numberOfCycles);
        //        extensionData->numberOfFluxes = DSSecureCalloc(sizeof(DSUInteger), numberOfCycles);
        if (systemEquations == NULL) {
                goto bail;
        }
        for (i = 0; i < DSDesignSpaceNumberOfEquations(original); i++) {
                if (systemEquations[i] == NULL) {
                        error = true;
                }
        }
        if (error == true) {
                for (i = 0; i < DSDesignSpaceNumberOfEquations(original); i++) {
                        if (systemEquations[i])
                                DSSecureFree(systemEquations[i]);
                }
                DSSecureFree(systemEquations);
                goto bail;
        }
        for (i = 0; i < numberOfCycles; i++) {
                dsCyclicalCaseAugmentedEquationsForCycleALT(systemEquations, aCase, original, problematicEquations, coefficientArray, i, primaryVariables[i], numberAllSecondaryVariables, allSecondaryVariables, LI, Lc, Mb, yn, yc);
                //                printf("%s\n", systemEquations[primaryVariables[i]]);
                //                numberSecondaryVariables = dsCyclicalCaseSecondaryCycleVariableIndicesForCycle(problematicEquations, i, primaryVariables, &secondaryVariables);
                //                cycleEquations = dsCyclicalCaseEquationsForCycle(aCase,
                //                                                                 original,
                //                                                                 coefficientArray,
                //                                                                 i,
                //                                                                 primaryVariables[i],
                //                                                                 numberSecondaryVariables,
                //                                                                 secondaryVariables,
                //                                                                 LI,
                //                                                                 Lc,
                //                                                                 Mb,
                //                                                                 yn,
                //                                                                 yc);
                //                if (cycleEquations == NULL) {
                //                        DSSecureFree(secondaryVariables);
                //                        break;
                //                }
                //                dsExtensionDataForCycle(aCase, original, extensionData, i, primaryVariables[i], numberSecondaryVariables, secondaryVariables);
                //                for (j = 0; j < numberSecondaryVariables+1; j++) {
                //                        if (j == 0) {
                //                                index = primaryVariables[i];
                //                        } else {
                //                                index = secondaryVariables[j-1];
                //                        }
                //                        DSSecureFree(systemEquations[index]);
                //                        systemEquations[index] = DSExpressionAsString(cycleEquations[j]);
                //                        DSExpressionFree(cycleEquations[j]);
                //                }
                //                if (secondaryVariables != NULL) {
                //                        DSSecureFree(cycleEquations);
                //                        DSSecureFree(secondaryVariables);
                //                }
        }
        if (i != numberOfCycles) {
                for (i = 0; i < DSDesignSpaceNumberOfEquations(original); i++) {
                        DSSecureFree(systemEquations[i]);
                }
                DSSecureFree(systemEquations);
                systemEquations = NULL;
        }
bail:
        if (primaryVariables != NULL)
                DSSecureFree(primaryVariables);
        if (allSecondaryVariables != NULL)
                DSSecureFree(allSecondaryVariables);
        if (coefficientMultipliers != NULL)
                DSSecureFree(coefficientMultipliers);
        if (LI != NULL)
                DSMatrixFree(LI);
        if (Lc != NULL)
                DSMatrixFree(Lc);
        if (Mb != NULL)
                DSMatrixFree(Mb);
        if (yn != NULL)
                DSVariablePoolFree(yn);
        if (yc != NULL)
                DSVariablePoolFree(yc);
        return systemEquations;
}

static char ** dsCyclicalCaseEquations(const DSCase * aCase,
                                       const DSDesignSpace * original,
                                       DSMatrix * problematicEquations,
                                       const DSMatrixArray * coefficientArray,
                                       DSCycleExtensionData * extensionData)
{
        DSMatrix * Mb = NULL, *LI = NULL, *Lc = NULL;
        DSVariablePool * yn = NULL, *yc = NULL;
        char ** systemEquations = NULL;
        const DSVariablePool *Xd;
        bool error=false;
        DSUInteger i, numberOfCycles, numberSecondaryVariables, *primaryVariables = NULL, *secondaryVariables = NULL;
        double  * coefficientMultipliers = NULL;
//        DSCycleExtensionData * newExtensionData;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (original == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseNumberOfEquations(aCase) != DSDesignSpaceNumberOfEquations(original)) {
                DSError(M_DS_WRONG ": Number of equation in design space must match number of equations in case", A_DS_ERROR);
                goto bail;
        }
        if (problematicEquations == NULL)
                goto bail;
        if (coefficientArray == NULL)
                goto bail;
        numberOfCycles = dsCyclicalCasePrimaryCycleVariableIndices(aCase, problematicEquations, &primaryVariables);
        numberSecondaryVariables = dsCyclicalCaseAllSecondaryCycleVariables(problematicEquations, coefficientArray, numberOfCycles, primaryVariables, &secondaryVariables, &coefficientMultipliers);
        if (primaryVariables == NULL) {
                goto bail;
        }
        if (secondaryVariables == NULL && numberSecondaryVariables > 0) {
                goto bail;
        }
        if (numberSecondaryVariables > 0) {
                dsCyclicalCaseSolutionOfPartitionedMatrices(aCase, numberSecondaryVariables, secondaryVariables, &LI, &Lc, &Mb, &yn, &yc);
        }
//        extensionData = DSSecureCalloc(sizeof(extensionData), 1);
        systemEquations = dsCyclicalCaseOriginalEquationsWithEquilibriumConstraints(aCase, original, numberSecondaryVariables, secondaryVariables, coefficientMultipliers, LI, Lc, Mb, yn, yc);

        Xd = DSGMASystemXd(DSDesignSpaceGMASystem(original));
//        extensionData->numberCycles = numberOfCycles;
//        extensionData->cycleVariables = DSSecureCalloc(sizeof(DSUInteger), numberOfCycles);
//        extensionData->fluxEquations =  DSSecureCalloc(sizeof(DSExpression **), numberOfCycles);
//        extensionData->fluxIndex =  DSSecureCalloc(sizeof(DSUInteger *), numberOfCycles);
//        extensionData->numberOfFluxes = DSSecureCalloc(sizeof(DSUInteger), numberOfCycles);
        if (systemEquations == NULL) {
                goto bail;
        }
        for (i = 0; i < DSDesignSpaceNumberOfEquations(original); i++) {
                if (systemEquations[i] == NULL) {
                        error = true;
                }
        }
        if (error == true) {
                for (i = 0; i < DSDesignSpaceNumberOfEquations(original); i++) {
                        if (systemEquations[i])
                                DSSecureFree(systemEquations[i]);
                }
                DSSecureFree(systemEquations);
                goto bail;
        }
        for (i = 0; i < numberOfCycles; i++) {
                dsCyclicalCaseAugmentedEquationsForCycle(systemEquations, aCase,
                                                         original, problematicEquations,
                                                         coefficientArray, i, primaryVariables[i],
                                                         numberSecondaryVariables,
                                                         secondaryVariables, LI,
                                                         Lc, Mb, yn, yc);
//                numberSecondaryVariables = dsCyclicalCaseSecondaryCycleVariableIndicesForCycle(problematicEquations, i, primaryVariables, &secondaryVariables);
//                cycleEquations = dsCyclicalCaseEquationsForCycle(aCase,
//                                                                 original,
//                                                                 coefficientArray,
//                                                                 i,
//                                                                 primaryVariables[i],
//                                                                 numberSecondaryVariables,
//                                                                 secondaryVariables,
//                                                                 LI,
//                                                                 Lc,
//                                                                 Mb,
//                                                                 yn,
//                                                                 yc);
//                if (cycleEquations == NULL) {
//                        DSSecureFree(secondaryVariables);
//                        break;
//                }
//                dsExtensionDataForCycle(aCase, original, extensionData, i, primaryVariables[i], numberSecondaryVariables, secondaryVariables);
//                for (j = 0; j < numberSecondaryVariables+1; j++) {
//                        if (j == 0) {
//                                index = primaryVariables[i];
//                        } else {
//                                index = secondaryVariables[j-1];
//                        }
//                        DSSecureFree(systemEquations[index]);
//                        systemEquations[index] = DSExpressionAsString(cycleEquations[j]);
//                        DSExpressionFree(cycleEquations[j]);
//                }
//                if (secondaryVariables != NULL) {
//                        DSSecureFree(cycleEquations);
//                        DSSecureFree(secondaryVariables);
//                }
        }
        if (i != numberOfCycles) {
                for (i = 0; i < DSDesignSpaceNumberOfEquations(original); i++) {
                        DSSecureFree(systemEquations[i]);
                }
                DSSecureFree(systemEquations);
                systemEquations = NULL;
        }
bail:
        if (primaryVariables != NULL)
                DSSecureFree(primaryVariables);
        if (secondaryVariables != NULL)
                DSSecureFree(secondaryVariables);
        if (coefficientMultipliers != NULL)
                DSSecureFree(coefficientMultipliers);
        if (LI != NULL)
                DSMatrixFree(LI);
        if (Lc != NULL)
                DSMatrixFree(Lc);
        if (Mb != NULL)
                DSMatrixFree(Mb);
        if (yn != NULL)
                DSVariablePoolFree(yn);
        if (yc != NULL)
                DSVariablePoolFree(yc);
        return systemEquations;
}

DSCycleExtensionData * dsCycleExtensionDataInitForCyclicalCase(const DSCase * aCase,
                                                               const DSDesignSpace * original)
{
        DSCycleExtensionData * extensionData = NULL;
//        DSDictionary * cycleFluxes;
//        DSUInteger i;
//        char * name;
//        DSExpression * flux;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (original == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
//        extensionData = DSSecureCalloc(sizeof(DSCycleExtensionData), 1);
//        cycleFluxes = DSDictionaryAlloc();
//        for (i = 0; i < DSDesignSpaceNumberOfEquations(original); i++) {
//                name = DSVariableName(DSVariablePoolVariableAtIndex(DSCaseXd(aCase), i));
//                if (original->extensionData != NULL) {
//                        flux = DSDictionaryValueForName(original->extensionData->cycleFluxes, name);
//                        DSDictionaryAddValueWithName(cycleFluxes, name, DSExpressionCopy(flux));
//                } else {
//                        flux = DSGMASystemPositiveTermForEquations(DSDesignSpaceGMASystem(original), i, DSCaseSignature(aCase)[i*2]-1);
//                        DSDictionaryAddValueWithName(cycleFluxes, name, flux);
//                }
//        }
//        extensionData->cycleFluxes = cycleFluxes;
bail:
        return extensionData;
}


DSDesignSpace * dsCyclicalCaseCollapsedSystem(const DSCase * aCase,
                                              const DSDesignSpace * original,
                                              DSMatrix * problematicEquations,
                                              const DSMatrixArray * coefficientArray)
{
        DSDesignSpace * collapsed = NULL;
        char ** systemEquations = NULL;
        DSUInteger i;
        DSCycleExtensionData * extensionData;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (original == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseNumberOfEquations(aCase) != DSDesignSpaceNumberOfEquations(original)) {
                DSError(M_DS_WRONG ": Number of equation in design space must match number of equations in case", A_DS_ERROR);
                goto bail;
        }
        if (problematicEquations == NULL)
                goto bail;
        if (coefficientArray == NULL)
                goto bail;
        extensionData = dsCycleExtensionDataInitForCyclicalCase(aCase, original);
        systemEquations = dsCyclicalCaseEquationsSplitVariables(aCase, original, problematicEquations, coefficientArray, extensionData);
        if (systemEquations == NULL) {
                goto bail;
        }
        collapsed = DSDesignSpaceByParsingStringsWithXi(systemEquations,
                                                        DSGMASystemXd_a(DSDesignSpaceGMASystem(original)),
                                                        DSGMASystemXi(DSDesignSpaceGMASystem(original)),
                                                        DSDesignSpaceNumberOfEquations(original));

        DSDesignSpaceAddConditions(collapsed, DSCaseCd(aCase), DSCaseCi(aCase), DSCaseDelta(aCase));
        collapsed->casePrefix = strdup(DSCaseIdentifier(aCase));
        DSDesignSpaceSetSerial(collapsed, true);
        DSDesignSpaceSetCyclical(collapsed, true);
        DSDesignSpaceSetResolveCoDominance(collapsed, DSDesignSpaceResolveCoDominance(original));
        collapsed->extensionData = extensionData;
        if (DSDesignSpaceNumberOfCases(collapsed) != 0) {
                DSDesignSpaceCalculateCyclicalCases(collapsed);

        }
        for (i = 0; i < DSDesignSpaceNumberOfEquations(original); i++) {
                DSSecureFree(systemEquations[i]);
        }
        DSSecureFree(systemEquations);
bail:
        return collapsed;
}


#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Exposed function to generate the internal systems for cyclical cases -
#endif

extern DSDesignSpace * DSCyclicalCaseDesignSpacesForUnderdeterminedCase(const DSCase * aCase, const DSDesignSpace * original)
{
        DSMatrix * problematicEquations = NULL;
        DSMatrixArray * problematicTerms = NULL;
        DSMatrixArray * coefficientArray = NULL;
        DSDesignSpace * subcase = NULL;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (original == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseNumberOfEquations(aCase) != DSDesignSpaceNumberOfEquations(original)) {
                DSError(M_DS_WRONG ": Number of equation in design space must match number of equations in case", A_DS_ERROR);
                goto bail;
        }
        problematicEquations = dsSubcaseProblematicEquations(aCase);
        if (problematicEquations == NULL)
                goto bail;
        problematicTerms = dsSubcaseProblematicTerms(aCase, problematicEquations);
        if (problematicTerms == NULL)
                goto bail;
        coefficientArray = dsSubcaseCoefficientsOfInterest(aCase, problematicTerms);
        if (coefficientArray == NULL)
                goto bail;
        if (DSMatrixArrayNumberOfMatrices(problematicTerms) != DSMatrixArrayNumberOfMatrices(coefficientArray))
                goto bail;
        subcase = dsCyclicalCaseCollapsedSystem(aCase, original, problematicEquations, coefficientArray);
bail:
        if (problematicEquations != NULL)
                DSMatrixFree(problematicEquations);
        if (problematicTerms != NULL)
                DSMatrixArrayFree(problematicTerms);
        if (coefficientArray != NULL)
                DSMatrixArrayFree(coefficientArray);
        return subcase;
}
