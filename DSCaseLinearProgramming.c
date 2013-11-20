/**
 * \file DSCaseLinearProgramming.c
 * \brief Implementation file with functions for linear programming
 *        operations dealing with cases in design space.
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
 * \todo Find/write a parallelizable linear programming package
 */

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
                if (glp_get_obj_val(linearProblem) <= -1E-14 && glp_get_prim_stat(linearProblem) == GLP_FEAS) {
                        isValid = true;
                }
                glp_delete_prob(linearProblem);
        }
bail:
        return isValid;
}

extern const bool DSCaseIsValidInStateSpace(const DSCase *aCase)
{
        bool isValid = false;
        glp_prob *linearProblem = NULL;
        DSMatrix * C;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        C  = DSMatrixAppendMatrices(DSCaseCd(aCase), DSCaseCi(aCase), true);
        linearProblem = dsCaseLinearProblemForCaseValidity(C, DSCaseDelta(aCase));
        if (linearProblem != NULL) {
                glp_simplex(linearProblem, NULL);
                if (glp_get_obj_val(linearProblem) <= -1E-14 && glp_get_prim_stat(linearProblem) == GLP_FEAS) {
                        isValid = true;
                }
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

extern const bool DSCaseIsValidInStateSpaceAtPoint(const DSCase *aCase, const DSVariablePool * Xd_p, const DSVariablePool * Xi_p)
{
        bool isValid = false;
        DSUInteger i, numberOfXi, numberOfXd, indexOfVariable;
        DSMatrix *result, *CdYd, *CiYi, *Yi, *Yd;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseHasSolution(aCase) == false) {
                goto bail;
        }
        if (Xd_p == NULL) {
                DSError(M_DS_VAR_NULL ": Variable pool with values for dependent variable is NULL", A_DS_ERROR);
                goto bail;
        }
        if (Xi_p == NULL) {
                DSError(M_DS_VAR_NULL ": Variable pool with values for independent variable is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(Xd_p) != DSVariablePoolNumberOfVariables(DSCaseXd(aCase))) {
                DSError(M_DS_WRONG ": Inconsistent number of dependent variables", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(Xi_p) != DSVariablePoolNumberOfVariables(DSCaseXi(aCase))) {
                DSError(M_DS_WRONG ": Inconsistent number of independent variables", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(Xi_p) == 0) {
                DSError(M_DS_WRONG ": Case has no independent variables", A_DS_WARN);
                isValid = DSCaseIsValid(aCase);
                goto bail;
        }
        numberOfXi = DSVariablePoolNumberOfVariables(Xi_p);
        numberOfXd = DSVariablePoolNumberOfVariables(Xd_p);
        Yd = DSMatrixAlloc(numberOfXd, 1);
        Yi = DSMatrixAlloc(numberOfXi, 1);
        for (i = 0; i < numberOfXd; i++) {
                indexOfVariable = DSVariablePoolIndexOfVariableWithName(DSCaseXd(aCase),
                                                                        DSVariableName(DSVariablePoolAllVariables(Xd_p)[i]));
                if (indexOfVariable >= DSMatrixRows(Yd)) {
                        DSMatrixFree(Yi);
                        DSMatrixFree(Yd);
                        goto bail;
                }
                DSMatrixSetDoubleValue(Yd, indexOfVariable, 0, log10(DSVariableValue(DSVariablePoolAllVariables(Xd_p)[i])));
        }
        for (i = 0; i < numberOfXi; i++) {
                indexOfVariable = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase),
                                                                        DSVariableName(DSVariablePoolAllVariables(Xi_p)[i]));
                if (indexOfVariable >= DSMatrixRows(Yi)) {
                        DSMatrixFree(Yi);
                        DSMatrixFree(Yd);
                        goto bail;
                }
                DSMatrixSetDoubleValue(Yi, indexOfVariable, 0, log10(DSVariableValue(DSVariablePoolAllVariables(Xi_p)[i])));
        }
        CdYd = DSMatrixByMultiplyingMatrix(DSCaseCd(aCase), Yd);
        CiYi = DSMatrixByMultiplyingMatrix(DSCaseCi(aCase), Yi);
        result = DSMatrixByAddingMatrix(CdYd, CiYi);
        DSMatrixAddByMatrix(result, DSCaseDelta(aCase));
        for (i = 0; i < DSMatrixRows(result); i++)
                if (DSMatrixDoubleValue(result, i, 0) < 0)
                        break;
        if (i == DSMatrixRows(result))
                isValid = true;
        DSMatrixFree(result);
        DSMatrixFree(Yi);
        DSMatrixFree(Yd);
        DSMatrixFree(CiYi);
        DSMatrixFree(CdYd);
bail:
        return isValid;
}

extern DSVariablePool * DSCaseValidParameterSet(const DSCase *aCase)
{
        DSVariablePool * Xi = NULL;
        glp_prob *linearProblem = NULL;
        DSUInteger i;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseIsValid(aCase) == false)
                goto bail;
        linearProblem = dsCaseLinearProblemForCaseValidity(DSCaseU(aCase), DSCaseZeta(aCase));
        if (linearProblem != NULL) {
                glp_simplex(linearProblem, NULL);
                Xi = DSVariablePoolCopy(DSCaseXi(aCase));
                DSVariablePoolSetReadWriteAdd(Xi);
                for (i = 0; i < DSVariablePoolNumberOfVariables(Xi); i++) {
                        DSVariableSetValue(DSVariablePoolAllVariables(Xi)[i], pow(10, glp_get_col_prim(linearProblem, i+1)));
                }
                glp_delete_prob(linearProblem);
        }
        if (DSCaseIsValid(aCase) == false)
                goto bail;
bail:
        return Xi;
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
        for (i = 0; i < glp_get_num_cols(linearProblem); i++) {
                glp_set_col_bnds(linearProblem, i+1, GLP_FR, 0.0, 0.0);
        }
        for (i = 0; i < DSVariablePoolNumberOfVariables(lowerBounds); i++) {
                
                lowVariable = DSVariablePoolAllVariables(lowerBounds)[i];
                highVariable = DSVariablePoolVariableWithName(upperBounds, DSVariableName(lowVariable));
                if (lowVariable == NULL || highVariable == NULL) {
                        DSError(M_DS_WRONG ": Variables to bound are not consistent", A_DS_WARN);
                        freeVariables = 0;
                        break;
                }
                
                variableIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), 
                                                                      DSVariableName(lowVariable));
                
                low = DSVariableValue(lowVariable);
                high = DSVariableValue(highVariable);
                
                if (low > high) {
                        DSError(M_DS_WRONG ": Variable bounds are not consistent", A_DS_WARN);
                        freeVariables = 0;
                        break;
                }
                
                if (variableIndex >= DSVariablePoolNumberOfVariables(DSCaseXi(aCase))) {
                        freeVariables = 0;
                        break;
                }
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

extern DSVariablePool * DSCaseValidParameterSetAtSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds)
{
        bool isValid = false;
        glp_prob *linearProblem = NULL;
        DSVariablePool * Xi = NULL;
        DSUInteger i;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseHasSolution(aCase) == false) {
                goto bail;
        }
        if (lowerBounds == NULL || upperBounds == NULL) {
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
                if (glp_get_obj_val(linearProblem) <= -1E-14 && glp_get_prim_stat(linearProblem) == GLP_FEAS)
                        isValid = true;
        }
        if (isValid == true) {
                Xi = DSVariablePoolCopy(DSCaseXi(aCase));
                DSVariablePoolSetReadWrite(Xi);
                for (i = 0; i < DSVariablePoolNumberOfVariables(Xi); i++) {
                        DSVariableSetValue(DSVariablePoolAllVariables(Xi)[i],
                                           pow(10, glp_get_col_prim(linearProblem, i+1)));
                }
        }
        glp_delete_prob(linearProblem);
