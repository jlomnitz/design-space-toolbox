/**
 * \file DSGMASystem.c
 * \brief Implementation file with functions for dealing with GMA Systems.
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
#include "DSTypes.h"
#include "DSErrors.h"
#include "DSMemoryManager.h"
#include "DSGMASystem.h"
#include "DSExpression.h"
#include "DSExpressionTokenizer.h"
#include "DSGMASystemGrammar.h"
#include "DSGMASystemParsingAux.h"
#include "DSMatrix.h"
#include "DSMatrixArray.h"

#define M_DS_GMA_NULL              M_DS_NULL ": GMA System is NULL"

/**
 * \defgroup DSGMAACCESSORS
 *
 * \brief Internal GMA Accessor macros.
 * 
 * \details Used within DSGMASystem.c to access the data within a GMA data type.
 * These macros are not to be used putside of this file, as they do not check the
 * data dor consistency and thus would not invoke the DSError function, making
 * it harder to trace errors.
 */
/*\{*/
#define DSGMAXi(x)                       ((x)->Xi)
#define DSGMAXd(x)                       ((x)->Xd)
#define DSGMAAlpha(x)                    ((x)->alpha)
#define DSGMABeta(x)                     ((x)->beta)
#define DSGMAGd(x)                       ((x)->Gd)
#define DSGMAGi(x)                       ((x)->Gi)
#define DSGMAHd(x)                       ((x)->Hd)
#define DSGMAHi(x)                       ((x)->Hi)
#define DSGMASignature(x)                   ((x)->signature)
/*\}*/


#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, deallocation and initialization
#endif

/**
 * \brief Creates a empty GMA System.
 * 
 * This function allocates the necessary memory space used by a GMA and
 * initializes it so that it is ready for processing.  The initialized GMA
 * has all its fields set to 0 or NULL.  This is interpreted as an empty GMA
 * and is necessary for parsing a set of equations.
 *
 * \return A DSGMASystem pointer to the newly allocated GMASystem.
 */
static DSGMASystem * DSGMASystemAlloc(void)
{
        DSGMASystem *gma = NULL;
        gma = DSSecureCalloc(sizeof(DSGMASystem), 1);
        return gma;
}

extern void DSGMASystemFree(DSGMASystem * gma)
{
        if (gma == NULL) {
                DSError(M_DS_NULL ": GMA to free is NULL", A_DS_ERROR);
                goto bail;
        }
        DSVariablePoolSetReadWrite(DSGMAXd(gma));
        DSVariablePoolFree(DSGMAXd(gma));
        DSVariablePoolSetReadWrite(DSGMAXi(gma));
        DSVariablePoolFree(DSGMAXi(gma));
        DSMatrixFree(DSGMAAlpha(gma));
        DSMatrixFree(DSGMABeta(gma));
        DSMatrixArrayFree(DSGMAGd(gma));
        if (DSGMAGi(gma) != NULL)
                DSMatrixArrayFree(DSGMAGi(gma));
        DSMatrixArrayFree(DSGMAHd(gma));
        if (DSGMAHi(gma) != NULL)
                DSMatrixArrayFree(DSGMAHi(gma));
        DSSecureFree(DSGMASignature(gma));
        DSSecureFree(gma);
bail:
        return;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Factory methods
#endif


#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Internal Parsing Functions
#endif


static gma_parseraux_t * dsGmaSystemParseStringToTermList(const char * string)
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
        parser = DSGMASystemParserAlloc(DSSecureMalloc);
        root = DSGMAParserAuxAlloc();
        parser_aux = root;
        current = tokens;
        while (current != NULL) {
                if (DSExpressionTokenType(current) == DS_EXPRESSION_TOKEN_START) {
                        current = DSExpressionTokenNext(current);
                        continue;
                }
                DSGMASystemParser(parser, 
                                  DSExpressionTokenType(current), 
                                  current,
                                  ((void**)&parser_aux));
                current = DSExpressionTokenNext(current);
        }
        DSGMASystemParser(parser, 
                          0, 
                          NULL,
                          ((void **)&parser_aux));
        DSGMASystemParserFree(parser, DSSecureFree);
        DSExpressionTokenFree(tokens);
        if (DSGMAParserAuxParsingFailed(root) == true) {
                DSGMAParserAuxFree(root);
                root = NULL;
        }
bail:
        return root;
}

