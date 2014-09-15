/**
 * \file DSDesignSpace.c
 * \brief Implementation file with functions for dealing with Design Spaces
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
#include <pthread.h>
#include <unistd.h>
#include <stdarg.h>
#include <glpk.h>
#include "DSMemoryManager.h"
#include "DSDesignSpace.h"
#include "DSMatrix.h"
#include "DSGMASystem.h"
#include "DSSSystem.h"
#include "DSCase.h"
#include "DSStack.h"
#include "DSDesignSpaceParallel.h"
#include "DSCyclicalCase.h"
#include "DSGMASystemParsingAux.h"
#include "DSDesignSpaceConditionGrammar.h"
#include "DSExpressionTokenizer.h"

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Internal Variable Access Macros -
#endif

#define DSDSGMA(x)                              ((x)->gma)
//#define DSDSCases(x)                            ((x)->cases)
#define DSDSNumCases(x)                         ((x)->numberOfCases)
//#define DSDSValid(x)                            ((x)->validCases)
#define DSDSXd(x)                               ((x)->Xd)
#define DSDSXi(x)                               ((x)->Xi)
#define DSDSCyclical(x)                         ((x)->cyclicalCases)
#define DSDSCi(x)                               ((x)->Ci)
#define DSDSCd(x)                               ((x)->Cd)
#define DSDSDelta(x)                            ((x)->delta)
#define DSDSValidPool(x)                        ((x)->validCases)

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, deallocation and initialization -
#endif

static void dsDesignSpaceCalculatePrunedValidityParallelBSD(DSDesignSpace *ds, const DSUInteger numberOfCases, const DSUInteger * caseNumber);

extern DSDesignSpace * DSDesignSpaceAlloc(void)
{
        DSDesignSpace * ds = NULL;
        ds = DSSecureCalloc(sizeof(DSDesignSpace), 1);
        DSDSCyclical(ds) = DSDictionaryAlloc();
        return ds;
}

void DSDesignSpaceFree(DSDesignSpace * ds)
{
//        DSUInteger i;
//        DSStack *aStack = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDSGMA(ds) != NULL)
                DSGMASystemFree(DSDSGMA(ds));
        if (DSDSCd(ds) != NULL)
                DSMatrixFree(DSDSCd(ds));
        if (DSDSCi(ds) != NULL)
                DSMatrixFree(DSDSCi(ds));
        if (DSDSDelta(ds) != NULL)
                DSMatrixFree(DSDSDelta(ds));
        if (DSDSValidPool(ds) != NULL) 
                DSDictionaryFree(DSDSValidPool(ds));
        DSDictionaryFreeWithFunction(DSDSCyclical(ds), DSCyclicalCaseFree);
        if (ds->extensionData != NULL) {
                // free extension data
//                DSDictionaryFreeWithFunction(ds->cycleFluxes, DSSecureFree);
        }
        DSSecureFree(ds);
bail:
        return;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Factory -
#endif


extern DSDesignSpace * DSDesignSpaceByParsingStringList(const char * string, const DSVariablePool * const Xd_a, ...)
{
        DSDesignSpace *ds = NULL;
        DSUInteger numberOfStrings = 0;
        char const ** strings = NULL;
        const char * aString = NULL;
        if (string == NULL) {
                DSError(M_DS_NULL ": String to parse is NULL", A_DS_ERROR);
        }
        va_list ap;
	va_start(ap, Xd_a);
        strings = DSSecureCalloc(sizeof(char *), 1);
        strings[0] = string;
        numberOfStrings++;
        aString = va_arg(ap, char *);
        while (aString != NULL) {
                strings = DSSecureRealloc(strings, sizeof(char *)*(numberOfStrings+1));
                strings[numberOfStrings++] = aString;
                aString = va_arg(ap, char *);
        }
        ds = DSDesignSpaceByParsingStrings((char * const * )strings, Xd_a, numberOfStrings);
        DSSecureFree(strings);
bail:
        return ds;
}

extern DSDesignSpace * DSDesignSpaceByParsingStrings(char * const * const strings, const DSVariablePool * const Xd_a, const DSUInteger numberOfEquations)
{
        DSDesignSpace * ds = NULL;
        DSGMASystem *gma = NULL;
        if (strings == NULL) {
                DSError(M_DS_NULL ": Array of strings is NULL", A_DS_ERROR);
                goto bail;
        }
        if (numberOfEquations == 0) {
                DSError(M_DS_WRONG ": No equations to parse", A_DS_WARN);
                goto bail;
        }
        gma = DSGMASystemByParsingStrings(strings, Xd_a, numberOfEquations);
        if (gma != NULL) {
                ds = DSDesignSpaceAlloc();
                DSDesignSpaceSetGMA(ds, gma);
        }
bail:
        return ds;
}

extern DSDesignSpace * DSDesignSpaceByParsingStringsWithXi(char * const * const strings, const DSVariablePool * const Xd_a, const DSVariablePool * const Xi, const DSUInteger numberOfEquations)
{
        DSDesignSpace * ds = NULL;
        DSGMASystem *gma = NULL;
        if (strings == NULL) {
                DSError(M_DS_NULL ": Array of strings is NULL", A_DS_ERROR);
                goto bail;
        }
        if (numberOfEquations == 0) {
                DSError(M_DS_WRONG ": No equations to parse", A_DS_WARN);
                goto bail;
        }
        gma = DSGMASystemByParsingStringsWithXi(strings, Xd_a, Xi, numberOfEquations);
        if (gma != NULL) {
                ds = DSDesignSpaceAlloc();
                DSDesignSpaceSetGMA(ds, gma);
        }
bail:
        return ds;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Setters -
#endif

extern void DSDesignSpaceSetGMA(DSDesignSpace * ds, DSGMASystem *gma)
{
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (gma == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        DSDSGMA(ds) = gma;
        DSDSXd(ds) = DSGMASystemXd(gma);
        DSDSXi(ds) = DSGMASystemXi(gma);
        DSDSNumCases(ds) = DSGMASystemNumberOfCases(DSDSGMA(ds));
bail:
        return;
}


#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Getters -
#endif

extern const DSVariablePool * DSDesignSpaceXi(const DSDesignSpace *ds)
{
        const DSVariablePool * Xi = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        Xi = DSDSXi(ds);
bail:
        return Xi;
}

extern const DSUInteger DSDesignSpaceNumberOfEquations(const DSDesignSpace *ds)
{
        DSUInteger numberOfEquations = 0;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfEquations = DSGMASystemNumberOfEquations(DSDSGMA(ds));
bail:
        return numberOfEquations;
}

extern DSExpression ** DSDesignSpaceEquations(const DSDesignSpace *ds)
{
        DSExpression ** equations =NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        equations = DSGMASystemEquations(DSDSGMA(ds));
bail:
        return equations;
}

extern const DSUInteger DSDesignSpaceNumberOfCases(const DSDesignSpace *ds)
{
        DSUInteger numberOfCases = 0;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDSGMA(ds) == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfCases = DSDSNumCases(ds);
bail:
        return numberOfCases;
}

extern const DSUInteger DSDesignSpaceNumberOfValidCases(const DSDesignSpace *ds)
{
        DSUInteger numberValdCases = 0;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDSValidPool(ds) == NULL)
                DSDesignSpaceCalculateValidityOfCases((DSDesignSpace *)ds);
        numberValdCases = DSDictionaryCount(DSDSValidPool(ds));
bail:
        return numberValdCases;
}

extern const DSUInteger DSDesignSpaceNumberOfValidCasesFromPrunedCases(const DSDesignSpace *ds, DSUInteger numberOfCases, DSUInteger * caseNumbers)
{
        DSUInteger numberValdCases = 0;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDSValidPool(ds) == NULL)
                dsDesignSpaceCalculatePrunedValidityParallelBSD((DSDesignSpace *)ds, numberOfCases, caseNumbers);
        numberValdCases = DSDictionaryCount(DSDSValidPool(ds));
bail:
        return numberValdCases;
}

extern const DSUInteger * DSDesignSpaceSignature(const DSDesignSpace *ds)
{
        const DSUInteger * signature = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDesignSpaceGMASystem(ds) == NULL) {
                goto bail;
        }
        signature = DSGMASystemSignature(DSDesignSpaceGMASystem(ds));
bail:
        return signature;
}

extern char * DSDesignSpaceSignatureToString(const DSDesignSpace *ds)
{
        char * string = NULL;
        const DSUInteger * signature;
        DSUInteger i, current;
        if (ds == NULL) {
                DSError(M_DS_NULL, A_DS_ERROR);
                goto bail;
        }
        signature = DSDesignSpaceSignature(ds);
        string = DSSecureCalloc(sizeof(char), 2*DSDesignSpaceNumberOfEquations(ds));
        for (i = 0; i < 2*DSDesignSpaceNumberOfEquations(ds); i++) {
                current =signature[i];
                if (current >= 10)
                        asprintf(&string, "%s(%i)", string, current);
                else
                        asprintf(&string, "%s%i", string, current);
        }
bail:
        return string;
}

extern DSCase * DSDesignSpaceCaseWithCaseNumber(const DSDesignSpace * ds, const DSUInteger caseNumber)
{
        DSCase * aCase = NULL;
        DSUInteger * terms = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDSGMA(ds) == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (caseNumber == 0) {
                DSError(M_DS_WRONG ": Case number is 0", A_DS_ERROR);
                goto bail;
        }
        if (caseNumber > DSDSNumCases(ds)) {
                DSError(M_DS_WRONG ": Case number out of bounds", A_DS_ERROR);
                goto bail;
        }
        terms = DSCaseSignatureForCaseNumber(caseNumber, DSDSGMA(ds));
        if (terms != NULL) {
                aCase = DSCaseWithTermsFromDesignSpace(ds, terms);
                DSSecureFree(terms);
        }
bail:
        return aCase;
}

extern DSCase * DSDesignSpaceCaseWithCaseSignature(const DSDesignSpace * ds, const DSUInteger * signature)
{
        DSCase * aCase = NULL;
        DSUInteger caseNumber = 0;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDSGMA(ds) == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (signature == NULL) {
                DSError(M_DS_WRONG ": Signature is NULL", A_DS_ERROR);
                goto bail;
        }
        caseNumber = DSCaseNumberForSignature(signature, DSDSGMA(ds));
        if (caseNumber == 0) {
                DSError(M_DS_WRONG ": Case number out of bounds", A_DS_ERROR);
                goto bail;
        }
        if (caseNumber > DSDesignSpaceNumberOfCases(ds)) {
                DSError(M_DS_WRONG ": Case number out of bounds", A_DS_ERROR);
                goto bail;
        }
//        aCase = DSDSCases(ds)[caseNumber-1];
//        if (aCase == NULL) {
                aCase = DSCaseWithTermsFromGMA(DSDSGMA(ds), signature);
//                DSDSCases(ds)[caseNumber-1] = aCase;
//        }
bail:
        return aCase;
}

extern DSCase * DSDesignSpaceCaseWithCaseSignatureList(const DSDesignSpace *ds, const DSUInteger firstTerm, ...);

extern const bool DSDesignSpaceCaseWithCaseNumberIsValid(const DSDesignSpace *ds, const DSUInteger caseNumber)
{
        bool isValid = false;
        char * string = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (caseNumber == 0) {
                DSError(M_DS_WRONG ": Case number cannot be 0", A_DS_ERROR);
                goto bail;
        }
        if (caseNumber > DSDesignSpaceNumberOfCases(ds)) {
                DSError(M_DS_WRONG ": Case number out of bounds", A_DS_ERROR);
                goto bail;
        }
        if (DSDSValidPool(ds) == NULL)
                DSDesignSpaceCalculateValidityOfCases((DSDesignSpace *)ds);
        string = DSSecureCalloc(sizeof(char), 100);
        sprintf(string, "%d", caseNumber);
        isValid = ((DSDictionaryValueForName(DSDSValidPool(ds), string) != NULL) ? true : false);
//        isValid = DSCaseIsValid(DSDesignSpaceCaseWithCaseNumber(ds, caseNumber));
        DSSecureFree(string);
bail:
        return isValid;
}

extern const bool DSDesignSpaceCaseWithCaseSignatureIsValid(const DSDesignSpace *ds, const DSUInteger * signature)
{
        bool isValid = false;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (signature == NULL) {
                DSError(M_DS_WRONG ": Case number cannot be 0", A_DS_ERROR);
                goto bail;
        }
        isValid = DSDesignSpaceCaseWithCaseNumberIsValid(ds, DSCaseNumberForSignature(signature, DSDSGMA(ds)));//DSCaseIsValid(DSDesignSpaceCaseWithCaseSignature(ds, signature));
bail:
        return isValid;
}

extern const bool DSDesignSpaceCaseWithCaseSignatureListIsValid(const DSDesignSpace *ds, const DSUInteger firstTerm, ...);
//
//extern const DSStack * DSDesignSpaceSubcasesForCaseNumber(DSDesignSpace *ds, const DSUInteger caseNumber)
//{
//        DSStack * subcases = NULL;
//        char * string = NULL;
//        if (ds == NULL) {
//                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        if (caseNumber == 0 || caseNumber > DSDesignSpaceNumberOfCases(ds)) {
//                DSError(M_DS_WRONG ": Case number is out of bounds", A_DS_ERROR);
//                goto bail;
//        }
//        DSDesignSpaceCalculateUnderdeterminedCaseWithCaseNumber(ds, caseNumber);
//        string = DSSecureCalloc(sizeof(char), 100);
//        sprintf(string, "%i", caseNumber);
//        subcases = DSDictionaryValueForName(DSDSCyclical(ds), string);
//        DSSecureFree(string);
//bail:
//        return subcases;
//}

extern const DSGMASystem * DSDesignSpaceGMASystem(const DSDesignSpace * ds)
{
        DSGMASystem *gma = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        gma = DSDSGMA(ds);
bail:
        return gma;
}

extern const DSDictionary * DSDesignSpaceCyclicalCaseDictionary(const DSDesignSpace *ds)
{
        DSDictionary * dictionary = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        dictionary = DSDSCyclical(ds);
bail:
        return dictionary;
}
//
//extern DSDictionary * DSDesignSpaceCycleDictionaryForSignature(const DSDesignSpace * ds, const DSUInteger * signature)
//{
//        DSDictionary * cycleFluxes = NULL;
//        DSUInteger i, index, fluxNumber;
//        DSCycleExtensionData * extensionData;
//        DSExpression * flux;
//        char * name;
//        const DSVariablePool * Xd;
//        if (ds == NULL)  {
//                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        if (signature == NULL) {
//                DSError(M_DS_NULL ": Case signature is NULL", A_DS_ERROR);
//                goto bail;
//        }
//        if (ds->extensionData == NULL) {
//                goto bail;
//        }
//        Xd = DSGMASystemXd(DSDesignSpaceGMASystem(ds));
//        cycleFluxes = DSDictionaryAlloc();
//        extensionData = ds->extensionData;
//        for (i = 0; i < extensionData->numberCycles; i++) {
//                index = extensionData->cycleVariables[i];
//                fluxNumber = signature[2*index+1]-1;
//                name = DSVariableName(DSVariablePoolVariableAtIndex(Xd, extensionData->fluxIndex[i][fluxNumber]));
//                flux = extensionData->fluxEquations[i][fluxNumber];
//                DSDictionaryAddValueWithName(cycleFluxes,
//                                             name,
//                                             flux);
//        }
//        for (i = 0; i < DSVariablePoolNumberOfVariables(Xd); i++) {
//                name = DSVariableName(DSVariablePoolVariableAtIndex(Xd, i));
//                if (DSDictionaryValueForName(cycleFluxes, name) != NULL)
//                        continue;
//                flux = DSDictionaryValueForName(ds->extensionData->cycleFluxes, name);
//                DSDictionaryAddValueWithName(cycleFluxes,
//                                             name,
//                                             flux);
//        }
//bail:
//        return cycleFluxes;
//}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Utility -
#endif

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Additional Constraints
#endif

extern void DSDesignSpaceAddConditions(DSDesignSpace *ds, const DSMatrix * Cd, const DSMatrix * Ci, const DSMatrix * delta)
{
        DSMatrix *temp = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (Cd == NULL) {
                DSError(M_DS_MAT_NULL ": Cd is NULL", A_DS_ERROR);
                goto bail;
        }
        if (delta == NULL) {
                DSError(M_DS_MAT_NULL ": Delta is NULL", A_DS_ERROR);
                goto bail;
        }
        if (Ci == NULL && DSVariablePoolNumberOfVariables(DSDSXi(ds)) != 0) {
                DSError(M_DS_MAT_NULL ": Ci is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSMatrixColumns(Cd) != DSVariablePoolNumberOfVariables(DSDSXd(ds))) {
                DSError(M_DS_WRONG ": Number of dep. variables must match number of columns of Cd", A_DS_ERROR);
                goto bail;
        }
        if (Ci != NULL) {
                if (DSMatrixColumns(Ci) != DSVariablePoolNumberOfVariables(DSDSXi(ds))) {
                        DSError(M_DS_WRONG ": Number of indep. variables must match number of columns of Ci", A_DS_ERROR);
                        goto bail;
                }
                if (DSMatrixRows(Cd) != DSMatrixRows(Ci)) {
                        DSError(M_DS_WRONG ": Rows of Ci must match rows of Cd", A_DS_ERROR);
                        goto bail;
                }
        }
        if (DSMatrixRows(Cd) != DSMatrixRows(delta)) {
                DSError(M_DS_WRONG ": Rows of Cd must match rows of delta", A_DS_ERROR);
                goto bail;
        }
        if (DSDSCd(ds) == NULL) {
                DSDSCd(ds) = DSMatrixCopy(Cd);
                DSDSDelta(ds) = DSMatrixCopy(delta);
                if (Ci != NULL)
                        DSDSCi(ds) = DSMatrixCopy(Ci);
        } else {
                temp = DSMatrixAppendMatrices(DSDSCd(ds), Cd, false);
                DSMatrixFree(DSDSCd(ds));
                DSDSCd(ds) = temp;
                temp = DSMatrixAppendMatrices(DSDSDelta(ds), delta, false);
                DSMatrixFree(DSDSDelta(ds));
                DSDSDelta(ds) = temp;
                if (Ci != NULL) {
                        temp = DSMatrixAppendMatrices(DSDSCi(ds), Ci, false);
                        DSMatrixFree(DSDSCi(ds));
                        DSDSCi(ds) = temp;
                }
        }
bail:
        return;
}

static void dsDesignSpaceConstraintsProcessExponentBasePairs(const DSGMASystem *gma, gma_parseraux_t *current, DSInteger sign,
                                                            DSUInteger index, DSMatrix * Cd, DSMatrix * Ci, DSMatrix *delta)
{
        DSUInteger j, varIndex;
        const char *varName;
        double currentValue;
        if (current == NULL) {
                goto bail;
        }
        if (sign == AUX_SIGN_NEGATIVE) {
                sign = -1;
        } else {
                sign = 1;
        }
        for (j = 0; j < DSGMAParserAuxNumberOfBases(current); j++) {
                if (DSGMAParserAuxBaseAtIndexIsVariable(current, j) == false) {
                        currentValue = DSMatrixDoubleValue(delta, index, 0);
                        currentValue += sign * log10(DSGMAParseAuxsConstantBaseAtIndex(current, j));
                        DSMatrixSetDoubleValue(delta,
                                               index, 0,
                                               currentValue);
                        continue;
                }
                varName = DSGMAParserAuxVariableAtIndex(current, j);
                if (DSVariablePoolHasVariableWithName(DSGMASystemXd(gma), varName) == true) {
                        varIndex = DSVariablePoolIndexOfVariableWithName(DSGMASystemXd(gma), varName);
                        currentValue = DSMatrixDoubleValue(Cd, index, varIndex);
                        currentValue += sign * DSGMAParserAuxExponentAtIndex(current, j);
                        DSMatrixSetDoubleValue(Cd, index, varIndex, currentValue);
                } else if (DSVariablePoolHasVariableWithName(DSGMASystemXi(gma), varName) == true) {
                        varIndex = DSVariablePoolIndexOfVariableWithName(DSGMASystemXi(gma), varName);
                        currentValue = DSMatrixDoubleValue(Ci, index, varIndex);
                        currentValue += sign * DSGMAParserAuxExponentAtIndex(current, j);
                        DSMatrixSetDoubleValue(Ci, index, varIndex, currentValue);
                }
        }
bail:
        return;
}

static void dsDesignSpaceConstraintsCreateSystemMatrices(DSDesignSpace *ds, DSUInteger numberOfConstraints, gma_parseraux_t **aux)
{
        const DSGMASystem * gma;
        gma_parseraux_t *current;
        DSUInteger i;
        DSMatrix * Cd, *Ci, *delta;
        if (ds == NULL) {
                DSError(M_DS_NULL ": GMA being modified is NULL", A_DS_ERROR);
                goto bail;
        }
        gma = DSDesignSpaceGMASystem(ds);
        if (aux == NULL) {
                DSError(M_DS_NULL ": Parser auxiliary data is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSGMASystemXd(gma) == NULL || DSGMASystemXi(gma) == NULL) {
                DSError(M_DS_WRONG ": GMA data is incomplete: Need Xi and Xd", A_DS_ERROR);
                goto bail;
        }
        Cd = DSMatrixCalloc(numberOfConstraints, DSVariablePoolNumberOfVariables(DSGMASystemXd(gma)));
        Ci = DSMatrixCalloc(numberOfConstraints, DSVariablePoolNumberOfVariables(DSGMASystemXi(gma)));
        delta = DSMatrixCalloc(numberOfConstraints, 1);
        for (i = 0; i < numberOfConstraints; i++) {
                current = aux[i];
                dsDesignSpaceConstraintsProcessExponentBasePairs(gma, current, current->sign, i, Cd, Ci, delta);
                current = DSGMAParserAuxNextNode(current);
                dsDesignSpaceConstraintsProcessExponentBasePairs(gma, current, current->sign, i, Cd, Ci, delta);
        }
        DSDesignSpaceAddConditions(ds, Cd, Ci, delta);
        DSMatrixFree(Cd);
        DSMatrixFree(Ci);
        DSMatrixFree(delta);
bail:
        return;
}


static gma_parseraux_t * dsDesignSpaceParseStringToTermList(const char * string)
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
        parser = DSDesignSpaceConstraintParserAlloc(DSSecureMalloc);//DSGMASystemParserAlloc(DSSecureMalloc);
        root = DSGMAParserAuxAlloc();
        parser_aux = root;
        current = tokens;
        while (current != NULL) {
                if (DSExpressionTokenType(current) == DS_EXPRESSION_TOKEN_START) {
                        current = DSExpressionTokenNext(current);
                        continue;
                }
                DSDesignSpaceConstraintParser(parser,
                                              DSExpressionTokenType(current),
                                              current,
                                              ((void**)&parser_aux));
                current = DSExpressionTokenNext(current);
        }
        DSDesignSpaceConstraintParser(parser,
                                      0,
                                      NULL,
                                      ((void **)&parser_aux));
        DSDesignSpaceConstraintParserFree(parser, DSSecureFree);
        DSExpressionTokenFree(tokens);
        if (DSGMAParserAuxParsingFailed(root) == true) {
                DSGMAParserAuxFree(root);
                root = NULL;
        }
bail:
        return root;
}

extern void * DSDesignSpaceTermListForAllStrings(char * const * const strings, const DSUInteger numberOfEquations)
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
                        aux[i] = dsDesignSpaceParseStringToTermList(aString);
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

extern void DSDesignSpaceAddConstraints(DSDesignSpace * ds, const char ** strings, DSUInteger numberOfConstraints)
{
        DSUInteger i;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        gma_parseraux_t **aux = NULL;
        aux = (gma_parseraux_t **)DSDesignSpaceTermListForAllStrings(strings, numberOfConstraints);
        if (aux == NULL) {
                goto bail;
        }
        dsDesignSpaceConstraintsCreateSystemMatrices(ds, numberOfConstraints, aux);
        for (i=0; i < numberOfConstraints; i++) {
                if (aux[i] != NULL)
                        DSGMAParserAuxFree(aux[i]);
        }
        DSSecureFree(aux);
bail:
        return;
}

static DSUInteger ** dsDesignSpaceAllTermSignatures(const DSDesignSpace * ds)
{
        DSUInteger ** termArray = NULL;
        DSUInteger i, j, temp;
        DSGMASystem *gma = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDSGMA(ds) == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSGMASystemSignature(DSDSGMA(ds)) == NULL) {
                DSError(M_DS_WRONG ": GMA signature is NULL", A_DS_ERROR);
                goto bail;
        }
        gma = DSDSGMA(ds);
        termArray = DSSecureCalloc(sizeof(DSUInteger *), DSGMASystemNumberOfCases(gma));
        
        for (i = 0; i < DSGMASystemNumberOfCases(gma); i++) {
                termArray[i] = DSSecureMalloc(sizeof(DSUInteger)*2*DSGMASystemNumberOfEquations(gma));
                temp = i;
                for (j = 0; j < 2*DSGMASystemNumberOfEquations(gma); j++) {
                        termArray[i][j] = (temp % DSGMASystemSignature(gma)[j])+1;
                        temp /= DSGMASystemSignature(gma)[j];
                }
        }
bail:
        return termArray;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Parallel and series functions for case validity and cycles.
#endif

static void dsDesignSpaceCalculateCyclicalCasesSeries(DSDesignSpace *ds)
{
        DSUInteger i, numberOfCases;
        DSCase * aCase = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfCases = DSDesignSpaceNumberOfCases(ds);
        if (numberOfCases == 0) {
                goto bail;
        }
        for (i = 0; i < numberOfCases; i++) {
                aCase = DSDesignSpaceCaseWithCaseNumber(ds, i+1);
                DSDesignSpaceCalculateCyclicalCase(ds, aCase);
                DSCaseFree(aCase);
        }
bail:
        return;
}

static void dsDesignSpaceCalculateCyclicalCasesParallelBSD(DSDesignSpace *ds)
{
        DSUInteger i;
        DSUInteger numberOfThreads = (DSUInteger)sysconf(_SC_NPROCESSORS_ONLN);
        DSUInteger numberOfCases;
        pthread_t * threads = NULL;
        pthread_attr_t attr;
        ds_parallelstack_t *stack;
        struct pthread_struct *pdatas;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDSGMA(ds) == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSGMASystemSignature(DSDSGMA(ds)) == NULL) {
                DSError(M_DS_WRONG ": GMA signature is NULL", A_DS_ERROR);
                goto bail;
        }
        numberOfCases = DSDesignSpaceNumberOfCases(ds);
        if (numberOfCases == 0) {
                DSError(M_DS_WRONG ": Number of cases to process must be more than 0", A_DS_ERROR);
                goto bail;
        }
        DSParallelInitMutexes();
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        /* Should optimize number of threads to system Optimal ~ 2*number of processors */
        
        /* Initializing parallel data stacks and pthreads data structure */
        pdatas = DSSecureCalloc(sizeof(struct pthread_struct),numberOfThreads);
        stack = DSParallelStackAlloc();
        for (i = 0; i < numberOfThreads; i++) {
                pdatas[i].ds = ds;
                pdatas[i].stack = stack;
        }
        for (i = 0; i < numberOfCases; i++)
                DSParallelStackPush(stack, i+1);
        
        threads = DSSecureCalloc(sizeof(pthread_t), numberOfThreads);
        for (i = 0; i < numberOfThreads; i++)
                pthread_create(&threads[i], &attr, DSParallelWorkerCyclicalCases, (void *)(&pdatas[i]));
        /* Joining all the N-threads, indicating all cases have been processed */
        for (i = 0; i < numberOfThreads; i++)
                pthread_join(threads[i], NULL);
        DSParallelStackFree(stack);
        DSSecureFree(threads);
        DSSecureFree(pdatas);
        pthread_attr_destroy(&attr);
