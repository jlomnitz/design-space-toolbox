//
//  DSCaseLinearProgramming.c
//  DesignSpaceToolboxV2
//
//  Created by Jason Lomnitz on 8/30/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <glpk.h>

#include "DSMemoryManager.h"
#include "DSCase.h"
#include "DSVariable.h"
#include "DSGMASystem.h"
#include "DSSSystem.h"
#include "DSDesignSpace.h"
#include "DSExpression.h"
#include "DSMatrix.h"
#include "DSMatrixArray.h"
#include "DSVertices.h"


#define DSCaseXi(x)                  ((x)->Xi)
#define DSCaseXd(x)                  ((x)->Xd)
#define DSCaseSSys(x)                ((x)->ssys)
#define DSCaseCd(x)                  ((x)->Cd)
#define DSCaseCi(x)                  ((x)->Ci)
#define DSCaseU(x)                   ((x)->U)
#define DSCaseDelta(x)               ((x)->delta)
#define DSCaseZeta(x)                ((x)->zeta)
#define DSCaseSig(x)                 ((x)->signature)
#define DSCaseNum(x)                 ((x)->caseNumber)

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Linear programming functions
#endif

typedef DSCase DSPseudoCase;

static glp_prob * dsCaseLinearProblemForMatrices(const DSMatrix *A, const DSMatrix *B)
{
        glp_prob *linearProblem = NULL;
        int * ia = NULL, *ja = NULL;
        double *ar = NULL;
        DSUInteger i, numberOfXi, numberOfBoundaries;
        
        glp_term_out(GLP_OFF);
        linearProblem = glp_create_prob();
        if (linearProblem == NULL) {
                DSError(M_DS_NULL ": Linear problem is NULL", A_DS_ERROR);
                goto bail;
        }
        numberOfXi = DSMatrixColumns(A);
        numberOfBoundaries = DSMatrixRows(A);
        
        ia = DSMatrixRowsForGLPK(A);
        ja = DSMatrixColumnsForGLPK(A);
        ar = DSMatrixDataForGLPK(A);
        
        glp_add_rows(linearProblem, numberOfBoundaries);
        glp_add_cols(linearProblem, numberOfXi);
        
        glp_set_obj_dir(linearProblem, GLP_MIN);
        glp_load_matrix(linearProblem, numberOfBoundaries*numberOfXi,
                        ia, ja, ar);
        for (i = 0; i < numberOfBoundaries; i++) {
                glp_set_row_bnds(linearProblem, i+1, GLP_UP, 0.0,
                                 DSMatrixDoubleValue(B, i, 0));
        }
        for (i = 0; i < numberOfXi; i++)
                glp_set_col_bnds(linearProblem, i+1, GLP_FR, 0.0, 0.0);
        
        if (ia != NULL)
                DSSecureFree(ia);
        if (ja != NULL)
                DSSecureFree(ja);
        if (ar != NULL)
                DSSecureFree(ar);
bail:
        return linearProblem;
}

static glp_prob * dsCaseLinearProblemForCaseValidity(const DSMatrix * U, const DSMatrix *zeta)
{
        glp_prob *linearProblem = NULL;
        DSMatrix *slacks = NULL, * coefficients;
        DSUInteger numberOfXi, numberOfBoundaries;
        
        if (zeta == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (U == NULL)
                numberOfXi = 0;
        else 
                numberOfXi = DSMatrixColumns(U);
        
        numberOfBoundaries = DSMatrixRows(zeta);
        if (numberOfXi > 0) {
                slacks = DSMatrixAlloc(numberOfBoundaries, 1);
                DSMatrixSetDoubleValueAll(slacks, 1.0);
                coefficients = DSMatrixAppendMatrices(U, slacks, true);
                DSMatrixMultiplyByScalar(coefficients, -1.0);
        } else {
                coefficients = DSMatrixAlloc(numberOfBoundaries, 1);
                DSMatrixSetDoubleValueAll(coefficients, -1.0);
                
        }
        
        linearProblem = dsCaseLinearProblemForMatrices(coefficients, zeta);
        glp_set_col_bnds(linearProblem, glp_get_num_cols(linearProblem), GLP_LO, -1.0, 0.0);
        glp_set_obj_coef(linearProblem, glp_get_num_cols(linearProblem), 1.0);
        
        
        DSMatrixFree(coefficients);
        if (slacks != NULL)
                DSMatrixFree(slacks);
bail:
        return linearProblem;
}


extern const bool DSCaseIsValid(const DSCase *aCase)
{
        bool isValid = false;
        glp_prob *linearProblem = NULL;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseHasSolution(aCase) == false) {
                goto bail;
        }
        linearProblem = dsCaseLinearProblemForCaseValidity(DSCaseU(aCase), DSCaseZeta(aCase));
        if (linearProblem != NULL) {
                glp_simplex(linearProblem, NULL);
                if (glp_get_obj_val(linearProblem) < 0 && glp_get_prim_stat(linearProblem) == GLP_FEAS)
                        isValid = true;
                
                glp_delete_prob(linearProblem);
        }
bail:
        return isValid;
}