bail:
        return Xi;
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
        if (lowerBounds == upperBounds) {
                isValid = DSCaseIsValidAtPoint(aCase, lowerBounds);
                goto bail;
        }
        if (dsCaseNumberOfFreeVariablesForBounds(aCase, lowerBounds, upperBounds) == 0) {
                isValid = DSCaseIsValidAtPoint(aCase, lowerBounds);
                goto bail;
        }
        linearProblem = dsCaseLinearProblemForCaseValidity(DSCaseU(aCase), DSCaseZeta(aCase));
        if (linearProblem == NULL) {
                DSError(M_DS_NULL ": Linear problem was not created", A_DS_WARN);
                goto bail;
        }
        if (dsCaseSetVariableBoundsLinearProblem(aCase, linearProblem, lowerBounds, upperBounds) <= DSVariablePoolNumberOfVariables(DSCaseXi(aCase))) {
                glp_simplex(linearProblem, NULL);
                if (glp_get_obj_val(linearProblem) <= -1E-14 && glp_get_prim_stat(linearProblem) == GLP_FEAS)
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

static DSVertices * dsCaseCalculateBoundingRange(const DSCase * aCase, glp_prob * linearProblem, const DSUInteger index)
{
        DSVertices *vertices = NULL;
        double minVal, maxVal, val[1] = {INFINITY};
        vertices = DSVerticesAlloc(1);
        glp_set_obj_coef(linearProblem, index+1, 1.0);
        glp_simplex(linearProblem, NULL);
        maxVal = glp_get_obj_val(linearProblem);
        if (glp_get_prim_stat(linearProblem) == GLP_FEAS) {
                val[0] = maxVal;
                DSVerticesAddVertex(vertices, val);
        }
        glp_set_obj_coef(linearProblem, index+1, -1.0);
        glp_simplex(linearProblem, NULL);
        if (glp_get_prim_stat(linearProblem) == GLP_FEAS) {
                minVal = -glp_get_obj_val(linearProblem);
                if (minVal != maxVal) {
                        val[0] = minVal;
                        DSVerticesAddVertex(vertices, val);
                }
        }
bail:
        return vertices;
}

extern DSVertices * DSCaseBoundingRangeForVariableWithConstraints(const DSCase *aCase, const char * variable, DSVariablePool * lowerBounds, DSVariablePool * upperBounds)
{
        DSVertices *vertices = NULL;
        DSUInteger index;
        DSMatrix *A = NULL, *Zeta = NULL, *temp;
        glp_prob * linearProblem = NULL;
        
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        
        index = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), variable);
        
        if (index >= DSVariablePoolNumberOfVariables(DSCaseXi(aCase))) {
                DSError(M_DS_WRONG ": Case does not have variable", A_DS_ERROR);
                goto bail;
        }
        
        temp = DSMatrixCalloc(2, DSVariablePoolNumberOfVariables(DSCaseXi(aCase)));
        DSMatrixSetDoubleValue(temp, 0, index, 1.0);
        DSMatrixSetDoubleValue(temp, 1, index, -1.0);
        A = DSMatrixAppendMatrices(DSCaseU(aCase), temp, false);
        DSMatrixFree(temp);
        temp = DSMatrixCalloc(2, 1);
        DSMatrixSetDoubleValue(temp, 0, 0, 15.0f);
        DSMatrixSetDoubleValue(temp, 1, 0, 15.0f);
        Zeta = DSMatrixAppendMatrices(DSCaseZeta(aCase), temp, false);
        DSMatrixFree(temp);
        DSMatrixMultiplyByScalar(A, -1.0);
        linearProblem = dsCaseLinearProblemForMatrices(A, Zeta);
        
        if (linearProblem == NULL) {
                DSError(M_DS_NULL ": Linear problem is NULL", A_DS_ERROR);
                goto bail;
        }
        
        if (dsCaseSetVariableBoundsLinearProblem(aCase, linearProblem, lowerBounds, upperBounds) == 0) {
                DSError(M_DS_WRONG ": Needs at least one free variables", A_DS_ERROR);
                glp_delete_prob(linearProblem);
                goto bail;
        }
        
        if (glp_get_col_type(linearProblem, DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), variable)+1) == GLP_FX) {
                DSError(M_DS_WRONG ": variable is fixed", A_DS_ERROR);
                glp_delete_prob(linearProblem);
                goto bail;
        }
        
        vertices = dsCaseCalculateBoundingRange(aCase, linearProblem, index);
        glp_delete_prob(linearProblem);
