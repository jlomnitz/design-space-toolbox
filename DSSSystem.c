/**
 * \file DSSSystem.m
 * \brief Header file with functions for dealing with S-Systems.
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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "DSTypes.h"
#include "DSErrors.h"
#include "DSMemoryManager.h"
#include "DSVariable.h"
#include "DSSSystem.h"
#include "DSExpression.h"
#include "DSExpressionTokenizer.h"
#include "DSSSystemGrammar.h"
#include "DSGMASystemParsingAux.h"
#include "DSMatrix.h"
#include "DSMatrixArray.h"
#include "DSGMASystem.h"

/**
 * \defgroup DSSSysACCESSORS
 *
 * \brief Internal S-System Accessor macros.
 * 
 * \details Used within DSSSystem.c to access the data within a S-System data type.
 * These macros are not to be used putside of this file, as they do not check the
 * data dor consistency and thus would not invoke the DSError function, making
 * it harder to trace errors.
 */
/*\{*/
#define DSSSysXi(x)                       ((x)->Xi)
#define DSSSysXd(x)                       ((x)->Xd)
#define DSSSysXd_a(x)                     ((x)->Xd_a)
#define DSSSysXd_t(x)                     ((x)->Xd_t)
#define DSSSysAlpha(x)                    ((x)->alpha)
#define DSSSysBeta(x)                     ((x)->beta)
#define DSSSysGd(x)                       ((x)->Gd)
#define DSSSysGi(x)                       ((x)->Gi)
#define DSSSysHd(x)                       ((x)->Hd)
#define DSSSysHi(x)                       ((x)->Hi)
#define DSSSysM(x)                        ((x)->M)

#define DSSSysIsSingular(x)               ((x)->isSingular)
#define DSSSysShouldFreeXd(x)             ((x)->shouldFreeXd)
#define DSSSysShouldFreeXi(x)             ((x)->shouldFreeXi)
/*\}*/

#if defined (__APPLE__) && defined (__MACH__)
#pragma  mark - Function Prototypes
#endif

static void dsSSystemSolveEquations(DSSSystem *ssys);


#if defined (__APPLE__) && defined (__MACH__)
#pragma  mark - Allocation, deallocation and initialization
#endif

/**
 * \brief Creates a empty S-System.
 * 
 * This function allocates the necessary memory space used by a S-System and
 * initializes it so that it is ready for processing.  The initialized GMA
 * has all its fields set to 0 or NULL.  This is interpreted as an empty GMA
 * and is necessary for parsing a set of equations.
 *
 * \return A DSSSystem pointer to the newly allocated GMASystem.
 */
static DSSSystem * DSSSystemAlloc(void)
{
        DSSSystem *sys  = NULL;
        sys  = DSSecureCalloc(sizeof(DSSSystem), 1);
        return sys ;
}