bail:
        return;
}

static void dsDesignSpaceCalculateValiditySeries(DSDesignSpace *ds)
{
        DSUInteger i;
        char * string = NULL;
        DSCase * aCase = NULL;
        const DSCyclicalCase * cyclicalCase;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDSGMA(ds) == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDSValidPool(ds) != NULL) {
                DSError(M_DS_WRONG ": Valid cases has already been calculated.", A_DS_WARN);
                goto bail;
        }
        
        if (DSGMASystemSignature(DSDSGMA(ds)) == NULL) {
                DSError(M_DS_WRONG ": GMA signature is NULL", A_DS_ERROR);
                goto bail;
        }
        DSDSValidPool(ds) = DSDictionaryAlloc();
        string = DSSecureCalloc(sizeof(char), 100);
        for (i = 0; i < DSDSNumCases(ds); i++) {
                aCase = DSDesignSpaceCaseWithCaseNumber(ds, i+1);
                if (aCase == NULL)
                        continue;
                sprintf(string, "%d", i+1);
                if (DSCaseIsValid(aCase) == true) {
                        DSDictionaryAddValueWithName(ds->validCases, string, (void*)1);
                } else if (DSDictionaryValueForName(ds->cyclicalCases, string) != NULL) {
                        cyclicalCase = DSDesignSpaceCyclicalCaseWithCaseNumber(ds, i+1);
                        if (DSCyclicalCaseIsValid(cyclicalCase) == true)
                                DSDictionaryAddValueWithName(ds->validCases, string, (void*)1);
                }
                DSCaseFree(aCase);
                
        }
        DSSecureFree(string);
        