bail:
        if (A != NULL)
                DSMatrixFree(A);
        if (Zeta != NULL)
                DSMatrixFree(Zeta);
        return vertices;
}

extern DSVertices * DSCaseBoundingRangeForVariable(const DSCase *aCase, const char * variable)
{
        DSVertices *vertices = NULL;
        DSUInteger i, index;
        DSMatrix *A = NULL, *Zeta = NULL, *temp;
        glp_prob * linearProblem = NULL;
        
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        
        index = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), variable);
        
        if (index >= DSVariablePoolNumberOfVariables(DSCaseXi(aCase))) {
                DSError(M_DS_WRONG ": Case does not have variable", A_DS_ERROR);
                goto bail;
        }
        
        temp = DSMatrixCalloc(2, DSVariablePoolNumberOfVariables(DSCaseXi(aCase)));
        DSMatrixSetDoubleValue(temp, 0, index, 1.0);
        DSMatrixSetDoubleValue(temp, 1, index, -1.0);
        A = DSMatrixAppendMatrices(DSCaseU(aCase), temp, false);
        DSMatrixFree(temp);
        temp = DSMatrixCalloc(2, 1);
        DSMatrixSetDoubleValue(temp, 0, 0, 15.0f);
        DSMatrixSetDoubleValue(temp, 1, 0, 15.0f);
        Zeta = DSMatrixAppendMatrices(DSCaseZeta(aCase), temp, false);
        DSMatrixFree(temp);
        DSMatrixMultiplyByScalar(A, -1.0);
        linearProblem = dsCaseLinearProblemForMatrices(A, Zeta);
        
        if (linearProblem == NULL) {
                DSError(M_DS_NULL ": Linear problem is NULL", A_DS_ERROR);
                goto bail;
        }
        
        for (i = 0; i < DSVariablePoolNumberOfVariables(DSCaseXd(aCase)); i++)
                glp_set_col_bnds(linearProblem, i+1, GLP_FR, 0.0, 0.0);
        
        vertices = dsCaseCalculateBoundingRange(aCase, linearProblem, index);
        glp_delete_prob(linearProblem);