extern DSSSystem * DSSSystemCopy(const DSSSystem * original)
{
        DSSSystem * newSSys = NULL;
        if (original == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        newSSys = DSSSystemAlloc();
        DSSSysXd(newSSys) = DSVariablePoolCopy(DSSSystemXd(original));
        DSSSysXd_t(newSSys) = DSVariablePoolCopy(DSSSystemXd_t(original));
        DSSSysXd_a(newSSys) = DSVariablePoolCopy(DSSSystemXd_a(original));
        DSSSysXi(newSSys) = DSVariablePoolCopy(DSSSystemXi(original));
        DSSSysShouldFreeXd(newSSys) = true;
        DSSSysShouldFreeXi(newSSys) = true;
        DSSSysGd(newSSys) = DSMatrixCopy(DSSSysGd(original));
        DSSSysHd(newSSys) = DSMatrixCopy(DSSSysHd(original));
        DSSSysGi(newSSys) = DSMatrixCopy(DSSSysGi(original));
        DSSSysHi(newSSys) = DSMatrixCopy(DSSSysHi(original));
        DSSSysAlpha(newSSys) = DSMatrixCopy(DSSSysAlpha(original));
        DSSSysBeta(newSSys) = DSMatrixCopy(DSSSysBeta(original));
        DSSSysIsSingular(newSSys) = DSSSysIsSingular(original);
        if (DSSSysIsSingular(newSSys) == false) {
                DSSSysM(newSSys) = DSMatrixCopy(DSSSysM(original));
        }
bail:
        return newSSys;
}

extern void DSSSystemFree(DSSSystem * sys)
{
        if (sys  == NULL) {
                DSError(M_DS_NULL ": S-System to free is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSSSysShouldFreeXd(sys) == true) {
                DSVariablePoolSetReadWriteAdd(DSSSysXd(sys));
                DSVariablePoolFree(DSSSysXd(sys));
                if (DSSSysXd_t(sys) != NULL) {
                        DSVariablePoolSetReadWriteAdd(DSSSysXd_t(sys));
                        DSVariablePoolFree(DSSSysXd_t(sys));
                }
                if (DSSSysXd_a(sys) != NULL) {
                        DSVariablePoolSetReadWriteAdd(DSSSysXd_a(sys));
                        DSVariablePoolFree(DSSSysXd_a(sys));
                }
        }
        if (DSSSysShouldFreeXi(sys) == true) {
                DSVariablePoolSetReadWriteAdd(DSSSysXi(sys));
                DSVariablePoolFree(DSSSysXi(sys));
        }
        DSMatrixFree(DSSSysAlpha(sys));
        DSMatrixFree(DSSSysBeta(sys));
        DSMatrixFree(DSSSysGd(sys));
        if (DSSSysGi(sys) != NULL)
                DSMatrixFree(DSSSysGi(sys));
        if (DSSSysHd(sys) != NULL)
                DSMatrixFree(DSSSysHd(sys));
        if (DSSSysHi(sys) != NULL)
                DSMatrixFree(DSSSysHi(sys));
        if (DSSSysM(sys) != NULL)
                DSMatrixFree(DSSSysM(sys));
        DSSecureFree(sys);
bail:
        return;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma  mark - Factory methods
#endif



#if defined (__APPLE__) && defined (__MACH__)
#pragma  mark Internal Parsing Functions
#endif

static gma_parseraux_t * dsSSystemParseStringToTermList(const char * string)
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
        parser = DSSSystemParserAlloc(DSSecureMalloc);
        root = DSGMAParserAuxAlloc();
        parser_aux = root;
        current = tokens;
        while (current != NULL) {
                if (DSExpressionTokenType(current) == DS_EXPRESSION_TOKEN_START) {
                        current = DSExpressionTokenNext(current);
                        continue;
                }
                DSSSystemParser(parser, 
                                DSExpressionTokenType(current),
                                current,
                                ((void**)&parser_aux));
                current = DSExpressionTokenNext(current);
        }
        DSSSystemParser(parser, 
                          0, 
                          NULL,
                          ((void **)&parser_aux));
        DSSSystemParserFree(parser, DSSecureFree);
        DSExpressionTokenFree(tokens);
        if (DSGMAParserAuxParsingFailed(root) == true) {
                DSGMAParserAuxFree(root);
                root = NULL;
        }
bail:
        return root;
}

static gma_parseraux_t ** dsSSysTermListForAllStrings(char * const * const strings, const DSUInteger numberOfEquations)
{
        DSUInteger i;
        gma_parseraux_t **aux = NULL;
        DSExpression *expr;
        char *aString;
        bool failed = false;
        aux = DSSecureCalloc(sizeof(gma_parseraux_t *), numberOfEquations);
        for (i = 0; i < numberOfEquations; i++) {
                if (strings[i] == NULL) {
                        DSError(M_DS_WRONG ": String to parse is NULL", A_DS_ERROR);
                        failed = true;
                        break;
                }
                if (strlen(strings[i]) == 0) {
                        DSError(M_DS_WRONG ": String to parse is empty", A_DS_ERROR);
                        failed = true;
                        break;
                }
                expr = DSExpressionByParsingString(strings[i]);
                if (expr != NULL) {
                        aString = DSExpressionAsString(expr);
                        aux[i] = dsSSystemParseStringToTermList(aString);
                        DSSecureFree(aString);
                        DSExpressionFree(expr);
                }
                if (aux[i] == NULL) {
                        DSError(M_DS_PARSE ": Expression not in S-System format", A_DS_ERROR);
                        failed = true;
                        break;
                }
        }
        if (failed == true) {
                for (i = 0; i < numberOfEquations; i++)
                        if (aux[i] != NULL)
                                DSGMAParserAuxFree(aux[i]);
                DSSecureFree(aux);
                aux = NULL;
        }
bail:
        return aux;
}

static DSVariablePool * dsSSystemIdentifyIndependentVariables(const DSVariablePool * const Xd, gma_parseraux_t ** aux, const DSUInteger numberOfEquations)
{
        DSVariablePool * Xi = NULL;
        gma_parseraux_t *current = NULL;
        DSUInteger i, j;
        const char *name;
        if (aux == NULL) {
                DSError(M_DS_NULL ": Parser auxiliary data is NULL", A_DS_ERROR);
                goto bail;
        }
        if (Xd == NULL) {
                DSError(M_DS_NULL ": Independent variables are NULL", A_DS_ERROR);
                goto bail;
        }
        if (numberOfEquations == 0) {
                DSError(M_DS_WRONG ": No equations to parse", A_DS_WARN);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(Xd) != numberOfEquations) {
                DSError(M_DS_WRONG ": Number of independent variables does not match number of equations", A_DS_ERROR);
                goto bail;
        }
        Xi = DSVariablePoolAlloc();
        for (i = 0; i < numberOfEquations; i++) {
                current = aux[i];
                while (current) {
                        for (j = 0; j < DSGMAParserAuxNumberOfBases(current); j++) {
                                if (DSGMAParserAuxBaseAtIndexIsVariable(current, j) == false)
                                        continue;
                                name = DSGMAParserAuxVariableAtIndex(current, j);
                                if (DSVariablePoolHasVariableWithName(Xd, name) == true)
                                        continue;
                                if (DSVariablePoolHasVariableWithName(Xi, name) == true)
                                        continue;
                                DSVariablePoolAddVariableWithName(Xi, name);
                        }
                        current = DSGMAParserAuxNextNode(current);
                }
        }
bail:
        return Xi;
}

static void dsSSystemInitializeMatrices(DSSSystem *sys)
{
        DSUInteger numberOfEquations, numberOfXd, numberOfXi;
        numberOfEquations = DSVariablePoolNumberOfVariables(DSSSysXd(sys));
        numberOfXd = numberOfEquations;
        numberOfXi = DSVariablePoolNumberOfVariables(DSSSysXi(sys));
        DSSSysAlpha(sys) = DSMatrixCalloc(numberOfEquations, 1);
        DSSSysBeta(sys) = DSMatrixCalloc(numberOfEquations, 1);
        DSSSysGd(sys) = DSMatrixCalloc(numberOfEquations, numberOfXd);
        if (numberOfXi != 0)
                DSSSysGi(sys) = DSMatrixCalloc(numberOfEquations, numberOfXi);
        DSSSysHd(sys) = DSMatrixCalloc(numberOfEquations, numberOfXd);
        if (numberOfXi != 0)
                DSSSysHi(sys) = DSMatrixCalloc(numberOfEquations, numberOfXi);
}

static void dsSSysProcessPositiveExponentBasePairs(DSSSystem *sys, gma_parseraux_t *current, DSUInteger equation)
{
        DSUInteger j;
        const char *varName;
        double currentValue;
        for (j = 0; j < DSGMAParserAuxNumberOfBases(current); j++) {
                if (DSGMAParserAuxBaseAtIndexIsVariable(current, j) == false) { 
                        DSMatrixSetDoubleValue(DSSSysAlpha(sys), 
                                               equation, 0, 
                                               DSGMAParseAuxsConstantBaseAtIndex(current, j));
                        continue;
                }
                varName = DSGMAParserAuxVariableAtIndex(current, j);
                if (DSVariablePoolHasVariableWithName(DSSSysXd(sys), varName) == true) {
                        currentValue = DSMatrixDoubleValue(DSSSysGd(sys), equation, DSVariablePoolIndexOfVariableWithName(DSSSysXd(sys),
                                                                                                                          varName));
                        DSMatrixSetDoubleValue(DSSSysGd(sys),
                                               equation,
                                               DSVariablePoolIndexOfVariableWithName(DSSSysXd(sys),
                                                                                     varName),
                                               currentValue+DSGMAParserAuxExponentAtIndex(current, j));
                } else if (DSVariablePoolHasVariableWithName(DSSSysXi(sys), varName) == true) {
                        currentValue = DSMatrixDoubleValue(DSSSysHi(sys), equation, DSVariablePoolIndexOfVariableWithName(DSSSysXi(sys),
                                                                                                                          varName));
                        DSMatrixSetDoubleValue(DSSSysGi(sys),
                                               equation,
                                               DSVariablePoolIndexOfVariableWithName(DSSSysXi(sys),
                                                                                     varName),
                                               currentValue+DSGMAParserAuxExponentAtIndex(current, j));
                }
        }
}

static void dsSSysProcessNegativeExponentBasePairs(DSSSystem *sys, gma_parseraux_t *current, DSUInteger equation)
{
        DSUInteger j;
        const char *varName;
        double currentValue;
        for (j = 0; j < DSGMAParserAuxNumberOfBases(current); j++) {
                if (DSGMAParserAuxBaseAtIndexIsVariable(current, j) == false) { 
                        DSMatrixSetDoubleValue(DSSSysBeta(sys), 
                                               equation, 0, 
                                               DSGMAParseAuxsConstantBaseAtIndex(current, j));
                        continue;
                }
                varName = DSGMAParserAuxVariableAtIndex(current, j);
                if (DSVariablePoolHasVariableWithName(DSSSysXd(sys), varName) == true) {
                        currentValue = DSMatrixDoubleValue(DSSSysHd(sys), equation, DSVariablePoolIndexOfVariableWithName(DSSSysXd(sys),
                                                                                                                          varName));
                        DSMatrixSetDoubleValue(DSSSysHd(sys),
                                               equation,
                                               DSVariablePoolIndexOfVariableWithName(DSSSysXd(sys),
                                                                                     varName),
                                               currentValue+DSGMAParserAuxExponentAtIndex(current, j));
                } else if (DSVariablePoolHasVariableWithName(DSSSysXi(sys), varName) == true) {
                        currentValue = DSMatrixDoubleValue(DSSSysHi(sys), equation, DSVariablePoolIndexOfVariableWithName(DSSSysXi(sys),
                                                                                                                          varName));
                        DSMatrixSetDoubleValue(DSSSysHi(sys),
                                               equation,
                                               DSVariablePoolIndexOfVariableWithName(DSSSysXi(sys),
                                                                                     varName),
                                               currentValue+DSGMAParserAuxExponentAtIndex(current, j));
                }
        }
}

#include <unistd.h> 

static void dsSSystemSolveEquations(DSSSystem *ssys)
{
        DSMatrix *M, *Ad;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System being modified is NULL", A_DS_ERROR);
                goto bail;
        }
        DSSSysIsSingular(ssys) = true;
        Ad = DSMatrixBySubstractingMatrix(DSSSysGd(ssys), DSSSysHd(ssys));
        M = DSMatrixInverse(Ad);
        DSSSysIsSingular(ssys) = true;
        if (M != NULL) {
                DSSSysIsSingular(ssys) = false;
                DSSSysM(ssys) = M;
        }
        DSMatrixFree(Ad);
bail:
        return;
}

static void dsSSystemCreateSystemMatrices(DSSSystem *sys, gma_parseraux_t **aux)
{
        gma_parseraux_t *current;
        DSUInteger numberOfEquations;
        DSUInteger i;
        if (sys == NULL) {
                DSError(M_DS_NULL ": S-System being modified is NULL", A_DS_ERROR);
                goto bail;
        }
        if (aux == NULL) {
                DSError(M_DS_NULL ": Parser auxiliary data is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSSSysXd(sys) == NULL || DSSSysXi(sys) == NULL) {
                DSError(M_DS_WRONG ": S-System data is incomplete: Need Xi and Xd", A_DS_ERROR);
                goto bail;
        }
        numberOfEquations = DSVariablePoolNumberOfVariables(DSSSysXd(sys));
        dsSSystemInitializeMatrices(sys);
        for (i = 0; i < numberOfEquations; i++) {
                current = aux[i];
                while (current) {
                        switch (DSGMAParserAuxSign(current)) {
                                case AUX_SIGN_POSITIVE:
                                        DSMatrixSetDoubleValue(DSSSysAlpha(sys), 
                                                               i, 0, 
                                                               1.0);
                                        dsSSysProcessPositiveExponentBasePairs(sys, current, i);
                                        break;
                                case AUX_SIGN_NEGATIVE:
                                        DSMatrixSetDoubleValue(DSSSysBeta(sys), 
                                                               i, 0, 
                                                               1.0);
                                        dsSSysProcessNegativeExponentBasePairs(sys, current, i);
                                        break;
                                default:
                                        break;
                        }
                        current = DSGMAParserAuxNextNode(current);
                }
        }
        dsSSystemSolveEquations(sys);
bail:
        return;
}

DSSSystem * dsSSystemWithAlgebraicConstraints(const DSSSystem * originalSystem, DSVariablePool * newXd, DSUInteger numberDifferentialVariables, DSUInteger * differentialIndices, DSUInteger numberOfAlgebraicVariables, DSUInteger * algebraicIndices) {
        DSUInteger i, j, k;
        DSSSystem * collapsedSSystem = NULL;
        const DSMatrix * oldAd, *oldAi, *oldB;
        DSMatrix *temp, *subM, *subAdAlgebraic, *subAd, *subB, *subAi;
        double value, factor;
        oldAd = DSSSystemAd(originalSystem);
        oldAi = DSSSystemAi(originalSystem);
        oldB = DSSSystemB(originalSystem);
        subAdAlgebraic = DSMatrixSubMatrixExcludingRowsAndColumns(oldAd, numberDifferentialVariables, numberDifferentialVariables, differentialIndices, differentialIndices);
        subAd = DSMatrixSubMatrixExcludingRowsAndColumns(oldAd, numberDifferentialVariables, numberOfAlgebraicVariables, differentialIndices, algebraicIndices);
        subAi = DSMatrixSubMatrixExcludingRows(oldAi, numberDifferentialVariables, differentialIndices);
        DSMatrixMultiplyByScalar(subAi, -1.0f);
        subB = DSMatrixSubMatrixExcludingRows(oldB, numberDifferentialVariables, differentialIndices);
        DSMatrixMultiplyByScalar(subAd, -1.0f);
        subM = DSMatrixInverse(subAdAlgebraic);
        DSMatrixFree(subAdAlgebraic);
        temp = DSMatrixByMultiplyingMatrix(subM, subAd);
        DSMatrixFree(subAd);
        subAd = temp;
        temp = DSMatrixByMultiplyingMatrix(subM, subAi);
        DSMatrixFree(subAi);
        subAi = temp;
        temp = DSMatrixByMultiplyingMatrix(subM, subB);
        DSMatrixFree(subB);
        subB = temp;
        collapsedSSystem = DSSSystemAlloc();
        DSSSysXd(collapsedSSystem) = newXd;
        DSSSysXd_t(collapsedSSystem) = DSVariablePoolCopy(newXd);
        DSSSysXd_a(collapsedSSystem) = DSVariablePoolAlloc();
        DSSSysXi(collapsedSSystem) = (DSVariablePool *)DSSSystemXi(originalSystem);
        DSSSysShouldFreeXd(collapsedSSystem) = true;
        DSSSysShouldFreeXi(collapsedSSystem) = false;
        DSSSysGd(collapsedSSystem) = DSMatrixSubMatrixExcludingRowsAndColumns(DSSSystemGd(originalSystem), numberOfAlgebraicVariables,numberOfAlgebraicVariables, algebraicIndices, algebraicIndices);
        DSSSysHd(collapsedSSystem) = DSMatrixSubMatrixExcludingRowsAndColumns(DSSSystemHd(originalSystem), numberOfAlgebraicVariables,numberOfAlgebraicVariables, algebraicIndices, algebraicIndices);
        DSSSysGi(collapsedSSystem) = DSMatrixSubMatrixExcludingRows(DSSSystemGi(originalSystem), numberOfAlgebraicVariables, algebraicIndices);
        DSSSysHi(collapsedSSystem) = DSMatrixSubMatrixExcludingRows(DSSSystemHi(originalSystem), numberOfAlgebraicVariables, algebraicIndices);
        DSSSysAlpha(collapsedSSystem) = DSMatrixSubMatrixExcludingRows(DSSSystemAlpha(originalSystem), numberOfAlgebraicVariables, algebraicIndices);
        DSSSysBeta(collapsedSSystem) = DSMatrixSubMatrixExcludingRows(DSSSystemBeta(originalSystem), numberOfAlgebraicVariables, algebraicIndices);
        for (i = 0; i < DSMatrixRows(DSSSysGd(collapsedSSystem)); i++) {
                for (j = 0; j < numberOfAlgebraicVariables; j++) {
                        factor = DSMatrixDoubleValue(DSSSystemGd(originalSystem), i, algebraicIndices[j]);
                        for (k = 0; k < DSMatrixColumns(DSSSysGd(collapsedSSystem)); k++) {
                                value = DSMatrixDoubleValue(DSSSysGd(collapsedSSystem),
                                                            i, k)+ factor*DSMatrixDoubleValue(subAd,
                                                                                              j, k);
                                DSMatrixSetDoubleValue(DSSSysGd(collapsedSSystem),
                                                       i, k, value);
                        }
                        for (k = 0; k < DSMatrixColumns(DSSSysGi(collapsedSSystem)); k++) {
                                value = DSMatrixDoubleValue(DSSSysGi(collapsedSSystem),
                                                            i, k)+ factor*DSMatrixDoubleValue(subAi,
                                                                                              j, k);
                                DSMatrixSetDoubleValue(DSSSysGi(collapsedSSystem),
                                                       i, k, value);
                        }
                        value = DSMatrixDoubleValue(DSSSysAlpha(collapsedSSystem), i, 0.0f)+ factor*DSMatrixDoubleValue(subB,
                                                                                                                        j, 0.0f);
                        DSMatrixSetDoubleValue(DSSSysAlpha(collapsedSSystem), i, 0.0f, value);
                        factor = DSMatrixDoubleValue(DSSSystemHd(originalSystem), i, algebraicIndices[j]);
                        for (k = 0; k < DSMatrixColumns(DSSSysHd(collapsedSSystem)); k++) {
                                value = DSMatrixDoubleValue(DSSSysHd(collapsedSSystem),
                                                            i, k)+ factor*DSMatrixDoubleValue(subAd,
                                                                                              j, k);
                                DSMatrixSetDoubleValue(DSSSysHd(collapsedSSystem),
                                                       i, k, value);
                        }
                        for (k = 0; k < DSMatrixColumns(DSSSysHi(collapsedSSystem)); k++) {
                                value = DSMatrixDoubleValue(DSSSysHi(collapsedSSystem),
                                                            i, k)+ factor*DSMatrixDoubleValue(subAi,
                                                                                              j, k);
                                DSMatrixSetDoubleValue(DSSSysHi(collapsedSSystem),
                                                       i, k, value);
                        }
                        value = DSMatrixDoubleValue(DSSSysBeta(collapsedSSystem), i, 0.0f)+ factor*DSMatrixDoubleValue(subB,
                                                                                                                    j, 0.0f);
                        DSMatrixSetDoubleValue(DSSSysBeta(collapsedSSystem), i, 0.0f, value);
                }
        }
        dsSSystemSolveEquations(collapsedSSystem);
        DSMatrixFree(subAd);
        DSMatrixFree(subAi);
        DSMatrixFree(subB);
bail:
        return collapsedSSystem;
}

extern DSSSystem * DSSSystemByRemovingAlgebraicConstraints(const DSSSystem * originalSSystem)
{
        DSUInteger numberOfAlgebraicVaiables = 0, numberOfDifferentialVariables = 0;
        DSUInteger i, j, k;
        DSUInteger * algebraicIndices, *differentialIndices;
        DSSSystem * collapsedSystem = NULL;
        const DSVariablePool * oldXd;
        DSVariablePool * newXd = NULL;
        char * name;
        if (originalSSystem == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSSSystemXd_a(originalSSystem) == NULL) {
                DSError(M_DS_VAR_NULL ": Xd_a variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(DSSSystemXd_a(originalSSystem)) > DSVariablePoolNumberOfVariables(DSSSystemXd(originalSSystem))) {
                DSError(M_DS_WRONG ": Number of algebraic variables exceeds number of total variables", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(DSSSystemXd_a(originalSSystem)) == 0) {
                collapsedSystem = DSSSystemCopy(originalSSystem);
                goto bail;
        }
        oldXd = DSSSystemXd(originalSSystem);
        newXd = DSVariablePoolAlloc();
        for (i = 0; i < DSVariablePoolNumberOfVariables(oldXd); i++) {
                name = DSVariableName(DSVariablePoolVariableAtIndex(oldXd, i));
                if (DSVariablePoolHasVariableWithName(DSSSystemXd_a(originalSSystem), name) == true) {
                        continue;
                }
                DSVariablePoolAddVariableWithName(newXd, name);
        }
        if (DSVariablePoolNumberOfVariables(oldXd) - DSVariablePoolNumberOfVariables(newXd) != DSVariablePoolNumberOfVariables(DSSSystemXd_a(originalSSystem))) {
                DSError(M_DS_WRONG, A_DS_ERROR);
                DSVariablePoolFree(newXd);
                goto bail;
        }
        numberOfDifferentialVariables = DSVariablePoolNumberOfVariables(newXd);
        numberOfAlgebraicVaiables = DSVariablePoolNumberOfVariables(DSSSystemXd_a(originalSSystem));
        differentialIndices = DSSecureCalloc(sizeof(DSUInteger), numberOfDifferentialVariables);
        algebraicIndices = DSSecureCalloc(sizeof(DSUInteger), numberOfAlgebraicVaiables);
        for (i = 0, j = 0, k = 0; i < DSVariablePoolNumberOfVariables(oldXd); i++) {
                if (DSVariablePoolHasVariableWithName(newXd, DSVariableName(DSVariablePoolVariableAtIndex(oldXd, i))) == true) {
                        differentialIndices[j++] = i;
                } else {
                        algebraicIndices[k++] = i;
                }
        }
        collapsedSystem = dsSSystemWithAlgebraicConstraints(originalSSystem, newXd, numberOfDifferentialVariables, differentialIndices, numberOfAlgebraicVaiables, algebraicIndices);
        DSSecureFree(differentialIndices);
        DSSecureFree(algebraicIndices);
bail:
        return collapsedSystem;
}
#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Public Functions
#endif


extern DSSSystem * DSSSystemByParsingStringList(char * const * const string, const DSVariablePool * const Xd_a, ...)
{
        DSSSystem *gma = NULL;
        DSUInteger numberOfStrings = 0;
        char const ** strings = NULL;
        const char * aString = NULL;
        if (strings == NULL) {
                DSError(M_DS_NULL ": String to parse is NULL", A_DS_ERROR);
        }
        va_list ap;
	va_start(ap, Xd_a);
        strings = DSSecureCalloc(sizeof(char *), 1);
        strings[0] = (const char *)string;
        numberOfStrings++;
        aString = va_arg(ap, char *);
        while (aString != NULL) {
                strings = DSSecureRealloc(strings, sizeof(char *)*(numberOfStrings+1));
                strings[numberOfStrings++] = aString;
                aString = va_arg(ap, char *);
        }
        gma = DSSSystemByParsingStrings((char * const * )strings, Xd_a, numberOfStrings);
        DSSecureFree(strings);
bail:
        return gma;
}

extern DSSSystem * DSSSystemByParsingStrings(char * const * const strings, const DSVariablePool * const Xd_a, const DSUInteger numberOfEquations)
{
        DSSSystem * sys = NULL;
        gma_parseraux_t **aux = NULL;
        DSUInteger i, j;
        DSVariablePool *tempPool, * Xd, * Xda, * Xdt;
        char * variableName;
        DSExpression * expr, * lhs;
        if (strings == NULL) {
                DSError(M_DS_NULL ": Array of strings is NULL", A_DS_ERROR);
                goto bail;
        }
        if (numberOfEquations == 0) {
                DSError(M_DS_WRONG ": No equations to parse", A_DS_WARN);
                goto bail;
        }
        aux = dsSSysTermListForAllStrings(strings, numberOfEquations);
        if (aux == NULL)
                goto bail;
        Xd = DSVariablePoolAlloc();
        Xda = DSVariablePoolAlloc();
        Xdt = DSVariablePoolAlloc();
        for (i=0; i < numberOfEquations; i++) {
                expr = DSExpressionByParsingString(strings[i]);
                lhs = DSExpressionEquationLHSExpression(expr);
                if (DSExpressionType(lhs) == DS_EXPRESSION_TYPE_CONSTANT) {
                        // If different from 0, should substract rhs by lhs
                }
                tempPool = DSExpressionVariablesInExpression(lhs);
                if (DSVariablePoolNumberOfVariables(tempPool) == 1) {
                        variableName = DSVariableName(DSVariablePoolVariableAtIndex(tempPool, 0));
                        if (DSExpressionType(lhs) == DS_EXPRESSION_TYPE_VARIABLE) {
                                DSVariablePoolAddVariableWithName(Xda, variableName);
                        } else {
                                DSVariablePoolAddVariableWithName(Xdt, variableName);
                        }
                        if (DSVariablePoolHasVariableWithName(Xd, variableName) == false) {
                                DSVariablePoolAddVariableWithName(Xd, variableName);
                        }
                }
                DSExpressionFree(lhs);
                DSExpressionFree(expr);
                DSVariablePoolFree(tempPool);
        }
        if (Xd_a != NULL) {
                for (j = 0; j < DSVariablePoolNumberOfVariables(Xd_a); j++) {
                        variableName = DSVariableName(DSVariablePoolVariableAtIndex(Xd_a, j));
                        if (DSVariablePoolHasVariableWithName(Xd, variableName) == false) {
                                DSVariablePoolAddVariableWithName(Xd, variableName);
                                DSVariablePoolAddVariableWithName(Xda, variableName);
                        }
                }
        }
        if (DSVariablePoolNumberOfVariables(Xd) != numberOfEquations) {
                DSError(M_DS_WRONG ": Number of dependent variables does not match number of equations", A_DS_ERROR);
                goto bail;
        }
        
        sys = DSSSystemAlloc();
        DSSSysXd(sys) = Xd;
        DSVariablePoolSetReadWrite(DSSSysXd(sys));
        DSSSysXd_a(sys) = Xda;
        DSSSysXd_t(sys) = Xdt;
        DSVariablePoolSetReadWrite(DSSSysXd_a(sys));
        DSVariablePoolSetReadWrite(DSSSysXd_t(sys));
        DSSSysXi(sys) = dsSSystemIdentifyIndependentVariables(Xd, aux, numberOfEquations);
        DSSSysShouldFreeXd(sys) = true;
        DSSSysShouldFreeXi(sys) = true;
        DSVariablePoolSetReadWrite(DSSSysXi(sys));
        dsSSystemCreateSystemMatrices(sys, aux);
        for (i=0; i < numberOfEquations; i++)
                if (aux[i] != NULL)
                        DSGMAParserAuxFree(aux[i]);
        DSSecureFree(aux);
bail:
        return sys;
}

extern DSSSystem * DSSSystemFromGMAWithDominantTerms(const DSGMASystem * gma, const DSUInteger * termArray)
{
        return DSSSystemWithTermsFromGMA(gma, termArray);
}

extern DSSSystem * DSSSystemWithTermsFromGMA(const DSGMASystem * gma, const DSUInteger * termArray)
{
        DSSSystem *ssys = NULL;
        DSUInteger i, j, term1, term2, numberOfEquations, numberOfXi;
        if (gma == NULL) {
                DSError(M_DS_NULL ": Template GMA to make S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        if (termArray == NULL) {
                DSError(M_DS_NULL ": Array of dominant terms is NULL", A_DS_ERROR);
                goto bail;
        }
        ssys = DSSSystemAlloc();
        DSSSysXd(ssys) = (DSVariablePool *)DSGMASystemXd(gma);
        DSSSysXi(ssys) = (DSVariablePool *)DSGMASystemXi(gma);
        DSSSysXd_a(ssys) = (DSVariablePool *)DSGMASystemXd_a(gma);
        DSSSysXd_t(ssys) = (DSVariablePool *)DSGMASystemXd_t(gma);
        DSSSysShouldFreeXd(ssys) = false;
        DSSSysShouldFreeXi(ssys) = false;
        dsSSystemInitializeMatrices(ssys);
        numberOfEquations = DSGMASystemNumberOfEquations(gma);
        numberOfXi = DSVariablePoolNumberOfVariables(DSSSysXi(ssys));
        for (i = 0; i < 2*numberOfEquations; i+=2) {
                term1 = termArray[i];
                term2 = termArray[i+1];
                if (term1 > DSGMASystemSignature(gma)[i] || term2 > DSGMASystemSignature(gma)[i+1])
                        break;
                DSMatrixSetDoubleValue(DSSSysAlpha(ssys), i/2, 0, 
                                       DSMatrixDoubleValue(DSGMASystemAlpha(gma), i/2, term1-1));
                DSMatrixSetDoubleValue(DSSSysBeta(ssys), i/2, 0, 
                                       DSMatrixDoubleValue(DSGMASystemBeta(gma), i/2, term2-1));
                for (j = 0; j < numberOfEquations; j++) {
                        DSMatrixSetDoubleValue(DSSSysGd(ssys), i/2, j, 
                                               DSMatrixArrayDoubleWithIndices(DSGMASystemGd(gma), i/2, term1-1, j));
                        DSMatrixSetDoubleValue(DSSSysHd(ssys), i/2, j, 
                                               DSMatrixArrayDoubleWithIndices(DSGMASystemHd(gma), i/2, term2-1, j));
                }
                for (j = 0; j < numberOfXi; j++) {
                        DSMatrixSetDoubleValue(DSSSysGi(ssys), i/2, j, 
                                               DSMatrixArrayDoubleWithIndices(DSGMASystemGi(gma), i/2, term1-1, j));
                        DSMatrixSetDoubleValue(DSSSysHi(ssys), i/2, j, 
                                               DSMatrixArrayDoubleWithIndices(DSGMASystemHi(gma), i/2, term2-1, j));
                }
        }
        if (i == 2*numberOfEquations) {
                dsSSystemSolveEquations(ssys);
        } else {
                DSSSystemFree(ssys);
                ssys = NULL;
        }
bail:
        return ssys;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Getter Methods
#endif

extern const DSUInteger DSSSystemNumberOfEquations(const DSSSystem * ssys)
{
        DSUInteger numberOfEquations = 0;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        numberOfEquations = DSVariablePoolNumberOfVariables(DSSSysXd(ssys));
bail:
        return numberOfEquations;
}

static void dsSSystemEquationAddPositiveTermToString(const DSSSystem *ssys, 
                                                       const DSUInteger equation,
                                                       char ** string, 
                                                       DSUInteger *length)
{
        DSUInteger i, numberOfXd, numberOfXi;
        DSMatrix *Gd, *Gi;
        DSMatrix *alpha;
        double value;
        char tempString[100];
        const char * name;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfXd = DSVariablePoolNumberOfVariables(DSSSysXd(ssys));
        if (equation >= numberOfXd) {
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
        numberOfXi = DSVariablePoolNumberOfVariables(DSSSysXi(ssys));
        Gd = DSSSysGd(ssys);
        Gi = DSSSysGi(ssys);
        alpha = DSSSysAlpha(ssys);
        sprintf(tempString, "%lf", DSMatrixDoubleValue(alpha, equation, 0));
        if (*length-strlen(*string) < 100) {
                *length += 1000;
                *string = DSSecureRealloc(*string, sizeof(char)**length);
        }
        strncat(*string, tempString, *length-strlen(*string));
        for (i = 0; i < numberOfXd+numberOfXi; i++) {
                if (*length-strlen(*string) < 100) {
                        *length += 1000;
                        *string = DSSecureRealloc(*string, sizeof(char)**length);
                }
                if (i < numberOfXi) {
                        name = DSVariableName(DSVariablePoolAllVariables(DSSSysXi(ssys))[i]);
                        value = DSMatrixDoubleValue(Gi, equation, i);
                        
                } else {
                        name = DSVariableName(DSVariablePoolAllVariables(DSSSysXd(ssys))[i-numberOfXi]);
                        value = DSMatrixDoubleValue(Gd, equation, i-numberOfXi);
                }
                if (value == 0.0)
                        continue;
                if (value == 1.0)
                        sprintf(tempString, "*%s", name);
                else
                        sprintf(tempString, "*%s^%lf", name, value);
                strncat(*string, tempString, *length-strlen(*string));
        }
bail:
        return;
}

static void dsSSystemEquationAddNegativeTermToString(const DSSSystem *ssys, 
                                                       const DSUInteger equation, 
                                                       char ** string, 
                                                       DSUInteger *length)
{
        DSUInteger i, numberOfXd, numberOfXi;
        DSMatrix *Hd, *Hi;
        DSMatrix *beta;
        double value;
        char tempString[100];
        const char * name;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfXd = DSVariablePoolNumberOfVariables(DSSSysXd(ssys));
        if (equation >= numberOfXd) {
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
        numberOfXi = DSVariablePoolNumberOfVariables(DSSSysXi(ssys));
        Hd = DSSSysHd(ssys);
        Hi = DSSSysHi(ssys);
        beta = DSSSysBeta(ssys);
        sprintf(tempString, "%lf", DSMatrixDoubleValue(beta, equation, 0));
        if (*length-strlen(*string) < 100) {
                *length += 1000;
                *string = DSSecureRealloc(*string, sizeof(char)**length);
        }
        strncat(*string, tempString, *length-strlen(*string));
        for (i = 0; i < numberOfXd+numberOfXi; i++) {
                if (*length-strlen(*string) < 100) {
                        *length += 1000;
                        *string = DSSecureRealloc(*string, sizeof(char)**length);
                }
                if (i < numberOfXi) {
                        name = DSVariableName(DSVariablePoolAllVariables(DSSSysXi(ssys))[i]);
                        value = DSMatrixDoubleValue(Hi, equation, i);
                        
                } else {
                        name = DSVariableName(DSVariablePoolAllVariables(DSSSysXd(ssys))[i-numberOfXi]);
                        value = DSMatrixDoubleValue(Hd, equation, i-numberOfXi);
                }
                if (value == 0.0)
                        continue;
                if (value == 1.0)
                        sprintf(tempString, "*%s", name);
                else
                        sprintf(tempString, "*%s^%lf", name, value);
                strncat(*string, tempString, *length-strlen(*string));
        }
bail:
        return;
}


extern DSExpression ** DSSSystemEquations(const DSSSystem *ssys)
{
        DSUInteger i, numberOfEquations, length;
        DSExpression ** equations = NULL;
        char *tempString, *equationString, *varName;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfEquations = DSSSystemNumberOfEquations(ssys);
        if (numberOfEquations == 0) {
                DSError("S-System being accessed has no equations", A_DS_ERROR);
                goto bail;
        }
        equations = DSSecureCalloc(sizeof(DSExpression *), numberOfEquations);
        length = 1000;
        tempString = DSSecureCalloc(sizeof(char), length);
        for (i = 0; i < numberOfEquations; i++) {
                tempString[0] = '\0';
                varName = DSVariableName(DSVariablePoolVariableAtIndex(DSSSystemXd(ssys), i));
                dsSSystemEquationAddPositiveTermToString(ssys, i, &tempString, &length);
                strncat(tempString, "-", length-strlen(tempString));
                dsSSystemEquationAddNegativeTermToString(ssys, i, &tempString, &length);
                equationString = DSSecureCalloc(sizeof(char),
                                                strlen(tempString)+strlen(varName)+6);
                // Check if varName is algebraic
                if (DSVariablePoolHasVariableWithName(DSSSysXd_a(ssys), varName) == false) {
                        equationString = strcpy(equationString, varName);
                        equationString = strcat(equationString, ". = ");
                } else {
                        equationString = strcpy(equationString, "0 = ");
                }
                equationString = strcat(equationString, tempString);
                equations[i] = DSExpressionByParsingString(equationString);
                DSSecureFree(equationString);
        }
        DSSecureFree(tempString);
bail:
        return equations;
}

static void dsSSystemSolutionToString(const DSSSystem *ssys, 
                                      const DSUInteger equation, 
                                      char ** string, 
                                      DSUInteger *length, const bool inLog)
{
        DSUInteger i, numberOfXd, numberOfXi;
        DSMatrix *MAi, *MB, *B, *Ai;
        char tempString[100] = "\0";
        const char *name;
        double value;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfXd = DSVariablePoolNumberOfVariables(DSSSysXd(ssys));
        if (equation >= numberOfXd) {
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
        if (DSSSystemHasSolution(ssys) == false) {
                goto bail;
        }
        numberOfXi = DSVariablePoolNumberOfVariables(DSSSysXi(ssys));
        B = DSSSystemB(ssys);
        Ai = DSSSystemAi(ssys);
        if (numberOfXi != 0) {
                MAi = DSMatrixByMultiplyingMatrix(DSSSysM(ssys), Ai);
                DSMatrixFree(Ai);
        }
        MB = DSMatrixByMultiplyingMatrix(DSSSysM(ssys), B);
        DSMatrixFree(B);
        if (inLog == true) 
                sprintf(tempString, "%lf", DSMatrixDoubleValue(MB, equation, 0));
        else
                sprintf(tempString, "10^%lf", DSMatrixDoubleValue(MB, equation, 0));
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
                name = DSVariableName(DSVariablePoolAllVariables(DSSSysXi(ssys))[i]);
                value = -DSMatrixDoubleValue(MAi, equation, i);
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
        if (numberOfXi != 0)
                DSMatrixFree(MAi);
        DSMatrixFree(MB);
bail:
        return;
}

extern DSExpression ** DSSSystemSolution(const DSSSystem *ssys)
{
        DSUInteger i, numberOfEquations, length;
        DSExpression ** solution = NULL;
        char *tempString, * equationString, *varName;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfEquations = DSSSystemNumberOfEquations(ssys);
        if (numberOfEquations == 0) {
                DSError("S-System being accessed has no equations", A_DS_ERROR);
                goto bail;
        }
        if (DSSSystemHasSolution(ssys) == false) {
                goto bail;
        }
        solution = DSSecureCalloc(sizeof(DSExpression *), numberOfEquations);
        length = 1000;
        tempString = DSSecureCalloc(sizeof(char), length);
        for (i = 0; i < numberOfEquations; i++) {
                tempString[0] = '\0';
                dsSSystemSolutionToString(ssys, i, &tempString, &length, false);
                if (strlen(tempString) == 0)
                        break;
                varName = DSVariableName(DSVariablePoolVariableAtIndex(DSSSystemXd(ssys), i));
                equationString = DSSecureCalloc(
                                                sizeof(char),
                                                strlen(tempString)+strlen(varName)+4);
                equationString = strcpy(equationString, varName);
                equationString = strcat(equationString, " = ");
                equationString = strcat(equationString, tempString);
                solution[i] = DSExpressionByParsingString(equationString);
                DSSecureFree(equationString);
        }
        DSSecureFree(tempString);
bail:
        return solution;
}

extern DSExpression ** DSSSystemLogarithmicSolution(const DSSSystem *ssys)
{
        DSUInteger i, numberOfEquations, length;
        DSExpression ** solution = NULL;
        char *tempString;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfEquations = DSSSystemNumberOfEquations(ssys);
        if (numberOfEquations == 0) {
                DSError("S-System being accessed has no equations", A_DS_ERROR);
                goto bail;
        }
        if (DSSSystemHasSolution(ssys) == false) {
                goto bail;
        }
        solution = DSSecureCalloc(sizeof(DSExpression *), numberOfEquations);
        length = 1000;
        tempString = DSSecureCalloc(sizeof(char), length);
        for (i = 0; i < numberOfEquations; i++) {
                tempString[0] = '\0';
                dsSSystemSolutionToString(ssys, i, &tempString, &length, true);
                if (strlen(tempString) == 0)
                        break;
                solution[i] = DSExpressionByParsingString(tempString);
        }
        DSSecureFree(tempString);
bail:
        return solution;
}

extern const DSMatrix * DSSSystemAlpha(const DSSSystem * ssys)
{
        DSMatrix *matrix = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        matrix = DSSSysAlpha(ssys);
bail:
        return matrix;
}

extern const DSMatrix * DSSSystemBeta(const DSSSystem * ssys)
{
        DSMatrix *matrix = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        matrix = DSSSysBeta(ssys);
bail:
        return matrix;
}

extern const DSMatrix * DSSSystemGd(const DSSSystem * ssys)
{
        DSMatrix *matrix = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        matrix = DSSSysGd(ssys);
bail:
        return matrix;
}

extern const DSMatrix * DSSSystemGi(const DSSSystem * ssys)
{
        DSMatrix *matrix = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        matrix = DSSSysGi(ssys);
bail:
        return matrix;
}

extern const DSMatrix * DSSSystemHd(const DSSSystem * ssys)
{
        DSMatrix *matrix = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        matrix = DSSSysHd(ssys);
bail:
        return matrix;
}

extern const DSMatrix * DSSSystemHi(const DSSSystem * ssys)
{
        DSMatrix *matrix = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        matrix = DSSSysHi(ssys);
bail:
        return matrix;
}

extern const DSVariablePool * DSSSystemXd(const DSSSystem * ssys)
{
        DSVariablePool *pool = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        pool = DSSSysXd(ssys);
bail:
        return pool;
}

extern const DSVariablePool * DSSSystemXd_a(const DSSSystem * ssys)
{
        DSVariablePool *pool = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        pool = DSSSysXd_a(ssys);
bail:
        return pool;
}

extern const DSVariablePool * DSSSystemXd_t(const DSSSystem * const ssys)
{
        DSVariablePool *pool = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        pool = DSSSysXd_t(ssys);
bail:
        return pool;
}

extern const DSVariablePool * DSSSystemXi(const DSSSystem * ssys)
{
        DSVariablePool *pool = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        pool = DSSSysXi(ssys);
bail:
        return pool;
}

extern const DSMatrix * DSSSystemM(const DSSSystem * ssys)
{
        DSMatrix *M = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        M = DSSSysM(ssys);
bail:
        return M;
}

extern DSMatrix * DSSSystemM_a(const DSSSystem * ssys)
{
        DSMatrix *Qd_a, *M_a = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        Qd_a = DSSSystemQd_a(ssys);
        M_a = DSMatrixInverse(Qd_a);
bail:
        return M_a;
}

extern DSMatrix * DSSSystemAd(const DSSSystem * ssys)
{
        DSMatrix *Ad = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(DSSSysXd(ssys)) == 0)
                goto bail;
        if (DSSSysGd(ssys) == NULL || DSSSysHd(ssys) == NULL) {
                DSError(M_DS_MAT_NULL ": Gd/hd matrix is null", A_DS_ERROR);
                goto bail;
        }
        Ad = DSMatrixBySubstractingMatrix(DSSSysGd(ssys), DSSSysHd(ssys));
bail:
        return Ad;
}

extern DSMatrix * DSSSystemQd_a(const DSSSystem * ssys)
{
        DSMatrix *Ad_a = NULL;
        DSMatrix *Gd_a = NULL, *Hd_a = NULL;
        DSUInteger i, index, numberOfAuxiliary, * auxiliary_indices = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        numberOfAuxiliary =DSVariablePoolNumberOfVariables(DSSSysXd_a(ssys));
        if (numberOfAuxiliary == 0)
                goto bail;
        auxiliary_indices = DSSecureCalloc(sizeof(DSUInteger *), numberOfAuxiliary);
        index = 0;
        for (i = 0; i < DSVariablePoolNumberOfVariables(DSSSysXd(ssys)); i++) {
                if (DSVariablePoolHasVariableWithName(DSSSysXd_a(ssys), DSVariableName(DSVariablePoolVariableAtIndex(DSSSysXd(ssys), i)))==true)
                        auxiliary_indices[index++] = i;
        }
        
        Gd_a = DSMatrixSubMatrixIncludingRowsAndColumns(DSSSysGd(ssys), numberOfAuxiliary, numberOfAuxiliary, auxiliary_indices, auxiliary_indices);
        Hd_a = DSMatrixSubMatrixIncludingRowsAndColumns(DSSSysHd(ssys), numberOfAuxiliary, numberOfAuxiliary, auxiliary_indices, auxiliary_indices);
        if (Gd_a == NULL || Hd_a == NULL) {
                DSError(M_DS_MAT_NULL ": Gd/hd matrix is null", A_DS_ERROR);
                goto bail;
        }
        Ad_a = DSMatrixBySubstractingMatrix(Gd_a, Hd_a);
bail:
        if (Gd_a != NULL)
                DSMatrixFree(Gd_a);
        if (Hd_a != NULL)
                DSMatrixFree(Hd_a);
        if (auxiliary_indices != NULL)
                DSSecureFree(auxiliary_indices);
        return Ad_a;
}

extern DSMatrix * DSSSystemQd_t(const DSSSystem * ssys)
{
        DSMatrix *Ad_a = NULL;
        DSMatrix *Gd_a = NULL, *Hd_a = NULL;
        DSUInteger i, index_a, index_t, numberOfAuxiliary, numberOfTime, * auxiliary_indices = NULL, *time_indices = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        numberOfAuxiliary =DSVariablePoolNumberOfVariables(DSSSysXd_a(ssys));
        numberOfTime =DSVariablePoolNumberOfVariables(DSSSysXd_t(ssys));
        if (numberOfAuxiliary == 0)
                goto bail;
        if (numberOfTime == 0)
                goto bail;
        auxiliary_indices = DSSecureCalloc(sizeof(DSUInteger *), numberOfAuxiliary);
        time_indices = DSSecureCalloc(sizeof(DSUInteger *), numberOfTime);
        index_a = 0;
        index_t = 0;
        for (i = 0; i < DSVariablePoolNumberOfVariables(DSSSysXd(ssys)); i++) {
                if (DSVariablePoolHasVariableWithName(DSSSysXd_a(ssys), DSVariableName(DSVariablePoolVariableAtIndex(DSSSysXd(ssys), i)))==true)
                        auxiliary_indices[index_a++] = i;
                else
                        time_indices[index_t++] = i;
        }
        
        Gd_a = DSMatrixSubMatrixIncludingRowsAndColumns(DSSSysGd(ssys), numberOfAuxiliary, numberOfTime, auxiliary_indices, time_indices);
        Hd_a = DSMatrixSubMatrixIncludingRowsAndColumns(DSSSysHd(ssys), numberOfAuxiliary, numberOfTime, auxiliary_indices, time_indices);
        if (Gd_a == NULL || Hd_a == NULL) {
                DSError(M_DS_MAT_NULL ": Gd/hd matrix is null", A_DS_ERROR);
                goto bail;
        }
        Ad_a = DSMatrixBySubstractingMatrix(Gd_a, Hd_a);
bail:
        if (Gd_a != NULL)
                DSMatrixFree(Gd_a);
        if (Hd_a != NULL)
                DSMatrixFree(Hd_a);
        if (auxiliary_indices != NULL)
                DSSecureFree(auxiliary_indices);
        if (time_indices != NULL)
                DSSecureFree(time_indices);
        return Ad_a;
}

extern DSMatrix * DSSSystemQi_a(const DSSSystem * ssys)
{
        DSMatrix *Ai_a = NULL;
        DSMatrix *Gi_a = NULL, *Hi_a = NULL;
        DSUInteger i, index, numberOfAuxiliary, * auxiliary_indices = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        numberOfAuxiliary =DSVariablePoolNumberOfVariables(DSSSysXd_a(ssys));
        if (numberOfAuxiliary == 0)
                goto bail;
        auxiliary_indices = DSSecureCalloc(sizeof(DSUInteger *), numberOfAuxiliary);
        index = 0;
        for (i = 0; i < DSVariablePoolNumberOfVariables(DSSSysXd(ssys)); i++) {
                if (DSVariablePoolHasVariableWithName(DSSSysXd_a(ssys), DSVariableName(DSVariablePoolVariableAtIndex(DSSSysXd(ssys), i)))==true)
                        auxiliary_indices[index++] = i;
        }
        Gi_a = DSMatrixSubMatrixIncludingRows(DSSSysGi(ssys), numberOfAuxiliary, auxiliary_indices);
        Hi_a = DSMatrixSubMatrixIncludingRows(DSSSysHi(ssys), numberOfAuxiliary, auxiliary_indices);
        if (Gi_a == NULL || Hi_a == NULL) {
                DSError(M_DS_MAT_NULL ": Gd/hd matrix is null", A_DS_ERROR);
                goto bail;
        }
        Ai_a = DSMatrixBySubstractingMatrix(Gi_a, Hi_a);
bail:
        if (Gi_a != NULL)
                DSMatrixFree(Gi_a);
        if (Hi_a != NULL)
                DSMatrixFree(Hi_a);
        if (auxiliary_indices != NULL)
                DSSecureFree(auxiliary_indices);
        return Ai_a;
}

extern DSMatrix * DSSSystemQB_a(const DSSSystem * ssys)
{
        DSMatrix *B_a = NULL, *B;
        DSUInteger i, index, numberOfAuxiliary, * auxiliary_indices = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        numberOfAuxiliary =DSVariablePoolNumberOfVariables(DSSSysXd_a(ssys));
        if (numberOfAuxiliary == 0)
                goto bail;
        auxiliary_indices = DSSecureCalloc(sizeof(DSUInteger *), numberOfAuxiliary);
        index = 0;
        for (i = 0; i < DSVariablePoolNumberOfVariables(DSSSysXd(ssys)); i++) {
                if (DSVariablePoolHasVariableWithName(DSSSysXd_a(ssys), DSVariableName(DSVariablePoolVariableAtIndex(DSSSysXd(ssys), i)))==true)
                        auxiliary_indices[index++] = i;
        }
        B = DSSSystemB(ssys);
        B_a = DSMatrixSubMatrixIncludingRows(B, numberOfAuxiliary, auxiliary_indices);
        if (B_a == NULL) {
                DSError(M_DS_MAT_NULL ": B matrix is null", A_DS_ERROR);
                goto bail;
        }
        DSMatrixFree(B);
bail:
        if (auxiliary_indices != NULL)
                DSSecureFree(auxiliary_indices);
        return B_a;
}

extern DSMatrix * DSSSystemAd_a(const DSSSystem * ssys)
{
        DSMatrix *Ad_a = NULL;
        DSMatrix *Gd_a = NULL, *Hd_a = NULL;
        DSUInteger i, index, numberOfAuxiliary, * auxiliary_indices = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        numberOfAuxiliary =DSVariablePoolNumberOfVariables(DSSSysXd_a(ssys));
        if (numberOfAuxiliary == 0)
                goto bail;
        auxiliary_indices = DSSecureCalloc(sizeof(DSUInteger *), numberOfAuxiliary);
        index = 0;
        for (i = 0; i < DSVariablePoolNumberOfVariables(DSSSysXd(ssys)); i++) {
                if (DSVariablePoolHasVariableWithName(DSSSysXd_a(ssys), DSVariableName(DSVariablePoolVariableAtIndex(DSSSysXd(ssys), i)))==true)
                        auxiliary_indices[index++] = i;
        }
        
        Gd_a = DSMatrixSubMatrixIncludingColumns(DSSSysGd(ssys), numberOfAuxiliary, auxiliary_indices);
        Hd_a = DSMatrixSubMatrixIncludingColumns(DSSSysHd(ssys), numberOfAuxiliary, auxiliary_indices);
        if (Gd_a == NULL || Hd_a == NULL) {
                DSError(M_DS_MAT_NULL ": Gd/hd matrix is null", A_DS_ERROR);
                goto bail;
        }
        Ad_a = DSMatrixBySubstractingMatrix(Gd_a, Hd_a);
bail:
        if (Gd_a != NULL)
                DSMatrixFree(Gd_a);
        if (Hd_a != NULL)
                DSMatrixFree(Hd_a);
        if (auxiliary_indices != NULL)
                DSSecureFree(auxiliary_indices);
        return Ad_a;
}

extern DSMatrix * DSSSystemAd_t(const DSSSystem * ssys)
{
        DSMatrix *Ad_t = NULL;
        DSMatrix *Gd_t = NULL, *Hd_t = NULL;
        DSUInteger i, index, numberOfAuxiliary, * auxiliary_indices = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        numberOfAuxiliary =DSVariablePoolNumberOfVariables(DSSSysXd_a(ssys));
        if (numberOfAuxiliary == 0)
                goto bail;
        auxiliary_indices = DSSecureCalloc(sizeof(DSUInteger *), numberOfAuxiliary);
        index = 0;
        for (i = 0; i < DSVariablePoolNumberOfVariables(DSSSysXd(ssys)); i++) {
                if (DSVariablePoolHasVariableWithName(DSSSysXd_a(ssys), DSVariableName(DSVariablePoolVariableAtIndex(DSSSysXd(ssys), i)))==true)
                        auxiliary_indices[index++] = i;
        }
        Gd_t = DSMatrixSubMatrixExcludingColumns(DSSSysGd(ssys), numberOfAuxiliary, auxiliary_indices);
        Hd_t = DSMatrixSubMatrixExcludingColumns(DSSSysHd(ssys), numberOfAuxiliary, auxiliary_indices);
        if (Gd_t == NULL || Hd_t == NULL) {
                DSError(M_DS_MAT_NULL ": Gd/hd matrix is null", A_DS_ERROR);
                goto bail;
        }
        Ad_t = DSMatrixBySubstractingMatrix(Gd_t, Hd_t);
bail:
        if (Gd_t != NULL)
                DSMatrixFree(Gd_t);
        if (Hd_t != NULL)
                DSMatrixFree(Hd_t);
        if (auxiliary_indices != NULL)
                DSSecureFree(auxiliary_indices);
        return Ad_t;
}

extern DSMatrix * DSSSystemAi(const DSSSystem * ssys)
{
        DSMatrix *Ai = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) == 0)
                goto bail;
        if (DSSSysGi(ssys) == NULL || DSSSysHi(ssys) == NULL) {
                DSError(M_DS_MAT_NULL ": Gi/hi matrix is null", A_DS_ERROR);
                goto bail;
        }
        Ai = DSMatrixBySubstractingMatrix(DSSSysGi(ssys), DSSSysHi(ssys));
bail:
        return Ai;
}

extern DSMatrix * DSSSystemB(const DSSSystem * ssys)
{
        DSMatrix *B = NULL;
        DSUInteger i;
        if (ssys == NULL) {
                DSError(M_DS_MAT_NULL ": B is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSSSysAlpha(ssys) == NULL || DSSSysBeta(ssys) == NULL) {
                DSError(M_DS_MAT_NULL ": Alpha/beta matrix is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSMatrixRows(DSSSysAlpha(ssys)) != DSMatrixRows(DSSSysBeta(ssys))) {
                DSError(M_DS_WRONG ": S-System alpha/beta matrix rows do not match", A_DS_ERROR);
                goto bail;
        }
        B = DSMatrixAlloc(DSMatrixRows(DSSSysBeta(ssys)), 1);
        for (i = 0; i < DSMatrixRows(B); i++) {
                DSMatrixSetDoubleValue(B, i, 0,
                                       log10(DSMatrixDoubleValue(DSSSysBeta(ssys), i, 0)/DSMatrixDoubleValue(DSSSysAlpha(ssys), i, 0)));
        }
bail:
        return B;
}

extern DSMatrix * DSSSystemA(const DSSSystem * ssys)
{
        DSMatrix *A = NULL, *G, *H;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        G = DSSSystemG(ssys);
        H = DSSSystemH(ssys);
        A = DSMatrixBySubstractingMatrix(G, H);
        DSMatrixFree(G);
        DSMatrixFree(H);
bail:
        return A;       
}

extern DSMatrix * DSSSystemG(const DSSSystem *ssys)
{
        DSMatrix *G = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(DSSSystemXi(ssys)) != 0)
                G = DSMatrixAppendMatrices(DSSSysGd(ssys), DSSSysGi(ssys), true);
        else
                G = DSMatrixCopy(DSSSysGd(ssys));
bail:
        return G;
}

extern DSMatrix * DSSSystemH(const DSSSystem *ssys)
{
        DSMatrix *H = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(DSSSystemXi(ssys)) != 0)
                H = DSMatrixAppendMatrices(DSSSysHd(ssys), DSSSysHi(ssys), true);
        else
                H = DSMatrixCopy(DSSSysHd(ssys));
bail:
        return H;        
}


extern const bool DSSSystemHasSolution(const DSSSystem * ssys)
{
        bool hasSolution = false;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSSSysM(ssys) != NULL)
                hasSolution = true;
bail:
        return hasSolution;
}

extern const bool DSSSystemIsSingular(const DSSSystem *ssys)
{
        bool isSigular = false;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        isSigular = DSSSysIsSingular(ssys);
bail:
        return isSigular;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - S-System functions
#endif

extern DSMatrix * DSSSystemSteadyStateValues(const DSSSystem *ssys, const DSVariablePool *Xi0)
{
        DSMatrix * steadyState = NULL;
        DSMatrix *Xi = NULL;
        DSMatrix *MAi, *MAiXi, *B = NULL, *Ai = NULL;
        DSVariablePool *pool = NULL;
        DSUInteger i;
        const char *name;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (Xi0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xi0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSSSystemHasSolution(ssys) == false)
                goto bail;
        B = DSSSystemB(ssys);
        steadyState = DSMatrixByMultiplyingMatrix(DSSSysM(ssys), B);
        if (DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) != 0) {
                pool = DSVariablePoolAlloc();
                for (i=0; i < DSVariablePoolNumberOfVariables(DSSSysXi(ssys)); i++) {
                        name =  DSVariableName(DSVariablePoolAllVariables(DSSSysXi(ssys))[i]);
                        DSVariablePoolAddVariableWithName(pool, name);
                        if (DSVariablePoolHasVariableWithName(Xi0, name) == false) {
                                        DSMatrixFree(steadyState);
                                steadyState = NULL;
                                goto bail;
                        }
                        DSVariablePoolSetValueForVariableWithName(pool,  name, DSVariableValue(DSVariablePoolVariableWithName(Xi0, name)));
                }
                Ai = DSSSystemAi(ssys);
                Xi = DSVariablePoolValuesAsVector(pool, false);
                DSMatrixApplyFunction(Xi, log10);
                MAi = DSMatrixByMultiplyingMatrix(DSSSysM(ssys), Ai);
                MAiXi = DSMatrixByMultiplyingMatrix(MAi, Xi);
                DSMatrixSubstractByMatrix(steadyState, MAiXi);
                DSMatrixFree(Ai);
                DSMatrixFree(Xi);
                DSMatrixFree(MAi);
                DSMatrixFree(MAiXi);
                DSVariablePoolFree(pool);
        }
bail:
        if (B != NULL)
                DSMatrixFree(B);
        return steadyState;
}

extern DSMatrix * DSSSystemAuxiliaryVariablesForSteadyState(const DSSSystem *ssys, const DSVariablePool *Xdt0, const DSVariablePool *Xi0)
{
        DSMatrix * auxSolution = NULL;
        DSMatrix *Yi = NULL, *Yd_t = NULL;
        DSMatrix *M = NULL, *MA, *MAY, *B = NULL, *Ai = NULL, *Ad_t = NULL;
        DSVariablePool *pool = NULL;
        DSUInteger i;
        const char *name;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (Xi0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xi0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (Xdt0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXd_t(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xd_t0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        B = DSSSystemQB_a(ssys);
        M = DSSSystemM_a(ssys);
        auxSolution = DSMatrixByMultiplyingMatrix(M, B);
        DSMatrixFree(B);
        if (DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) != 0) {
                pool = DSVariablePoolAlloc();
                for (i=0; i < DSVariablePoolNumberOfVariables(DSSSysXi(ssys)); i++) {
                        name =  DSVariableName(DSVariablePoolAllVariables(DSSSysXi(ssys))[i]);
                        DSVariablePoolAddVariableWithName(pool, name);
                        if (DSVariablePoolHasVariableWithName(Xi0, name) == false) {
                                DSMatrixFree(auxSolution);
                                auxSolution = NULL;
                                goto bail;
                        }
                        DSVariablePoolSetValueForVariableWithName(pool,  name, DSVariableValue(DSVariablePoolVariableWithName(Xi0, name)));
                }
                Yi = DSVariablePoolValuesAsVector(pool, false);
                DSMatrixApplyFunction(Yi, log10);
                Ai = DSSSystemQi_a(ssys);
                MA = DSMatrixByMultiplyingMatrix(M, Ai);
                MAY = DSMatrixByMultiplyingMatrix(MA, Yi);
                DSMatrixSubstractByMatrix(auxSolution, MAY);
                DSMatrixFree(Yi);
                DSMatrixFree(Ai);
                DSMatrixFree(MA);
                DSMatrixFree(MAY);
                DSVariablePoolFree(pool);
        }
        if (DSVariablePoolNumberOfVariables(DSSSysXd_t(ssys)) != 0) {
                pool = DSVariablePoolAlloc();
                for (i=0; i < DSVariablePoolNumberOfVariables(DSSSysXd_t(ssys)); i++) {
                        name =  DSVariableName(DSVariablePoolAllVariables(DSSSysXd_t(ssys))[i]);
                        DSVariablePoolAddVariableWithName(pool, name);
                        if (DSVariablePoolHasVariableWithName(Xdt0, name) == false) {
                                DSMatrixFree(auxSolution);
                                auxSolution = NULL;
                                goto bail;
                        }
                        DSVariablePoolSetValueForVariableWithName(pool,  name, DSVariableValue(DSVariablePoolVariableWithName(Xdt0, name)));
                }
                Yd_t = DSVariablePoolValuesAsVector(pool, false);
                DSMatrixApplyFunction(Yd_t, log10);
                Ad_t = DSSSystemQd_t(ssys);
                MA = DSMatrixByMultiplyingMatrix(M, Ad_t);
                MAY = DSMatrixByMultiplyingMatrix(MA, Yd_t);
                DSMatrixSubstractByMatrix(auxSolution, MAY);
                DSMatrixFree(Yd_t);
                DSMatrixFree(Ad_t);
                DSMatrixFree(MA);
                DSMatrixFree(MAY);
                DSVariablePoolFree(pool);
        }
bail:
        if (M != NULL)
                DSMatrixFree(M);
        return auxSolution;
}

extern double DSSSystemSteadyStateFunction(const DSSSystem *ssys, const DSVariablePool *Xi0, const char * function)
{
        DSMatrix *ss=NULL;
        DSUInteger i;
        DSVariablePool *pool = NULL;
        const char *name;
        double value = NAN;
        DSExpression *expr = NULL;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (Xi0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xi0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSSSystemHasSolution(ssys) == false)
                goto bail;
        ss = DSSSystemSteadyStateValues(ssys, Xi0);
        if (ss == NULL)
                goto bail;
        pool = DSVariablePoolAlloc();
        for (i = 0; i < DSVariablePoolNumberOfVariables(DSSSysXd(ssys)); i++) {
                name = DSVariableName(DSVariablePoolAllVariables(DSSSysXd(ssys))[i]);
                DSVariablePoolAddVariableWithName(pool, name);
                DSVariablePoolSetValueForVariableWithName(pool, name, pow(10, DSMatrixDoubleValue(ss, i, 0)));
        }
        for (i = 0; i < DSVariablePoolNumberOfVariables(Xi0); i++) {
                name = DSVariableName(DSVariablePoolAllVariables(Xi0)[i]);
                DSVariablePoolAddVariableWithName(pool, name);
                DSVariablePoolSetValueForVariableWithName(pool, name, DSVariableValue(DSVariablePoolAllVariables(Xi0)[i]));
        }
        expr = DSExpressionByParsingString(function);
        if (expr != NULL)
                value = DSExpressionEvaluateWithVariablePool(expr, pool);
bail:
        if (ss != NULL)
                DSMatrixFree(ss);
        if (pool != NULL)
                DSVariablePoolFree(pool);
        if (expr != NULL)
                DSExpressionFree(expr);
        return value;   
}

extern DSMatrix * DSSSystemSteadyStateFluxForDependentVariables(const DSSSystem * ssys,
                                                                const DSVariablePool * Xd0,
                                                                const DSVariablePool * Xi0)
{
        DSMatrix * flux = NULL, *Xi = NULL, *ss=NULL;
        DSMatrix * gi0= NULL, *alpha = NULL;
        DSUInteger i;
        DSVariablePool *pool = NULL;
        const char *name;
        double value;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (Xd0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXd(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xd0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (Xi0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xi0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        alpha = DSMatrixCopy(DSSSystemAlpha(ssys));
        DSMatrixApplyFunction(alpha, log10);
        ss = DSVariablePoolValuesAsVector(Xd0, false);
        DSMatrixApplyFunction(ss, log10);
        flux = DSMatrixByMultiplyingMatrix(DSSSystemGd(ssys), ss);
        if (DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) != 0) {
                pool = DSVariablePoolAlloc();
                for (i=0; i < DSVariablePoolNumberOfVariables(DSSSysXi(ssys)); i++) {
                        name =  DSVariableName(DSVariablePoolAllVariables(DSSSysXi(ssys))[i]);
                        DSVariablePoolAddVariableWithName(pool, name);;
                        if (DSVariablePoolHasVariableWithName(Xi0, name) == false) {
                                DSError(M_DS_WRONG ": Variable Pool does not have independent variable", A_DS_ERROR);
                                DSMatrixFree(flux);
                                flux = NULL;
                                goto bail;
                        }
                        value = DSVariableValue(DSVariablePoolVariableWithName(Xi0, name));
                        DSVariablePoolSetValueForVariableWithName(pool, name, value);
                }
                Xi = DSVariablePoolValuesAsVector(pool, false);
                DSMatrixApplyFunction(Xi, log10);
                gi0 = DSMatrixByMultiplyingMatrix(DSSSystemGi(ssys), Xi);
                DSMatrixAddByMatrix(flux, gi0);
                DSMatrixFree(Xi);
                DSMatrixFree(gi0);
        }
        DSMatrixAddByMatrix(flux, alpha);
bail:
        if (alpha != NULL)
                DSMatrixFree(alpha);
        if (ss != NULL)
                DSMatrixFree(ss);
        if (pool != NULL)
                DSVariablePoolFree(pool);
        return flux;
}
extern DSMatrix * DSSSystemSteadyStateFlux(const DSSSystem *ssys, const DSVariablePool *Xi0)
{
        DSMatrix * flux = NULL, *Xi = NULL, *ss=NULL;
        DSMatrix * gi0= NULL, *alpha = NULL;
        DSUInteger i;
        DSVariablePool *pool = NULL;
        const char *name;
        double value;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (Xi0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xi0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSSSystemHasSolution(ssys) == false)
                goto bail;
        alpha = DSMatrixCopy(DSSSystemAlpha(ssys));
        DSMatrixApplyFunction(alpha, log10);
        ss = DSSSystemSteadyStateValues(ssys, Xi0);
        flux = DSMatrixByMultiplyingMatrix(DSSSystemGd(ssys), ss);
        if (DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) != 0) {
                pool = DSVariablePoolAlloc();
                for (i=0; i < DSVariablePoolNumberOfVariables(DSSSysXi(ssys)); i++) {
                        name =  DSVariableName(DSVariablePoolAllVariables(DSSSysXi(ssys))[i]);
                        DSVariablePoolAddVariableWithName(pool, name);;
                        if (DSVariablePoolHasVariableWithName(Xi0, name) == false) {
                                DSError(M_DS_WRONG ": Variable Pool does not have independent variable", A_DS_ERROR);
                                DSMatrixFree(flux);
                                flux = NULL;
                                goto bail;
                        }
                        value = DSVariableValue(DSVariablePoolVariableWithName(Xi0, name));
                        DSVariablePoolSetValueForVariableWithName(pool, name, value);
                }
                Xi = DSVariablePoolValuesAsVector(pool, false);
                DSMatrixApplyFunction(Xi, log10);
                gi0 = DSMatrixByMultiplyingMatrix(DSSSystemGi(ssys), Xi);
                DSMatrixAddByMatrix(flux, gi0);
                DSMatrixFree(Xi);
                DSMatrixFree(gi0);
        }
        DSMatrixAddByMatrix(flux, alpha);
bail:
        if (alpha != NULL)
                DSMatrixFree(alpha);
        if (ss != NULL)
                DSMatrixFree(ss);
        if (pool != NULL)
                DSVariablePoolFree(pool);
        return flux;   
}

static void dsSSystemRouthArrayProcessZeroRoots(DSMatrix * routhMatrix, const DSUInteger row, const double threshold)
{
        DSUInteger i, order, newRowSize;
        bool rowEmpty;
        double value;
        if (routhMatrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (row >= DSMatrixRows(routhMatrix)) {
                DSError(M_DS_MAT_OUTOFBOUNDS, A_DS_ERROR);
                goto bail;
        }
        rowEmpty = true;
        for (i = 0; i < DSMatrixColumns(routhMatrix); i++) {
                if (fabs(DSMatrixDoubleValue(routhMatrix, row, i)) >= threshold) {
                        rowEmpty = false;
                }
        }
        if (rowEmpty == false) {
                DSMatrixSetDoubleValue(routhMatrix, row, 0, threshold);
                goto bail;
        }
        order = DSMatrixRows(routhMatrix)-row;
        newRowSize = order/2 + order % 2;
        for (i = 0; i < newRowSize; i++) {
                value = order*DSMatrixDoubleValue(routhMatrix, row-1, i);
                DSMatrixSetDoubleValue(routhMatrix, row, i, value);
                order -= 2;
        }
bail:
        return;
}

extern DSMatrix * DSSSystemRouthArrayForPoolTurnover(const DSSSystem *ssys, const DSMatrix * F, bool * hasImaginaryRoots)
{
        DSSSystem * reduced = NULL;
        DSMatrix * FA = NULL;
        DSMatrix * routhArray = NULL;
        DSMatrix * phi = NULL;
        DSMatrix * routhMatrix = NULL;
        DSUInteger i, j;
        double value;
        double threshold = 1e-8;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (F == NULL) {
                DSError(M_DS_MAT_NULL ": F matrix is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(DSSSystemXd(ssys)) > 0) {
                reduced = DSSSystemByRemovingAlgebraicConstraints(ssys);
                ssys = reduced;
        }
        if (DSMatrixRows(F) != DSSSystemNumberOfEquations(ssys)) {
                DSError(M_DS_MAT_OUTOFBOUNDS, A_DS_ERROR);
                goto bail;
        }
        FA = DSMatrixByMultiplyingMatrix(F, DSSSystemAd(ssys));
        phi = DSMatrixCharacteristicPolynomialCoefficients(FA);
        DSMatrixFree(FA);
        routhMatrix = DSMatrixCalloc(DSMatrixColumns(phi), DSMatrixColumns(phi));
//        routhArray = DSMatrixCalloc(DSMatrixRows(routhMatrix), 1);
        /* Make first row of routh matrix */
        for (i = 0; i < DSMatrixColumns(routhMatrix); i++) {
                value = 0.0f;
                if (2*i < DSMatrixColumns(phi))
                        value = DSMatrixDoubleValue(phi, 0, 2*i);
                DSMatrixSetDoubleValue(routhMatrix, 0, i, value);
        }
        for (i = 0; i < DSMatrixColumns(routhMatrix); i++) {
                value = 0.0f;
                if ((2*i)+1 < DSMatrixColumns(phi))
                        value = DSMatrixDoubleValue(phi, 0, (2*i)+1);
                DSMatrixSetDoubleValue(routhMatrix, 1, i, value);
        }
        if (hasImaginaryRoots != NULL) {
                *hasImaginaryRoots = false;
        }
        for (i = 2; i < DSMatrixRows(routhMatrix); i++) {
                for (j = 0; j < DSMatrixColumns(routhMatrix); j++) {
                        if (j == DSMatrixColumns(routhMatrix)-1) {
                                DSMatrixSetDoubleValue(routhMatrix, i, j, 0.0f);
                                continue;
                        }
                        value = DSMatrixDoubleValue(routhMatrix, i-1, 0);
                        value = (value*DSMatrixDoubleValue(routhMatrix, i-2, j+1)-DSMatrixDoubleValue(routhMatrix, i-2, 0)*DSMatrixDoubleValue(routhMatrix, i-1, j+1))/value;
                        if (fabs(value) < threshold)
                                value = 0.f;
                        DSMatrixSetDoubleValue(routhMatrix, i, j, value);
                        if (value == 0.f && j == 0) {
                                dsSSystemRouthArrayProcessZeroRoots(routhMatrix, i, threshold);
                                if (hasImaginaryRoots != NULL) {
                                        *hasImaginaryRoots = true;
                                }
                        }
//                        if (j == 0) {
//                                DSMatrixSetDoubleValue(routhArray, i, 0, DSMatrixDoubleValue(routhMatrix, i, j));
//                        }
                }
        }
        routhArray = DSMatrixSubMatrixIncludingColumnList(routhMatrix, 1, 0);
        DSMatrixFree(routhMatrix);
        DSMatrixFree(phi);
bail:
        if (reduced != NULL) {
                DSSSystemFree(reduced);
        }
        return routhArray;
}

extern DSMatrix * DSSSystemRouthArrayForSteadyState(const DSSSystem *ssys,
                                                    const DSVariablePool *Xd0,
                                                    const DSVariablePool *Xi0)
{
        DSMatrix * routhArray = NULL;
        DSMatrix * steadyState = NULL;
        DSMatrix * flux = NULL;
        DSMatrix * F = NULL;
        DSUInteger i;
        
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (Xd0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXd(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xd0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (Xi0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xi0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSSSystemHasSolution(ssys) == false)
                goto bail;
        steadyState = DSVariablePoolValuesAsVector(Xd0, false);
        flux = DSSSystemSteadyStateFluxForDependentVariables(ssys, Xd0, Xi0);
        F = DSMatrixIdentity(DSMatrixRows(flux));
        for (i = 0; i < DSMatrixColumns(F); i++) {
                DSMatrixSetDoubleValue(F,
                                       i,
                                       i,
                                       pow(10, DSMatrixDoubleValue(flux, i, 0))/pow(10,DSMatrixDoubleValue(steadyState, i, 0)));
        }
        routhArray = DSSSystemRouthArrayForPoolTurnover(ssys, F, NULL);
        DSMatrixFree(F);
bail:
        return routhArray;
}

extern DSMatrix * DSSSystemRouthArray(const DSSSystem *ssys, const DSVariablePool *Xi0, bool * hasImaginaryRoots)
{
        DSMatrix * routhArray = NULL;
        DSMatrix * steadyState = NULL;
        DSMatrix * flux = NULL;
        DSMatrix * F = NULL;
        DSUInteger i;
        
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (Xi0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xi0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSSSystemHasSolution(ssys) == false)
                goto bail;
        steadyState = DSSSystemSteadyStateValues(ssys, Xi0);
        flux = DSSSystemSteadyStateFlux(ssys, Xi0);
        F = DSMatrixIdentity(DSMatrixRows(flux));
        for (i = 0; i < DSMatrixColumns(F); i++) {
                DSMatrixSetDoubleValue(F,
                                       i,
                                       i,
                                       pow(10, DSMatrixDoubleValue(flux, i, 0))/pow(10,DSMatrixDoubleValue(steadyState, i, 0)));
        }
        routhArray = DSSSystemRouthArrayForPoolTurnover(ssys, F, hasImaginaryRoots);
        DSMatrixFree(F);
bail:
        return routhArray;
}

extern DSUInteger DSSSystemNumberOfPositiveRootsForRouthArray(const DSMatrix *routhArray)
{
        DSUInteger positiveRoots = 0;
        DSUInteger i, length;
        double sign, baseSign = 1;

        if (routhArray == NULL) {
                DSError(M_DS_MAT_NULL ": Routh Array Matrix is Null", A_DS_ERROR);
                goto bail;
        }
        
        length = DSMatrixRows(routhArray);
        baseSign = (DSMatrixDoubleValue(routhArray, 0, 0) > 0) ? 1. : -1.;
        for (i = 1; i < length; i++) {
                sign = (DSMatrixDoubleValue(routhArray, i, 0) > 0) ? 1. : -1.;
                if (sign*baseSign < 0)
                        positiveRoots++;
                baseSign = sign;
        }
bail:
        return positiveRoots;
}

extern DSUInteger DSSSystemPositiveRootsForSteadyStateAndFlux(const DSSSystem *ssys,
                                                              const DSVariablePool *Xd0,
                                                              const DSVariablePool *Xi0,
                                                              const DSVariablePool *flux0)
{
        DSMatrix *steadyState, *flux, *F, * routhArray = NULL;
        char flux_name[100];
        DSUInteger i, positiveRoots = 0;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (Xd0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXd(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xd0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (Xi0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xi0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (flux0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXd(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Flux variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        steadyState = DSVariablePoolValuesAsVector(Xd0, false);
        flux = DSMatrixAlloc(DSVariablePoolNumberOfVariables(DSSSystemXd(ssys)), 1);
        for (i = 0; i < DSVariablePoolNumberOfVariables(DSSSystemXd(ssys)); i++) {
                sprintf(flux_name, "V_%s", DSVariableName(DSVariablePoolVariableAtIndex(DSSSysXd(ssys), i)));
                DSMatrixSetDoubleValue(flux, i, 0, DSVariablePoolValueForVariableWithName(flux0, flux_name));
        }
        F = DSMatrixIdentity(DSMatrixRows(flux));
        for (i = 0; i < DSMatrixColumns(F); i++) {
                DSMatrixSetDoubleValue(F,
                                       i,
                                       i,
                                       pow(10, DSMatrixDoubleValue(flux, i, 0))/pow(10,DSMatrixDoubleValue(steadyState, i, 0)));
        }
        routhArray = DSSSystemRouthArrayForPoolTurnover(ssys, F, NULL);
        DSMatrixFree(F);
        DSMatrixFree(flux);
        DSMatrixFree(steadyState);
//        routhArray = DSSSystemRouthArrayForSteadyState(ssys, Xd0, Xi0);
        if (routhArray == NULL) {
                goto bail;
        }
        
        positiveRoots = DSSSystemNumberOfPositiveRootsForRouthArray(routhArray);
        DSMatrixFree(routhArray);
bail:
        return positiveRoots;
}

extern DSUInteger DSSSystemPositiveRootsForSteadyState(const DSSSystem *ssys,
                                                       const DSVariablePool *Xd0,
                                                       const DSVariablePool *Xi0)
{
        DSMatrix * routhArray = NULL;
        DSUInteger positiveRoots = 0;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (Xd0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXd(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xd0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (Xi0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xi0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        routhArray = DSSSystemRouthArrayForSteadyState(ssys, Xd0, Xi0);
        if (routhArray == NULL) {
                goto bail;
        }
        positiveRoots = DSSSystemNumberOfPositiveRootsForRouthArray(routhArray);
        DSMatrixFree(routhArray);
bail:
        return positiveRoots;
}

extern DSUInteger DSSSystemPositiveRoots(const DSSSystem *ssys, const DSVariablePool *Xi0, bool * hasImaginaryRoots)
{
        DSMatrix * routhArray = NULL;
        DSUInteger positiveRoots = 0;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (Xi0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xi0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSSSystemHasSolution(ssys) == false)
                goto bail;
        routhArray = DSSSystemRouthArray(ssys, Xi0, hasImaginaryRoots);
        if (routhArray == NULL) {
                goto bail;
        }
        
        positiveRoots = DSSSystemNumberOfPositiveRootsForRouthArray(routhArray);
        if (positiveRoots == 3) {
                printf("%i\n", DSSSystemCharacteristicEquationCoefficientsNumberSignChanges(ssys, Xi0));
        }
        DSMatrixFree(routhArray);
bail:
        return positiveRoots;
}

extern DSUInteger DSSSystemRouthIndex(const DSSSystem *ssys, const DSVariablePool *Xi0)
{
        DSMatrix * routhArray = NULL;
        DSUInteger routhIndex = 0;
        DSUInteger i, length;
        double value, baseSign = 1;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (Xi0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xi0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSSSystemHasSolution(ssys) == false)
                goto bail;
        routhArray = DSSSystemRouthArray(ssys, Xi0, NULL);
        if (routhArray == NULL) {
                goto bail;
        }
                
        length = DSMatrixRows(routhArray);
        baseSign = (DSMatrixDoubleValue(routhArray, 0, 0) > 0) ? 1. : -1.;
        for (i = 0; i < length; i++) {
                value = DSMatrixDoubleValue(routhArray, i, 0);
                value *= baseSign;
                if (value < 0)
                        routhIndex += pow(2, i);
        }
        DSMatrixFree(routhArray);
bail:
        return routhIndex;
}

extern DSUInteger DSSSystemCharacteristicEquationCoefficientIndex(const DSSSystem *ssys, const DSVariablePool *Xi0)
{
        DSMatrix * coefficientArray = NULL;
        DSMatrix * F;
        DSMatrix * FA;
        DSMatrix * steadyState, * flux;
        DSUInteger Index = 0;
        DSUInteger i, length;
        double value, baseSign = 1;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (Xi0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xi0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSSSystemHasSolution(ssys) == false)
                goto bail;
        steadyState = DSSSystemSteadyStateValues(ssys, Xi0);
        flux = DSSSystemSteadyStateFlux(ssys, Xi0);
        F = DSMatrixIdentity(DSMatrixRows(flux));
        for (i = 0; i < DSMatrixColumns(F); i++) {
                DSMatrixSetDoubleValue(F,
                                       i,
                                       i,
                                       pow(10, DSMatrixDoubleValue(flux, i, 0))/pow(10,DSMatrixDoubleValue(steadyState, i, 0)));
        }
        FA = DSMatrixByMultiplyingMatrix(F, DSSSystemAd(ssys));
        coefficientArray = DSMatrixCharacteristicPolynomialCoefficients(FA);
        DSMatrixFree(steadyState);
        DSMatrixFree(flux);
        DSMatrixFree(F);
        DSMatrixFree(FA);
        if (coefficientArray == NULL) {
                goto bail;
        }
        
        length = DSMatrixRows(coefficientArray);
        baseSign = (DSMatrixDoubleValue(coefficientArray, 0, 0) > 0) ? 1. : -1.;
        for (i = 0; i < length; i++) {
                value = DSMatrixDoubleValue(coefficientArray, i, 0);
                value *= baseSign;
                if (value < 0)
                        Index += pow(2, i);
        }
        DSMatrixFree(coefficientArray);
bail:
        return Index;
}

extern DSUInteger DSSSystemCharacteristicEquationCoefficientsNumberSignChanges(const DSSSystem *ssys, const DSVariablePool *Xi0)
{
        DSMatrix * coefficientArray = NULL;
        DSMatrix * F;
        DSMatrix * FA;
        DSMatrix * steadyState, * flux;
        DSUInteger Index = 0;
        DSUInteger i, length;
        double value, baseSign = 1;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (Xi0 == NULL && DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) != 0) {
                DSError(M_DS_VAR_NULL ": Xi0 variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSSSystemHasSolution(ssys) == false)
                goto bail;
        steadyState = DSSSystemSteadyStateValues(ssys, Xi0);
        flux = DSSSystemSteadyStateFlux(ssys, Xi0);
        F = DSMatrixIdentity(DSMatrixRows(flux));
        for (i = 0; i < DSMatrixColumns(F); i++) {
                DSMatrixSetDoubleValue(F,
                                       i,
                                       i,
                                       pow(10, DSMatrixDoubleValue(flux, i, 0))/pow(10,DSMatrixDoubleValue(steadyState, i, 0)));
        }
        FA = DSMatrixByMultiplyingMatrix(F, DSSSystemAd(ssys));
        coefficientArray = DSMatrixCharacteristicPolynomialCoefficients(FA);
        DSMatrixFree(steadyState);
        DSMatrixFree(flux);
        DSMatrixFree(F);
        DSMatrixFree(FA);
        if (coefficientArray == NULL) {
                goto bail;
        }
        length = DSMatrixColumns(coefficientArray);
        baseSign = 1.;
        for (i = 1; i < length; i++) {
                value = DSMatrixDoubleValue(coefficientArray, 0, i);
                value *= baseSign;
                if (value < 0)
                        Index++;
                baseSign = (DSMatrixDoubleValue(coefficientArray, 0, i) > 0) ? 1. : -1.;
        }
        DSMatrixFree(coefficientArray);
bail:
        return Index;
}

extern double DSSSystemLogarithmicGain(const DSSSystem *ssys, const char *XdName, const char *XiName)
{
        double logGain = INFINITY;
        DSUInteger XdIndex = 0;
        DSUInteger XiIndex = 0;
        DSMatrix * L = NULL;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (XdName == NULL) {
                DSError(M_DS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (XiName == NULL) {
                DSError(M_DS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolHasVariableWithName(DSSSysXd(ssys), XdName) == false) {
                DSError(M_DS_WRONG, A_DS_ERROR);
                goto bail;
        } else {
                XdIndex=DSVariablePoolIndexOfVariableWithName(DSSSysXd(ssys), XdName);
        }
        if (DSVariablePoolHasVariableWithName(DSSSysXi(ssys), XiName) == false) {
                DSError(M_DS_WRONG, A_DS_ERROR);
                goto bail;
        } else {
                XiIndex = DSVariablePoolIndexOfVariableWithName(DSSSysXi(ssys), XiName);                
        }
        L = DSMatrixByMultiplyingMatrix(DSSSystemM(ssys), DSSSystemAi(ssys));
        if (L == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        logGain = -DSMatrixDoubleValue(L, XdIndex, XiIndex);
bail:
        return logGain;
}


#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

extern void DSSSystemPrint(const DSSSystem * ssys)
{
        int (*print)(const char *, ...);
        //        DSUInteger i;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System to print is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSPrintf == NULL)
                print = printf;
        else
                print = DSPrintf;
        print("\t  # Xd: %i\n\t  # Xi: %i\n\t     G: %ix%i\n\t     H: %ix%i\n\t Alpha: %ix1\n\t  Beta: %ix1\n\t   Sol: %s",
              DSVariablePoolNumberOfVariables(DSSSysXd(ssys)),
              DSVariablePoolNumberOfVariables(DSSSysXi(ssys)),
              DSMatrixRows(DSSSysGd(ssys)), 
              DSMatrixColumns(DSSSysGd(ssys))+((DSSSysGi(ssys) != NULL) ? DSMatrixColumns(DSSSysGi(ssys)) : 0),
              DSMatrixRows(DSSSysHd(ssys)), 
              DSMatrixColumns(DSSSysHd(ssys))+((DSSSysHi(ssys) != NULL) ? DSMatrixColumns(DSSSysHi(ssys)) : 0),
              DSMatrixRows(DSSSysAlpha(ssys)),
              DSMatrixRows(DSSSysBeta(ssys)),
              (DSSSystemHasSolution(ssys)  ? "YES" : "NO"));
        print("\n");
bail:
        return;
}


extern void DSSSystemPrintEquations(const DSSSystem *ssys)
{
        DSUInteger i;
        DSExpression ** equations = NULL;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        equations = DSSSystemEquations(ssys);
        if (equations != NULL) {
                for (i= 0; i < DSSSystemNumberOfEquations(ssys); i++) {
                        DSExpressionPrint(equations[i]);
                        DSExpressionFree(equations[i]);
                }
                DSSecureFree(equations);
        }
bail:
        return;
}

extern void DSSSystemPrintSolution(const DSSSystem *ssys)
{
        DSUInteger i;
        DSExpression ** solution = NULL;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        solution = DSSSystemSolution(ssys);
        if (solution != NULL) {
                for (i= 0; i < DSSSystemNumberOfEquations(ssys); i++) {
                        DSExpressionPrint(solution[i]);
                        DSExpressionFree(solution[i]);
                }
                DSSecureFree(solution);
        }
bail:
        return;
}

extern void DSSSystemPrintLogarithmicSolution(const DSSSystem *ssys)
{
        DSUInteger i;
        DSExpression ** solution = NULL;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        solution = DSSSystemLogarithmicSolution(ssys);
        if (solution != NULL) {
                for (i= 0; i < DSSSystemNumberOfEquations(ssys); i++) {
                        DSExpressionPrint(solution[i]);
                        DSExpressionFree(solution[i]);
                }
                DSSecureFree(solution);
        }
bail:
        return;
}