bail:
        return;
}

static DSDictionary * dsDesignSpaceCalculateValidityOfCaseSetParallelBSD(DSDesignSpace *ds, DSUInteger numberOfCases, DSCase ** cases)
{
        DSDictionary * caseDictionary = NULL;
        DSUInteger i, j, numberValid = 0;
        DSUInteger validCaseNumbers = 0;
        const char * name;
        long int numberOfThreads = sysconf(_SC_NPROCESSORS_ONLN);
        pthread_t * threads = NULL;
        pthread_attr_t attr;
        ds_parallelstack_t *stack;
        struct pthread_struct *pdatas;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDSGMA(ds) == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSGMASystemSignature(DSDSGMA(ds)) == NULL) {
                DSError(M_DS_WRONG ": GMA signature is NULL", A_DS_ERROR);
                goto bail;
        }
        caseDictionary = DSDictionaryAlloc();
        if (numberOfCases == 0) {
                goto bail;
        }
//        numberValid = DSDesignSpaceNumberOfValidCases(ds);
//        if (numberValid == 0)
//                goto bail;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        /* Should optimize number of threads to system */
        
        /* Initializing parallel data stacks and pthreads data structure */
        
        stack = DSParallelStackAlloc();
        stack->argument_type = DS_STACK_ARG_CASE;
        pdatas = DSSecureMalloc(sizeof(struct pthread_struct)*numberOfThreads);
        for (i = 0; i < numberOfThreads; i++) {
                pdatas[i].ds = ds;
                pdatas[i].stack = stack;
                pdatas[i].numberOfArguments = 0;
                pdatas[i].functionArguments = NULL;//DSSecureMalloc(sizeof(DSVariablePool *)*2);
        }
        for (i = 0; i < numberOfCases; i++) {
                DSParallelStackPush(stack, cases[i]);
        }
        threads = DSSecureCalloc(sizeof(pthread_t), numberOfThreads);
        /* Creating the N-threads with their data */
        for (i = 0; i < numberOfThreads; i++)
                pthread_create(&threads[i], &attr, DSParallelWorkerValidity, (void *)(&pdatas[i]));
        /* Joining all the N-threads, indicating all cases have been processed */
        for (i = 0; i < numberOfThreads; i++) {
                pthread_join(threads[i], NULL);
                for (j = 0; j < DSDictionaryCount(pdatas[i].returnPointer); j++) {
                        name = DSDictionaryNames((DSDictionary *)pdatas[i].returnPointer)[j];
                        DSDictionaryAddValueWithName(caseDictionary, name, DSDictionaryValueForName(pdatas[i].returnPointer, name));
                }
                DSDictionaryFree((DSDictionary*)pdatas[i].returnPointer);
                DSSecureFree(pdatas[i].functionArguments);
        }
        DSParallelStackFree(stack);
        DSSecureFree(threads);
        DSSecureFree(pdatas);
        pthread_attr_destroy(&attr);