extern const bool DSCaseIsValidAtPoint(const DSCase *aCase, const DSVariablePool * variablesToFix)
{
        bool isValid = false;
        DSUInteger i, numberToRemove, indexOfVariable;
        DSMatrix *result, *Xi;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseHasSolution(aCase) == false) {
                goto bail;
        }
        if (variablesToFix == NULL) {
                DSError(M_DS_VAR_NULL ": Variable pool with variables to fix is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(variablesToFix) != DSVariablePoolNumberOfVariables(DSCaseXi(aCase))) {
                DSError(M_DS_WRONG ": Number of variables the same as the number Xi", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(variablesToFix) == 0) {
                DSError(M_DS_WRONG ": Case has no independent variables", A_DS_WARN);
                isValid = DSCaseIsValid(aCase);
                goto bail;
        }
        numberToRemove = DSVariablePoolNumberOfVariables(variablesToFix);
        Xi = DSMatrixAlloc(DSVariablePoolNumberOfVariables(DSCaseXi(aCase)), 1);
        for (i = 0; i < numberToRemove; i++) {
                indexOfVariable = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), 
                                                                        DSVariableName(DSVariablePoolAllVariables(variablesToFix)[i]));
                if (indexOfVariable >= DSMatrixRows(Xi)) {
                        DSMatrixFree(Xi);
                        goto bail;
                }
                DSMatrixSetDoubleValue(Xi, indexOfVariable, 0, log10(DSVariableValue(DSVariablePoolAllVariables(variablesToFix)[i])));
        }
        result = DSMatrixByMultiplyingMatrix(DSCaseU(aCase), Xi);
        DSMatrixAddByMatrix(result, DSCaseZeta(aCase));
        
        
        for (i = 0; i < DSMatrixRows(result); i++)
                if (DSMatrixDoubleValue(result, i, 0) < 0)
                        break;
        if (i == DSMatrixRows(result))
                isValid = true;
        DSMatrixFree(result);
        DSMatrixFree(Xi);
bail:
        return isValid;
}

