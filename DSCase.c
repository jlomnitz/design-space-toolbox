/**
 * \file DSCase.c
 * \brief Implementation file with functions for dealing with cases in design space.
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
#include "DSCyclicalCase.h"

#define DSCaseXi(x)                  ((x)->Xi)
#define DSCaseXd(x)                  ((x)->Xd)
#define DSCaseXd_a(x)                ((x)->Xd_a)
#define DSCaseSSys(x)                ((x)->ssys)
#define DSCaseCd(x)                  ((x)->Cd)
#define DSCaseCi(x)                  ((x)->Ci)
#define DSCaseU(x)                   ((x)->U)
#define DSCaseDelta(x)               ((x)->delta)
#define DSCaseZeta(x)                ((x)->zeta)
#define DSCaseSig(x)                 ((x)->signature)
#define DSCaseNum(x)                 ((x)->caseNumber)

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - DSCase Global behavior
#endif

/* A + B > 0 => B > -A:S if S > 0*/

static char endian;

extern void DSCaseSetEndianness(char endianness)
{
        if (endianness != DS_CASE_NUMBER_BIG_ENDIAN && endianness != DS_CASE_NUMBER_SMALL_ENDIAN) {
                DSError(M_DS_WRONG ": Endianness must be big or small", A_DS_ERROR);
                goto bail;
        }
        endian = endianness;
bail:
        return;
}

extern char DSCaseEndianness(void)
{
        return endian;
}
#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, deallocation and initialization
#endif

static DSCase * DSCaseAlloc(void)
{
        DSCase * aCase = NULL;
        aCase = DSSecureCalloc(sizeof(DSCase), 1);
        return aCase;
}

extern DSCase * DSCaseCopy(const DSCase * aCase)
{
        DSCase * newCase = NULL;
        DSUInteger i, numberOfEquations;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        newCase = DSCaseAlloc();
        newCase->caseNumber = aCase->caseNumber;
        numberOfEquations = DSCaseNumberOfEquations(aCase);
        if (DSCaseSig(aCase) != NULL) {
                DSCaseSig(newCase) = DSSecureCalloc(sizeof(DSUInteger), numberOfEquations*2);
                for (i = 0; i < numberOfEquations; i++) {
                        DSCaseSig(newCase)[i] = DSCaseSig(aCase)[i];
                }
        }
        DSCaseSSys(newCase) = DSSSystemCopy(DSCaseSSystem(aCase));
        if (DSCaseCd(aCase) != NULL)
                DSCaseCd(newCase) = DSMatrixCopy(DSCaseCd(aCase));
        if (DSCaseCi(aCase) != NULL)
                DSCaseCi(newCase) = DSMatrixCopy(DSCaseCi(aCase));
        if (DSCaseZeta(aCase) != NULL)
                DSCaseZeta(newCase) = DSMatrixCopy(DSCaseZeta(aCase));
        if (DSCaseDelta(aCase) != NULL)
                DSCaseDelta(newCase) = DSMatrixCopy(DSCaseDelta(aCase));
        if (DSCaseU(aCase) != NULL)
                DSCaseU(newCase) = DSMatrixCopy(DSCaseU(aCase));
        DSCaseXd(newCase) = DSSSystemXd(DSCaseSSys(newCase));
        DSCaseXi(newCase) = DSSSystemXi(DSCaseSSys(newCase));
        DSCaseXd_a(newCase) =DSSSystemXd_a(DSCaseSSys(newCase));
bail:
        return newCase;
}

extern void DSCaseFree(DSCase * aCase)
{
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseSSys(aCase) != NULL)
                DSSSystemFree(DSCaseSSys(aCase));
        if (DSCaseSig(aCase) != NULL)
                DSSecureFree(DSCaseSig(aCase));
        if (DSCaseCd(aCase) != NULL)
                DSMatrixFree(DSCaseCd(aCase));
        if (DSCaseCi(aCase) != NULL)
                DSMatrixFree(DSCaseCi(aCase));
        if (DSCaseZeta(aCase) != NULL)
                DSMatrixFree(DSCaseZeta(aCase));
        if (DSCaseDelta(aCase) != NULL)
                DSMatrixFree(DSCaseDelta(aCase));
        if (DSCaseU(aCase) != NULL)
                DSMatrixFree(DSCaseU(aCase));
        DSSecureFree(aCase);
