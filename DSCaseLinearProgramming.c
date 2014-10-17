/**
 * \file DSCaseLinearProgramming.c
 * \brief Implementation file with functions for linear programming
 *        operations dealing with cases in design space.
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
#include "DSNVertexEnumeration.h"
#include "DSGMASystemParsingAux.h"
#include "DSExpressionTokenizer.h"
#include "DSCaseOptimizationFunctionGrammar.h"
#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Linear programming functions
#endif


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

extern const bool DSCaseConditionsAreValid(const DSCase *aCase)
{
        bool isValid = false;
        glp_prob *linearProblem = NULL;
        DSMatrix * U, * Zeta;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        U = DSMatrixAppendMatrices(DSCaseCd(aCase), DSCaseCi(aCase), true);
        Zeta = DSCaseDelta(aCase);
        linearProblem = dsCaseLinearProblemForCaseValidity(U, Zeta);
        DSMatrixFree(U);
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
                if (glp_get_obj_val(linearProblem) <= -1E-14 && glp_get_prim_stat(linearProblem) == GLP_FEAS) {
                        Xi = DSVariablePoolCopy(DSCaseXi(aCase));
                        DSVariablePoolSetReadWriteAdd(Xi);
                        for (i = 0; i < DSVariablePoolNumberOfVariables(Xi); i++) {
                                DSVariableSetValue(DSVariablePoolAllVariables(Xi)[i], pow(10, glp_get_col_prim(linearProblem, i+1)));
                        }
                }
                glp_delete_prob(linearProblem);
        }
bail:
        return Xi;
}

extern DSVariablePool * DSCaseValidParameterSetByOptimizingFunction(const DSCase *aCase, const char * function, const bool minimize)
{
        DSVariablePool * Xi = NULL;
        glp_prob *linearProblem = NULL;
        DSUInteger i;
        DSMatrixArray * objective;
        DSMatrix * Od, *U, *Zeta;
        DSMatrix * delta;
        DSExpression * expression;
        char * processedFunction;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseIsValid(aCase) == false)
                goto bail;
        expression = DSExpressionByParsingString(function);
        if (expression == NULL) {
                DSError(M_DS_NULL ": Could not parse function string", A_DS_ERROR);
                goto bail;
        }
        processedFunction = DSExpressionAsString(expression);
        DSExpressionFree(expression);
        objective = DSCaseParseOptimizationFunction(aCase, processedFunction);
        DSSecureFree(processedFunction);
        if (objective == NULL) {
                goto bail;
        }
        U = DSMatrixCopy(DSCaseU(aCase));
        Zeta = DSMatrixCopy(DSCaseZeta(aCase));
        Od = DSMatrixArrayMatrix(objective, 0);
        delta = DSMatrixArrayMatrix(objective, 1);
        DSMatrixMultiplyByScalar(U, -1.0);
        linearProblem = dsCaseLinearProblemForMatrices(U, Zeta);
        DSMatrixFree(U);
        DSMatrixFree(Zeta);
        if (linearProblem == NULL) {
                DSMatrixArrayFree(objective);
                goto bail;
        }
        if (minimize == false) {
                glp_set_obj_dir(linearProblem, GLP_MAX);
        }
        for (i = 0; i < DSMatrixColumns(Od); i++) {
                glp_set_obj_coef(linearProblem, i+1, DSMatrixDoubleValue(Od, 0, i));
                // Limits on optimization bounded between 1e-20 and 1e20
                glp_set_col_bnds(linearProblem, i+1, GLP_DB, -20, 20);
        }
        glp_set_obj_coef(linearProblem, 0, DSMatrixDoubleValue(delta, 0, 0));
        DSMatrixPrint(Od);
        glp_simplex(linearProblem, NULL);
        if (glp_get_status(linearProblem) != GLP_OPT) {
                glp_delete_prob(linearProblem);
                DSMatrixArrayFree(objective);
                goto bail;
        }
        Xi = DSVariablePoolCopy(DSCaseXi(aCase));
        DSVariablePoolSetReadWriteAdd(Xi);
        for (i = 0; i < DSVariablePoolNumberOfVariables(Xi); i++) {
                DSVariableSetValue(DSVariablePoolAllVariables(Xi)[i], pow(10, glp_get_col_prim(linearProblem, i+1)));
        }
        glp_delete_prob(linearProblem);
        DSMatrixArrayFree(objective);
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

extern DSVariablePool * DSCaseValidParameterSetAtSliceByOptimizingFunction(const DSCase *aCase,
                                                                           const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds,
                                                                           const char * function, const bool minimize)
{
        glp_prob *linearProblem = NULL;
        DSVariablePool * Xi = NULL;
        DSUInteger i;
        DSMatrixArray * objective;
        DSMatrix * Oi, *delta, *U, *Zeta;
        DSExpression * expression;
        char * processedFunction;
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
        expression = DSExpressionByParsingString(function);
        if (expression == NULL) {
                DSError(M_DS_NULL ": Could not parse function string", A_DS_ERROR);
                goto bail;
        }
        processedFunction = DSExpressionAsString(expression);
        DSExpressionFree(expression);
        objective = DSCaseParseOptimizationFunction(aCase, processedFunction);
        DSSecureFree(processedFunction);
        if (objective == NULL) {
                goto bail;
        }
        Oi = DSMatrixArrayMatrix(objective, 0);
        delta = DSMatrixArrayMatrix(objective, 1);
        U = DSMatrixCopy(DSCaseU(aCase));
        Zeta = DSMatrixCopy(DSCaseZeta(aCase));
        DSMatrixMultiplyByScalar(U, -1.0);
        linearProblem = dsCaseLinearProblemForMatrices(U, Zeta);
        DSMatrixFree(U);
        DSMatrixFree(Zeta);        if (linearProblem == NULL) {
                DSError(M_DS_NULL ": Linear problem is null", A_DS_WARN);
                DSMatrixArrayFree(objective);
                goto bail;
        }
        if (dsCaseSetVariableBoundsLinearProblem(aCase, linearProblem, lowerBounds, upperBounds) > DSVariablePoolNumberOfVariables(DSCaseXi(aCase))) {
                glp_delete_prob(linearProblem);
                DSMatrixArrayFree(objective);
                goto bail;
        }
        for (i = 0; i < DSMatrixColumns(Oi); i++) {
                glp_set_obj_coef(linearProblem, i+1, DSMatrixDoubleValue(Oi, 0, i));
        }
        if (minimize == false) {
                glp_set_obj_dir(linearProblem, GLP_MAX);
        }
        glp_set_obj_coef(linearProblem, 0, DSMatrixDoubleValue(delta, 0, 0));
        if (linearProblem != NULL) {
                glp_simplex(linearProblem, NULL);
                if (glp_get_status(linearProblem) != GLP_OPT) {
                        glp_delete_prob(linearProblem);
                        DSMatrixArrayFree(objective);
                        goto bail;
                }
                Xi = DSVariablePoolCopy(DSCaseXi(aCase));
                DSVariablePoolSetReadWriteAdd(Xi);
                for (i = 0; i < DSVariablePoolNumberOfVariables(Xi); i++) {
                        DSVariableSetValue(DSVariablePoolAllVariables(Xi)[i], pow(10, glp_get_col_prim(linearProblem, i+1)));
                }
        }
        Xi = DSVariablePoolCopy(DSCaseXi(aCase));
        DSVariablePoolSetReadWrite(Xi);
        for (i = 0; i < DSVariablePoolNumberOfVariables(Xi); i++) {
                DSVariableSetValue(DSVariablePoolAllVariables(Xi)[i],
                                   pow(10, glp_get_col_prim(linearProblem, i+1)));
        }
        DSMatrixArrayFree(objective);
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
        double minVal = 0, maxVal = 0, val[1] = {INFINITY};
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


static DSVertices * dsCaseCalculate3DVertices(const DSCase * aCase, glp_prob * linearProblem, const DSMatrix * A, const DSMatrix *Zeta, const DSUInteger xIndex, const DSUInteger yIndex, const DSUInteger zIndex)
{
        DSVertices *vertices = NULL;
        DSUInteger numberOfCombinations, i, firstIndex, secondIndex, thirdIndex;
        DSUInteger numberOfBoundaries;
        double bnd, xVal, yVal, zVal, vals[3];
        
        numberOfBoundaries = DSMatrixRows(A);
        numberOfCombinations = 0;
        
        vertices = DSVerticesAlloc(3);
        for (firstIndex = 0; firstIndex < numberOfBoundaries; firstIndex++) {
                for (secondIndex = firstIndex+1; secondIndex < numberOfBoundaries; secondIndex++) {
                        for (thirdIndex = secondIndex+1; thirdIndex < numberOfBoundaries; thirdIndex++) {

                                for (i = 0; i < numberOfBoundaries; i++) {
                                        bnd = glp_get_row_ub(linearProblem, i+1);
                                        glp_set_row_bnds(linearProblem, i+1, GLP_UP, 0.0, bnd);
                                }
                                bnd = glp_get_row_ub(linearProblem, firstIndex+1);
                                glp_set_row_bnds(linearProblem, firstIndex+1, GLP_FX, bnd, bnd);
                                bnd = glp_get_row_ub(linearProblem, secondIndex+1);
                                glp_set_row_bnds(linearProblem, secondIndex+1, GLP_FX, bnd, bnd);
                                bnd = glp_get_row_ub(linearProblem, thirdIndex+1);
                                glp_set_row_bnds(linearProblem, thirdIndex+1, GLP_FX, bnd, bnd);
                                for (i = 0; i < DSVariablePoolNumberOfVariables(DSCaseXi(aCase)); i++)
                                        glp_set_obj_coef(linearProblem, i+1, 0.0);
                                glp_set_obj_coef(linearProblem, xIndex+1, 1.0);
                                glp_simplex(linearProblem, NULL);
                                if (glp_get_prim_stat(linearProblem) != GLP_FEAS)
                                        continue;
                                xVal = glp_get_obj_val(linearProblem);
                                glp_set_obj_coef(linearProblem, xIndex+1, 0.0);
                                glp_set_obj_coef(linearProblem, yIndex+1, 1.0);
                                glp_simplex(linearProblem, NULL);
                                if (glp_get_prim_stat(linearProblem) != GLP_FEAS)
                                        continue;
                                yVal = glp_get_obj_val(linearProblem);
                                glp_set_obj_coef(linearProblem, yIndex+1, 0.0);
                                glp_set_obj_coef(linearProblem, zIndex+1, 1.0);
                                glp_simplex(linearProblem, NULL);
                                if (glp_get_prim_stat(linearProblem) != GLP_FEAS)
                                        continue;
                                zVal = glp_get_obj_val(linearProblem);
                                vals[0] = xVal;
                                vals[1] = yVal;
                                vals[2] = zVal;
                                DSVerticesAddVertex(vertices, vals);
                                numberOfCombinations++;
                        }
                }
        }
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

extern DSMatrixArray * DSCaseFacesFor3DSliceAndConnectivity(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable, const char *zVariable)
{
        DSMatrixArray * faces = NULL;
        DSVertices * vertices;
        DSUInteger xIndex, yIndex, zIndex;
        vertices = DSCaseVerticesFor3DSlice(aCase, lowerBounds, upperBounds, xVariable, yVariable, zVariable);
        if (vertices == NULL) {
                goto exit;
        }
        yIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), yVariable);
        xIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), xVariable);
        zIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), zVariable);
        faces = DSVertices3DFaces(vertices, aCase, lowerBounds, upperBounds, xIndex, yIndex, zIndex);
        DSVerticesFree(vertices);
exit:
        return faces;
}


extern DSMatrixArray * DSCaseVerticesFor3DSliceAndConnectivity(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable, const char *zVariable)
{
        DSMatrixArray * verticesAndConnectivity = NULL;
        DSVertices * vertices;
        DSMatrix * connectivity;
        DSUInteger xIndex, yIndex, zIndex;
        vertices = DSCaseVerticesFor3DSlice(aCase, lowerBounds, upperBounds, xVariable, yVariable, zVariable);
        if (vertices == NULL) {
                goto exit;
        }
        yIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), yVariable);
        xIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), xVariable);
        zIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), zVariable);
        connectivity = DSVertices3DConnectivityMatrix(vertices, aCase, lowerBounds, upperBounds, xIndex, yIndex, zIndex);
        verticesAndConnectivity = DSMatrixArrayAlloc();
        DSMatrixArrayAddMatrix(verticesAndConnectivity, DSVerticesToMatrix(vertices));
        DSMatrixArrayAddMatrix(verticesAndConnectivity, connectivity);
        DSVerticesFree(vertices);
exit:
        return verticesAndConnectivity;
}
extern DSVertices * DSCaseVerticesFor3DSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable, const char *zVariable)
{
        DSVertices *vertices = NULL;
        DSUInteger yIndex, xIndex, zIndex;
        DSMatrix *A, *Zeta, *temp;
        glp_prob * linearProblem = NULL;
        
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        
        if (dsCaseNumberOfFreeVariablesForBounds(aCase, lowerBounds, upperBounds) != 3) {
                DSError(M_DS_WRONG ": Must have only three free variables", A_DS_ERROR);
                goto bail;
        }
        
        yIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), yVariable);
        xIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), xVariable);
        zIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), zVariable);
        
        if (xIndex >= DSVariablePoolNumberOfVariables(DSCaseXi(aCase))) {
                DSError(M_DS_WRONG ": Case does not have X variable", A_DS_ERROR);
                goto bail;
        }
        if (yIndex >= DSVariablePoolNumberOfVariables(DSCaseXi(aCase))) {
                DSError(M_DS_WRONG ": Case does not have Y variable", A_DS_ERROR);
                goto bail;
        }
        if (zIndex >= DSVariablePoolNumberOfVariables(DSCaseXi(aCase))) {
                DSError(M_DS_WRONG ": Case does not have Z variable", A_DS_ERROR);
                goto bail;
        }
        temp = DSMatrixCalloc(6, DSVariablePoolNumberOfVariables(DSCaseXi(aCase)));
        DSMatrixSetDoubleValue(temp, 0, xIndex, 1.0);
        DSMatrixSetDoubleValue(temp, 1, xIndex, -1.0);
        DSMatrixSetDoubleValue(temp, 2, yIndex, 1.0);
        DSMatrixSetDoubleValue(temp, 3, yIndex, -1.0);
        DSMatrixSetDoubleValue(temp, 4, zIndex, 1.0);
        DSMatrixSetDoubleValue(temp, 5, zIndex, -1.0);
        
        A = DSMatrixAppendMatrices(DSCaseU(aCase), temp, false);
        DSMatrixFree(temp);
        temp = DSMatrixCalloc(6, 1);
        DSMatrixSetDoubleValue(temp, 0, 0, -log10(DSVariableValue(DSVariablePoolVariableWithName(lowerBounds, xVariable))));
        DSMatrixSetDoubleValue(temp, 1, 0, log10(DSVariableValue(DSVariablePoolVariableWithName(upperBounds, xVariable))));
        DSMatrixSetDoubleValue(temp, 2, 0, -log10(DSVariableValue(DSVariablePoolVariableWithName(lowerBounds, yVariable))));
        DSMatrixSetDoubleValue(temp, 3, 0, log10(DSVariableValue(DSVariablePoolVariableWithName(upperBounds, yVariable))));
        DSMatrixSetDoubleValue(temp, 4, 0, -log10(DSVariableValue(DSVariablePoolVariableWithName(lowerBounds, zVariable))));
        DSMatrixSetDoubleValue(temp, 5, 0, log10(DSVariableValue(DSVariablePoolVariableWithName(upperBounds, zVariable))));
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
        
        if (dsCaseSetVariableBoundsLinearProblem(aCase, linearProblem, lowerBounds, upperBounds) != 3) {
                DSError(M_DS_WRONG ": Need three free variables", A_DS_ERROR);
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
        if (glp_get_col_type(linearProblem, DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), zVariable)+1) == GLP_FX) {
                DSError(M_DS_WRONG ": Z Variable is fixed", A_DS_ERROR);
                DSMatrixFree(A);
                DSMatrixFree(Zeta);
                glp_delete_prob(linearProblem);
                goto bail;
        }
        vertices = dsCaseCalculate3DVertices(aCase, linearProblem, A, Zeta, xIndex, yIndex, zIndex);
        DSMatrixFree(A);
        DSMatrixFree(Zeta);
        glp_delete_prob(linearProblem);
bail:
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

extern DSMatrixArray * DSCaseVerticesForNDSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds)
{
        DSUInteger i, j;
        DSVertices * vertices = NULL;
        DSMatrix * vertexMatrix;
        DSMatrix * cobasisMatrix;
        DSMatrixArray * matrixArray, *vertexAndConnectivity = NULL;
        double * coordinates;
        matrixArray = DSCaseNDVertexEnumeration(aCase, lowerBounds, upperBounds);
        if (matrixArray == NULL) {
                goto exit;
        }
        if (DSMatrixArrayNumberOfMatrices(matrixArray) < 2) {
                goto exit;
        }
        vertexMatrix = DSMatrixArrayMatrix(matrixArray, 0);
        cobasisMatrix = DSMatrixArrayMatrix(matrixArray, 1);
        if (vertexMatrix == NULL) {
                goto exit;
        }
        if (cobasisMatrix == NULL) {
                goto exit;
        }
        vertices = DSVerticesAlloc(DSVariablePoolNumberOfVariables(lowerBounds));
        coordinates = DSSecureCalloc(sizeof(double), DSVariablePoolNumberOfVariables(lowerBounds));
        for (i = 0; i < DSMatrixRows(vertexMatrix); i++) {
                for (j = 0; j < DSVariablePoolNumberOfVariables(lowerBounds); j++) {
                        coordinates[j] = DSMatrixDoubleValue(vertexMatrix, i, j);
                }
                DSVerticesAddVertex(vertices, coordinates);
        }
        vertexAndConnectivity = DSMatrixArrayAlloc();
        DSMatrixArrayAddMatrix(vertexAndConnectivity, DSMatrixCopy(vertexMatrix));
        DSMatrixArrayAddMatrix(vertexAndConnectivity, DSVerticesConnectivityMatrix(vertices, aCase, lowerBounds, upperBounds));
        DSSecureFree(coordinates);
        DSMatrixArrayFree(matrixArray);
        DSVerticesFree(vertices);
exit:
        return vertexAndConnectivity;
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

static gma_parseraux_t * dsCaseParseStringToTermList(const char * string)
{
        void *parser = NULL;
        struct expression_token *tokens, *current;
        gma_parseraux_t *root = NULL, *parser_aux;
        if (string == NULL) {
                DSError(M_DS_WRONG ": String to parse is NULL", A_DS_ERROR);
                goto bail;
        }
        if (strlen(string) == 0) {
                DSError(M_DS_WRONG ": String to parse is empty", A_DS_WARN);
                goto bail;
        }
        tokens = DSExpressionTokenizeString(string);
        if (tokens == NULL) {
                DSError(M_DS_PARSE ": Token stream is NULL", A_DS_ERROR);
                goto bail;
        }
        parser = DSCaseOptimizationFunctionParserAlloc(DSSecureMalloc);//DSGMASystemParserAlloc(DSSecureMalloc);
        root = DSGMAParserAuxAlloc();
        parser_aux = root;
        current = tokens;
        while (current != NULL) {
                if (DSExpressionTokenType(current) == DS_EXPRESSION_TOKEN_START) {
                        current = DSExpressionTokenNext(current);
                        continue;
                }
                DSCaseOptimizationFunctionParser(parser,
                                              DSExpressionTokenType(current),
                                              current,
                                              ((void**)&parser_aux));
                current = DSExpressionTokenNext(current);
        }
        DSCaseOptimizationFunctionParser(parser,
                                      0,
                                      NULL,
                                      ((void **)&parser_aux));
        DSCaseOptimizationFunctionParserFree(parser, DSSecureFree);
        DSExpressionTokenFree(tokens);
        if (DSGMAParserAuxParsingFailed(root) == true) {
                DSGMAParserAuxFree(root);
                root = NULL;
        }
bail:
        return root;
}

//extern void * DSDesignSpaceTermListForAllStrings(char * const * const strings, const DSUInteger numberOfEquations)
//{
//        DSUInteger i;
//        gma_parseraux_t **aux = NULL;
//        DSExpression *expr;
//        char *aString;
//        bool failed = false;
//        aux = DSSecureCalloc(sizeof(gma_parseraux_t *), numberOfEquations);
//        for (i = 0; i < numberOfEquations; i++) {
//                if (strings[i] == NULL) {
//                        DSError(M_DS_WRONG ": String to parse is NULL", A_DS_ERROR);
//                        failed = true;
//                        break;
//                }
//                if (strlen(strings[i]) == 0) {
//                        DSError(M_DS_WRONG ": String to parse is empty", A_DS_ERROR);
//                        failed = true;
//                        break;
//                }
//                expr = DSExpressionByParsingString(strings[i]);
//                if (expr != NULL) {
//                        aString = DSExpressionAsString(expr);
//                        aux[i] = dsDesignSpaceParseStringToTermList(aString);
//                        DSSecureFree(aString);
//                        DSExpressionFree(expr);
//                }
//                if (aux[i] == NULL) {
//                        DSError(M_DS_PARSE ": Expression not in GMA format", A_DS_ERROR);
//                        failed = true;
//                        break;
//                }
//        }
//        if (failed == true) {
//                for (i = 0; i < numberOfEquations; i++)
//                        if (aux[i] != NULL)
//                                DSGMAParserAuxFree(aux[i]);
//                DSSecureFree(aux);
//                aux = NULL;
//        }
//bail:
//        return aux;
//}
//

static void dsCaseOptimizationFunctionProcessExponentBasePairs(const DSCase *aCase, gma_parseraux_t *aux,
                                                               DSMatrix * Od, DSMatrix * Oi, DSMatrix *delta)
{
        DSUInteger j, varIndex;
        const char *varName;
        double currentValue;
        if (aux == NULL) {
                goto bail;
        }
        for (j = 0; j < DSGMAParserAuxNumberOfBases(aux); j++) {
                if (DSGMAParserAuxBaseAtIndexIsVariable(aux, j) == false) {
                        currentValue = DSMatrixDoubleValue(delta, index, 0);
                        currentValue += log10(DSGMAParseAuxsConstantBaseAtIndex(aux, j));
                        DSMatrixSetDoubleValue(delta,
                                               index, 0,
                                               currentValue);
                        continue;
                }
                varName = DSGMAParserAuxVariableAtIndex(aux, j);
                if (DSVariablePoolHasVariableWithName(DSCaseXd(aCase), varName) == true) {
                        varIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXd(aCase), varName);
                        currentValue = DSMatrixDoubleValue(Od, 0, varIndex);
                        currentValue += DSGMAParserAuxExponentAtIndex(aux, j);
                        DSMatrixSetDoubleValue(Od, 0, varIndex, currentValue);
                } else if (DSVariablePoolHasVariableWithName(DSCaseXi(aCase), varName) == true) {
                        varIndex = DSVariablePoolIndexOfVariableWithName(DSCaseXi(aCase), varName);
                        currentValue = DSMatrixDoubleValue(Oi, 0, varIndex);
                        currentValue += DSGMAParserAuxExponentAtIndex(aux, j);
                        DSMatrixSetDoubleValue(Oi, 0, varIndex, currentValue);
                }
        }
bail:
        return;
}

static DSMatrixArray * dsCaseOptimiztionFunctionCreateMatrix(const DSCase *aCase, gma_parseraux_t *aux, bool hasXd)
{
        DSMatrixArray * optimizationMatrices = NULL;
        DSMatrix * Od, *Oi, *delta;
        const DSSSystem * ssystem;
        const DSVariablePool * Xd, *Xi;
        DSMatrix * Ai, * B, * MAi, * Mb;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (aux == NULL) {
                DSError(M_DS_NULL ": Parser auxiliary data is NULL", A_DS_ERROR);
                goto bail;
        }
        Xd = DSCaseXd(aCase);
        Xi = DSCaseXi(aCase);
        if (Xd == NULL || Xi == NULL) {
                DSError(M_DS_WRONG ": Need Xi and Xd", A_DS_ERROR);
                goto bail;
        }
        ssystem = DSCaseSSystem(aCase);
        Od = DSMatrixCalloc(1, DSVariablePoolNumberOfVariables(Xd));
        Oi = DSMatrixCalloc(1, DSVariablePoolNumberOfVariables(Xi));
        delta = DSMatrixCalloc(1, 1);
        dsCaseOptimizationFunctionProcessExponentBasePairs(aCase, aux, Od, Oi, delta);
        if (hasXd == true) {
                if (DSSSystemHasSolution(ssystem) == false) {
                        DSMatrixFree(Od);
                        DSMatrixFree(Oi);
                        DSMatrixFree(delta);
                        goto bail;
                }
                Ai = DSSSystemAi(ssystem);
                B = DSSSystemB(ssystem);
                MAi = DSMatrixByMultiplyingMatrix(DSSSystemM(ssystem), Ai);
                Mb = DSMatrixByMultiplyingMatrix(DSSSystemM(ssystem), B);
                DSMatrixFree(Ai);
                DSMatrixFree(B);
                Ai = MAi;
                MAi = DSMatrixByMultiplyingMatrix(Od, MAi);
                DSMatrixFree(Ai);
                B = Mb;
                Mb = DSMatrixByMultiplyingMatrix(Od, Mb);
                DSMatrixFree(B);
                DSMatrixSubstractByMatrix(Oi, MAi);
                DSMatrixAddByMatrix(delta, Mb);
                DSMatrixFree(MAi);
                DSMatrixFree(Mb);
        }
        DSMatrixFree(Od);
        optimizationMatrices = DSMatrixArrayAlloc();
        DSMatrixArrayAddMatrix(optimizationMatrices, Oi);
        DSMatrixArrayAddMatrix(optimizationMatrices, delta);
bail:
        return optimizationMatrices;
}

extern DSMatrixArray * DSCaseParseOptimizationFunction(const DSCase * aCase, const char * string)
{
        DSMatrixArray * O = NULL;
        DSVariablePool * eqVars = NULL;
        DSExpression * expr;
        bool hasXd = false;
        DSUInteger i;
        char * name;
        if (aCase == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        gma_parseraux_t *aux = NULL;
        aux = dsCaseParseStringToTermList(string);
        if (aux == NULL) {
                goto bail;
        }
        expr = DSExpressionByParsingString(string);
        eqVars = DSExpressionVariablesInExpression(expr);
        for (i = 0; i < DSVariablePoolNumberOfVariables(eqVars); i++) {
                name = DSVariableName(DSVariablePoolVariableAtIndex(eqVars, i));
                if (DSVariablePoolHasVariableWithName(DSCaseXd(aCase), name)) {
                        hasXd = true;
                        break;
                }
        }
        DSExpressionFree(expr);
        DSVariablePoolFree(eqVars);
        O = dsCaseOptimiztionFunctionCreateMatrix(aCase, aux, hasXd);
        DSGMAParserAuxFree(aux);

bail:
        return O;
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

///**
// *
// */
//static DSPseudoCase * dsPseudoCaseFromConcurrentCasisInSlice(const DSUInteger numberOfCases, const DSCase ** cases, )
//{
//        DSUInteger i;
//        DSPseudoCase * caseIntersection = NULL;
//        DSMatrix *U = NULL, *Zeta = NULL, *temp;
//        if (numberOfCases == 0) {
//                DSError(M_DS_WRONG ": Number of cases must be at least one", A_DS_ERROR);
//                goto bail;
//        }
//        if (cases == NULL) {
//                DSError(M_DS_NULL ": Array of cases is NULL", A_DS_ERROR);
//                goto bail;
//        }
//        for (i = 0; i < numberOfCases; i++) {
//                if (DSCaseHasSolution(cases[i]) == false)
//                        goto bail;
//        }
//        U = DSMatrixCopy(DSCaseU(cases[0]));
//        Zeta = DSMatrixCopy(DSCaseZeta(cases[0]));
//        for (i = 1; i < numberOfCases; i++) {
//                temp = DSMatrixAppendMatrices(U, DSCaseU(cases[i]), false);
//                DSMatrixFree(U);
//                U = temp;
//                temp = DSMatrixAppendMatrices(Zeta, DSCaseZeta(cases[i]), false);
//                DSMatrixFree(Zeta);
//                Zeta = temp;
//                if (U == NULL || Zeta == NULL)
//                        goto bail;
//        }
//        caseIntersection = DSSecureCalloc(1, sizeof(DSCase));
//        DSCaseXd(caseIntersection) = DSCaseXd(cases[0]);
//        DSCaseXi(caseIntersection) = DSCaseXi(cases[0]);
//        DSCaseU(caseIntersection) = U;
//        DSCaseZeta(caseIntersection) = Zeta;
//        U = NULL;
//        Zeta = NULL;
//bail:
//        if (U != NULL)
//                DSMatrixFree(U);
//        if (Zeta != NULL)
//                DSMatrixFree(Zeta);
//        return caseIntersection;
//}