//extern const bool DSCaseIsValidAtSlice(const DSCase *aCase, const DSVariablePool * variablesToFix)
//{
//        bool isValid = false;
//        glp_prob *linearProblem = NULL;
//        DSUInteger i, numberToRemove, variableIndex;
//        const DSVariable * variable;
//        
//        if (aCase == NULL) {
//                DSError(M_DS_CASE_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        if (DSCaseHasSolution(aCase) == false) {
//                goto bail;
//        }
//        if (variablesToFix == NULL) {
//                DSError(M_DS_VAR_NULL ": Variable pool with variables to fix is NULL", A_DS_ERROR);
//                goto bail;
//        }
//        if (DSVariablePoolNumberOfVariables(variablesToFix) > DSVariablePoolNumberOfVariables(DSCaseXi(aCase))) {
//                DSError(M_DS_WRONG ": Number of variables to fix is greater than number Xi", A_DS_ERROR);
//                goto bail;
//        }
//        if (DSVariablePoolNumberOfVariables(variablesToFix) == 0) {
//                DSError(M_DS_WRONG ": Case has no independent variables", A_DS_WARN);
//                isValid = DSCaseIsValid(aCase);
//                goto bail;
//        }
//        if (DSVariablePoolNumberOfVariables(variablesToFix) == DSVariablePoolNumberOfVariables(DSCaseXi(aCase))) {
//                isValid = DSCaseIsValidAtPoint(aCase, variablesToFix);
//                goto bail;
//        }
//        numberToRemove = DSVariablePoolNumberOfVariables(variablesToFix);
//        linearProblem = dsCaseLinearProblemForCaseValidity(DSCaseU(aCase), DSCaseZeta(aCase));
//        if (linearProblem == NULL) {
//                DSError(M_DS_NULL ": Linear problem was not created", A_DS_ERROR);
//                goto bail;
//        }
//        for (i = 0; i < numberToRemove; i++) {
//                
//                variable = DSVariablePoolAllVariables(variablesToFix)[i];
//                variableIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), 
//                                                                      DSVariableName(variable));
//                glp_set_col_bnds(linearProblem, variableIndex+1, GLP_FX, log10(DSVariableValue(variable)), 0.0);
//                
//                if (variableIndex >= DSVariablePoolNumberOfVariables(DSCaseXi(aCase))) {
//                        glp_delete_prob(linearProblem);
//                        goto bail;
//                }
//        }
//        glp_simplex(linearProblem, NULL);
//        if (glp_get_obj_val(linearProblem) < 0 && glp_get_prim_stat(linearProblem) == GLP_FEAS)
//                isValid = true;
//        
//        glp_delete_prob(linearProblem);
//bail:
//        return isValid;
//}
//

static DSUInteger dsCaseNumberOfFreeVariablesForBounds(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds)
{
        DSUInteger i, variableIndex, freeVariables = 0;
        const DSVariable * lowVariable, *highVariable;
        double low, high;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (lowerBounds == NULL && upperBounds == NULL) {
                DSError(M_DS_VAR_NULL ": Variable pool with variables to fix is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(lowerBounds) != DSVariablePoolNumberOfVariables(upperBounds)) {
                DSError(M_DS_WRONG ": Number of variables to bound must match", A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < DSVariablePoolNumberOfVariables(lowerBounds); i++) {
                
                lowVariable = DSVariablePoolAllVariables(lowerBounds)[i];
                highVariable = DSVariablePoolVariableWithName(upperBounds, DSVariableName(lowVariable));
                
                if (lowVariable == NULL || highVariable == NULL) {
                        DSError(M_DS_WRONG ": Variables to bound are not consistent", A_DS_WARN);
                        continue;
                }
                
                variableIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), 
                                                                      DSVariableName(lowVariable));
                low = DSVariableValue(lowVariable);
                high = DSVariableValue(highVariable);
                
                if (low > high) {
                        DSError(M_DS_WRONG ": Variable bounds are not consistent", A_DS_WARN);
                        continue;
                }
                
                if (variableIndex >= DSVariablePoolNumberOfVariables(DSCaseXi(aCase)))
                        continue;
                if (low == high) 
                        continue;
                freeVariables++;
        }
bail:
        return freeVariables;
}