bail:
        if (A != NULL)
                DSMatrixFree(A);
        if (Zeta != NULL)
                DSMatrixFree(Zeta);
        return vertices;
}

static DSVertices * dsCaseCalculate1DVertices(const DSCase * aCase, glp_prob * linearProblem, const DSMatrix * A, const DSMatrix *Zeta, const DSUInteger xIndex, const DSVariablePool * lower, const DSVariablePool * upper)
{
        DSVertices *vertices = NULL;
        double minVal, maxVal, val[1] = {INFINITY};
        vertices = DSVerticesAlloc(1);
        glp_set_obj_coef(linearProblem, xIndex+1, 1.0);
        glp_simplex(linearProblem, NULL);
        if (glp_get_prim_stat(linearProblem) == GLP_FEAS) {
                maxVal = glp_get_obj_val(linearProblem);
                val[0] = maxVal;
                DSVerticesAddVertex(vertices, val);
        }
        glp_set_obj_coef(linearProblem, xIndex+1, -1.0);
        glp_simplex(linearProblem, NULL);
        if (glp_get_prim_stat(linearProblem) == GLP_FEAS) {
                minVal = -glp_get_obj_val(linearProblem);
                if (minVal != maxVal) {
                        val[0] = minVal;
                        DSVerticesAddVertex(vertices, val);
                }
        }
bail:
        return vertices;
}

