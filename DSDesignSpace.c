/**
 * \file DSDesignSpace.c
 * \brief Implementation file with functions for dealing with Design Spaces
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
#include "DSSubcase.h"

#define __DS_MAC_OS_X__
#define DS_PARALLEL_DEFAULT_THREADS             4

#define DSDSGMA(x)                              ((x)->gma)
//#define DSDSCases(x)                            ((x)->cases)
#define DSDSNumCases(x)                         ((x)->numberOfCases)
//#define DSDSValid(x)                            ((x)->validCases)
#define DSDSXd(x)                               ((x)->Xd)
#define DSDSXi(x)                               ((x)->Xi)
#define DSDSSubcases(x)                         ((x)->subcases)
#define DSDSCi(x)                               ((x)->Ci)
#define DSDSCd(x)                               ((x)->Cd)
#define DSDSDelta(x)                            ((x)->delta)
#define DSDSValidPool(x)                        ((x)->validCases)

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, deallocation and initialization
#endif

extern DSDesignSpace * DSDesignSpaceAlloc(void)
{
        DSDesignSpace * ds = NULL;
        ds = DSSecureCalloc(sizeof(DSDesignSpace), 1);
        DSDSSubcases(ds) = DSDictionaryAlloc();
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
//        for (i = 0; i < DSDSSubcases(ds)->count; i++) {
//                aStack = DSDictionaryValueForName(DSDSSubcases(ds), DSDSSubcases(ds)->names[i]);
//                DSStackFreeWithFunction(aStack, DSSubcaseFree);
//        }
        DSDictionaryFreeWithFunction(DSDSSubcases(ds), DSSubcaseFree);
        DSSecureFree(ds);
bail:
        return;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Factory functions
#endif


extern DSDesignSpace * DSDesignSpaceByParsingStringList(const DSVariablePool * const Xd, const char * const string, ...)
{
        DSDesignSpace *ds = NULL;
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
        ds = DSDesignSpaceByParsingStrings(Xd, (char * const * )strings, numberOfStrings);
        DSSecureFree(strings);
bail:
        return ds;
}

extern DSDesignSpace * DSDesignSpaceByParsingStrings(const DSVariablePool * const Xd, char * const * const strings, const DSUInteger numberOfEquations)
{
        DSDesignSpace * ds = NULL;
        DSGMASystem *gma = NULL;
        if (Xd == NULL) {
                DSError(M_DS_NULL ": Dependent Variables are NULL", A_DS_ERROR);
                goto bail;
        }
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
        gma = DSGMASystemByParsingStrings(Xd, strings, numberOfEquations);
        if (gma != NULL) {
                ds = DSDesignSpaceAlloc();
                DSDesignSpaceSetGMA(ds, gma);
        }
bail:
        return ds;
}

extern DSDesignSpace * DSDesignSpaceByParsingStringsWithXi(const DSVariablePool * const Xd, const DSVariablePool * const Xi, char * const * const strings, const DSUInteger numberOfEquations)
{
        DSDesignSpace * ds = NULL;
        DSGMASystem *gma = NULL;
        if (Xd == NULL) {
                DSError(M_DS_NULL ": Dependent Variables are NULL", A_DS_ERROR);
                goto bail;
        }
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
        gma = DSGMASystemByParsingStringsWithXi(Xd, Xi, strings, numberOfEquations);
        if (gma != NULL) {
                ds = DSDesignSpaceAlloc();
                DSDesignSpaceSetGMA(ds, gma);
        }
bail:
        return ds;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Setter functions
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

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Getter functions
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

extern const DSStack * DSDesignSpaceSubcasesForCaseNumber(DSDesignSpace *ds, const DSUInteger caseNumber)
{
        DSStack * subcases = NULL;
        char * string = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (caseNumber == 0 || caseNumber > DSDesignSpaceNumberOfCases(ds)) {
                DSError(M_DS_WRONG ": Case number is out of bounds", A_DS_ERROR);
                goto bail;
        }
        DSDesignSpaceCalculateUnderdeterminedCaseWithCaseNumber(ds, caseNumber);
        string = DSSecureCalloc(sizeof(char), 100);
        sprintf(string, "%i", caseNumber);
        subcases = DSDictionaryValueForName(ds->subcases, string);
        DSSecureFree(string);
bail:
        return subcases;
}

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

extern const DSDictionary * DSDesignSpaceSubcaseDictionary(const DSDesignSpace *ds)
{
        DSDictionary * dictionary = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        dictionary = DSDSSubcases(ds);
bail:
        return dictionary;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

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

//static void dsDesignSpaceCalculateCasesSeries(DSDesignSpace *ds)
//{
//        DSUInteger i;
//        if (ds == NULL) {
//                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
//                goto bail;
//        }
////        if (DSDSCases(ds) == NULL) {
////                DSError(M_DS_NULL ": Array of cases is NULL", A_DS_ERROR);
////        }
//        if (DSDSGMA(ds) == NULL) {
//                DSError(M_DS_GMA_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        if (DSGMASystemSignature(DSDSGMA(ds)) == NULL) {
//                DSError(M_DS_WRONG ": GMA signature is NULL", A_DS_ERROR);
//                goto bail;
//        }
//        for (i = 0; i < DSDSNumCases(ds); i++)
//                DSDesignSpaceCaseWithCaseNumber(ds, i+1);
//bail:
//        return;  
//}

static void dsDesignSpaceCalculateValiditySeries(DSDesignSpace *ds)
{
        DSUInteger i;
        char * string = NULL;
        DSCase * aCase = NULL;
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
        DSDSValidPool(ds) = DSDictionaryAlloc();//DSVariablePoolAlloc();
        string = DSSecureCalloc(sizeof(char), 100);
        for (i = 0; i < DSDSNumCases(ds); i++) {
                aCase = DSDesignSpaceCaseWithCaseNumber(ds, i+1);
                if (aCase == NULL)
                        continue;
                if (DSCaseIsValid(aCase) == true) {
                        sprintf(string, "%d", aCase->caseNumber);
                        DSDictionaryAddValueWithName(DSDSValidPool(ds), string, (void*)1);
                }
                DSCaseFree(aCase);
                
        }
        DSSecureFree(string);
        
bail:
        return;
}

static DSCase ** dsDesignSpaceCalculateCasesParallelBSD(DSDesignSpace *ds, const DSUInteger numberOfCases, DSUInteger *cases)
{
        DSUInteger i;
        DSUInteger numberOfThreads = sysconf(_SC_NPROCESSORS_ONLN);
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


static void  dsDesignSpaceCalculateValidityParallelBSD(DSDesignSpace *ds)
{
        DSUInteger i;
        DSUInteger numberOfThreads = sysconf(_SC_NPROCESSORS_ONLN);
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
//        stack = DSSecureMalloc(sizeof(ds_parallelstack_t *)*numberOfThreads);
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
        //        dsDesignSpaceCalculateCasesSeries(ds);
        //        dsDesignSpaceCalculateCasesParallelBSD(ds);
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

extern DSDictionary * DSDesignSpaceCalculateAllValidCasesForSlice(DSDesignSpace *ds, const DSVariablePool *lower, const DSVariablePool *upper)
{
        DSDictionary * caseDictionary = NULL;
        DSUInteger i, numberValid = 0, numberValidSlice = 0;
        DSUInteger validCaseNumbers = 0;
        char nameString[100];
        DSCase * aCase = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        caseDictionary = DSDictionaryAlloc();
        numberValidSlice = 0;
        numberValid = DSDesignSpaceNumberOfValidCases(ds);
        if (numberValid == 0)
                goto bail;
        for (i = 0; i < numberValid; i++) {
                validCaseNumbers = atoi(ds->validCases->names[i]);
                aCase = DSDesignSpaceCaseWithCaseNumber(ds, validCaseNumbers);
                sprintf(nameString, "%d", validCaseNumbers);
                if (DSCaseIsValidAtSlice(aCase, lower, upper) == true)
                        DSDictionaryAddValueWithName(caseDictionary, nameString, aCase);
                else
                        DSCaseFree(aCase);
        }
bail:
        return caseDictionary;
}

extern void DSDesignSpaceCalculateUnderdeterminedCaseWithCaseNumber(DSDesignSpace *ds, DSUInteger caseNumber)
{
        DSUInteger numberOfCases;
        char * string = NULL;
        DSCase *aCase = NULL;
        DSSubcase * aSubcase = NULL;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfCases = DSDesignSpaceNumberOfCases(ds);
        if (numberOfCases == 0 || caseNumber > numberOfCases) {
                goto bail;
        }
        if (DSDesignSpaceCaseWithCaseNumberIsValid(ds, caseNumber) == true) {
                goto bail;
        }
        string = DSSecureCalloc(sizeof(char), 100);
        aCase = DSDesignSpaceCaseWithCaseNumber(ds, caseNumber);
        sprintf(string, "%i", caseNumber);
        if (DSDictionaryValueForName(DSDSSubcases(ds), string) == NULL) {
//                DSSubcaseDesignSpaceForUnderdeterminedCase(aCase, ds);
                aSubcase = DSSubcaseForCaseInDesignSpace(ds, aCase);
                if (aSubcase != NULL)
//                        if (DSSubcaseIsValid(aSubcase) == true)
                                DSDictionaryAddValueWithName(ds->subcases, string, aSubcase);
        }
        if (string != NULL)
                DSSecureFree(string);
        if (aCase != NULL)
                DSCaseFree(aCase);
bail:
        return;
}

extern void DSDesignSpaceCalculateUnderdeterminedCases(DSDesignSpace *ds)
{
        DSUInteger i, numberOfCases;
        if (ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfCases = DSDesignSpaceNumberOfCases(ds);
        if (numberOfCases == 0) {
                goto bail;
        }
        for (i = 0; i < numberOfCases; i++) {
                DSDesignSpaceCalculateUnderdeterminedCaseWithCaseNumber(ds, i+1);
//                if (DSDesignSpaceCaseWithCaseNumberIsValid(ds, i+1) == true)
//                        continue;
//                aCase = DSDesignSpaceCaseWithCaseNumber(ds, i+1);
//                DSSubcaseDesignSpaceForUnderdeterminedCase(aCase, ds);
//                DSCaseFree(aCase);
        }
bail:
        return;
}

extern void DSDesignSpaceCalculateValidityOfCases(DSDesignSpace *ds)
{
        dsDesignSpaceCalculateValidityParallelBSD(ds);
//        dsDesignSpaceCalculateValiditySeries(ds);
}

extern void DSDesignSpacePrint(const DSDesignSpace * ds)
{
        int (*print)(const char *, ...);
//        printf("%p\n", ds);
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