static DSUInteger dsCaseSetVariableBoundsLinearProblem(const DSCase *aCase, glp_prob *linearProblem,  const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds)
{
        DSUInteger i, variableIndex, freeVariables = 0;
        const DSVariable * lowVariable, *highVariable;
        double low, high;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (lowerBounds == NULL && upperBounds == NULL) {
                DSError(M_DS_VAR_NULL ": Variable pool with variables to fix is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(lowerBounds) != DSVariablePoolNumberOfVariables(upperBounds)) {
                DSError(M_DS_WRONG ": Number of variables to bound must match", A_DS_ERROR);
                goto bail;
        }
        if (linearProblem == NULL) {
                DSError(M_DS_NULL ": Linear problem is NULL", A_DS_WARN);
                goto bail;
        }
        for (i = 0; i < DSVariablePoolNumberOfVariables(lowerBounds); i++) {
                
                lowVariable = DSVariablePoolAllVariables(lowerBounds)[i];
                highVariable = DSVariablePoolVariableWithName(upperBounds, DSVariableName(lowVariable));
                
                if (lowVariable == NULL || highVariable == NULL) {
                        DSError(M_DS_WRONG ": Variables to bound are not consistent", A_DS_WARN);
                        continue;
                }
                
                variableIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), 
                                                                      DSVariableName(lowVariable));
                
                low = DSVariableValue(lowVariable);
                high = DSVariableValue(highVariable);
                
                if (low > high) {
                        DSError(M_DS_WRONG ": Variable bounds are not consistent", A_DS_WARN);
                        continue;
                }
                
                if (variableIndex >= DSVariablePoolNumberOfVariables(DSCaseXi(aCase)))
                        continue;
                if (low == -INFINITY && high == INFINITY)
                        glp_set_col_bnds(linearProblem, variableIndex+1, GLP_FR, 0.0, 0.0);
                else if (low == -INFINITY)
                        glp_set_col_bnds(linearProblem, variableIndex+1, GLP_UP, 0.0, log10(high));
                else if (high == INFINITY)
                        glp_set_col_bnds(linearProblem, variableIndex+1, GLP_LO, log10(low), 0.0);
                else if (low == high)
                        glp_set_col_bnds(linearProblem, variableIndex+1, GLP_FX, log10(low), 0.0);
                else
                        glp_set_col_bnds(linearProblem, variableIndex+1, GLP_DB, log10(low), log10(high));
                if (glp_get_col_type(linearProblem, variableIndex+1) != GLP_FX)
                        freeVariables++;
                
        }
bail:
        return freeVariables;
}

extern const bool DSCaseIsValidAtSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds)
{
        bool isValid = false;
        glp_prob *linearProblem = NULL;
        
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseHasSolution(aCase) == false) {
                goto bail;
        }
        if (lowerBounds == NULL && upperBounds == NULL) {
                DSError(M_DS_VAR_NULL ": Variable pool with variables to fix is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(lowerBounds) != DSVariablePoolNumberOfVariables(upperBounds)) {
                DSError(M_DS_WRONG ": Number of variables to bound must match", A_DS_ERROR);
                goto bail;
        }
        linearProblem = dsCaseLinearProblemForCaseValidity(DSCaseU(aCase), DSCaseZeta(aCase));
        if (linearProblem == NULL) {
                DSError(M_DS_NULL ": Linear problem was not created", A_DS_WARN);
                goto bail;
        }
        if (dsCaseSetVariableBoundsLinearProblem(aCase, linearProblem, lowerBounds, upperBounds) <= DSVariablePoolNumberOfVariables(DSCaseXi(aCase))) {
                glp_simplex(linearProblem, NULL);
                if (glp_get_obj_val(linearProblem) < 0 && glp_get_prim_stat(linearProblem) == GLP_FEAS)
                        isValid = true;
        }
        
        glp_delete_prob(linearProblem);
bail:
        return isValid;
}

static DSUInteger nchoosek(DSUInteger n, DSUInteger k)
{
        double denominator, numerator;
        DSUInteger i, lowerBound, upperBound, value = 0;
        if (n == 0 || k == 0)
                goto bail;
        lowerBound = (k > n-k) ? k : (n-k);
        upperBound = (k < n-k) ? k : (n-k);
        denominator = 1;
        for (i = 2; i <= upperBound; i++)
                denominator *= (double)i;
        numerator = lowerBound+1;
        for (i = lowerBound+2; i <= n; i++)
                numerator *= (double)i;
        value = (DSUInteger)(numerator/denominator);
bail:
        return value;
}