bail:
        return caseDictionary;
}


static void  dsDesignSpaceCalculateValidityParallelBSD(DSDesignSpace *ds)
{
        DSUInteger i;
        long int numberOfThreads = sysconf(_SC_NPROCESSORS_ONLN);
        pthread_t * threads = NULL;
        pthread_attr_t attr;
        ds_parallelstack_t *stack;
        struct pthread_struct *pdatas;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDSGMA(ds) == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSGMASystemSignature(DSDSGMA(ds)) == NULL) {
                DSError(M_DS_WRONG ": GMA signature is NULL", A_DS_ERROR);
                goto bail;
        }
        DSDSValidPool(ds) = DSDictionaryAlloc();//DSVariablePoolAlloc();
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        /* Should optimize number of threads to system */
        
        /* Initializing parallel data stacks and pthreads data structure */
        
        stack = DSParallelStackAlloc();
        pdatas = DSSecureMalloc(sizeof(struct pthread_struct)*numberOfThreads);
        for (i = 0; i < numberOfThreads; i++) {
                pdatas[i].ds = ds;
                pdatas[i].stack = stack;
        }
        for (i = 0; i < DSDSNumCases(ds); i++)
                DSParallelStackPush(stack, i+1);
        
        threads = DSSecureCalloc(sizeof(pthread_t), numberOfThreads);
        /* Creating the N-threads with their data */
        for (i = 0; i < numberOfThreads; i++)
                pthread_create(&threads[i], &attr, DSParallelWorkerValidity, (void *)(&pdatas[i]));
        /* Joining all the N-threads, indicating all cases have been processed */
        for (i = 0; i < numberOfThreads; i++)
                pthread_join(threads[i], NULL);
        DSParallelStackFree(stack);
        
        DSSecureFree(threads);
        DSSecureFree(pdatas);
        pthread_attr_destroy(&attr);
bail:
        return;
}

static DSDictionary * dsDesignSpaceCalculateAllValidCasesByResolvingCyclicalCasesSeries(DSDesignSpace *ds)
{
        DSDictionary * caseDictionary = NULL, *subcaseDictionary;
        DSUInteger i, j, numberValid = 0, numberValidSubcases;
        DSUInteger validCaseNumbers = 0;
        char nameString[100], * subcaseString = NULL;
        const char **subcaseNames;
        DSCase * aCase = NULL;
        const DSCyclicalCase * cyclicalCase = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        caseDictionary = DSDictionaryAlloc();
        numberValid = DSDesignSpaceNumberOfValidCases(ds);
        if (numberValid == 0)
                goto bail;
        for (i = 0; i < numberValid; i++) {
                validCaseNumbers = atoi(ds->validCases->names[i]);
                aCase = DSDesignSpaceCaseWithCaseNumber(ds, validCaseNumbers);
                sprintf(nameString, "%d", validCaseNumbers);
                cyclicalCase = DSDesignSpaceCyclicalCaseWithCaseNumber(ds, validCaseNumbers);
                if (cyclicalCase != NULL) {
                        subcaseDictionary = DSCyclicalCaseCalculateAllValidSubcasesByResolvingCyclicalCases((DSCyclicalCase *)cyclicalCase);
                        if (subcaseDictionary == NULL) {
                                DSCaseFree(aCase);
                                continue;
                        }
                        numberValidSubcases = DSDictionaryCount(subcaseDictionary);
                        subcaseNames = DSDictionaryNames(subcaseDictionary);
                        for (j = 0; j < numberValidSubcases; j++) {
                                asprintf(&subcaseString, "%s_%s", nameString, subcaseNames[j]);
                                DSDictionaryAddValueWithName(caseDictionary, subcaseString, DSDictionaryValueForName(subcaseDictionary, subcaseNames[j]));
                        }
                        DSDictionaryFree(subcaseDictionary);
                } else if (DSCaseIsValid(aCase) == true) {
                        DSDictionaryAddValueWithName(caseDictionary, nameString, aCase);
                } else {
                        DSCaseFree(aCase);
                }
        }
bail:
        if (subcaseString != NULL)
                DSSecureFree(subcaseString);
        return caseDictionary;
}