/**
 * 
 */
extern DSPseudoCase * DSPseudoCaseFromIntersectionOfCases(const DSUInteger numberOfCases, const DSCase ** cases)
{
        DSUInteger i;
        DSPseudoCase * caseIntersection = NULL;
        DSMatrix *Cd = NULL, *Ci = NULL, *delta = NULL, *U = NULL, *Zeta = NULL, *temp;
        char caseIdentifier[1000];
        if (numberOfCases == 0) {
                DSError(M_DS_WRONG ": Number of cases must be at least one", A_DS_ERROR);
                goto bail;
        }
        if (cases == NULL) {
                DSError(M_DS_NULL ": Array of cases is NULL", A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < numberOfCases; i++) {
                if (cases[i] == NULL) {
                        DSError(M_DS_CASE_NULL, A_DS_ERROR);
                        goto bail;
                }
                if (DSCaseHasSolution(cases[i]) == false)
                        goto bail;
        }
        U = DSMatrixCopy(DSCaseU(cases[0]));
        Zeta = DSMatrixCopy(DSCaseZeta(cases[0]));
        Ci = DSMatrixCopy(DSCaseCi(cases[0]));
        Cd = DSMatrixCopy(DSCaseCd(cases[0]));
        delta = DSMatrixCopy(DSCaseDelta(cases[0]));
        sprintf(caseIdentifier, "%s", DSCaseIdentifier(cases[0]));
        for (i = 1; i < numberOfCases; i++) {
                sprintf(caseIdentifier, "%s, %s", caseIdentifier, DSCaseIdentifier(cases[i]));
                temp = DSMatrixAppendMatrices(U, DSCaseU(cases[i]), false);
                DSMatrixFree(U);
                U = temp;
                temp = DSMatrixAppendMatrices(Zeta, DSCaseZeta(cases[i]), false);
                DSMatrixFree(Zeta);
                Zeta = temp;
                temp = DSMatrixAppendMatrices(Cd, DSCaseCd(cases[i]), false);
                DSMatrixFree(Cd);
                Cd = temp;
                temp = DSMatrixAppendMatrices(Ci, DSCaseCi(cases[i]), false);
                DSMatrixFree(Ci);
                Ci = temp;
                temp = DSMatrixAppendMatrices(delta, DSCaseDelta(cases[i]), false);
                DSMatrixFree(delta);
                delta = temp;
                if (U == NULL || Zeta == NULL || Cd == NULL || Ci == NULL || delta == NULL)
                        goto bail;
        }
        caseIntersection = DSSecureCalloc(1, sizeof(DSCase));
        caseIntersection->Xd = DSCaseXd(cases[0]);
        caseIntersection->Xd_a = DSCaseXd_a(cases[0]);
        caseIntersection->Xi = DSCaseXi(cases[0]);
        DSCaseU(caseIntersection) = U;
        DSCaseZeta(caseIntersection) = Zeta;
        DSCaseCi(caseIntersection) = Ci;
        DSCaseCd(caseIntersection) = Cd;
        DSCaseDelta(caseIntersection) = delta;
        DSCaseId(caseIntersection) = strdup(caseIdentifier);
        U = NULL;
        Zeta = NULL;
        Cd = NULL;
        Ci = NULL;
        delta = NULL;
        caseIntersection->ssys = NULL;
bail:
        if (U != NULL)
                DSMatrixFree(U);
        if (Zeta != NULL)
                DSMatrixFree(Zeta);
        if (Ci != NULL)
                DSMatrixFree(Ci);
        if (Cd != NULL)
                DSMatrixFree(Cd);
        if (delta != NULL)
                DSMatrixFree(delta);
        return caseIntersection;
}

/**
 *
 */
extern DSPseudoCase * DSPseudoCaseFromIntersectionOfCasesExcludingSlice(const DSUInteger numberOfCases, const DSCase ** cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames)
{
        DSUInteger i, j, k, currentRow, numberOfExtraColumns, rows, columns, *indices;
        DSPseudoCase * caseIntersection = NULL;
        DSMatrix * Cd = NULL, *Ci = NULL, *delta = NULL;
        DSMatrix *U = NULL, *Zeta = NULL, *tempU, *tempZeta;
        DSVariablePool * Xi;
        char * name = NULL;
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
        Xi = DSVariablePoolAlloc();
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
        k = 0;
        for (i = 0; i < DSVariablePoolNumberOfVariables(DSCaseXi(cases[0])); i++) {
                for (j = 0; j < numberOfExceptions; j++) {
                        if (i == indices[j])
                                break;
                }
                if (j == numberOfExceptions) {
                        DSVariablePoolAddVariableWithName(Xi, DSVariablePoolAllVariableNames(DSCaseXi(cases[0]))[i]);
                } else {
                        asprintf(&name, "$%s_0", DSVariablePoolAllVariableNames(DSCaseXi(cases[0]))[i]);
                        DSVariablePoolAddVariableWithName(Xi, name);
                }
        }
        for (i = 0; i < numberOfExceptions*(numberOfCases-1); i++) {
                j = i % numberOfExceptions;
                asprintf(&name, "$%s_%i", DSVariablePoolAllVariableNames(DSCaseXi(cases[0]))[indices[j]], i / numberOfExceptions + 1);
                DSVariablePoolAddVariableWithName(Xi, name);
        }
        DSSecureFree(name);
        numberOfExtraColumns = numberOfExceptions*(numberOfCases-1);
        rows = 0;
        columns = DSMatrixColumns(DSCaseU(cases[0]))+numberOfExtraColumns;
        for (i = 0; i < numberOfCases; i++) {
                rows += DSMatrixRows(DSCaseZeta(cases[i]));
        }
        Cd = DSMatrixCalloc(rows, DSMatrixColumns(DSCaseCd(cases[0])));
        Ci = DSMatrixCalloc(rows, columns);
        delta = DSMatrixCalloc(rows, 1);
        U = DSMatrixCalloc(rows, columns);
        Zeta = DSMatrixCalloc(rows, 1);
        currentRow = 0;
        for (i = 0; i < numberOfCases; i++) {
                tempU = DSCaseU(cases[i]);
                tempZeta = DSCaseZeta(cases[i]);
                for (j = 0; j < DSMatrixRows(tempZeta); j++) {
                        DSMatrixSetDoubleValue(Zeta, currentRow, 0, DSMatrixDoubleValue(tempZeta, j, 0));
                        DSMatrixSetDoubleValue(delta, currentRow, 0, DSMatrixDoubleValue(DSCaseDelta(cases[i]), j, 0));
                        for (k = 0; k < DSMatrixColumns(tempU); k++) {
                                DSMatrixSetDoubleValue(Ci, currentRow, k, DSMatrixDoubleValue(DSCaseCi(cases[i]), j, k));
                                DSMatrixSetDoubleValue(U, currentRow, k, DSMatrixDoubleValue(tempU, j, k));
                        }
                        for (k = 0; k < DSMatrixColumns(DSCaseCd(cases[i])); k++) {
                                DSMatrixSetDoubleValue(Cd, currentRow, k, DSMatrixDoubleValue(DSCaseCd(cases[i]), j, k));
                        }
                        if (i > 0) {
                                for (k = 0; k < numberOfExceptions; k++) {
                                        DSMatrixSetDoubleValue(U,
                                                               currentRow,
                                                               DSMatrixColumns(tempU)+numberOfExceptions*(i-1)+k,
                                                               DSMatrixDoubleValue(U, currentRow, indices[k]));
                                        DSMatrixSetDoubleValue(U, currentRow, indices[k], 0.0f);
                                        DSMatrixSetDoubleValue(Ci,
                                                               currentRow,
                                                               DSMatrixColumns(DSCaseCi(cases[i]))+numberOfExceptions*(i-1)+k,
                                                               DSMatrixDoubleValue(Ci, currentRow, indices[k]));
                                        DSMatrixSetDoubleValue(Ci, currentRow, indices[k], 0.0f);
                                }
                        }
                        currentRow++;
                }
        }
        caseIntersection = DSSecureCalloc(1, sizeof(DSCase));
        caseIntersection->Xd_a = DSCaseXd_a(cases[0]);
        caseIntersection->Xd = DSCaseXd(cases[0]);
        caseIntersection->Xi = Xi;
        DSCaseU(caseIntersection) = U;
        DSCaseZeta(caseIntersection) = Zeta;
        DSCaseSSys(caseIntersection) = DSSecureCalloc(1, sizeof(DSSSystem));
        DSCaseSSys(caseIntersection)->shouldFreeXi = true;
        DSCaseCi(caseIntersection) = Ci;
        DSCaseCd(caseIntersection) = Cd;
        DSCaseDelta(caseIntersection) = delta;
        caseIntersection->ssys = NULL;
        U = NULL;
        Zeta = NULL;
        Ci = NULL;
        Cd = NULL;
        delta = NULL;
        DSSecureFree(indices);
bail:
        if (Cd != NULL)
                DSMatrixFree(Cd);
        if (Ci != NULL)
                DSMatrixFree(Ci);
        if (delta != NULL)
                DSMatrixFree(delta);
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
        caseIntersection = DSPseudoCaseFromIntersectionOfCases(numberOfCases, cases);
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
        caseIntersection = DSPseudoCaseFromIntersectionOfCases(numberOfCases, cases);
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
        caseIntersection = DSPseudoCaseFromIntersectionOfCasesExcludingSlice(numberOfCases, cases, numberOfExceptions, exceptionVarNames);
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
        caseIntersection = DSPseudoCaseFromIntersectionOfCasesExcludingSlice(numberOfCases, cases, numberOfExceptions, exceptionVarNames);
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
        caseIntersection = DSPseudoCaseFromIntersectionOfCasesExcludingSlice(numberOfCases, cases, numberOfExceptions, exceptionVarNames);
        if (caseIntersection == NULL)
                goto bail;
        variablePool = DSCaseValidParameterSet(caseIntersection);
        DSSecureFree(caseIntersection);
bail:
        return variablePool;
}

extern DSVariablePool * DSCaseIntersectionExceptSliceValidParameterSetByOptimizingFunction(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames, const char * function, bool minimize)
{
        DSPseudoCase *caseIntersection = NULL;
        DSVariablePool * variablePool = NULL;
        caseIntersection = DSPseudoCaseFromIntersectionOfCasesExcludingSlice(numberOfCases, cases, numberOfExceptions, exceptionVarNames);
        if (caseIntersection == NULL)
                goto bail;
        variablePool = DSCaseValidParameterSetByOptimizingFunction(caseIntersection, function, minimize);
        DSSecureFree(caseIntersection);
bail:
        return variablePool;
}

extern DSVariablePool * DSCaseIntersectionExceptSliceValidParameterSetWithConstraints(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames, const char ** constraints, DSUInteger numberOfConstraints)
{
        DSPseudoCase *caseIntersection = NULL;
        DSVariablePool * variablePool = NULL;
        caseIntersection = DSPseudoCaseFromIntersectionOfCasesExcludingSlice(numberOfCases, cases, numberOfExceptions, exceptionVarNames);
        DSCaseAddConstraints(caseIntersection, constraints, numberOfConstraints);
        if (caseIntersection == NULL)
                goto bail;
        variablePool = DSCaseValidParameterSet(caseIntersection);
        DSSecureFree(caseIntersection);
bail:
        return variablePool;
}

extern DSVariablePool * DSCaseIntersectionExceptSliceValidParameterSetWithConstraintsByOptimizingFunction(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames, const char ** constraints, DSUInteger numberOfConstraints, const char * function, bool minimize)
{
        DSPseudoCase *caseIntersection = NULL;
        DSVariablePool * variablePool = NULL;
        caseIntersection = DSPseudoCaseFromIntersectionOfCasesExcludingSlice(numberOfCases, cases, numberOfExceptions, exceptionVarNames);
        DSCaseAddConstraints(caseIntersection, constraints, numberOfConstraints);
        if (caseIntersection == NULL)
                goto bail;
        variablePool = DSCaseValidParameterSetByOptimizingFunction(caseIntersection, function, minimize);
        DSSecureFree(caseIntersection);
bail:
        return variablePool;
}

extern DSVariablePool * DSCaseIntersectionExceptSliceValidParameterSetAtSlice(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames, const DSVariablePool * lowerBounds, const DSVariablePool * upperBounds)
{
        DSPseudoCase *caseIntersection = NULL;
        DSVariablePool * variablePool = NULL;
        caseIntersection = DSPseudoCaseFromIntersectionOfCasesExcludingSlice(numberOfCases, cases, numberOfExceptions, exceptionVarNames);
        if (caseIntersection == NULL)
                goto bail;
        variablePool = DSCaseValidParameterSetAtSlice(caseIntersection, lowerBounds, upperBounds);
        DSSecureFree(caseIntersection);
bail:
        return variablePool;
}

extern DSVariablePool * DSCaseIntersectionExceptSliceValidParameterSetAtSliceByOptimizingFunction(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames, const DSVariablePool * lowerBounds, const DSVariablePool * upperBounds, const char * function, bool minimize)
{
        DSPseudoCase *caseIntersection = NULL;
        DSVariablePool * variablePool = NULL;
        caseIntersection = DSPseudoCaseFromIntersectionOfCasesExcludingSlice(numberOfCases, cases, numberOfExceptions, exceptionVarNames);
        if (caseIntersection == NULL)
                goto bail;
        variablePool = DSCaseValidParameterSetAtSliceByOptimizingFunction(caseIntersection, lowerBounds, upperBounds, function, minimize);
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
        caseIntersection = DSPseudoCaseFromIntersectionOfCases(numberOfCases, cases);
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

extern DSMatrixArray * DSCaseIntersectionFacesFor3DSliceAndConnectivity(const DSUInteger numberOfCases, const DSCase **cases, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable, const char *zVariable)
{
        DSMatrixArray * faces = NULL;
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
        caseIntersection = DSPseudoCaseFromIntersectionOfCases(numberOfCases, cases);
        if (cases == NULL)
                goto bail;
        faces = DSCaseFacesFor3DSliceAndConnectivity(caseIntersection, lowerBounds, upperBounds, xVariable, yVariable, zVariable);
        DSSecureFree(caseIntersection);
bail:
        return faces;
}