extern DSVertices * DSCaseVerticesFor1DSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable)
{
        DSVertices *vertices = NULL;
        DSUInteger xIndex;
        DSMatrix *A, *Zeta, *temp;
        glp_prob * linearProblem = NULL;
        
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        
        if (dsCaseNumberOfFreeVariablesForBounds(aCase, lowerBounds, upperBounds) != 1) {
                DSError(M_DS_WRONG ": Must have only one free variables", A_DS_ERROR);
                goto bail;
        }
        
        xIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), xVariable);
        
        if (xIndex >= DSVariablePoolNumberOfVariables(DSCaseXi(aCase))) {
                DSError(M_DS_WRONG ": Case does not have X variable", A_DS_ERROR);
                goto bail;
        }
        
        temp = DSMatrixCalloc(2, DSVariablePoolNumberOfVariables(DSCaseXi(aCase)));
        DSMatrixSetDoubleValue(temp, 0, xIndex, 1.0);
        DSMatrixSetDoubleValue(temp, 1, xIndex, -1.0);
        A = DSMatrixAppendMatrices(DSCaseU(aCase), temp, false);
        DSMatrixFree(temp);
        temp = DSMatrixCalloc(2, 1);
        DSMatrixSetDoubleValue(temp, 0, 0, -log10(DSVariableValue(DSVariablePoolVariableWithName(lowerBounds, xVariable))));
        DSMatrixSetDoubleValue(temp, 1, 0, log10(DSVariableValue(DSVariablePoolVariableWithName(upperBounds, xVariable))));
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
        
        if (dsCaseSetVariableBoundsLinearProblem(aCase, linearProblem, lowerBounds, upperBounds) != 1) {
                DSError(M_DS_WRONG ": Need one free variables", A_DS_ERROR);
                DSMatrixFree(A);
                DSMatrixFree(Zeta);
                glp_delete_prob(linearProblem);
                goto bail;
        }
        
        if (glp_get_col_type(linearProblem, DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), xVariable)+1) != GLP_DB) {
                DSError(M_DS_WRONG ": X Variable is not double bound", A_DS_ERROR);
                DSMatrixFree(A);
                DSMatrixFree(Zeta);
                glp_delete_prob(linearProblem);
                goto bail;
        }
        
        vertices = dsCaseCalculate1DVertices(aCase, linearProblem, A, Zeta, xIndex, lowerBounds, upperBounds);
        DSMatrixFree(A);
        DSMatrixFree(Zeta);
        glp_delete_prob(linearProblem);
bail:
        return vertices;
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