static DSDictionary * dsDesignSpaceCalculateAllValidCasesByResolvingCyclicalCasesSeriesParallelBSD(DSDesignSpace *ds)
{
        DSDictionary * caseDictionary = NULL;
        DSUInteger i, j, numberValid = 0;
        DSUInteger validCaseNumbers = 0;
        const char * name;
        long int numberOfThreads = sysconf(_SC_NPROCESSORS_ONLN);
        pthread_t * threads = NULL;
        pthread_attr_t attr;
        ds_parallelstack_t *stack;
        struct pthread_struct *pdatas;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDSGMA(ds) == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSGMASystemSignature(DSDSGMA(ds)) == NULL) {
                DSError(M_DS_WRONG ": GMA signature is NULL", A_DS_ERROR);
                goto bail;
        }
        caseDictionary = DSDictionaryAlloc();
        numberValid = DSDesignSpaceNumberOfValidCases(ds);
        if (numberValid == 0)
                goto bail;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        /* Should optimize number of threads to system */
        
        /* Initializing parallel data stacks and pthreads data structure */
        
        stack = DSParallelStackAlloc();
        pdatas = DSSecureMalloc(sizeof(struct pthread_struct)*numberOfThreads);
        for (i = 0; i < numberOfThreads; i++) {
                pdatas[i].ds = ds;
                pdatas[i].stack = stack;
        }
        for (i = 0; i < numberValid; i++) {
                validCaseNumbers = atoi(ds->validCases->names[i]);
                DSParallelStackPush(stack, validCaseNumbers);
        }
        threads = DSSecureCalloc(sizeof(pthread_t), numberOfThreads);
        /* Creating the N-threads with their data */
        for (i = 0; i < numberOfThreads; i++)
                pthread_create(&threads[i], &attr, DSParallelWorkerValidityResolveCycles, (void *)(&pdatas[i]));
        /* Joining all the N-threads, indicating all cases have been processed */
        for (i = 0; i < numberOfThreads; i++) {
                pthread_join(threads[i], NULL);
                for (j = 0; j < DSDictionaryCount(pdatas[i].returnPointer); j++) {
                        name = DSDictionaryNames((DSDictionary *)pdatas[i].returnPointer)[j];
                        DSDictionaryAddValueWithName(caseDictionary, name, DSDictionaryValueForName(pdatas[i].returnPointer, name));
                }
                DSDictionaryFree((DSDictionary*)pdatas[i].returnPointer);
        }
        DSParallelStackFree(stack);
        DSSecureFree(threads);
        DSSecureFree(pdatas);
        pthread_attr_destroy(&attr);
bail:
        return caseDictionary;
}

static DSDictionary * dsDesignSpaceCalculateAllValidCasesForSliceByResolvingCyclicalCasesSeries(DSDesignSpace *ds,
                                                                                                const DSVariablePool * lower,
                                                                                                const DSVariablePool *upper)
{
        DSDictionary * caseDictionary = NULL, *subcaseDictionary;
        DSUInteger i, j, numberValid = 0, numberValidSubcases;
        DSUInteger validCaseNumbers = 0;
        char nameString[100], * subcaseString = NULL;
        const char **subcaseNames;
        DSCase * aCase = NULL;
        const DSCyclicalCase * cyclicalCase = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (lower == NULL || upper == NULL) {
                DSError(M_DS_VAR_NULL, A_DS_ERROR);
                goto bail;
        }
        caseDictionary = DSDictionaryAlloc();
        numberValid = DSDesignSpaceNumberOfValidCases(ds);
        if (numberValid == 0)
                goto bail;
        for (i = 0; i < numberValid; i++) {
                validCaseNumbers = atoi(ds->validCases->names[i]);
                aCase = DSDesignSpaceCaseWithCaseNumber(ds, validCaseNumbers);
                sprintf(nameString, "%d", validCaseNumbers);
                cyclicalCase = DSDesignSpaceCyclicalCaseWithCaseNumber(ds, validCaseNumbers);
                if (cyclicalCase != NULL) {
                        subcaseDictionary = DSCyclicalCaseCalculateAllValidSubcasesForSliceByResolvingCyclicalCases((DSCyclicalCase *)cyclicalCase,
                                                                                                                    lower,
                                                                                                                    upper);
                        if (subcaseDictionary == NULL) {
                                DSCaseFree(aCase);
                                continue;
                        }
                        numberValidSubcases = DSDictionaryCount(subcaseDictionary);
                        subcaseNames = DSDictionaryNames(subcaseDictionary);
                        for (j = 0; j < numberValidSubcases; j++) {
                                asprintf(&subcaseString, "%s_%s", nameString, subcaseNames[j]);
                                DSDictionaryAddValueWithName(caseDictionary, subcaseString, DSDictionaryValueForName(subcaseDictionary, subcaseNames[j]));
                        }
                        DSDictionaryFree(subcaseDictionary);
                } else if (DSCaseIsValidAtSlice(aCase, lower, upper) == true) {
                        DSDictionaryAddValueWithName(caseDictionary, nameString, aCase);
                } else {
                        DSCaseFree(aCase);
                }
        }
bail:
        if (subcaseString != NULL)
                DSSecureFree(subcaseString);
        return caseDictionary;
}

static DSDictionary * dsDesignSpaceCalculateAllValidCasesForSliceByResolvingCyclicalCasesSeriesParallelBSD(DSDesignSpace *ds,
                                                                                                           const DSVariablePool * lower,
                                                                                                           const DSVariablePool * upper)
{
        DSDictionary * caseDictionary = NULL;
        DSUInteger i, j, numberValid = 0;
        DSUInteger validCaseNumbers = 0;
        const char * name;
        long int numberOfThreads = sysconf(_SC_NPROCESSORS_ONLN);
        pthread_t * threads = NULL;
        pthread_attr_t attr;
        ds_parallelstack_t *stack;
        struct pthread_struct *pdatas;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDSGMA(ds) == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSGMASystemSignature(DSDSGMA(ds)) == NULL) {
                DSError(M_DS_WRONG ": GMA signature is NULL", A_DS_ERROR);
                goto bail;
        }
        if (lower == NULL || upper == NULL) {
                DSError(M_DS_VAR_NULL, A_DS_ERROR);
                goto bail;
        }
        caseDictionary = DSDictionaryAlloc();
        numberValid = DSDesignSpaceNumberOfValidCases(ds);
        if (numberValid == 0)
                goto bail;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        /* Should optimize number of threads to system */
        
        /* Initializing parallel data stacks and pthreads data structure */
        
        stack = DSParallelStackAlloc();
        pdatas = DSSecureMalloc(sizeof(struct pthread_struct)*numberOfThreads);
        for (i = 0; i < numberOfThreads; i++) {
                pdatas[i].ds = ds;
                pdatas[i].stack = stack;
                pdatas[i].numberOfArguments = 2;
                pdatas[i].functionArguments = DSSecureMalloc(sizeof(DSVariablePool *)*2);
                pdatas[i].functionArguments[0] = (void*)lower;
                pdatas[i].functionArguments[1] = (void*)upper;
        }
        for (i = 0; i < numberValid; i++) {
                validCaseNumbers = atoi(ds->validCases->names[i]);
                DSParallelStackPush(stack, validCaseNumbers);
        }
        threads = DSSecureCalloc(sizeof(pthread_t), numberOfThreads);
        /* Creating the N-threads with their data */
        for (i = 0; i < numberOfThreads; i++)
                pthread_create(&threads[i], &attr, DSParallelWorkerValidityForSliceResolveCycles, (void *)(&pdatas[i]));
        /* Joining all the N-threads, indicating all cases have been processed */
        for (i = 0; i < numberOfThreads; i++) {
                pthread_join(threads[i], NULL);
                for (j = 0; j < DSDictionaryCount(pdatas[i].returnPointer); j++) {
                        name = DSDictionaryNames((DSDictionary *)pdatas[i].returnPointer)[j];
                        DSDictionaryAddValueWithName(caseDictionary, name, DSDictionaryValueForName(pdatas[i].returnPointer, name));
                }
                DSDictionaryFree((DSDictionary*)pdatas[i].returnPointer);
                DSSecureFree(pdatas[i].functionArguments);
        }
        DSParallelStackFree(stack);
        DSSecureFree(threads);
        DSSecureFree(pdatas);
        pthread_attr_destroy(&attr);
bail:
        return caseDictionary;
}

static DSDictionary * dsDesignSpaceCalculateAllValidCasesForSliceSeries(DSDesignSpace *ds, const DSVariablePool *lower, const DSVariablePool *upper)
{
        DSDictionary * caseDictionary = NULL;
        DSUInteger i, numberValid = 0;
        DSUInteger validCaseNumbers = 0;
        char nameString[100];
        DSCase * aCase = NULL;
        const DSCyclicalCase * cyclicalCase = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        caseDictionary = DSDictionaryAlloc();
        numberValid = DSDesignSpaceNumberOfValidCases(ds);
        if (numberValid == 0)
                goto bail;
        for (i = 0; i < numberValid; i++) {
                validCaseNumbers = atoi(ds->validCases->names[i]);
                aCase = DSDesignSpaceCaseWithCaseNumber(ds, validCaseNumbers);
                sprintf(nameString, "%d", validCaseNumbers);
                cyclicalCase = DSDesignSpaceCyclicalCaseWithCaseNumber(ds, validCaseNumbers);
                if (cyclicalCase != NULL) {
                        if (DSCyclicalCaseIsValidAtSlice(cyclicalCase, lower, upper) == true) {
                                DSDictionaryAddValueWithName(caseDictionary, nameString, aCase);
                        }
                } else if (DSCaseIsValidAtSlice(aCase, lower, upper) == true) {
                        DSDictionaryAddValueWithName(caseDictionary, nameString, aCase);
                } else {
                        DSCaseFree(aCase);
                }
        }
bail:
        return caseDictionary;
}

