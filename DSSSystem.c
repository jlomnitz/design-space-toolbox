/**
 * \file DSSSystem.m
 * \brief Header file with functions for dealing with S-Systems.
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

#define M_DS_SSYS_NULL                  M_DS_NULL ": S-System is NULL"

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
#define DSSSysAlpha(x)                    ((x)->alpha)
#define DSSSysBeta(x)                     ((x)->beta)
#define DSSSysGd(x)                       ((x)->Gd)
#define DSSSysGi(x)                       ((x)->Gi)
#define DSSSysHd(x)                       ((x)->Hd)
#define DSSSysHi(x)                       ((x)->Hi)
#define DSSSysMAi(x)                      ((x)->MAi)
#define DSSSysMB(x)                       ((x)->MB)
#define DSSSysIsSingular(x)               ((x)->isSingular)
/*\}*/


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
extern DSSSystem * DSSSystemAlloc(void)
{
        DSSSystem *sys  = NULL;
        sys  = DSSecureCalloc(sizeof(DSSSystem), 1);
        return sys ;
}

extern void DSSSystemFree(DSSSystem * sys)
{
        if (sys  == NULL) {
                DSError(M_DS_NULL ": S-System to free is NULL", A_DS_ERROR);
                goto bail;
        }
        DSVariablePoolSetReadWrite(DSSSysXd(sys));
        DSVariablePoolFree(DSSSysXd(sys));
        DSVariablePoolSetReadWrite(DSSSysXi(sys));
        DSVariablePoolFree(DSSSysXi(sys));
        DSMatrixFree(DSSSysAlpha(sys));
        DSMatrixFree(DSSSysBeta(sys));
        DSMatrixFree(DSSSysGd(sys));
        if (DSSSysGi(sys) != NULL)
                DSMatrixFree(DSSSysGi(sys));
        DSMatrixFree(DSSSysHd(sys));
        if (DSSSysHi(sys) != NULL)
                DSMatrixFree(DSSSysHi(sys));
        if (DSSSysMB(sys) != NULL)
                DSMatrixFree(DSSSysMB(sys));
        if (DSSSysMAi(sys) != NULL)
                DSMatrixFree(DSSSysMAi(sys));
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
        for (j = 0; j < DSGMAParserAuxNumberOfBases(current); j++) {
                if (DSGMAParserAuxBaseAtIndexIsVariable(current, j) == false) { 
                        DSMatrixSetDoubleValue(DSSSysAlpha(sys), 
                                               equation, 0, 
                                               DSGMAParseAuxsConstantBaseAtIndex(current, j));
                        continue;
                }
                varName = DSGMAParserAuxVariableAtIndex(current, j);
                if (DSVariablePoolHasVariableWithName(DSSSysXd(sys), varName) == true) {
                        DSMatrixSetDoubleValue(DSSSysGd(sys),
                                               equation,
                                               DSVariablePoolIndexOfVariableWithName(DSSSysXd(sys),
                                                                                     varName),
                                               DSGMAParserAuxExponentAtIndex(current, j));
                } else if (DSVariablePoolHasVariableWithName(DSSSysXi(sys), varName) == true) {
                        DSMatrixSetDoubleValue(DSSSysGi(sys),
                                               equation,
                                               DSVariablePoolIndexOfVariableWithName(DSSSysXi(sys),
                                                                                     varName),
                                               DSGMAParserAuxExponentAtIndex(current, j));
                }
        }
}

static void dsSSysProcessNegativeExponentBasePairs(DSSSystem *sys, gma_parseraux_t *current, DSUInteger equation)
{
        DSUInteger j;
        const char *varName;
        for (j = 0; j < DSGMAParserAuxNumberOfBases(current); j++) {
                if (DSGMAParserAuxBaseAtIndexIsVariable(current, j) == false) { 
                        DSMatrixSetDoubleValue(DSSSysBeta(sys), 
                                               equation, 0, 
                                               DSGMAParseAuxsConstantBaseAtIndex(current, j));
                        continue;
                }
                varName = DSGMAParserAuxVariableAtIndex(current, j);
                if (DSVariablePoolHasVariableWithName(DSSSysXd(sys), varName) == true) {
                        DSMatrixSetDoubleValue(DSSSysHd(sys),
                                               equation,
                                               DSVariablePoolIndexOfVariableWithName(DSSSysXd(sys),
                                                                                     varName),
                                               DSGMAParserAuxExponentAtIndex(current, j));
                } else if (DSVariablePoolHasVariableWithName(DSSSysXi(sys), varName) == true) {
                        DSMatrixSetDoubleValue(DSSSysHi(sys),
                                               equation,
                                               DSVariablePoolIndexOfVariableWithName(DSSSysXi(sys),
                                                                                     varName),
                                               DSGMAParserAuxExponentAtIndex(current, j));
                }
        }
}