static DSVertices * dsCaseCalculate2DVertices(const DSCase * aCase, glp_prob * linearProblem, const DSMatrix * A, const DSMatrix *Zeta, const DSUInteger xIndex, const DSUInteger yIndex)
{
        DSVertices *vertices = NULL;
        DSUInteger numberOfCombinations, i, j, firstIndex, secondIndex;
        DSUInteger numberOfBoundaries, activeIndex;
        double bnd, xVal, yVal, vals[2];
        
        numberOfBoundaries = DSMatrixRows(A);
        numberOfCombinations = nchoosek(numberOfBoundaries, 2);
        
        vertices = DSVerticesAlloc(2);
        for (i = 0; i < numberOfCombinations; i++) {
                
                for (j = 0; j < numberOfBoundaries; j++) {
                        bnd = glp_get_row_ub(linearProblem, j+1);
                        glp_set_row_bnds(linearProblem, j+1, GLP_UP, 0.0, bnd);
                }
                
                secondIndex = 0;
                for (j = 0, firstIndex = 1; j <= i; j += numberOfBoundaries-firstIndex, firstIndex++)
                        secondIndex += numberOfBoundaries-firstIndex;
                secondIndex -= i;
                secondIndex = numberOfBoundaries-secondIndex;
                firstIndex -= 2;
                
                bnd = glp_get_row_ub(linearProblem, firstIndex+1);
                glp_set_row_bnds(linearProblem, firstIndex+1, GLP_FX, bnd, bnd);
                bnd = glp_get_row_ub(linearProblem, secondIndex+1);
                glp_set_row_bnds(linearProblem, secondIndex+1, GLP_FX, bnd, bnd);
                
                for (j = 0; j < DSVariablePoolNumberOfVariables(DSCaseXi(aCase)); j++)
                        glp_set_obj_coef(linearProblem, j+1, 0.0);
                
                glp_set_obj_coef(linearProblem, xIndex+1, 1.0);
                
                if (fabs(DSMatrixDoubleValue(A, firstIndex, yIndex)) >= 1E-14)
                        activeIndex = firstIndex;
                else if (fabs(DSMatrixDoubleValue(A, secondIndex, yIndex)) >= 1E-14)
                        activeIndex = secondIndex;
                else
                        continue;
                
                glp_simplex(linearProblem, NULL);
                if (glp_get_prim_stat(linearProblem) != GLP_FEAS)
                        continue;
                xVal = glp_get_obj_val(linearProblem);
                yVal = -DSMatrixDoubleValue(Zeta, activeIndex, 0);
                for (j = 0; j < DSVariablePoolNumberOfVariables(DSCaseXi(aCase)); j++) {
                        if (j == yIndex)
                                continue;
                        if (j == xIndex) {
                                yVal += DSMatrixDoubleValue(A, activeIndex, j) * xVal;
                        } else {
                                yVal += DSMatrixDoubleValue(A, activeIndex, j) * glp_get_col_ub(linearProblem, j+1);
                        }
                }
                yVal /= -DSMatrixDoubleValue(A, activeIndex, yIndex);
                vals[0] = xVal;
                vals[1] = yVal;
                DSVerticesAddVertex(vertices, vals);
        }
        DSVerticesOrder2DVertices(vertices);
        return vertices;
}

//static DSVertices * DSCaseVertices2DForValidRegion(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable)
//{
//        
//}