static gma_parseraux_t ** dsGmaTermListForAllStrings(char * const * const strings, const DSUInteger numberOfEquations)
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
                        aux[i] = dsGmaSystemParseStringToTermList(aString);
                        DSSecureFree(aString);
                        DSExpressionFree(expr);
                }
                if (aux[i] == NULL) {
                        DSError(M_DS_PARSE ": Expression not in GMA format", A_DS_ERROR);
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

static DSVariablePool * dsGmaSystemIdentifyIndependentVariables(const DSVariablePool * const Xd, gma_parseraux_t ** aux, const DSUInteger numberOfEquations)
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

static void dsGMASystemMaxNumberOfTerms(gma_parseraux_t *const *aux, const DSUInteger numberOfEquations, DSUInteger * pterms, DSUInteger *nterms)
{
        DSUInteger i, p, n;
        gma_parseraux_t *current;
        if (aux == NULL) {
                DSError(M_DS_NULL ": Parser auxiliary data is NULL", A_DS_ERROR);
                goto bail;
        }
        if (pterms == NULL) {
                DSError(M_DS_WRONG ": Argument pterms is NULL", A_DS_ERROR);
                goto bail;
        }
        if (nterms == NULL) {
                DSError(M_DS_WRONG ": Argument pterms is NULL", A_DS_ERROR);
                goto bail;
        }        
        if (numberOfEquations == 0) {
                DSError(M_DS_WRONG ": No equations to parse", A_DS_WARN);
                goto bail;
        }
        *pterms = 0;
        *nterms = 0;
        for (i = 0; i < numberOfEquations; i++) {
                current = aux[i];
                p = 0;
                n = 0;
                while (current) {
                        switch (DSGMAParserAuxSign(current)) {
                                case AUX_SIGN_POSITIVE:
                                        p++;
                                        break;
                                case AUX_SIGN_NEGATIVE:
                                        n++;
                                        break;
                                case AUX_SIGN_UNDEFINED:
                                        break;
                                default:
                                        break;
                        }
                        current = DSGMAParserAuxNextNode(current);
                }
                if (p > *pterms)
                        *pterms = p;
                if (n > *nterms)
                        *nterms = n;
        }
bail:
        return;
}

static void dsGMASystemInitializeMatrices(DSGMASystem *gma, DSUInteger positiveTerms, DSUInteger negativeTerms)
{
        DSUInteger numberOfEquations, numberOfXd, numberOfXi;
        DSUInteger i;
        numberOfEquations = DSVariablePoolNumberOfVariables(DSGMAXd(gma));
        numberOfXd = numberOfEquations;
        numberOfXi = DSVariablePoolNumberOfVariables(DSGMAXi(gma));
        DSGMAAlpha(gma) = DSMatrixCalloc(numberOfEquations, positiveTerms);
        DSGMABeta(gma) = DSMatrixCalloc(numberOfEquations, negativeTerms);
        DSGMASignature(gma) = DSSecureCalloc(sizeof(DSUInteger), 2*numberOfEquations);
        DSGMAGd(gma) = DSMatrixArrayAlloc();
        if (numberOfXi > 0)
                DSGMAGi(gma) = DSMatrixArrayAlloc();
        DSGMAHd(gma) = DSMatrixArrayAlloc();
        if (numberOfXi > 0)
        DSGMAHi(gma) = DSMatrixArrayAlloc();
        for (i = 0; i < numberOfEquations; i++) {
                DSMatrixArrayAddMatrix(DSGMAGd(gma), DSMatrixCalloc(positiveTerms, 
                                                                    numberOfXd));
                if (numberOfXi > 0)
                        DSMatrixArrayAddMatrix(DSGMAGi(gma), DSMatrixCalloc(positiveTerms, 
                                                                            numberOfXi));
                DSMatrixArrayAddMatrix(DSGMAHd(gma), DSMatrixCalloc(negativeTerms, 
                                                                    numberOfXd));
                if (numberOfXi > 0)
                        DSMatrixArrayAddMatrix(DSGMAHi(gma), DSMatrixCalloc(negativeTerms, 
                                                                            numberOfXi));
        }
}

static void dsGMAProcessPositiveExponentBasePairs(DSGMASystem *gma, gma_parseraux_t *current, DSUInteger equation, DSUInteger p)
{
        DSUInteger j;
        const char *varName;
        for (j = 0; j < DSGMAParserAuxNumberOfBases(current); j++) {
                if (DSGMAParserAuxBaseAtIndexIsVariable(current, j) == false) { 
                        DSMatrixSetDoubleValue(DSGMAAlpha(gma), 
                                               equation, p, 
                                               DSGMAParseAuxsConstantBaseAtIndex(current, j));
                        continue;
                }
                varName = DSGMAParserAuxVariableAtIndex(current, j);
                if (DSVariablePoolHasVariableWithName(DSGMAXd(gma), varName) == true) {
                        DSMatrixSetDoubleValue(DSMatrixArrayMatrix(DSGMAGd(gma), equation),
                                               p,
                                               DSVariablePoolIndexOfVariableWithName(DSGMAXd(gma),
                                                                                     varName),
                                               DSGMAParserAuxExponentAtIndex(current, j));
                } else if (DSVariablePoolHasVariableWithName(DSGMAXi(gma), varName) == true) {
                        DSMatrixSetDoubleValue(DSMatrixArrayMatrix(DSGMAGi(gma), equation),
                                               p,
                                               DSVariablePoolIndexOfVariableWithName(DSGMAXi(gma),
                                                                                     varName),
                                               DSGMAParserAuxExponentAtIndex(current, j));
                }
        }
}

static void dsGMAProcessNegativeExponentBasePairs(DSGMASystem *gma, gma_parseraux_t *current, DSUInteger equation, DSUInteger n)
{
        DSUInteger j;
        const char *varName;
        for (j = 0; j < DSGMAParserAuxNumberOfBases(current); j++) {
                if (DSGMAParserAuxBaseAtIndexIsVariable(current, j) == false) { 
                        DSMatrixSetDoubleValue(DSGMABeta(gma), 
                                               equation, n, 
                                               DSGMAParseAuxsConstantBaseAtIndex(current, j));
                        continue;
                }
                varName = DSGMAParserAuxVariableAtIndex(current, j);
                if (DSVariablePoolHasVariableWithName(DSGMAXd(gma), varName) == true) {
                        DSMatrixSetDoubleValue(DSMatrixArrayMatrix(DSGMAHd(gma), equation),
                                               n,
                                               DSVariablePoolIndexOfVariableWithName(DSGMAXd(gma),
                                                                                     varName),
                                               DSGMAParserAuxExponentAtIndex(current, j));
                } else if (DSVariablePoolHasVariableWithName(DSGMAXi(gma), varName) == true) {
                        DSMatrixSetDoubleValue(DSMatrixArrayMatrix(DSGMAHi(gma), equation),
                                               n,
                                               DSVariablePoolIndexOfVariableWithName(DSGMAXi(gma),
                                                                                     varName),
                                               DSGMAParserAuxExponentAtIndex(current, j));
                }
        }
}

static void dsGMASystemCreateSystemMatrices(DSGMASystem *gma, gma_parseraux_t **aux)
{
        gma_parseraux_t *current;
        DSUInteger numberOfEquations, positiveTerms, negativeTerms;
        DSUInteger i, n, p;
        if (gma == NULL) {
                DSError(M_DS_NULL ": GMA being modified is NULL", A_DS_ERROR);
                goto bail;
        }
        if (aux == NULL) {
                DSError(M_DS_NULL ": Parser auxiliary data is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSGMAXd(gma) == NULL || DSGMAXi(gma) == NULL) {
                DSError(M_DS_WRONG ": GMA data is incomplete: Need Xi and Xd", A_DS_ERROR);
                goto bail;
        }
        numberOfEquations = DSVariablePoolNumberOfVariables(DSGMAXd(gma));
        dsGMASystemMaxNumberOfTerms(aux, numberOfEquations, &positiveTerms, &negativeTerms);
        dsGMASystemInitializeMatrices(gma, positiveTerms, negativeTerms);
        for (i = 0; i < numberOfEquations; i++) {
                current = aux[i];
                n = 0;
                p = 0;
                while (current) {
                        switch (DSGMAParserAuxSign(current)) {
                                case AUX_SIGN_POSITIVE:
                                        DSMatrixSetDoubleValue(DSGMAAlpha(gma), 
                                                               i, p, 
                                                               1.0);
                                        dsGMAProcessPositiveExponentBasePairs(gma, current, i, p);
                                        p++;
                                        break;
                                case AUX_SIGN_NEGATIVE:
                                        DSMatrixSetDoubleValue(DSGMABeta(gma), 
                                                               i, n, 
                                                               1.0);
                                        dsGMAProcessNegativeExponentBasePairs(gma, current, i, n);
                                        n++;
                                        break;
                                default:
                                        break;
                        }
                        current = DSGMAParserAuxNextNode(current);
                }
                DSGMASignature(gma)[2*i] = p;
                DSGMASignature(gma)[2*i+1] = n;
        }
bail:
        return;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Public Parsing Functions
#endif


extern DSGMASystem * DSGMASystemByParsingStringList(const DSVariablePool * const Xd, const char * const string, ...)
{
        DSGMASystem *gma = NULL;
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
        gma = DSGMASystemByParsingStrings(Xd, (char * const * )strings, numberOfStrings);
        DSSecureFree(strings);
bail:
        return gma;
}

extern DSGMASystem * DSGMASystemByParsingStrings(const DSVariablePool * const Xd, char * const * const strings, const DSUInteger numberOfEquations)
{
        DSGMASystem * gma = NULL;
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
        aux = dsGmaTermListForAllStrings(strings, numberOfEquations);
        if (aux == NULL)
                goto bail;
        gma = DSGMASystemAlloc();
        DSGMAXd(gma) = DSVariablePoolCopy(Xd);
        DSVariablePoolSetReadOnly(DSGMAXd(gma));
        DSGMAXi(gma) = dsGmaSystemIdentifyIndependentVariables(Xd, aux, numberOfEquations);
        DSVariablePoolSetReadOnly(DSGMAXi(gma));
        dsGMASystemCreateSystemMatrices(gma, aux);
        for (i=0; i < numberOfEquations; i++)
                if (aux[i] != NULL)
                        DSGMAParserAuxFree(aux[i]);
        DSSecureFree(aux);
bail:
        return gma;
}


#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Getter functions
#endif

extern const DSUInteger DSGMASystemNumberOfEquations(const DSGMASystem *gma)
{
        DSUInteger numberOfEquations = 0;
        if (gma == NULL) {
                DSError(M_DS_NULL ": GMA being accessed is NULL", A_DS_ERROR);
                goto bail;
        }
        numberOfEquations = DSVariablePoolNumberOfVariables(DSGMAXd(gma));
bail:
        return numberOfEquations;
}

static void dsGMASystemEquationAddPositiveTermToString(const DSGMASystem *gma, 
                                                       const DSUInteger equation, 
                                                       const DSUInteger pterm, 
                                                       char ** string, 
                                                       DSUInteger *length)
{
        DSUInteger i, numberOfXd, numberOfXi;
        DSMatrixArray *Gd, *Gi;
        DSMatrix *alpha;
        double value;
        char tempString[100];
        const char * name;
        if (gma == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfXd = DSVariablePoolNumberOfVariables(DSGMAXd(gma));
        if (equation >= numberOfXd) {
                DSError("Equation does not exist: Check number of equations", A_DS_ERROR);
                goto bail;
        }
        if (pterm >= DSGMASignature(gma)[2*equation]) {
                DSError("Term does not exist: Check number of terms", A_DS_ERROR);
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
        numberOfXi = DSVariablePoolNumberOfVariables(DSGMAXi(gma));
        Gd = DSGMAGd(gma);
        Gi = DSGMAGi(gma);
        alpha = DSGMAAlpha(gma);
        sprintf(tempString, "%lf", DSMatrixDoubleValue(alpha, equation, pterm));
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
                        name = DSVariableName(DSVariablePoolAllVariables(DSGMAXi(gma))[i]);
                        value = DSMatrixArrayDoubleWithIndices(Gi, equation, pterm, i);
                        
                } else {
                        name = DSVariableName(DSVariablePoolAllVariables(DSGMAXd(gma))[i-numberOfXi]);
                        value = DSMatrixArrayDoubleWithIndices(Gd, equation, pterm, i-numberOfXi);
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
  
static void dsGMASystemEquationAddNegativeTermToString(const DSGMASystem *gma, 
                                                       const DSUInteger equation, 
                                                       const DSUInteger nterm, 
                                                       char ** string, 
                                                       DSUInteger *length)
{
        DSUInteger i, numberOfXd, numberOfXi;
        DSMatrixArray *Hd, *Hi;
        DSMatrix *beta;
        char tempString[100];
        const char *name;
        double value;
        if (gma == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfXd = DSVariablePoolNumberOfVariables(DSGMAXd(gma));
        if (equation >= numberOfXd) {
                DSError("Equation does not exist: Check number of equations", A_DS_ERROR);
                goto bail;
        }
        if (nterm >= DSGMASignature(gma)[2*equation+1]) {
                DSError("Term does not exist: Check number of terms", A_DS_ERROR);
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
        numberOfXi = DSVariablePoolNumberOfVariables(DSGMAXi(gma));
        Hd = DSGMAHd(gma);
        Hi = DSGMAHi(gma);
        beta = DSGMABeta(gma);
        sprintf(tempString, "%lf", DSMatrixDoubleValue(beta, equation, nterm));
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
                        name = DSVariablePoolAllVariableNames(DSGMAXi(gma))[i];
                        value = DSMatrixArrayDoubleWithIndices(Hi, equation, nterm, i);
                        
                } else {
                        name = DSVariablePoolAllVariableNames(DSGMAXd(gma))[i-numberOfXi];
                        value = DSMatrixArrayDoubleWithIndices(Hd, equation, nterm, i-numberOfXi);
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

extern DSExpression ** DSGMASystemEquations(const DSGMASystem *gma)
{
        DSUInteger i, j, numberOfEquations, length;
        DSExpression ** equations = NULL;
        char *tempString;
        if (gma == NULL) {
                DSError(M_DS_NULL ": GMA being accessed is NULL", A_DS_ERROR);
                goto bail;
        }
        numberOfEquations = DSGMASystemNumberOfEquations(gma);
        if (numberOfEquations == 0) {
                DSError(M_DS_NULL ": GMA being accessed is empty", A_DS_ERROR);
                goto bail;
        }
        equations = DSSecureCalloc(sizeof(DSExpression *), numberOfEquations);
        length = 1000;
        tempString = DSSecureCalloc(sizeof(char), length);
        for (i = 0; i < numberOfEquations; i++) {
                tempString[0] = '\0';
                for (j = 0; j < DSGMASignature(gma)[2*i]; j++) {
                        dsGMASystemEquationAddPositiveTermToString(gma, i, j, &tempString, &length);
                        strncat(tempString, "+", length-strlen(tempString));
                }
                for (j = 0; j < DSGMASignature(gma)[2*i+1]; j++) {
                        strncat(tempString, "-", length-strlen(tempString));
                        dsGMASystemEquationAddNegativeTermToString(gma, i, j, &tempString, &length);
                }
                equations[i] = DSExpressionByParsingString(tempString);
        }
        DSSecureFree(tempString);
bail:
        return equations;
}

extern const DSMatrix *DSGMASystemAlpha(const DSGMASystem *gma)
{
        DSMatrix * alpha = NULL;
        if (gma == NULL) {
                DSError(M_DS_NULL ": GMA being accessed is NULL", A_DS_ERROR);
                goto bail;
        }
        alpha = DSGMAAlpha(gma);
bail:
        return alpha;
}

extern const DSMatrix *DSGMASystemBeta(const DSGMASystem *gma)
{
        DSMatrix * beta = NULL;
        if (gma == NULL) {
                DSError(M_DS_NULL ": GMA being accessed is NULL", A_DS_ERROR);
                goto bail;
        }
bail:
        beta = DSGMABeta(gma);
        return beta;
}

extern const DSMatrixArray *DSGMASystemGd(const DSGMASystem *gma)
{
        DSMatrixArray * gd = NULL;
        if (gma == NULL) {
                DSError(M_DS_NULL ": GMA being accessed is NULL", A_DS_ERROR);
                goto bail;
        }
        gd = DSGMAGd(gma);
bail:
        return gd;
}

extern const DSMatrixArray *DSGMASystemGi(const DSGMASystem *gma)
{
        DSMatrixArray * gi = NULL;
        if (gma == NULL) {
                DSError(M_DS_NULL ": GMA being accessed is NULL", A_DS_ERROR);
                goto bail;
        }
        gi = DSGMAGi(gma);
bail:
        return gi;
}

extern const DSMatrixArray *DSGMASystemHd(const DSGMASystem *gma)
{
        DSMatrixArray * hd = NULL;
        if (gma == NULL) {
                DSError(M_DS_NULL ": GMA being accessed is NULL", A_DS_ERROR);
                goto bail;
        }
        hd = DSGMAHd(gma);
bail:
        return hd;
}

extern const DSMatrixArray *DSGMASystemHi(const DSGMASystem *gma)
{
        DSMatrixArray * hi = NULL;
        if (gma == NULL) {
                DSError(M_DS_NULL ": GMA being accessed is NULL", A_DS_ERROR);
                goto bail;
        }
        hi = DSGMAHi(gma);
bail:
        return hi;
}

extern const DSVariablePool *DSGMASystemXd(const DSGMASystem *gma)
{
        DSVariablePool * Xd = NULL;
        if (gma == NULL) {
                DSError(M_DS_NULL ": GMA being accessed is NULL", A_DS_ERROR);
                goto bail;
        }
        Xd = DSGMAXd(gma);
bail:
        return Xd;
}
extern const DSVariablePool *DSGMASystemXi(const DSGMASystem *gma)
{
        DSVariablePool * Xi = NULL;
        if (gma == NULL) {
                DSError(M_DS_NULL ": GMA being accessed is NULL", A_DS_ERROR);
                goto bail;
        }
        Xi = DSGMAXi(gma);
bail:
        return Xi;
}

extern const DSUInteger * DSGMASystemSignature(const DSGMASystem *gma)
{
        DSUInteger *signature = NULL;
        if (gma == NULL) {
                DSError(M_DS_NULL ": GMA system is NULL", A_DS_ERROR);
                goto bail;
        }
        signature = DSGMASignature(gma);
bail:
        return signature;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - S-System functions
#endif


#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

extern void DSGMASystemPrint(const DSGMASystem * gma)
{
        int (*print)(const char *, ...);
        DSUInteger i;
        if (gma == NULL) {
                DSError(M_DS_NULL ": GMA to print is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSPrintf == NULL)
                print = printf;
        else
                print = DSPrintf;
        print("\t==================\n\t    GMA-System\n\t==================\n");
        print("\t  # Xd: %i\n\t  # Xi: %i\n\t   Sig: ",
              DSVariablePoolNumberOfVariables(DSGMAXd(gma)),
              DSVariablePoolNumberOfVariables(DSGMAXi(gma)));
        for (i = 0; i < DSGMASystemNumberOfEquations(gma); i++) {
                if (DSGMASignature(gma)[2*i] >= 10)
                        print("(");
                print("%i", DSGMASignature(gma)[2*i]);
                if (DSGMASignature(gma)[2*i] >= 10)
                        print(")");
                if (DSGMASignature(gma)[2*i+1] >= 10)
                        print("(");
                print("%i", DSGMASignature(gma)[2*i+1]);
                if (DSGMASignature(gma)[2*i+1] >= 10)
                        print(")");
        }
        print("\n");
bail:
        return;
}