static DSDictionary * dsDesignSpaceCalculateValidityAtSliceParallelBSD(DSDesignSpace *ds, const DSVariablePool * lower, const DSVariablePool * upper)
{
        DSDictionary * caseDictionary = NULL;
        DSUInteger i, j, numberValid = 0;
        DSUInteger validCaseNumbers = 0;
        const char * name;
        long int numberOfThreads = sysconf(_SC_NPROCESSORS_ONLN);
        pthread_t * threads = NULL;
        pthread_attr_t attr;
        ds_parallelstack_t *stack;
        struct pthread_struct *pdatas;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDSGMA(ds) == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSGMASystemSignature(DSDSGMA(ds)) == NULL) {
                DSError(M_DS_WRONG ": GMA signature is NULL", A_DS_ERROR);
                goto bail;
        }
        caseDictionary = DSDictionaryAlloc();
        numberValid = DSDesignSpaceNumberOfValidCases(ds);
        if (numberValid == 0)
                goto bail;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        /* Should optimize number of threads to system */
        
        /* Initializing parallel data stacks and pthreads data structure */
        
        stack = DSParallelStackAlloc();
        pdatas = DSSecureMalloc(sizeof(struct pthread_struct)*numberOfThreads);
        for (i = 0; i < numberOfThreads; i++) {
                pdatas[i].ds = ds;
                pdatas[i].stack = stack;
                pdatas[i].numberOfArguments = 2;
                pdatas[i].functionArguments = DSSecureMalloc(sizeof(DSVariablePool *)*2);
                pdatas[i].functionArguments[0] = (void*)lower;
                pdatas[i].functionArguments[1] = (void*)upper;
        }
        for (i = 0; i < numberValid; i++) {
                validCaseNumbers = atoi(ds->validCases->names[i]);
                DSParallelStackPush(stack, validCaseNumbers);
        }
        threads = DSSecureCalloc(sizeof(pthread_t), numberOfThreads);
        /* Creating the N-threads with their data */
        for (i = 0; i < numberOfThreads; i++)
                pthread_create(&threads[i], &attr, DSParallelWorkerValiditySlice, (void *)(&pdatas[i]));
        /* Joining all the N-threads, indicating all cases have been processed */
        for (i = 0; i < numberOfThreads; i++) {
                pthread_join(threads[i], NULL);
                for (j = 0; j < DSDictionaryCount(pdatas[i].returnPointer); j++) {
                        name = DSDictionaryNames((DSDictionary *)pdatas[i].returnPointer)[j];
                        DSDictionaryAddValueWithName(caseDictionary, name, DSDictionaryValueForName(pdatas[i].returnPointer, name));
                }
                DSDictionaryFree((DSDictionary*)pdatas[i].returnPointer);
                DSSecureFree(pdatas[i].functionArguments);
        }
        DSParallelStackFree(stack);
        DSSecureFree(threads);
        DSSecureFree(pdatas);
        pthread_attr_destroy(&attr);
bail:
        return caseDictionary;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Case and Case validity
#endif


extern DSUInteger * DSDesignSpaceCaseNumbersWithPrefix(const DSDesignSpace * ds, const DSUInteger sizeOfPrefix, const DSUInteger *prefix, DSUInteger * numberOfCases)
{
        DSUInteger i, j, pos, number_alt,  signature_length, current, temp;
        DSUInteger * caseNumbers = NULL, *signature;
        const DSUInteger * termList = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (prefix == NULL) {
                DSError(M_DS_NULL ": Prefix array is NULL", A_DS_ERROR);
                goto bail;
        }
        if (sizeOfPrefix == 0) {
                DSError(M_DS_NULL ": Array of cases to calculate is NULL", A_DS_ERROR);
                goto bail;
        }
        if (numberOfCases == NULL) {
                numberOfCases = &number_alt;
        }
        termList = DSDesignSpaceSignature(ds);
        signature_length = DSDesignSpaceNumberOfEquations(ds)*2;
        signature = DSSecureCalloc(signature_length, sizeof(DSUInteger));
        for (i = 0; i < sizeOfPrefix; i++) {
                signature[i] = prefix[i];
        }
        *numberOfCases = 1;
        for (i = sizeOfPrefix; i < signature_length; i++) {
                *numberOfCases *= termList[i];
        }
        caseNumbers = DSSecureMalloc(sizeof(DSUInteger)*(*numberOfCases));
        for (i = 0; i < *numberOfCases; i++) {
                current = i;
                temp = 1;
                for (j = 0; j < signature_length-sizeOfPrefix; j++) {
                        pos = signature_length-(j+1);
                        signature[pos] = (current % termList[pos])+1;
                        current = current / termList[pos];
                        temp *= termList[pos];
                }
                caseNumbers[i] = DSCaseNumberForSignature(signature, DSDesignSpaceGMASystem(ds));
        }
bail:
        return caseNumbers;
}