extern DSVertices * DSCaseVerticesFor2DSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable)
{
        DSVertices *vertices = NULL;
        DSUInteger yIndex, xIndex;
        DSMatrix *A, *Zeta, *temp;
        glp_prob * linearProblem;
        
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        
        if (dsCaseNumberOfFreeVariablesForBounds(aCase, lowerBounds, upperBounds) != 2) {
                DSError(M_DS_WRONG ": Must have obly two free variables", A_DS_ERROR);
                goto bail;
        }
        
        yIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), yVariable);
        xIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), xVariable);
        
        if (xIndex >= DSVariablePoolNumberOfVariables(DSCaseXi(aCase))) {
                DSError(M_DS_WRONG ": Case does not have X variable", A_DS_ERROR);
                goto bail;
        }
        if (yIndex >= DSVariablePoolNumberOfVariables(DSCaseXi(aCase))) {
                DSError(M_DS_WRONG ": Case does not have Y variable", A_DS_ERROR);
                goto bail;
        }
        
        temp = DSMatrixCalloc(4, DSVariablePoolNumberOfVariables(DSCaseXi(aCase)));
        DSMatrixSetDoubleValue(temp, 0, xIndex, 1.0);
        DSMatrixSetDoubleValue(temp, 1, xIndex, -1.0);
        DSMatrixSetDoubleValue(temp, 2, yIndex, 1.0);
        DSMatrixSetDoubleValue(temp, 3, yIndex, -1.0);
        A = DSMatrixAppendMatrices(DSCaseU(aCase), temp, false);
        DSMatrixFree(temp);
        temp = DSMatrixCalloc(4, 1);
        DSMatrixSetDoubleValue(temp, 0, 0, -log10(DSVariableValue(DSVariablePoolVariableWithName(lowerBounds, xVariable))));
        DSMatrixSetDoubleValue(temp, 1, 0, log10(DSVariableValue(DSVariablePoolVariableWithName(upperBounds, xVariable))));
        DSMatrixSetDoubleValue(temp, 2, 0, -log10(DSVariableValue(DSVariablePoolVariableWithName(lowerBounds, yVariable))));
        DSMatrixSetDoubleValue(temp, 3, 0, log10(DSVariableValue(DSVariablePoolVariableWithName(upperBounds, yVariable))));
        Zeta = DSMatrixAppendMatrices(DSCaseZeta(aCase), temp, false);
        DSMatrixFree(temp);
        DSMatrixMultiplyByScalar(A, -1.0);
        
        linearProblem = dsCaseLinearProblemForMatrices(A, Zeta);
        
        if (linearProblem == NULL) {
                DSError(M_DS_NULL ": Linear problem is NULL", A_DS_ERROR);
                DSMatrixFree(A);
                DSMatrixFree(Zeta);
                goto bail;
        }
        
        if (dsCaseSetVariableBoundsLinearProblem(aCase, linearProblem, lowerBounds, upperBounds) != 2) {
                DSError(M_DS_WRONG ": Need two free variables", A_DS_ERROR);
                DSMatrixFree(A);
                DSMatrixFree(Zeta);
                glp_delete_prob(linearProblem);
                goto bail;
        }
        
        if (glp_get_col_type(linearProblem, DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), xVariable)+1) == GLP_FX) {
                DSError(M_DS_WRONG ": X Variable is fixed", A_DS_ERROR);
                DSMatrixFree(A);
                DSMatrixFree(Zeta);
                glp_delete_prob(linearProblem);
                goto bail;
        }
        if (glp_get_col_type(linearProblem, DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), yVariable)+1) == GLP_FX) {
                DSError(M_DS_WRONG ": Y Variable is fixed", A_DS_ERROR);
                DSMatrixFree(A);
                DSMatrixFree(Zeta);
                glp_delete_prob(linearProblem);
                goto bail;
        }
        
        vertices = dsCaseCalculate2DVertices(aCase, linearProblem, A, Zeta, xIndex, yIndex);
        DSMatrixFree(A);
        DSMatrixFree(Zeta);
        glp_delete_prob(linearProblem);
bail:
        return vertices;
}