extern DSVertices * DSCaseVerticesFor2DSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable)
{
        DSVertices *vertices = NULL;
        DSUInteger yIndex, xIndex;
        DSMatrix *A, *Zeta, *temp;
        glp_prob * linearProblem = NULL;
        
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        
        if (dsCaseNumberOfFreeVariablesForBounds(aCase, lowerBounds, upperBounds) != 2) {
                DSError(M_DS_WRONG ": Must have only two free variables", A_DS_ERROR);
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
        DSSecureFree(cases);

bail:
        return isValid;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Pseudocase with intersection of cases
#endif

/**
 * 
 */
static DSPseudoCase * dsPseudoCaseFromIntersectionOfCases(const DSUInteger numberOfCases, const DSCase ** cases)
{
        DSUInteger i;
        DSPseudoCase * caseIntersection = NULL;
        DSMatrix *U = NULL, *Zeta = NULL, *temp;
        if (numberOfCases == 0) {
                DSError(M_DS_WRONG ": Number of cases must be at least one", A_DS_ERROR);
                goto bail;
        }
        if (cases == NULL) {
                DSError(M_DS_NULL ": Array of cases is NULL", A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < numberOfCases; i++) {
                if (DSCaseHasSolution(cases[i]) == false)
                        goto bail;
        }
        U = DSMatrixCopy(DSCaseU(cases[0]));
        Zeta = DSMatrixCopy(DSCaseZeta(cases[0]));
        for (i = 1; i < numberOfCases; i++) {
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
        U = NULL;
        Zeta = NULL;
bail:
        if (U != NULL)
                DSMatrixFree(U);
        if (Zeta != NULL)
                DSMatrixFree(Zeta);
        return caseIntersection;
}

/**
 *
 */
static DSPseudoCase * dsPseudoCaseFromIntersectionOfCasesExceptingSlice(const DSUInteger numberOfCases, const DSCase ** cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames)
{
        DSUInteger i, j, k, currentRow, numberOfExtraColumns, rows, columns, *indices;
        DSPseudoCase * caseIntersection = NULL;
        DSMatrix *U = NULL, *Zeta = NULL, *tempU, *tempZeta;
        if (numberOfCases == 0) {
                DSError(M_DS_WRONG ": Number of cases must be at least one", A_DS_ERROR);
                goto bail;
        }
        if (cases == NULL) {
                DSError(M_DS_NULL ": Array of cases is NULL", A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < numberOfCases; i++) {
                if (DSCaseHasSolution(cases[i]) == false)
                        goto bail;
        }
        indices = DSSecureCalloc(numberOfExceptions, sizeof(DSUInteger));
        for (j = 0; j < numberOfExceptions; j++) {
                for (i = 0; i < numberOfCases; i++) {
                        if (DSVariablePoolHasVariableWithName(DSCaseXi(cases[i]), exceptionVarNames[j]) == false) {
                                DSError(M_DS_WRONG ": Case does not have variable to except", A_DS_ERROR);
                                DSSecureFree(indices);
                                goto bail;
                        }
                }
                indices[j] = DSVariablePoolIndexOfVariableWithName(DSCaseXi(cases[0]), exceptionVarNames[j]);
        }
        numberOfExtraColumns = numberOfExceptions*(numberOfCases-1);
        rows = 0;
        columns = DSMatrixColumns(DSCaseU(cases[0]))+numberOfExtraColumns;
        for (i = 0; i < numberOfCases; i++) {
                rows += DSMatrixRows(DSCaseZeta(cases[i]));
        }
        U = DSMatrixAlloc(rows, columns);
        Zeta = DSMatrixAlloc(rows, 1);
        currentRow = 0;
        for (i = 0; i < numberOfCases; i++) {
                tempU = DSCaseU(cases[i]);
                tempZeta = DSCaseZeta(cases[i]);
                for (j = 0; j < DSMatrixRows(tempZeta); j++) {
                        DSMatrixSetDoubleValue(Zeta, currentRow, 0, DSMatrixDoubleValue(tempZeta, j, 0));
                        for (k = 0; k < DSMatrixColumns(tempU); k++) {
                                DSMatrixSetDoubleValue(U, currentRow, k, DSMatrixDoubleValue(tempU, j, k));
                        }
                        if (i > 0) {
                                for (k = 0; k < numberOfExceptions; k++) {
                                        DSMatrixSetDoubleValue(U,
                                                               currentRow,
                                                               DSMatrixColumns(tempU)+numberOfExceptions*(i-1)+k,
                                                               DSMatrixDoubleValue(U, currentRow, indices[k]));
                                        DSMatrixSetDoubleValue(U, currentRow, indices[k], 0.0f);
                                }
                        }
                        currentRow++;
                }
        }
        caseIntersection = DSSecureCalloc(1, sizeof(DSCase));
        DSCaseXd(caseIntersection) = DSCaseXd(cases[0]);
        DSCaseXi(caseIntersection) = DSCaseXi(cases[0]);
        DSCaseU(caseIntersection) = U;
        DSCaseZeta(caseIntersection) = Zeta;
        U = NULL;
        Zeta = NULL;
        DSSecureFree(indices);
bail:
        if (U != NULL)
                DSMatrixFree(U);
        if (Zeta != NULL)
                DSMatrixFree(Zeta);
        return caseIntersection;
}

extern const bool DSCaseIntersectionIsValid(const DSUInteger numberOfCases, const DSCase **cases)
{
        bool isValid = false;
        DSPseudoCase * caseIntersection = NULL;
        caseIntersection = dsPseudoCaseFromIntersectionOfCases(numberOfCases, cases);
        if (caseIntersection == NULL)
                goto bail;
        isValid = DSCaseIsValid(caseIntersection);
        DSSecureFree(caseIntersection);
bail:
        return isValid;
}

extern const bool DSCaseIntersectionIsValidAtSlice(const DSUInteger numberOfCases, const DSCase **cases,  const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds)
{
        bool isValid = false;
        DSPseudoCase *caseIntersection = NULL;
        caseIntersection = dsPseudoCaseFromIntersectionOfCases(numberOfCases, cases);
        if (caseIntersection == NULL)
                goto bail;
        isValid = DSCaseIsValidAtSlice(caseIntersection, lowerBounds, upperBounds);
        DSSecureFree(caseIntersection);
bail:
        return isValid;
}

extern const bool DSCaseIntersectionExceptSliceIsValid(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames)
{
        bool isValid = false;
        DSPseudoCase *caseIntersection = NULL;
        caseIntersection = dsPseudoCaseFromIntersectionOfCasesExceptingSlice(numberOfCases, cases, numberOfExceptions, exceptionVarNames);
        if (caseIntersection == NULL)
                goto bail;
        isValid = DSCaseIsValid(caseIntersection);
        DSSecureFree(caseIntersection);
bail:
        return isValid;
}

extern const bool DSCaseIntersectionExceptSliceIsValidAtSlice(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames, const DSVariablePool * lowerBounds, const DSVariablePool * upperBounds)
{
        bool isValid = false;
        DSPseudoCase *caseIntersection = NULL;
        caseIntersection = dsPseudoCaseFromIntersectionOfCasesExceptingSlice(numberOfCases, cases, numberOfExceptions, exceptionVarNames);
        if (caseIntersection == NULL)
                goto bail;
        isValid = DSCaseIsValidAtSlice(caseIntersection, lowerBounds, upperBounds);
        DSSecureFree(caseIntersection);
bail:
        return isValid;
}

extern DSVariablePool * DSCaseIntersectionExceptSliceValidParameterSet(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames)
{
        DSPseudoCase *caseIntersection = NULL;
        DSVariablePool * variablePool = NULL;
        caseIntersection = dsPseudoCaseFromIntersectionOfCasesExceptingSlice(numberOfCases, cases, numberOfExceptions, exceptionVarNames);
        if (caseIntersection == NULL)
                goto bail;
        variablePool = DSCaseValidParameterSet(caseIntersection);
        DSSecureFree(caseIntersection);
bail:
        return variablePool;
}

extern DSVariablePool * DSCaseIntersectionExceptSliceValidParameterSetAtSlice(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames, const DSVariablePool * lowerBounds, const DSVariablePool * upperBounds)
{
        DSPseudoCase *caseIntersection = NULL;
        DSVariablePool * variablePool = NULL;
        caseIntersection = dsPseudoCaseFromIntersectionOfCasesExceptingSlice(numberOfCases, cases, numberOfExceptions, exceptionVarNames);
        if (caseIntersection == NULL)
                goto bail;
        variablePool = DSCaseValidParameterSetAtSlice(caseIntersection, lowerBounds, upperBounds);
        DSSecureFree(caseIntersection);
bail:
        return variablePool;
}


extern DSVertices * DSCaseIntersectionVerticesForSlice(const DSUInteger numberOfCases, const DSCase **cases, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const DSUInteger numberOfVariables, const char ** variables)
{
        DSVertices * vertices = NULL;
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
        caseIntersection = dsPseudoCaseFromIntersectionOfCases(numberOfCases, cases);
        if (cases == NULL)
                goto bail;
        if (numberOfVariables == 1) {
                vertices = DSCaseVerticesFor1DSlice(caseIntersection, lowerBounds, upperBounds, variables[0]);
        } else {
                vertices = DSCaseVerticesForSlice(caseIntersection, lowerBounds, upperBounds, numberOfVariables, variables);
        }
        DSSecureFree(caseIntersection);
bail:
        return vertices;
}