static DSCase ** dsDesignSpaceCalculateCasesFromPrefixesParallelBSD(DSDesignSpace *ds, DSUInteger * numberOfCases, const DSUInteger numberOfPrefixes, const DSUInteger sizeOfPrefix, DSUInteger ** prefixes)
{
        DSUInteger i, j, numberInPrefix;
        DSUInteger numberOfThreads = (DSUInteger)sysconf(_SC_NPROCESSORS_ONLN);
        DSUInteger *temp;
        pthread_t * threads = NULL;
        pthread_attr_t attr;
        ds_parallelstack_t *stack;
        DSCase ** processedCases = NULL;
        struct pthread_struct *pdatas;
        
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (numberOfCases == NULL) {
                DSError(M_DS_WRONG ": Pointer to number of cases to process must not be NULL", A_DS_ERROR);
                goto bail;
        }
        if (prefixes == NULL) {
                DSError(M_DS_NULL ": Array of prefixes cannot be NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSDSGMA(ds) == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSGMASystemSignature(DSDSGMA(ds)) == NULL) {
                DSError(M_DS_WRONG ": GMA signature is NULL", A_DS_ERROR);
                goto bail;
        }

        DSParallelInitMutexes();
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        /* Should optimize number of threads to system Optimal ~ 2*number of processors */
        
        /* Initializing parallel data stacks and pthreads data structure */
        pdatas = DSSecureMalloc(sizeof(struct pthread_struct)*numberOfThreads);
        stack = DSParallelStackAlloc();
        for (i = 0; i < numberOfPrefixes; i++) {
                temp = DSDesignSpaceCaseNumbersWithPrefix(ds, sizeOfPrefix, prefixes[i], &numberInPrefix);
                *numberOfCases += numberInPrefix;
                for (j = 0; j < numberInPrefix; j++) {
                        DSParallelStackPush(stack, temp[j]);
                }
                DSSecureFree(temp);
        }
        processedCases = DSSecureCalloc(sizeof(DSCase *), *numberOfCases);
        stack->cases = processedCases;
        for (i = 0; i < numberOfThreads; i++) {
                pdatas[i].ds = ds;
                pdatas[i].stack = stack;
        }
        threads = DSSecureCalloc(sizeof(pthread_t), numberOfThreads);
        for (i = 0; i < numberOfThreads; i++)
                pthread_create(&threads[i], &attr, DSParallelWorkerCases, (void *)(&pdatas[i]));
        /* Joining all the N-threads, indicating all cases have been processed */
        for (i = 0; i < numberOfThreads; i++)
                pthread_join(threads[i], NULL);
        
        DSParallelStackFree(stack);
        DSSecureFree(threads);
        DSSecureFree(pdatas);
        pthread_attr_destroy(&attr);
bail:
        return processedCases;
}

extern DSCase ** DSDesignSpaceCalculateCasesWithPrefixSignatures(DSDesignSpace *ds, DSUInteger * numberOfCases, const DSUInteger numberOfPrefixes, const DSUInteger sizeOfPrefix, DSUInteger **prefixes)
{
        DSCase ** allCases = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (numberOfPrefixes == 0) {
                DSError(M_DS_WRONG ": Number of cases to calculate is 0", A_DS_WARN);
                goto bail;
        }
        if (prefixes == NULL) {
                DSError(M_DS_NULL ": Array of cases to calculate is NULL", A_DS_ERROR);
                goto bail;
        }
        if (sizeOfPrefix == 0) {
                DSError(M_DS_NULL ": Array of cases to calculate is NULL", A_DS_ERROR);
                goto bail;
        }
        if (numberOfCases == NULL) {
                DSError(M_DS_WRONG ": Number of cases to calculate is 0", A_DS_WARN);
                goto bail;
        }
        allCases = dsDesignSpaceCalculateCasesFromPrefixesParallelBSD(ds, numberOfCases, numberOfPrefixes, sizeOfPrefix, prefixes);
bail:
        return allCases;
}

static DSCase ** dsDesignSpaceCalculateCasesParallelBSD(DSDesignSpace *ds, const DSUInteger numberOfCases, DSUInteger *cases)
{
        DSUInteger i;
        DSUInteger numberOfThreads = (DSUInteger)sysconf(_SC_NPROCESSORS_ONLN);
        pthread_t * threads = NULL;
        pthread_attr_t attr;
        ds_parallelstack_t *stack;
        DSCase ** processedCases = NULL;
        struct pthread_struct *pdatas;
        
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (numberOfCases == 0) {
                DSError(M_DS_WRONG ": Number of cases to process must be more than 0", A_DS_ERROR);
                goto bail;
        }
        if (cases == NULL) {
                DSError(M_DS_NULL ": Array of cases cannot be NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSDSGMA(ds) == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSGMASystemSignature(DSDSGMA(ds)) == NULL) {
                DSError(M_DS_WRONG ": GMA signature is NULL", A_DS_ERROR);
                goto bail;
        }
        
        DSParallelInitMutexes();
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        processedCases = DSSecureCalloc(sizeof(DSCase *), numberOfCases);
        /* Should optimize number of threads to system Optimal ~ 2*number of processors */
        
        /* Initializing parallel data stacks and pthreads data structure */
        pdatas = DSSecureMalloc(sizeof(struct pthread_struct)*numberOfThreads);
        stack = DSParallelStackAlloc();
        stack->cases = processedCases;
        for (i = 0; i < numberOfThreads; i++) {
                pdatas[i].ds = ds;
                pdatas[i].stack = stack;
        }
        for (i = 0; i < numberOfCases; i++)
                DSParallelStackPush(stack, cases[i]);
        
        threads = DSSecureCalloc(sizeof(pthread_t), numberOfThreads);
        
        for (i = 0; i < numberOfThreads; i++)
                pthread_create(&threads[i], &attr, DSParallelWorkerCases, (void *)(&pdatas[i]));
        /* Joining all the N-threads, indicating all cases have been processed */
        for (i = 0; i < numberOfThreads; i++)
                pthread_join(threads[i], NULL);
        
        DSParallelStackFree(stack);
        
        DSSecureFree(threads);
        DSSecureFree(pdatas);
        pthread_attr_destroy(&attr);
bail:
        return processedCases;
}

static DSUInteger * dsDesignSpaceCalculateCasesNumbersWithPrefixSignatures(DSDesignSpace *ds, DSUInteger * numberOfCases, const DSUInteger numberOfPrefixes, const DSUInteger sizeOfPrefix, const DSUInteger **prefixes)
{
        DSUInteger i, j, numberInPrefix;
        DSUInteger * temp, * caseNumbers = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (numberOfPrefixes == 0) {
                DSError(M_DS_WRONG ": Number of cases to calculate is 0", A_DS_WARN);
                goto bail;
        }
        if (prefixes == NULL) {
                DSError(M_DS_NULL ": Array of cases to calculate is NULL", A_DS_ERROR);
                goto bail;
        }
        if (sizeOfPrefix == 0) {
                DSError(M_DS_NULL ": Array of cases to calculate is NULL", A_DS_ERROR);
                goto bail;
        }
        if (numberOfCases == NULL) {
                DSError(M_DS_WRONG ": Number of cases to calculate is 0", A_DS_WARN);
                goto bail;
        }
        *numberOfCases = 0;
        for (i = 0; i < numberOfPrefixes; i++) {
                temp = DSDesignSpaceCaseNumbersWithPrefix(ds, sizeOfPrefix, prefixes[i], &numberInPrefix);
                *numberOfCases += numberInPrefix;
                if (caseNumbers == NULL) {
                        caseNumbers = DSSecureMalloc(sizeof(DSUInteger)**numberOfCases);
                } else {
                        caseNumbers = DSSecureRealloc(caseNumbers, sizeof(DSUInteger)**numberOfCases);
                }
                for (j = 0; j < numberInPrefix; j++) {
                        caseNumbers[*numberOfCases-numberInPrefix+j] = temp[j];
                        printf("Temp:%i %i %i\n", temp[j], DSDesignSpaceNumberOfCases(ds), i);
                }
                DSSecureFree(temp);
        }
bail:
        return caseNumbers;
}
static DSDesignSpace * dsDesignSpaceSubDesignSpaceByRemovingLastEquation(DSDesignSpace * ds)
{
        DSDesignSpace * subds = NULL;
        DSVariablePool *vars, * Xd_a;
        DSUInteger i, j, numberOfEquations;
        DSExpression ** equations, *lhs;
        const char * name;
        char ** strings;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfEquations = DSDesignSpaceNumberOfEquations(ds);
        if (numberOfEquations == 1) {
                goto bail;
        }
        numberOfEquations--;
        equations = DSDesignSpaceEquations(ds);
        Xd_a = DSVariablePoolAlloc();
        strings = DSSecureMalloc(sizeof(char *)*numberOfEquations);
//        numberOfXd = 0;
        for (i = 0; i < numberOfEquations; i++) {
                strings[i] = DSExpressionAsString(equations[i]);
                lhs = DSExpressionEquationLHSExpression(equations[i]);
                vars = DSExpressionVariablesInExpression(lhs);
                if (DSVariablePoolNumberOfVariables(vars) == 1) {
                        DSExpressionFree(lhs);
                        DSSecureFree(vars);
                        DSExpressionFree(equations[i]);
                        continue;
                }
                DSExpressionFree(lhs);
                DSSecureFree(vars);
                lhs = DSExpressionEquationRHSExpression(equations[i]);
                vars = DSExpressionVariablesInExpression(lhs);
                for (j = 0; j < DSVariablePoolNumberOfVariables(vars); j++) {
                        name = DSVariableName(DSVariablePoolVariableAtIndex(vars, j));
                        if (DSVariablePoolHasVariableWithName(DSGMASystemXd_a(DSDesignSpaceGMASystem(ds)), name) == false)
                                continue;
                        if (DSVariablePoolHasVariableWithName(Xd_a, name) == true)
                                continue;
                        DSVariablePoolAddVariableWithName(Xd_a, name);
                        break;
                }
                DSExpressionFree(lhs);
                DSSecureFree(vars);
                DSExpressionFree(equations[i]);
        }
        DSSecureFree(equations);
        DSVariablePoolPrint(Xd_a);
        subds = DSDesignSpaceByParsingStrings(strings, Xd_a, numberOfEquations);
        for (i = 0; i < numberOfEquations; i++) {
                DSSecureFree(strings[i]);
        }
        DSVariablePoolFree(Xd_a);
        DSSecureFree(strings);
bail:
        return subds;
}

static void dsDesignSpaceCalculatePrunedValidityParallelBSD(DSDesignSpace *ds, const DSUInteger numberOfCases, const DSUInteger * caseNumber)
{
        DSUInteger i;
        long int numberOfThreads = sysconf(_SC_NPROCESSORS_ONLN);
        pthread_t * threads = NULL;
        pthread_attr_t attr;
        ds_parallelstack_t *stack;
        struct pthread_struct *pdatas;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDSGMA(ds) == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSGMASystemSignature(DSDSGMA(ds)) == NULL) {
                DSError(M_DS_WRONG ": GMA signature is NULL", A_DS_ERROR);
                goto bail;
        }
        DSDSValidPool(ds) = DSDictionaryAlloc();//DSVariablePoolAlloc();
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        /* Should optimize number of threads to system */
        
        /* Initializing parallel data stacks and pthreads data structure */
        
        stack = DSParallelStackAlloc();
        pdatas = DSSecureMalloc(sizeof(struct pthread_struct)*numberOfThreads);
        for (i = 0; i < numberOfThreads; i++) {
                pdatas[i].ds = ds;
                pdatas[i].stack = stack;
        }
        for (i = 0; i < numberOfCases; i++) {
                DSParallelStackPush(stack, caseNumber[i]);
                printf("%i\n", caseNumber[i]);
        }
        DSDesignSpacePrint(ds);
        threads = DSSecureCalloc(sizeof(pthread_t), numberOfThreads);
        /* Creating the N-threads with their data */
        for (i = 0; i < numberOfThreads; i++)
                pthread_create(&threads[i], &attr, DSParallelWorkerValidity, (void *)(&pdatas[i]));
        /* Joining all the N-threads, indicating all cases have been processed */
        for (i = 0; i < numberOfThreads; i++)
                pthread_join(threads[i], NULL);
        DSParallelStackFree(stack);
        DSSecureFree(threads);
        DSSecureFree(pdatas);
        pthread_attr_destroy(&attr);
bail:
        return;
}

static DSUInteger ** dsDesignSpaceCalculateValidCasesByPrunning(DSDesignSpace *ds, DSUInteger * numberValid)
{
        DSUInteger ** validSignatures = NULL;
        DSUInteger ** validPrefixes, *caseNumbers;
        DSCase ** validCases;
        const DSUInteger * termList;
        DSUInteger i, numberOfEquations, numberOfPrefixes, numberOfCases;
        DSDesignSpace * subds;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDesignSpaceNumberOfEquations(ds) == 1) {
                termList = DSDesignSpaceSignature(ds);
                if (termList[0] == 1 && termList[1] == 1) {
                        validSignatures = DSSecureMalloc(sizeof(DSUInteger *));
                        validSignatures[0] = DSSecureMalloc(sizeof(DSUInteger)*2);
                        validSignatures[0][0] = 1;
                        validSignatures[0][1] = 1;
                } else {
                        validCases = DSDesignSpaceCalculateAllValidCases(ds);
                        printf("Valid at root: %i\n", DSDesignSpaceNumberOfValidCases(ds));
                        for (i = 0; i < DSDesignSpaceNumberOfEquations(ds); i++) {
                                DSExpressionPrint(DSDesignSpaceEquations(ds)[i]);
                        }
                        *numberValid = DSDesignSpaceNumberOfValidCases(ds);
                        validSignatures = DSSecureMalloc(sizeof(DSUInteger *)**numberValid);
                        for (i = 0; i < *numberValid; i++) {
                                validSignatures[i] = DSCaseSignatureForCaseNumber(DSCaseNumber(validCases[i]), DSDesignSpaceGMASystem(ds));
                                DSCaseFree(validCases[i]);
                        }
                        DSSecureFree(validCases);
                        goto bail;
                }
        }
        subds = dsDesignSpaceSubDesignSpaceByRemovingLastEquation(ds);
        validPrefixes = dsDesignSpaceCalculateValidCasesByPrunning(subds, &numberOfPrefixes);
        termList = DSDesignSpaceSignature(subds);
        numberOfEquations = DSDesignSpaceNumberOfEquations(ds);
        if (termList[numberOfEquations*2-2] == 1 && termList[numberOfEquations*2-1] == 1) {
                for (i = 0; i < numberOfPrefixes; i++) {
                        validPrefixes[i] = DSSecureRealloc(validPrefixes[i],sizeof(DSUInteger)*numberOfEquations*2);
                        validPrefixes[i][numberOfEquations*2-2] = 1;
                        validPrefixes[i][numberOfEquations*2-1] = 1;
                }
                validSignatures = validPrefixes;
                *numberValid = numberOfPrefixes;
                DSSecureFree(subds);
                goto bail;
        }
        caseNumbers = dsDesignSpaceCalculateCasesNumbersWithPrefixSignatures(ds, &numberOfCases, numberOfPrefixes,
                                                                             DSDesignSpaceNumberOfEquations(subds)*2, (const DSUInteger **)validPrefixes);
        for (i = 0; i < numberOfPrefixes; i++) {
                DSSecureFree(validPrefixes[i]);
        }
        DSSecureFree(validPrefixes);
        DSDesignSpaceFree(subds);
        *numberValid = DSDesignSpaceNumberOfValidCasesFromPrunedCases(ds, numberOfCases, caseNumbers);
        validSignatures = DSSecureMalloc(sizeof(DSUInteger *)**numberValid);
        validCases = DSDesignSpaceCalculateAllValidCases(ds);
        for (i = 0; i < *numberValid; i++) {
                validSignatures[i] = DSCaseSignatureForCaseNumber(DSCaseNumber(validCases[i]), DSDesignSpaceGMASystem(ds));
                DSCaseFree(validCases[i]);
        }
        DSSecureFree(validCases);
bail:
        return validSignatures;
}