#include <unistd.h> 
static void dsSSystemSolveEquations(DSSSystem *ssys)
{
        DSMatrix *M, *Ad, *Ai = NULL, *B;
        DSUInteger i, numberOfEquations;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System being modified is NULL", A_DS_ERROR);
                goto bail;
        }
        numberOfEquations = DSVariablePoolNumberOfVariables(DSSSysXd(ssys));
        B = DSMatrixCalloc(numberOfEquations, 1);
        for (i = 0; i < numberOfEquations; i++)
                DSMatrixSetDoubleValue(B, i, 0, log10(DSMatrixDoubleValue(DSSSysBeta(ssys), i, 0)/DSMatrixDoubleValue(DSSSysAlpha(ssys), i, 0)));
        DSSSysIsSingular(ssys) = true;
        Ad = DSMatrixBySubstractingMatrix(DSSSysGd(ssys), DSSSysHd(ssys));
        if (DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) > 0)
                Ai = DSMatrixBySubstractingMatrix(DSSSysGi(ssys), DSSSysHi(ssys));
        M = DSMatrixInverse(Ad);
        if (M != NULL) {
                DSSSysIsSingular(ssys) = false;
                if (DSVariablePoolNumberOfVariables(DSSSysXi(ssys)) > 0)
                        DSSSysMAi(ssys) = DSMatrixByMultiplyingMatrix(M, Ai);
                DSSSysMB(ssys) = DSMatrixByMultiplyingMatrix(M, B);
                DSMatrixFree(M);
        }
        DSMatrixFree(Ad);
        if (Ai != NULL)
                DSMatrixFree(Ai);
        DSMatrixFree(B);
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

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Public Functions
#endif


extern DSSSystem * DSSSystemByParsingStringList(const DSVariablePool * const Xd, const char * const string, ...)
{
        DSSSystem *gma = NULL;
        DSUInteger numberOfStrings = 0;
        char const ** strings = NULL;
        const char * aString = NULL;
        if (string == NULL) {
                DSError(M_DS_NULL ": String to parse is NULL", A_DS_ERROR);
        }
        va_list ap;
	va_start(ap, string);
        strings = DSSecureCalloc(sizeof(char *), 1);
        strings[0] = string;
        numberOfStrings++;
        aString = va_arg(ap, char *);
        while (aString != NULL) {
                strings = DSSecureRealloc(strings, sizeof(char *)*(numberOfStrings+1));
                strings[numberOfStrings++] = aString;
                aString = va_arg(ap, char *);
        }
        gma = DSSSystemByParsingStrings(Xd, (char * const * )strings, numberOfStrings);
        DSSecureFree(strings);
bail:
        return gma;
}