extern DSVertices * DSCaseVerticesForSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const DSUInteger numberOfVariables, const char **variables)
{
        DSVertices *vertices = NULL;
        DSUInteger i, numberOfFreeVariables = 0;
        
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (lowerBounds == NULL && upperBounds == NULL) {
                DSError(M_DS_VAR_NULL, A_DS_ERROR);
                goto bail;
        }
        if (variables == NULL) {
                DSError(M_DS_NULL ": String with variable names is NULL", A_DS_ERROR);
                goto bail;
        }
        numberOfFreeVariables = dsCaseNumberOfFreeVariablesForBounds(aCase, lowerBounds, upperBounds);
        if (numberOfFreeVariables != numberOfVariables) {
                DSError(M_DS_WRONG ": Number of free variables does not match number of variables", A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < numberOfFreeVariables; i++) {
                if (variables[i] == NULL) {
                        DSError(M_DS_NULL ": String with variable is NULL", A_DS_ERROR);
                        goto bail;
                }
                if (strlen(variables[i]) == 0) {
                        DSError(M_DS_WRONG ": String with variable is empty", A_DS_ERROR);
                        goto bail;
                }
                if (DSVariablePoolHasVariableWithName(DSCaseXi(aCase), variables[i]) == false) {
                        DSError(M_DS_WRONG ": Case does not have variable for slice", A_DS_ERROR);
                        goto bail;
                }
        }
       
        if (numberOfFreeVariables == 2) {
                vertices = DSCaseVerticesFor2DSlice(aCase, lowerBounds, upperBounds, variables[0], variables[1]);
        } else {
                DSError(M_DS_NOT_IMPL ": N-dimensional vertex enumeration not implemented", A_DS_WARN);
        }
bail:
        return vertices;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Intersection of cases
#endif


extern const bool DSCaseIntersectionListIsValid(const DSUInteger numberOfCases, const DSCase *firstCase, ...)
{
        bool isValid = false;
        DSUInteger i;
        const DSCase ** cases = NULL;
        va_list ap;
        if (firstCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (numberOfCases == 0) {
                DSError(M_DS_WRONG ": Number of cases must be at least one", A_DS_WARN);
                goto bail;
        }
        va_start(ap, firstCase);
        cases = DSSecureMalloc(sizeof(DSCase *)*numberOfCases);
        cases[0] = firstCase;
        for (i = 1; i < numberOfCases; i++) {
                cases[i] = va_arg(ap, DSCase *);
                if (cases[i] == NULL) {
                        DSError(M_DS_CASE_NULL, A_DS_ERROR);
                        break;
                }
        }
        va_end(ap);
        if (i == numberOfCases)
                isValid = DSCaseIntersectionIsValid(numberOfCases, cases);
        //        submatrix = DSMatrixSubMatrixIncludingColumns(matrix, numberOfCases, cases);
        DSSecureFree(cases);

bail:
        return isValid;
}

extern const bool DSCaseIntersectionIsValid(const DSUInteger numberOfCases, const DSCase **cases)
{
        bool isValid = false;
        DSMatrix *U = NULL, *Zeta = NULL, *temp;
        DSUInteger i;
        DSPseudoCase * caseIntersection = NULL;
        if (numberOfCases == 0) {
                DSError(M_DS_WRONG ": Number of cases must be at least one", A_DS_ERROR);
                goto bail;
        }
        if (cases == NULL) {
                DSError(M_DS_NULL ": Array of cases is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSCaseHasSolution(cases[0]) == false)
                goto bail;
        U = DSMatrixCopy(DSCaseU(cases[0]));
        Zeta = DSMatrixCopy(DSCaseZeta(cases[0]));
        for (i = 0; i < numberOfCases; i++) {
                if (DSCaseHasSolution(cases[i]) == false)
                        goto bail;
                temp = DSMatrixAppendMatrices(U, DSCaseU(cases[i]), false);
                DSMatrixFree(U);
                U = temp;
                temp = DSMatrixAppendMatrices(Zeta, DSCaseZeta(cases[i]), false);
                DSMatrixFree(Zeta);
                Zeta = temp;
                if (U == NULL || Zeta == NULL)
                        goto bail;
        }
        caseIntersection = DSSecureCalloc(1, sizeof(DSCase));
        DSCaseXd(caseIntersection) = DSCaseXd(cases[0]);
        DSCaseXi(caseIntersection) = DSCaseXi(cases[0]);
        DSCaseU(caseIntersection) = U;
        DSCaseZeta(caseIntersection) = Zeta;
        isValid = DSCaseIsValid(caseIntersection);
        DSSecureFree(caseIntersection);
bail:
        if (U != NULL)
                DSMatrixFree(U);
        if (Zeta != NULL)
                DSMatrixFree(Zeta);
        return isValid;
}

extern const bool DSCaseIntersectionIsValidAtSlice(const DSUInteger numberOfCases, const DSCase **cases,  const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds)
{
        bool isValid = false;
        DSMatrix *U = NULL, *Zeta = NULL, *temp;
        DSUInteger i;
        DSPseudoCase *caseIntersection = NULL;
        if (numberOfCases == 0) {
                DSError(M_DS_WRONG ": Number of cases must be at least one", A_DS_ERROR);
                goto bail;
        }
        if (cases == NULL) {
                DSError(M_DS_NULL ": Array of cases is NULL", A_DS_ERROR);
                goto bail;
        }
        if (lowerBounds == NULL && upperBounds == NULL) {
                DSError(M_DS_VAR_NULL ": Variable pool with variables to fix is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(lowerBounds) != DSVariablePoolNumberOfVariables(upperBounds)) {
                DSError(M_DS_WRONG ": Number of variables to bound must match", A_DS_ERROR);
                goto bail;
        }
        if (DSCaseHasSolution(cases[0]) == false)
                goto bail;
        U = DSMatrixCopy(DSCaseU(cases[0]));
        Zeta = DSMatrixCopy(DSCaseZeta(cases[0]));
        for (i = 0; i < numberOfCases; i++) {
                if (DSCaseHasSolution(cases[i]) == false)
                        goto bail;
                temp = DSMatrixAppendMatrices(U, DSCaseU(cases[i]), false);
                DSMatrixFree(U);
                U = temp;
                temp = DSMatrixAppendMatrices(Zeta, DSCaseZeta(cases[i]), false);
                DSMatrixFree(Zeta);
                Zeta = temp;
                if (U == NULL || Zeta == NULL)
                        goto bail;
        }
        caseIntersection = DSSecureCalloc(1, sizeof(DSCase));
        DSCaseXd(caseIntersection) = DSCaseXd(cases[0]);
        DSCaseXi(caseIntersection) = DSCaseXi(cases[0]);
        DSCaseU(caseIntersection) = U;
        DSCaseZeta(caseIntersection) = Zeta;
        isValid = DSCaseIsValidAtSlice(caseIntersection, lowerBounds, upperBounds);
        DSSecureFree(caseIntersection);
bail:
        if (U != NULL)
                DSMatrixFree(U);
        if (Zeta != NULL)
                DSMatrixFree(Zeta);
        return isValid;
}

extern DSVertices * DSCaseIntersectionVerticesForSlice(const DSUInteger numberOfCases, const DSCase **cases, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const DSUInteger numberOfVariables, const char ** variables)
{
        DSVertices * vertices = NULL;
        DSMatrix *U = NULL, *Zeta = NULL, *temp;
        DSUInteger i;
        DSPseudoCase *caseIntersection = NULL;
        if (numberOfCases == 0) {
                DSError(M_DS_WRONG ": Number of cases must be at least one", A_DS_ERROR);
                goto bail;
        }
        if (cases == NULL) {
                DSError(M_DS_NULL ": Array of cases is NULL", A_DS_ERROR);
                goto bail;
        }
        if (lowerBounds == NULL && upperBounds == NULL) {
                DSError(M_DS_VAR_NULL ": Variable pool with variables to fix is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(lowerBounds) != DSVariablePoolNumberOfVariables(upperBounds)) {
                DSError(M_DS_WRONG ": Number of variables to bound must match", A_DS_ERROR);
                goto bail;
        }
        if (DSCaseHasSolution(cases[0]) == false)
                goto bail;
        U = DSMatrixCopy(DSCaseU(cases[0]));
        Zeta = DSMatrixCopy(DSCaseZeta(cases[0]));
        for (i = 0; i < numberOfCases; i++) {
                if (DSCaseHasSolution(cases[i]) == false)
                        goto bail;
                temp = DSMatrixAppendMatrices(U, DSCaseU(cases[i]), false);
                DSMatrixFree(U);
                U = temp;
                temp = DSMatrixAppendMatrices(Zeta, DSCaseZeta(cases[i]), false);
                DSMatrixFree(Zeta);
                Zeta = temp;
                if (U == NULL || Zeta == NULL)
                        goto bail;
        }
        caseIntersection = DSSecureCalloc(1, sizeof(DSCase));
        DSCaseXd(caseIntersection) = DSCaseXd(cases[0]);
        DSCaseXi(caseIntersection) = DSCaseXi(cases[0]);
        DSCaseU(caseIntersection) = U;
        DSCaseZeta(caseIntersection) = Zeta;
        vertices = DSCaseVerticesForSlice(caseIntersection, lowerBounds, upperBounds, numberOfVariables, variables);
        DSSecureFree(caseIntersection);
bail:
        if (U != NULL)
                DSMatrixFree(U);
        if (Zeta != NULL)
                DSMatrixFree(Zeta);
        return vertices;
}