extern DSCase ** DSDesignSpaceCalculateValidCasesByPrunning(DSDesignSpace *ds)
{
        DSCase ** validCases = NULL;
        DSUInteger ** validSignatures = NULL;
        DSUInteger i, numberValid;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        validSignatures = dsDesignSpaceCalculateValidCasesByPrunning(ds, &numberValid);
        for (i = 0; i < numberValid; i++) {
                DSSecureFree(validSignatures[i]);
        }
        DSSecureFree(validSignatures);
        validCases = DSDesignSpaceCalculateAllValidCases(ds);
bail:
        return validCases;
}

extern DSCase ** DSDesignSpaceCalculateCases(DSDesignSpace *ds, const DSUInteger numberOfCase, DSUInteger *cases)
{
        DSCase ** allCases = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (numberOfCase == 0) {
                DSError(M_DS_WRONG ": Number of cases to calculate is 0", A_DS_WARN);
                goto bail;
        }
        if (cases == NULL) {
                DSError(M_DS_NULL ": Array of cases to calculate is NULL", A_DS_ERROR);
                goto bail;
        }
        allCases = dsDesignSpaceCalculateCasesParallelBSD(ds, numberOfCase, cases);
bail:
        return allCases;
}

extern DSCase ** DSDesignSpaceCalculateAllValidCases(DSDesignSpace *ds)
{
        DSCase ** validCases = NULL;
        DSUInteger i, numberValid = 0;
        DSUInteger * validCaseNumbers = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        numberValid = DSDesignSpaceNumberOfValidCases(ds);
        if (numberValid == 0)
                goto bail;
        validCaseNumbers = DSSecureMalloc(sizeof(DSUInteger)*numberValid);
        for (i = 0; i < numberValid; i++) {
                validCaseNumbers[i] = atoi(ds->validCases->names[i]);
        }
        validCases = DSDesignSpaceCalculateCases(ds, numberValid, validCaseNumbers);
        DSSecureFree(validCaseNumbers);
bail:
        return validCases;
}

extern DSDictionary * DSDesignSpaceCalculateAllValidCasesForSliceByResolvingCyclicalCases(DSDesignSpace *ds,
                                                                                          const DSVariablePool * lower,
                                                                                          const DSVariablePool * upper)
{
        DSDictionary * caseDictionary = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (ds->seriesCalculations == false) {
                caseDictionary = dsDesignSpaceCalculateAllValidCasesForSliceByResolvingCyclicalCasesSeriesParallelBSD(ds,
                                                                                                                      lower,
                                                                                                                      upper);
        } else {
                caseDictionary = dsDesignSpaceCalculateAllValidCasesForSliceByResolvingCyclicalCasesSeries(ds,
                                                                                                           lower,
                                                                                                           upper);
        }
bail:
        return caseDictionary;
}

extern DSDictionary * DSDesignSpaceCalculateAllValidCasesByResolvingCyclicalCases(DSDesignSpace *ds)
{
        DSDictionary * caseDictionary = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (ds->seriesCalculations == false) {
                caseDictionary = dsDesignSpaceCalculateAllValidCasesByResolvingCyclicalCasesSeriesParallelBSD(ds);
        } else {
                caseDictionary = dsDesignSpaceCalculateAllValidCasesByResolvingCyclicalCasesSeries(ds);
        }
bail:
        return caseDictionary;
}

extern DSDictionary * DSDesignSpaceCalculateAllValidCasesForSlice(DSDesignSpace *ds, const DSVariablePool *lower, const DSVariablePool *upper)
{
        DSDictionary * caseDictionary = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (ds->seriesCalculations == false) {
                caseDictionary = dsDesignSpaceCalculateValidityAtSliceParallelBSD(ds, lower, upper);
        } else {
                caseDictionary = dsDesignSpaceCalculateAllValidCasesForSliceSeries(ds, lower, upper);
        }
bail:
        return caseDictionary;
}


extern void DSDesignSpaceCalculateValidityOfCases(DSDesignSpace *ds)
{
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (ds->seriesCalculations == false) {
                dsDesignSpaceCalculateValidityParallelBSD(ds);
        } else {
                dsDesignSpaceCalculateValiditySeries(ds);
        }
bail:
        return;
}

extern DSDictionary * DSDesignSpaceCalculateValidityOfCaseSet(DSDesignSpace *ds, DSUInteger numberOfCases, DSCase ** cases)
{
        return dsDesignSpaceCalculateValidityOfCaseSetParallelBSD(ds, numberOfCases, cases);
}

extern void DSDesignSpacePrint(const DSDesignSpace * ds)
{
        int (*print)(const char *, ...);
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSPrintf == NULL)
                print = printf;
        else
                print = DSPrintf;
        print("\t Cases: %i\n",
              DSDSNumCases(ds));
        DSGMASystemPrint(DSDSGMA(ds));
bail:
        return;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Cyclical Cases and Cyclical Case validity
#endif

extern DSUInteger DSDesignSpaceNumberOfCyclicalCases(const DSDesignSpace * ds)
{
        DSUInteger numberOfCyclicalCases = 0;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSDSCyclical(ds) == NULL) {
                DSError(M_DS_DICTIONARY_NULL ": Cyclical cases not calculated", A_DS_ERROR);
                goto bail;
        }
        numberOfCyclicalCases = DSDictionaryCount(DSDSCyclical(ds));
bail:
        return numberOfCyclicalCases;
}

extern const DSCyclicalCase * DSDesignSpaceCyclicalCaseWithCaseNumber(const DSDesignSpace *ds, DSUInteger caseNumber)
{
        char * string = NULL;
        DSCyclicalCase * cyclicalCase = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        string = DSSecureCalloc(sizeof(char), 100);
        sprintf(string, "%i", caseNumber);
        if (DSDictionaryValueForName(DSDSCyclical(ds), string) != NULL) {
                cyclicalCase = DSDictionaryValueForName(DSDSCyclical(ds), string);
        }
        if (string != NULL)
                DSSecureFree(string);
bail:
        return cyclicalCase;
}


extern void DSDesignSpaceCalculateCyclicalCase(DSDesignSpace *ds, DSCase * aCase)
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
        if (DSDictionaryValueForName(DSDSCyclical(ds), string) == NULL) {
                aSubcase = DSCyclicalCaseForCaseInDesignSpace(ds, aCase);
                if (aSubcase != NULL)
                        DSDictionaryAddValueWithName(DSDSCyclical(ds), string, aSubcase);
        }
        if (string != NULL)
                DSSecureFree(string);
bail:
        return;
}

extern void DSDesignSpaceCalculateCyclicalCases(DSDesignSpace *ds)
{
        return dsDesignSpaceCalculateCyclicalCasesParallelBSD(ds);
}