bail:
        return;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Factory functions
#endif

//static void dsCaseRemoveRedundantBoundaries(DSCase *aCase)
//{
//        DSMatrix *temp1, *temp2;
//        temp1 = DSMatrixAppendMatrices(DSCaseU(aCase), DSCaseZeta(aCase), true);
//        temp2 = DSMatrixWithUniqueRows(temp1);
//        DSMatrixFree(temp1);
//
//        if (temp2 == NULL)
//                goto bail;
//        DSMatrixFree(DSCaseU(aCase));
//        DSMatrixFree(DSCaseZeta(aCase));
//        DSCaseU(aCase) = DSMatrixSubMatrixExcludingColumnList(temp2, 1, DSMatrixColumns(temp2)-1);
//        DSCaseZeta(aCase) = DSMatrixSubMatrixIncludingColumnList(temp2, 1, DSMatrixColumns(temp2)-1);
//        
//        DSMatrixFree(temp2);
//bail:
//        return;
//}

static void dsCaseCreateBoundaryMatrices(DSCase *aCase)
{
        DSUInteger numberOfXi = 0;
        DSMatrix * W = NULL, *B, *Ai;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSSSystemHasSolution(DSCaseSSys(aCase)) == false) {
                goto bail;
        }
        B = DSSSystemB(DSCaseSSys(aCase));
        numberOfXi =DSVariablePoolNumberOfVariables(DSCaseXi(aCase));
        W = DSMatrixByMultiplyingMatrix(DSCaseCd(aCase), DSSSystemM(DSCaseSSys(aCase)));
        DSCaseZeta(aCase) = DSMatrixByMultiplyingMatrix(W, B);
        DSMatrixAddByMatrix(DSCaseZeta(aCase), DSCaseDelta(aCase));
        if (numberOfXi != 0) {
                Ai = DSSSystemAi(DSCaseSSys(aCase));
                DSCaseU(aCase) = DSMatrixByMultiplyingMatrix(W, Ai);
                if (DSCaseCi(aCase) != NULL)
                        DSMatrixSubstractByMatrix(DSCaseU(aCase), DSCaseCi(aCase));
                DSMatrixMultiplyByScalar(DSCaseU(aCase), -1.0);
//                dsCaseRemoveRedundantBoundaries(aCase);
                DSMatrixFree(Ai);
        }
        DSMatrixFree(W);
        DSMatrixFree(B);
bail:
        return;
}