extern DSSSystem * DSSSystemByParsingStrings(const DSVariablePool * const Xd, char * const * const strings, const DSUInteger numberOfEquations)
{
        DSSSystem * sys = NULL;
        gma_parseraux_t **aux = NULL;
        DSUInteger i;
        if (strings == NULL) {
                DSError(M_DS_NULL ": Array of strings is NULL", A_DS_ERROR);
                goto bail;
        }
        if (numberOfEquations == 0) {
                DSError(M_DS_WRONG ": No equations to parse", A_DS_WARN);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(Xd) != numberOfEquations) {
                DSError(M_DS_WRONG ": Number of dependent variables does not match number of equations", A_DS_ERROR);
                goto bail;
        }
        aux = dsSSysTermListForAllStrings(strings, numberOfEquations);
        if (aux == NULL)
                goto bail;
        sys = DSSSystemAlloc();
        DSSSysXd(sys) = DSVariablePoolCopy(Xd);
        DSVariablePoolSetReadOnly(DSSSysXd(sys));
        DSSSysXi(sys) = dsSSystemIdentifyIndependentVariables(Xd, aux, numberOfEquations);
        DSVariablePoolSetReadOnly(DSSSysXi(sys));
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
        DSSSysXd(ssys) = DSVariablePoolCopy(DSGMASystemXd(gma));
        DSSSysXi(ssys) = DSVariablePoolCopy(DSGMASystemXi(gma));
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
        equations = DSSecureCalloc(sizeof(DSExpression *), numberOfEquations);
        length = 1000;
        tempString = DSSecureCalloc(sizeof(char), length);
        for (i = 0; i < numberOfEquations; i++) {
                tempString[0] = '\0';
                dsSSystemEquationAddPositiveTermToString(ssys, i, &tempString, &length);
                strncat(tempString, "-", length-strlen(tempString));
                dsSSystemEquationAddNegativeTermToString(ssys, i, &tempString, &length);
                equations[i] = DSExpressionByParsingString(tempString);
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
        DSMatrix *MAi, *MB;
        char tempString[100];
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
        numberOfXi = DSVariablePoolNumberOfVariables(DSSSysXi(ssys));
        MAi = DSSSysMAi(ssys);
        MB = DSSSysMB(ssys);
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
                value = DSMatrixDoubleValue(MAi, equation, i);
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

extern DSExpression ** DSSSystemSolution(const DSSSystem *ssys)
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
        solution = DSSecureCalloc(sizeof(DSExpression *), numberOfEquations);
        length = 1000;
        tempString = DSSecureCalloc(sizeof(char), length);
        for (i = 0; i < numberOfEquations; i++) {
                tempString[0] = '\0';
                dsSSystemSolutionToString(ssys, i, &tempString, &length, false);
                solution[i] = DSExpressionByParsingString(tempString);
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
        solution = DSSecureCalloc(sizeof(DSExpression *), numberOfEquations);
        length = 1000;
        tempString = DSSecureCalloc(sizeof(char), length);
        for (i = 0; i < numberOfEquations; i++) {
                tempString[0] = '\0';
                dsSSystemSolutionToString(ssys, i, &tempString, &length, true);
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

extern const DSMatrix * DSSSystemMAi(const DSSSystem * ssys)
{
        DSMatrix *MAi = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        MAi = DSSSysMAi(ssys);
bail:
        return MAi;
}

extern const DSMatrix * DSSSystemMB(const DSSSystem * ssys)
{
        DSMatrix *MB = NULL;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        MB = DSSSysMB(ssys);
bail:
        return MB;
}

extern const bool DSSSystemHasSolution(const DSSSystem * ssys)
{
        bool hasSolution = false;
        if (ssys == NULL) {
                DSError(M_DS_NULL ": S-System is NULL", A_DS_ERROR);
                goto bail;
        }
        hasSolution = DSSSysIsSingular(ssys);
        if (DSSSysMB(ssys) == NULL)
                hasSolution = false;
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
        print("\t==================\n\t     S-System\n\t==================\n");
        print("\t  # Xd: %i\n\t  # Xi: %i\n\t     G: %ix%i\n\t     H: %ix%i\n\t Alpha: %ix1\n\t  Beta: %ix1\n\t   Sol: %s",
              DSVariablePoolNumberOfVariables(DSSSysXd(ssys)),
              DSVariablePoolNumberOfVariables(DSSSysXi(ssys)),
              DSMatrixRows(DSSSysGd(ssys)), 
              DSMatrixColumns(DSSSysGd(ssys))+((DSSSysGi(ssys) != NULL) ? DSMatrixColumns(DSSSysGi(ssys)) : 0),
              DSMatrixRows(DSSSysHd(ssys)), 
              DSMatrixColumns(DSSSysHd(ssys))+((DSSSysHi(ssys) != NULL) ? DSMatrixColumns(DSSSysHi(ssys)) : 0),
              DSMatrixRows(DSSSysAlpha(ssys)),
              DSMatrixRows(DSSSysBeta(ssys)),
              (DSSSysIsSingular(ssys) == false && DSSSysMB(ssys) != NULL  ? "YES" : "NO"));
        print("\n");
bail:
        return;
}











