//
//  DSCyclicalCaseConstruction.c
//  DesignSpaceToolboxV2
//
//  Created by Jason Lomnitz on 8/7/13.
//
//

#include <stdio.h>
#include <string.h>
#include "DSCyclicalCase.h"

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Subcase calculation functions
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
        nullspace = DSMatrixLeftNullspace(A);
        DSMatrixFree(A);
        if (nullspace == NULL)
                goto bail;
        
        isUnderdetermined = true;
        problematic = DSMatrixCalloc(DSMatrixRows(nullspace), DSMatrixColumns(nullspace));
        for (i = 0; i < DSMatrixColumns(nullspace); i++) {
                firstValue = NAN;
                for (j = 0; j < DSMatrixRows(nullspace); j++) {
                        current = DSMatrixDoubleValue(nullspace, j, i);
                        if (fabs(current) < 1E-14)
                                continue;
                        DSMatrixSetDoubleValue(problematic, j, i, 1.0);
                        if (isnan(firstValue) == true) {
                                firstValue = current;
                        } else if (fabs(current - firstValue) >= 1E-14) {
                                isUnderdetermined = false;
                                break;
                        }
                }
                if (j != DSMatrixRows(nullspace))
                        break;
        }
        if (isUnderdetermined == false) {
                DSMatrixFree(problematic);
                problematic = NULL;
        }
        DSMatrixFree(nullspace);
bail:
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
                g = DSMatrixSubMatrixIncludingRows(G, numDependent, dependent);
                h = DSMatrixSubMatrixIncludingRows(H, numDependent, dependent);
                termMatrix = DSMatrixAppendMatrices(g, h, false);
                DSMatrixFree(g);
                DSMatrixFree(h);
                nullspace = DSMatrixLeftNullspace(termMatrix);
                coefficients = DSMatrixCalloc(numDependent, DSMatrixColumns(nullspace));
                for (j = 0; j < DSMatrixRows(nullspace); j++) {
                        for (k = 0; k < DSMatrixColumns(nullspace); k++) {
                                value = DSMatrixDoubleValue(nullspace, j, k);
                                if (fabs(value) <= 1E-14) {
                                        DSMatrixSetDoubleValue(nullspace, j, k, 0.0);
                                        continue;
                                }
                                DSMatrixSetDoubleValue(nullspace, j, k, copysign(1.0, value));
                                if (j / numDependent == 0)
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

extern DSMatrixArray * dsSubcaseCoefficientsOfInterest(const DSCase * aCase, const DSMatrixArray * problematicTerms)
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
                        for (k = 0; k < DSMatrixColumns(problematic); k++) {
                                value = DSMatrixDoubleValue(coefficients, j, 0);
                                value += DSMatrixDoubleValue(problematic, j, k);
                                DSMatrixSetDoubleValue(coefficients, j, 0, value);
                        }
                }
                DSMatrixFree(problematic);
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
//                        DSSecureFree(equations[j]);
//                        eqLHS = DSExpressionEquationLHSExpression(caseEquations[j]);
//                        eqRHS = DSExpressionEquationRHSExpression(caseEquations[j]);
//                        temp = DSExpressionVariablesInExpression(eqLHS);
//                        DSVariablePoolCopyVariablesFromVariablePool(Xda, temp);
//                        temp_rhs = DSExpressionAsString(eqRHS);
//                        equations[j] = DSSecureCalloc(sizeof(char),strlen(temp_rhs) + 7);
//                        equations[j] = strcpy(equations[j], "0 = ");
//                        equations[j] = strcat(equations[j], temp_rhs);
//                        DSSecureFree(temp_rhs);
//                        DSExpressionFree(eqLHS);
//                        DSExpressionFree(eqRHS);
                        equationIndex[i] = j;
                        break;
                }
        }
//        for (i = 0; i < DSMatrixColumns(problematicEquations); i++) {
//                equationIndex[i] = DSMatrixRows(problematicEquations);
//                for (j = 0; j < DSMatrixRows(problematicEquations); j++) {
//                        if (DSMatrixDoubleValue(problematicEquations, j, i) == 0.0)
//                                continue;
//                        equationIndex[i] = j;
//                        break;
//                }
//        }
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
bail:
        if (equationIndex != NULL)
                DSSecureFree(equationIndex);
        return ds;
}

extern DSDesignSpace * DSCyclicalCaseInternalForUnderdeterminedCase(const DSCase * aCase, const DSDesignSpace * original)
{
        DSDesignSpace *subcases = NULL;
        DSGMASystem * temp = NULL;
        DSMatrix * problematicEquations = NULL;
        DSMatrixArray * problematicTerms = NULL;
        DSMatrixArray * coefficientArray = NULL;
        DSUInteger i, j, k, l;
        DSExpression **augmentedEquations;
        double value;
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
        temp = DSGMASystemCopy(original->gma);
        augmentedEquations = DSSecureCalloc(sizeof(DSExpression *), DSMatrixColumns(problematicEquations));        
        for (i = 0; i < DSMatrixColumns(problematicEquations); i++) {
                l = 0;
                for (j = 0; j < DSMatrixRows(problematicEquations); j++) {
                        if (DSMatrixDoubleValue(problematicEquations, j, i) == 0)
                                continue;
                        value = DSMatrixArrayDoubleWithIndices(coefficientArray, i, l, 0);
                        for (k = 0; k < DSMatrixColumns(DSGMASystemAlpha(temp)); k++) {
                                if (k+1 == aCase->signature[2*j]) {
                                        DSMatrixSetDoubleValue((DSMatrix *)DSGMASystemAlpha(temp), j, k, 0.0f);
                                } else {
                                        DSMatrixSetDoubleValue((DSMatrix *)DSGMASystemAlpha(temp), j, k,
                                                               DSMatrixDoubleValue(DSGMASystemAlpha(temp), j, k)*value);
                                }
                                if (k+1 == aCase->signature[2*j+1]) {
                                        DSMatrixSetDoubleValue((DSMatrix *)DSGMASystemBeta(temp), j, k, 0.0f);
                                } else if (k < DSMatrixColumns(DSGMASystemBeta(temp))) {
                                        DSMatrixSetDoubleValue((DSMatrix *)DSGMASystemBeta(temp), j, k,
                                                               DSMatrixDoubleValue(DSGMASystemBeta(temp), j, k)*value);
                                }
                        }
                        l++;
                        augmentedEquations[i] = DSExpressionAddExpressions(augmentedEquations[i], DSGMASystemPositiveTermsForEquations(temp, j));
                        augmentedEquations[i] = DSExpressionAddExpressions(augmentedEquations[i], DSGMASystemNegativeTermsForEquations(temp, j));
                }
        }
        
        subcases = dsSubcaseCreateUniqueSystemSubcase(aCase, temp, problematicEquations, (const DSExpression **)augmentedEquations);
        if (subcases != NULL) {
                DSDesignSpaceAddConditions(subcases, aCase->Cd, aCase->Ci, aCase->delta);
        }
        
        for (i = 0; i < DSMatrixColumns(problematicEquations); i++)
                DSExpressionFree(augmentedEquations[i]);
        DSSecureFree(augmentedEquations);
        DSGMASystemFree(temp);
bail:
        if (problematicEquations != NULL)
                DSMatrixFree(problematicEquations);
        if (problematicTerms != NULL)
                DSMatrixArrayFree(problematicTerms);
        if (coefficientArray != NULL)
                DSMatrixArrayFree(coefficientArray);
        return subcases;
}