static void dsCaseCreateConditionMatrices(DSCase *aCase, const DSGMASystem * gma)
{
        DSUInteger i, j, k, l, numberOfConditions, numberOfEquations;
        DSUInteger numberOfXi, numberOfXd;
        const DSUInteger *termArray;
        double value;
        const DSMatrix * (*a)(const DSGMASystem * gma);
        const DSMatrixArray * (*kd)(const DSGMASystem * gma);
        const DSMatrixArray * (*ki)(const DSGMASystem * gma);
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (gma == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfEquations = DSVariablePoolNumberOfVariables(DSCaseXd(aCase));
        numberOfXd = numberOfEquations;
        numberOfXi = DSVariablePoolNumberOfVariables(DSCaseXi(aCase));
        numberOfConditions = 0;
        termArray = DSCaseSig(aCase);
        for (i = 0; i < 2*numberOfEquations; i++)
                numberOfConditions += DSGMASystemSignature(gma)[i]-1;
        if (numberOfConditions == 0) {
                goto bail;
        }
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
bail:
        return;
}

static void dsCaseCalculateCaseNumber(DSCase * aCase, const DSGMASystem * gma, const char endianness)
{
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (gma == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        DSCaseNum(aCase) = DSCaseNumberForSignature(DSCaseSig(aCase), gma);
bail:
        return;
}

extern DSCase * DSCaseWithTermsFromGMA(const DSGMASystem * gma, const DSUInteger * termArray)
{
        DSCase *aCase = NULL;
        DSUInteger i, term1, term2, numberOfEquations, numberOfXi;
        if (gma == NULL) {
                DSError(M_DS_GMA_NULL ": Template GMA to make S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        if (termArray == NULL) {
                DSError(M_DS_NULL ": Array of dominant terms is NULL", A_DS_ERROR);
                goto bail;
        }
        aCase = DSCaseAlloc();
        DSCaseSSys(aCase) = DSSSystemWithTermsFromGMA(gma, termArray);
        DSCaseXi(aCase) = DSSSystemXi(DSCaseSSys(aCase));
        DSCaseXd(aCase) = DSSSystemXd(DSCaseSSys(aCase));
        numberOfEquations = DSGMASystemNumberOfEquations(gma);
        DSCaseSig(aCase) = DSSecureMalloc(sizeof(DSUInteger)*(2*numberOfEquations));
        for (i = 0; i < 2*numberOfEquations; i+=2) {
                term1 = termArray[i];
                term2 = termArray[i+1];
                DSCaseSig(aCase)[i] = term1;
                DSCaseSig(aCase)[i+1] = term2;
                if (term1 > DSGMASystemSignature(gma)[i] || term2 > DSGMASystemSignature(gma)[i+1])
                        break;
                if (term1 <= 0 || term2 <= 0)
                        break;
        }
        if (i == 2*numberOfEquations) {
                dsCaseCreateConditionMatrices(aCase, gma);
                dsCaseCreateBoundaryMatrices(aCase);
                dsCaseCalculateCaseNumber(aCase, gma, endian);
        } else {
                DSCaseFree(aCase);
                aCase = NULL;
        }
bail:
        return aCase;
}

static void dsCaseAppendDesignSpaceConditions(DSCase * aCase, const DSDesignSpace * ds)
{
        DSMatrix * temp;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (ds->Cd == NULL) 
                goto bail;
        if (DSCaseCd(aCase) == NULL) {
                DSCaseCd(aCase) = DSMatrixCopy(ds->Cd);
                DSCaseDelta(aCase) = DSMatrixCopy(ds->delta);
                if (DSVariablePoolNumberOfVariables(DSCaseXi(aCase)) > 0)
                        DSCaseCi(aCase) = DSMatrixCopy(ds->Ci);
        } else {
                temp = DSMatrixAppendMatrices(DSCaseCd(aCase), ds->Cd, false);
                DSMatrixFree(DSCaseCd(aCase));
                DSCaseCd(aCase) = temp;
                temp = DSMatrixAppendMatrices(DSCaseDelta(aCase), ds->delta, false);
                DSMatrixFree(DSCaseDelta(aCase));
                DSCaseDelta(aCase) = temp;
                if (DSVariablePoolNumberOfVariables(DSCaseXi(aCase)) > 0) {
                        temp = DSMatrixAppendMatrices(DSCaseCi(aCase), ds->Ci, false);
                        DSMatrixFree(DSCaseCi(aCase));
                        DSCaseCi(aCase) = temp;
                }

        }
bail:
        return;
}

extern DSCase * DSCaseWithTermsFromDesignSpace(const DSDesignSpace * ds, const DSUInteger * termArray)
{
        DSCase *aCase = NULL;
        DSUInteger i, term1, term2, numberOfEquations, numberOfXi;
        if (ds == NULL) {
                DSError(M_DS_NULL ": Template GMA to make S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        if (termArray == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL ": Array of dominant terms is NULL", A_DS_ERROR);
                goto bail;
        }
        aCase = DSCaseAlloc();
        DSCaseSSys(aCase) = DSSSystemWithTermsFromGMA(DSDesignSpaceGMASystem(ds), termArray);
        DSCaseXi(aCase) = DSSSystemXi(DSCaseSSys(aCase));
        DSCaseXd(aCase) = DSSSystemXd(DSCaseSSys(aCase));
        numberOfEquations = DSGMASystemNumberOfEquations(DSDesignSpaceGMASystem(ds));
        DSCaseSig(aCase) = DSSecureMalloc(sizeof(DSUInteger)*(2*numberOfEquations));
        for (i = 0; i < 2*numberOfEquations; i+=2) {
                term1 = termArray[i];
                term2 = termArray[i+1];
                DSCaseSig(aCase)[i] = term1;
                DSCaseSig(aCase)[i+1] = term2;
                if (term1 > DSGMASystemSignature(DSDesignSpaceGMASystem(ds))[i] || term2 > DSGMASystemSignature(DSDesignSpaceGMASystem(ds))[i+1])
                        break;
                if (term1 <= 0 || term2 <= 0)
                        break;
        }
        if (i == 2*numberOfEquations) {
                dsCaseCreateConditionMatrices(aCase, DSDesignSpaceGMASystem(ds));
                dsCaseAppendDesignSpaceConditions(aCase, ds);
                /* Load extra conditions here */
                dsCaseCreateBoundaryMatrices(aCase);
                dsCaseCalculateCaseNumber(aCase, DSDesignSpaceGMASystem(ds), endian);
//                if (DSSSystemIsSingular(DSCaseSSys(aCase)) == true)
//                        DSCyclicalCaseDesignSpaceForUnderdeterminedCase(aCase, ds);
        } else {
                DSCaseFree(aCase);
                aCase = NULL;
        }
bail:
        return aCase;
}


#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Getter functions
#endif

extern const bool DSCaseHasSolution(const DSCase *aCase)
{
        bool hasSolution = false;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        hasSolution = (DSCaseU(aCase) != NULL);
bail:
        return hasSolution;
}

extern const DSUInteger DSCaseNumberOfEquations(const DSCase *aCase)
{
        DSUInteger numberOfEquations = 0;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfEquations = DSSSystemNumberOfEquations(DSCaseSSys(aCase));
bail:
        return numberOfEquations;
}

extern DSExpression ** DSCaseEquations(const DSCase *aCase)
{
        DSExpression **equations = NULL;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseSSys(aCase) == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        equations = DSSSystemEquations(DSCaseSSys(aCase));
bail:
        return equations;
}

extern DSExpression ** DSCaseSolution(const DSCase *aCase)
{
        DSExpression **solution = NULL;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseSSys(aCase) == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        solution = DSSSystemSolution(DSCaseSSys(aCase));
bail:
        return solution;
}

extern DSExpression ** DSCaseLogarithmicSolution(const DSCase * aCase)
{
        DSExpression **solution = NULL;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseSSys(aCase) == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        solution = DSSSystemLogarithmicSolution(DSCaseSSys(aCase));
bail:
        return solution;
}

extern const DSUInteger DSCaseNumberOfConditions(const DSCase *aCase)
{
        DSUInteger numberOfConditions = 0;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseCd(aCase) == NULL) {
                goto bail;
        }
        numberOfConditions = DSMatrixRows(DSCaseCd(aCase));
bail:
        return numberOfConditions;
}

extern const DSUInteger DSCaseNumberOfBoundaries(const DSCase *aCase)
{
        DSUInteger numberOfConditions = 0;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseU(aCase) == NULL) {
                goto bail;
        }
        numberOfConditions = DSMatrixRows(DSCaseU(aCase));
bail:
        return numberOfConditions;
}

static void dsCaseConditionToString(const DSCase *aCase, 
                                    const DSUInteger condition, 
                                    char ** string, 
                                    DSUInteger *length, const bool inLog)
{
        DSUInteger i, numberOfXd, numberOfXi;
        char tempString[100];
        const char *name;
        double value;
        if (aCase == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfXd = DSVariablePoolNumberOfVariables(DSCaseXd(aCase));
        if (condition >= DSMatrixRows(DSCaseCd(aCase))) {
                DSError("Equation does not exist: Check number of equations", A_DS_ERROR);
                goto bail;
        }
        if (string == NULL) {
                DSError(M_DS_NULL ": Pointer to string is NULL", A_DS_ERROR);
                goto bail;
        }
        if (length == NULL) {
                DSError(M_DS_NULL ": Pointer to length is NULL", A_DS_ERROR);
                goto bail;
        }
        if (*string == NULL) {
                DSError(M_DS_NULL ": String should be initialized", A_DS_ERROR);
                goto bail;
        }
        numberOfXi = DSVariablePoolNumberOfVariables(DSCaseXi(aCase));
        if (inLog == true) 
                sprintf(tempString, "%lf", DSMatrixDoubleValue(DSCaseDelta(aCase), condition, 0));
        else
                sprintf(tempString, "10^%lf", DSMatrixDoubleValue(DSCaseDelta(aCase), condition, 0));
        if (*length-strlen(*string) < 100) {
                *length += 1000;
                *string = DSSecureRealloc(*string, sizeof(char)**length);
        }
        strncat(*string, tempString, *length-strlen(*string));
        for (i = 0; i < numberOfXi+numberOfXd; i++) {
                if (*length-strlen(*string) < 100) {
                        *length += 1000;
                        *string = DSSecureRealloc(*string, sizeof(char)**length);
                }
                if (i < numberOfXi) {
                        name = DSVariableName(DSVariablePoolAllVariables(DSCaseXi(aCase))[i]);
                        value = DSMatrixDoubleValue(DSCaseCi(aCase), condition, i);
                } else {
                        name = DSVariableName(DSVariablePoolAllVariables(DSCaseXd(aCase))[i-numberOfXi]);
                        value = DSMatrixDoubleValue(DSCaseCd(aCase), condition, i-numberOfXi);
                }
                if (value == 0.0)
                        continue;
                if (inLog == true)
                        sprintf(tempString, "+%lf*log(%s)", value, name);
                else if (value == 1.0)
                        sprintf(tempString, "*%s", name);
                else
                        sprintf(tempString, "*%s^%lf", name, value);
                strncat(*string, tempString, *length-strlen(*string));
        }
bail:
        return;
}

extern DSExpression ** DSCaseConditions(const DSCase *aCase)
{
        DSUInteger i, numberOfConditions, length;
        DSExpression ** conditions = NULL;
        char *tempString;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfConditions = DSMatrixRows(DSCaseCd(aCase));
        if (numberOfConditions == 0) {
                DSError("Case being accessed has no conditions", A_DS_ERROR);
                goto bail;
        }
        conditions = DSSecureCalloc(sizeof(DSExpression *), numberOfConditions);
        length = 1000;
        tempString = DSSecureCalloc(sizeof(char), length);
        for (i = 0; i < numberOfConditions; i++) {
                tempString[0] = '\0';
                dsCaseConditionToString(aCase, i, &tempString, &length, false);
                conditions[i] = DSExpressionByParsingString(tempString);
        }
        DSSecureFree(tempString);
bail:
        return conditions;
}

extern DSExpression ** DSCaseLogarithmicConditions(const DSCase *aCase)
{
        DSUInteger i, numberOfConditions, length;
        DSExpression ** conditions = NULL;
        char *tempString;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseCd(aCase) == NULL) {
                goto bail;
        }
        numberOfConditions = DSMatrixRows(DSCaseCd(aCase));
        if (numberOfConditions == 0) {
                DSError("Case being accessed has no conditions", A_DS_ERROR);
                goto bail;
        }
        conditions = DSSecureCalloc(sizeof(DSExpression *), numberOfConditions);
        length = 1000;
        tempString = DSSecureCalloc(sizeof(char), length);
        for (i = 0; i < numberOfConditions; i++) {
                tempString[0] = '\0';
                dsCaseConditionToString(aCase, i, &tempString, &length, true);
                conditions[i] = DSExpressionByParsingString(tempString);
        }
        DSSecureFree(tempString);
bail:
        return conditions;
}

static void dsCaseBoundaryToString(const DSCase *aCase, 
                                   const DSUInteger boundary, 
                                   char ** string, 
                                   DSUInteger *length, const bool inLog)
{
        DSUInteger i, numberOfXd, numberOfXi;
        char tempString[100];
        const char *name;
        double value;
        if (aCase == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSCaseU(aCase) == NULL) {
                goto bail;
        }
        if (boundary >= DSMatrixRows(DSCaseU(aCase))) {
                DSError("Equation does not exist: Check number of equations", A_DS_ERROR);
                goto bail;
        }
        if (string == NULL) {
                DSError(M_DS_NULL ": Pointer to string is NULL", A_DS_ERROR);
                goto bail;
        }
        if (length == NULL) {
                DSError(M_DS_NULL ": Pointer to length is NULL", A_DS_ERROR);
                goto bail;
        }
        if (*string == NULL) {
                DSError(M_DS_NULL ": String should be initialized", A_DS_ERROR);
                goto bail;
        }
        numberOfXi = DSVariablePoolNumberOfVariables(DSCaseXi(aCase));
        if (inLog == true) 
                sprintf(tempString, "%lf", DSMatrixDoubleValue(DSCaseZeta(aCase), boundary, 0));
        else
                sprintf(tempString, "10^%lf", DSMatrixDoubleValue(DSCaseZeta(aCase), boundary, 0));
        if (*length-strlen(*string) < 100) {
                *length += 1000;
                *string = DSSecureRealloc(*string, sizeof(char)**length);
        }
        strncat(*string, tempString, *length-strlen(*string));
        for (i = 0; i < numberOfXi; i++) {
                if (*length-strlen(*string) < 100) {
                        *length += 1000;
                        *string = DSSecureRealloc(*string, sizeof(char)**length);
                }
                name = DSVariableName(DSVariablePoolAllVariables(DSCaseXi(aCase))[i]);
                value = DSMatrixDoubleValue(DSCaseU(aCase), boundary, i);
                if (value == 0.0)
                        continue;
                if (inLog == true)
                        sprintf(tempString, "+%lf*log(%s)", value, name);
                else if (value == 1.0)
                        sprintf(tempString, "*%s", name);
                else
                        sprintf(tempString, "*%s^%lf", name, value);
                strncat(*string, tempString, *length-strlen(*string));
        }
bail:
        return;
}

extern DSExpression ** DSCaseBoundaries(const DSCase *aCase)
{
        DSUInteger i, numberOfConditions, length;
        DSExpression ** boundaries = NULL;
        char *tempString;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfConditions = DSMatrixRows(DSCaseCd(aCase));
        if (numberOfConditions == 0) {
                DSError("Case being accessed has no conditions", A_DS_ERROR);
                goto bail;
        }
        if (DSSSystemHasSolution(DSCaseSSys(aCase)) == false) {
                goto bail;
        }
        boundaries = DSSecureCalloc(sizeof(DSExpression *), numberOfConditions);
        length = 1000;
        tempString = DSSecureCalloc(sizeof(char), length);
        for (i = 0; i < numberOfConditions; i++) {
                tempString[0] = '\0';
                dsCaseBoundaryToString(aCase, i, &tempString, &length, false);
                boundaries[i] = DSExpressionByParsingString(tempString);
        }
        DSSecureFree(tempString);
bail:
        return boundaries;
}

extern DSExpression ** DSCaseLogarithmicBoundaries(const DSCase *aCase)
{
        DSUInteger i, numberOfConditions, length;
        DSExpression ** boundaries = NULL;
        char *tempString;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfConditions = DSMatrixRows(DSCaseU(aCase));
        if (numberOfConditions == 0) {
                DSError("Case being accessed has no conditions", A_DS_ERROR);
                goto bail;
        }
        boundaries = DSSecureCalloc(sizeof(DSExpression *), numberOfConditions);
        length = 1000;
        tempString = DSSecureCalloc(sizeof(char), length);
        for (i = 0; i < numberOfConditions; i++) {
                tempString[0] = '\0';
                dsCaseBoundaryToString(aCase, i, &tempString, &length, true);
                boundaries[i] = DSExpressionByParsingString(tempString);
        }
        DSSecureFree(tempString);
bail:
        return boundaries;
}

extern DSUInteger DSCaseNumber(const DSCase * aCase)
{
        DSUInteger caseNumber = 0;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        caseNumber = DSCaseNum(aCase);
bail:
        return caseNumber;
}

extern const DSUInteger * DSCaseSignature(const DSCase * aCase)
{
        const DSUInteger *signature = NULL;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        signature = DSCaseSig(aCase);
bail:
        return signature;
}


extern const DSSSystem *DSCaseSSystem(const DSCase * aCase)
{
        DSSSystem * ssys = NULL;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        ssys = DSCaseSSys(aCase);
bail:
        return ssys;
}

extern double DSCaseLogarithmicGain(const DSCase *aCase, const char *XdName, const char *XiName)
{
        double logGain = INFINITY;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        logGain = DSSSystemLogarithmicGain(DSCaseSSys(aCase), XdName, XiName);
bail:
        return logGain;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Case signature and case number
#endif

extern DSUInteger * DSCaseSignatureForCaseNumber(const DSUInteger caseNumber, const DSGMASystem * gma)
{
        DSUInteger *signature = NULL;
        DSUInteger num;
        DSInteger i;
        DSUInteger numberOfEquations;
        if (gma == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (caseNumber == 0) {
                DSError(M_DS_WRONG ": Case number is 0", A_DS_ERROR);
                goto bail;
        }
        if (caseNumber > DSGMASystemNumberOfCases(gma)) {
                DSError(M_DS_WRONG ": Case number is out of bounds", A_DS_ERROR);
                goto bail;
        }
        numberOfEquations = DSGMASystemNumberOfEquations(gma);
        signature = DSSecureMalloc(sizeof(DSUInteger)*2*numberOfEquations);
        num = caseNumber-1;
        switch (endian) {
                case DS_CASE_NUMBER_SMALL_ENDIAN:
                        for (i = 0; i < 2*numberOfEquations; i++) {
                                signature[i] = num % DSGMASystemSignature(gma)[i] +1;
                                num /= DSGMASystemSignature(gma)[i];
                        }
                        break;
                case DS_CASE_NUMBER_BIG_ENDIAN:
                default:
                        for (i = 2*numberOfEquations-1; i >= 0; i--) {
                                signature[i] = num % DSGMASystemSignature(gma)[i] +1;
                                num /= DSGMASystemSignature(gma)[i];
                        }
                        break;
        }
bail:
        return signature;
}

extern const DSUInteger DSCaseNumberForSignature(const DSUInteger * signature, const DSGMASystem * gma)
{
        DSInteger i;
        DSUInteger temp, numberOfEquations,  caseNumber = 0;
        if (signature == NULL) {
                DSError(M_DS_NULL ": Case Signature is NULL", A_DS_ERROR);
                goto bail;
        }
        if (gma == NULL) {
                DSError(M_DS_GMA_NULL ": Template GMA to make S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        numberOfEquations = DSVariablePoolNumberOfVariables(DSGMASystemXd(gma));
        caseNumber = 1;
        temp = 1;
        switch (endian) {
                case DS_CASE_NUMBER_SMALL_ENDIAN:
                        for (i = 0; i < 2*numberOfEquations; i++) {
                                caseNumber += (signature[i]-1)*temp;
                                temp *= DSGMASystemSignature(gma)[i];
                        }
                        break;
                case DS_CASE_NUMBER_BIG_ENDIAN:
                default:
                        for (i = 2*numberOfEquations-1; i >= 0; i--) {
                                caseNumber += (signature[i]-1)*temp;
                                temp *= DSGMASystemSignature(gma)[i];
                        }
                        break;
        }
bail:
        return caseNumber;
}

extern char * DSCaseSignatureToString(const DSCase *aCase)
{
        char temp[100];
        char * string = NULL;
        DSUInteger i;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        string = DSSecureCalloc(sizeof(char), 5*DSCaseNumberOfEquations(aCase));
        for (i = 0; i < 2*DSCaseNumberOfEquations(aCase); i++) {
                if (DSCaseSig(aCase)[i] >= 10)
                        sprintf(temp, "(%i)", DSCaseSig(aCase)[i]);
                else
                        sprintf(temp, "%i", DSCaseSig(aCase)[i]);
                strncat(string, temp, 100-strlen(string));
        }
bail:
        return string;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Printing functions
#endif

extern void DSCasePrint(const DSCase *aCase)
{
        DSUInteger i;
        int (*print)(const char *, ...);
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSPrintf == NULL)
                print = printf;
        else
                print = DSPrintf;
        print("\t  Case: %i\n\t   Sig: ",
              DSCaseNum(aCase));
        for (i = 0; i < DSCaseNumberOfEquations(aCase); i++) {
                if (DSCaseSig(aCase)[2*i] >= 10)
                        print("(");
                print("%i", DSCaseSig(aCase)[2*i]);
                if (DSCaseSig(aCase)[2*i] >= 10)
                        print(")");
                if (DSCaseSig(aCase)[2*i+1] >= 10)
                        print("(");
                print("%i", DSCaseSig(aCase)[2*i+1]);
                if (DSCaseSig(aCase)[2*i+1] >= 10)
                        print(")");
        }
        print("\n");
        DSSSystemPrint(DSCaseSSys(aCase));
bail:
        return;
}

extern void DSCasePrintEquations(const DSCase *aCase)
{
        DSUInteger i;
        DSExpression ** equations = NULL;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        equations = DSCaseEquations(aCase);
        if (equations != NULL) {
                for (i= 0; i < DSCaseNumberOfEquations(aCase); i++) {
                        DSExpressionPrint(equations[i]);
                        DSExpressionFree(equations[i]);
                }
                DSSecureFree(equations);
        }
bail:
        return;
}

extern void DSCasePrintSolution(const DSCase *aCase)
{
        DSUInteger i;
        DSExpression ** solution = NULL;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        solution = DSCaseSolution(aCase);
        if (solution != NULL) {
                for (i= 0; i < DSCaseNumberOfEquations(aCase); i++) {
                        DSExpressionPrint(solution[i]);
                        DSExpressionFree(solution[i]);
                }
                DSSecureFree(solution);
        }
bail:
        return;
}

extern void DSCasePrintLogarithmicSolution(const DSCase *aCase)
{
        DSUInteger i;
        DSExpression ** solution = NULL;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        solution = DSCaseLogarithmicSolution(aCase);
        if (solution != NULL) {
                for (i= 0; i < DSCaseNumberOfEquations(aCase); i++) {
                        DSExpressionPrint(solution[i]);
                        DSExpressionFree(solution[i]);
                }
                DSSecureFree(solution);
        }
bail:
        return;
}

extern void DSCasePrintConditions(const DSCase *aCase)
{
        DSUInteger i;
        DSExpression ** conditions = NULL;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        conditions = DSCaseConditions(aCase);
        if (conditions != NULL) {
                for (i= 0; i < DSCaseNumberOfConditions(aCase); i++) {
                        DSExpressionPrint(conditions[i]);
                        DSExpressionFree(conditions[i]);
                }
                DSSecureFree(conditions);
        }
bail:
        return;
}

extern void DSCasePrintLogarithmicConditions(const DSCase *aCase)
{
        DSUInteger i;
        DSExpression ** conditions = NULL;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        conditions = DSCaseLogarithmicConditions(aCase);
        if (conditions != NULL) {
                for (i= 0; i < DSCaseNumberOfConditions(aCase); i++) {
                        DSExpressionPrint(conditions[i]);
                        DSExpressionFree(conditions[i]);
                }
                DSSecureFree(conditions);
        }
bail:
        return;
}

extern void DSCasePrintBoundaries(const DSCase *aCase)
{
        DSUInteger i;
        DSExpression ** boundaries = NULL;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        boundaries = DSCaseBoundaries(aCase);
        if (boundaries != NULL) {
                for (i= 0; i < DSCaseNumberOfBoundaries(aCase); i++) {
                        printf("0 < ");
                        DSExpressionPrint(boundaries[i]);
                        DSExpressionFree(boundaries[i]);
                }
                DSSecureFree(boundaries);
        }
bail:
        return;
}

extern void DSCasePrintLogarithmicBoundaries(const DSCase *aCase)
{
        DSUInteger i;
        DSExpression ** boundaries = NULL;
        int (*print)(const char *, ...);
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        print = DSPrintf;
        if (print == NULL) 
                print = printf;
        boundaries = DSCaseLogarithmicBoundaries(aCase);
        if (boundaries != NULL) {
                for (i= 0; i < DSCaseNumberOfBoundaries(aCase); i++) {
                        print("0 < ");
                        DSExpressionPrint(boundaries[i]);
                        DSExpressionFree(boundaries[i]);
                }
                DSSecureFree(boundaries);
        }
bail:
        return;
}






