/**
 * \file DSDesignSpaceParallel.c
 * \brief Implementation file with functions for dealing with parallel operatirons used
 * by the design spaces.
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
#include <pthread.h>
#include <glpk.h>
#include "DSDesignSpaceParallel.h"
#include "DSErrors.h"
#include "DSMemoryManager.h"
#include "DSDesignSpace.h"
#include "DSGMASystem.h"
#include "DSSSystem.h"
#include "DSCase.h"
#include "DSCyclicalCase.h"
#include "DSMatrix.h"

#define PARALLEL_STACK_SIZE_INCREMENT     5000

pthread_mutex_t workeradd;
pthread_mutex_t iomutex;

extern void DSParallelInitMutexes(void)
{
        static bool hasInit;
        if (hasInit == false) {
                pthread_mutex_init(&workeradd, NULL);
                pthread_mutex_init(&iomutex, NULL);
                hasInit = true;
        }
        return;
}

extern ds_parallelstack_t * DSParallelStackAlloc(void)
{
        ds_parallelstack_t * stack = NULL;
        stack = DSSecureCalloc(sizeof(ds_parallelstack_t), 1);
        pthread_mutex_init(&stack->pushpop, NULL);
        return stack;
}

extern void DSParallelStackFree(ds_parallelstack_t *stack)
{
        if (stack == NULL) {
                DSError(M_DS_NULL ": Stack to free is NULL", A_DS_ERROR);
                goto bail;
        }
        if (stack->count != 0 && stack->base == NULL) {
                DSError(M_DS_NULL ": Stack base is null", A_DS_ERROR);
                DSSecureFree(stack);
                goto bail;
        }
        pthread_mutex_lock(&stack->pushpop);
        if (stack->base != NULL)
                DSSecureFree(stack->base);
        stack->base = NULL;
        stack->count = 0;
        pthread_mutex_unlock(&stack->pushpop);
        pthread_mutex_destroy(&stack->pushpop);
        DSSecureFree(stack);
bail:
        return;
}

extern void DSParallelStackPush(ds_parallelstack_t *stack, void * integer)
{
        if (stack == NULL) {
                DSError(M_DS_NULL ": Stack to push is NULL", A_DS_ERROR);
                goto bail;
        }
        pthread_mutex_lock(&stack->pushpop);
        stack->count++;
        if (stack->count >= stack->size) {
                stack->size += PARALLEL_STACK_SIZE_INCREMENT;
                if (stack->base == NULL)
                        stack->base = DSSecureMalloc(sizeof(void *)*stack->size);
                else
                        stack->base = DSSecureRealloc(stack->base, sizeof(void *)*stack->size);
        }
        stack->current = stack->base+(stack->count-1);
        *(stack->current) = integer;
        pthread_mutex_unlock(&stack->pushpop);
bail:
        return;
}

extern const void * DSParallelStackPop(ds_parallelstack_t *stack)
{
        void * integer = 0;
        if (stack == NULL) {
                DSError(M_DS_NULL ": Stack to pop is NULL", A_DS_ERROR);
                goto bail;
        }
        if (stack->base == NULL)
                goto bail;
        pthread_mutex_lock(&stack->pushpop);
        if (stack->count == 0) {
                pthread_mutex_unlock(&stack->pushpop);
                goto bail;
        }
        integer = *(stack->current);
        if (stack->count == stack->size-PARALLEL_STACK_SIZE_INCREMENT) {
                stack->size -= PARALLEL_STACK_SIZE_INCREMENT;
                if (stack->count == 0) {
                        DSSecureFree(stack->base);
                        stack->base = NULL;
                } else {
                        stack->base = DSSecureRealloc(stack->base, sizeof(void *)*stack->size);
                }
        }
        stack->count--;
        if (stack->count == 0)
                stack->current = NULL;
        else
                stack->current = stack->base+(stack->count-1);
        pthread_mutex_unlock(&stack->pushpop);
bail:
        return integer;
}

extern void DSParallelStackAddCase(ds_parallelstack_t *stack, DSCase * aCase)
{
        if (stack == NULL) {
                DSError(M_DS_NULL ": Stack to pop is NULL", A_DS_ERROR);
                goto bail;
        }
        pthread_mutex_lock(&stack->pushpop);
        stack->cases[stack->nextIndex++] = aCase;
        pthread_mutex_unlock(&stack->pushpop);
bail:
        return;
}

#include <unistd.h>

extern void * DSParallelWorker(void * pthread_struct)
{
        return DSParallelWorkerCases(pthread_struct);
}

extern void * DSParallelWorkerCyclicalCases(void * pthread_struct)
{
        DSUInteger * termSignature = NULL;
        struct pthread_struct * pdata = NULL;
        DSUInteger caseNumber;
        DSCase *aCase;
        if (pthread_struct == NULL) {
                DSError(M_DS_NULL ": Parallel worker data is NULL", A_DS_ERROR);
                goto bail;
        }
        pdata = (struct pthread_struct *)pthread_struct;
        if (pdata->stack == NULL) {
                DSError(M_DS_NULL ": Stack in parallel worker is NULL", A_DS_ERROR);
                goto bail;
        }
        if (pdata->ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (pdata->ds->gma == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        while (pdata->stack->count > 0)  {
                caseNumber = DSParallelStackPop(pdata->stack);
                if (caseNumber == 0)
                        continue;
                if (caseNumber > DSDesignSpaceNumberOfCases(pdata->ds)) {
                        DSError(M_DS_WRONG ": Case number out of bounds", A_DS_ERROR);
                        continue;
                }
                termSignature = DSCaseSignatureForCaseNumber(caseNumber, pdata->ds->gma);
                if (termSignature != NULL) {
                        aCase = DSCaseWithTermsFromDesignSpace(pdata->ds, termSignature, DSDesignSpaceCasePrefix(pdata->ds));
                        if (aCase != NULL) {
                                DSDesignSpaceCalculateCyclicalCase(pdata->ds, aCase);
                                DSCaseFree(aCase);
                        }
                        DSSecureFree(termSignature);
                }
        }
bail:
        pthread_exit(NULL);
}

extern void * DSParallelWorkerCases(void * pthread_struct)
{
        DSUInteger * termSignature = NULL;
        struct pthread_struct * pdata = NULL;
        DSUInteger caseNumber;
        DSCase *aCase; 
        if (pthread_struct == NULL) {
                DSError(M_DS_NULL ": Parallel worker data is NULL", A_DS_ERROR);
                goto bail;
        }
        pdata = (struct pthread_struct *)pthread_struct;
        if (pdata->stack == NULL) {
                DSError(M_DS_NULL ": Stack in parallel worker is NULL", A_DS_ERROR);
                goto bail;
        }
        if (pdata->ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (pdata->stack->cases == NULL) {
                DSError(M_DS_NULL ": Case array is NULL", A_DS_ERROR);
                goto bail;
        }
        if (pdata->ds->gma == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        while (pdata->stack->count > 0)  {
                caseNumber = DSParallelStackPop(pdata->stack);
                if (caseNumber == 0)
                        continue;
                if (caseNumber > DSDesignSpaceNumberOfCases(pdata->ds)) {
                        DSError(M_DS_WRONG ": Case number out of bounds", A_DS_ERROR);
                        continue;
                }
                termSignature = DSCaseSignatureForCaseNumber(caseNumber, pdata->ds->gma);
                if (termSignature != NULL) {
                        aCase = DSCaseWithTermsFromDesignSpace(pdata->ds, termSignature, DSDesignSpaceCasePrefix(pdata->ds));
                        DSParallelStackAddCase(pdata->stack, aCase);
                        DSSecureFree(termSignature);
                }
        }
bail:
        pthread_exit(NULL);
}

extern void * DSParallelWorkerValidity(void * pthread_struct)
{
        struct pthread_struct * pdata = NULL;
        DSUInteger caseNumber;
        DSCase *aCase, *toFree;
        const DSCyclicalCase * cyclicalCase;
        char string[100];
        if (pthread_struct == NULL) {
                DSError(M_DS_NULL ": Parallel worker data is NULL", A_DS_ERROR);
                goto bail;
        }
        pdata = (struct pthread_struct *)pthread_struct;
        if (pdata->stack == NULL) {
                DSError(M_DS_NULL ": Stack in parallel worker is NULL", A_DS_ERROR);
                goto bail;
        }
        if (pdata->ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (pdata->ds->validCases == NULL) {
                DSError(M_DS_NULL ": Dictionary of valid cases is NULL", A_DS_ERROR);
                goto bail;
        }
        if (pdata->ds->gma == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        glp_init_env();
        /** Data in stack MUST be a case number, if not an error will occur **/
        while (pdata->stack->count > 0)  {
                if (pdata->stack->argument_type == DS_STACK_ARG_CASENUM) {
                        caseNumber = DSParallelStackPop(pdata->stack);
                        if (caseNumber == 0) {
                                continue;
                        }
                        if (caseNumber > DSDesignSpaceNumberOfCases(pdata->ds)) {
                                DSError(M_DS_WRONG ": Case number out of bounds", A_DS_ERROR);
                                continue;
                        }
                        aCase = DSDesignSpaceCaseWithCaseNumber(pdata->ds, caseNumber);
                        toFree = aCase;
                } else if (pdata->stack->argument_type == DS_STACK_ARG_CASE) {
                        aCase = (DSCase *)DSParallelStackPop(pdata->stack);
                        toFree = NULL;
                }
                sprintf(string, "%d", aCase->caseNumber);//caseNumber);//aCase->caseNumber);
                if (DSCaseIsValid(aCase) == true) {
                        DSDictionaryAddValueWithName(pdata->ds->validCases, string, (void*)1);
                } else if (DSDictionaryValueForName(pdata->ds->cyclicalCases, string) != NULL) {
                        cyclicalCase = DSDesignSpaceCyclicalCaseWithCaseNumber(pdata->ds, caseNumber);
                        if (DSCyclicalCaseIsValid(cyclicalCase) == true)
                                DSDictionaryAddValueWithName(pdata->ds->validCases, string, (void*)1);
                }
                DSCaseFree(aCase);
        }
        glp_free_env();
bail:
        pthread_exit(NULL);
}

extern void * DSParallelWorkerValidityResolveCycles(void * pthread_struct)
{
        struct pthread_struct * pdata = NULL;
        DSUInteger j, numberValidSubcases, caseNumber;
        DSCase *aCase;
        const DSCyclicalCase * cyclicalCase;
        char nameString[100], *subcaseString = NULL;
        const char ** subcaseNames;
        DSDictionary * subcaseDictionary;
        if (pthread_struct == NULL) {
                DSError(M_DS_NULL ": Parallel worker data is NULL", A_DS_ERROR);
                goto bail;
        }
        pdata = (struct pthread_struct *)pthread_struct;
        if (pdata->stack == NULL) {
                DSError(M_DS_NULL ": Stack in parallel worker is NULL", A_DS_ERROR);
                goto bail;
        }
        if (pdata->ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (pdata->ds->validCases == NULL) {
                DSError(M_DS_NULL ": Dictionary of valid cases is NULL", A_DS_ERROR);
                goto bail;
        }
        if (pdata->ds->gma == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        pdata->returnPointer = DSDictionaryAlloc();
        glp_init_env();
        /** Data in stack MUST be a case number, if not an error will occur **/
        while (pdata->stack->count > 0)  {
                caseNumber = DSParallelStackPop(pdata->stack);
                if (caseNumber == 0) {
                        continue;
                }
                if (caseNumber > DSDesignSpaceNumberOfCases(pdata->ds)) {
                        DSError(M_DS_WRONG ": Case number out of bounds", A_DS_ERROR);
                        continue;
                }
                aCase = DSDesignSpaceCaseWithCaseNumber(pdata->ds, caseNumber);
                sprintf(nameString, "%d", caseNumber);
                cyclicalCase = DSDesignSpaceCyclicalCaseWithCaseNumber(pdata->ds, caseNumber);
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
                                DSDictionaryAddValueWithName((DSDictionary*)pdata->returnPointer, subcaseString, DSDictionaryValueForName(subcaseDictionary, subcaseNames[j]));
                        }
                        DSDictionaryFree(subcaseDictionary);
                } else if (DSCaseIsValid(aCase) == true) {
                        DSDictionaryAddValueWithName((DSDictionary*)pdata->returnPointer, nameString, aCase);
                } else {
                        DSCaseFree(aCase);
                }
        }
        glp_free_env();
bail:
        if (subcaseString != NULL)
                DSSecureFree(subcaseString);
        pthread_exit(NULL);
}

extern void * DSParallelWorkerValidityForSliceResolveCycles(void * pthread_struct)
{
        struct pthread_struct * pdata = NULL;
        DSUInteger j, numberValidSubcases, caseNumber;
        DSCase *aCase;
        const DSCyclicalCase * cyclicalCase;
        char nameString[100], *subcaseString = NULL;
        const char ** subcaseNames;
        DSVariablePool * lower, *upper;
        DSDictionary * subcaseDictionary;
        if (pthread_struct == NULL) {
                DSError(M_DS_NULL ": Parallel worker data is NULL", A_DS_ERROR);
                goto bail;
        }
        pdata = (struct pthread_struct *)pthread_struct;
        if (pdata->stack == NULL) {
                DSError(M_DS_NULL ": Stack in parallel worker is NULL", A_DS_ERROR);
                goto bail;
        }
        if (pdata->ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (pdata->ds->validCases == NULL) {
                DSError(M_DS_NULL ": Dictionary of valid cases is NULL", A_DS_ERROR);
                goto bail;
        }
        if (pdata->ds->gma == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (pdata->numberOfArguments == 0) {
                DSError(M_DS_WRONG ": p_data structure needs two arguments", A_DS_ERROR);
                goto bail;
        }
        lower = pdata->functionArguments[0];
        upper = pdata->functionArguments[1];
        if (lower == NULL || upper == NULL) {
                DSError(M_DS_VAR_NULL, A_DS_ERROR);
                goto bail;
        }
        pdata->returnPointer = DSDictionaryAlloc();
        glp_init_env();
        /** Data in stack MUST be a case number, if not an error will occur **/
        while (pdata->stack->count > 0)  {
                caseNumber = DSParallelStackPop(pdata->stack);
                if (caseNumber == 0) {
                        continue;
                }
                if (caseNumber > DSDesignSpaceNumberOfCases(pdata->ds)) {
                        DSError(M_DS_WRONG ": Case number out of bounds", A_DS_ERROR);
                        continue;
                }
                aCase = DSDesignSpaceCaseWithCaseNumber(pdata->ds, caseNumber);
                sprintf(nameString, "%d", caseNumber);
                cyclicalCase = DSDesignSpaceCyclicalCaseWithCaseNumber(pdata->ds, caseNumber);
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
                                DSDictionaryAddValueWithName((DSDictionary*)pdata->returnPointer, subcaseString, DSDictionaryValueForName(subcaseDictionary, subcaseNames[j]));
                        }
                        DSDictionaryFree(subcaseDictionary);
                } else if (DSCaseIsValidAtSlice(aCase, lower, upper) == true) {
                        DSDictionaryAddValueWithName((DSDictionary*)pdata->returnPointer, nameString, aCase);
                } else {
                        DSCaseFree(aCase);
                }
        }
        glp_free_env();
bail:
        if (subcaseString != NULL)
                DSSecureFree(subcaseString);
        pthread_exit(NULL);
}

extern void * DSParallelWorkerValiditySlice(void * pthread_struct)
{
        struct pthread_struct * pdata = NULL;
        DSUInteger caseNumber;
        DSCase *aCase;
        DSVariablePool * lower, *upper;
        const DSCyclicalCase * cyclicalCase;
        char string[100];
        if (pthread_struct == NULL) {
                DSError(M_DS_NULL ": Parallel worker data is NULL", A_DS_ERROR);
                goto bail;
        }
        pdata = (struct pthread_struct *)pthread_struct;
        if (pdata->stack == NULL) {
                DSError(M_DS_NULL ": Stack in parallel worker is NULL", A_DS_ERROR);
                goto bail;
        }
        if (pdata->ds == NULL) {
                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
                goto bail;
        }
        if (pdata->ds->validCases == NULL) {
                DSError(M_DS_NULL ": Dictionary of valid cases is NULL", A_DS_ERROR);
                goto bail;
        }
        if (pdata->ds->gma == NULL) {
                DSError(M_DS_GMA_NULL, A_DS_ERROR);
                goto bail;
        }
        if (pdata->numberOfArguments == 0) {
                DSError(M_DS_WRONG ": p_data structure needs two arguments", A_DS_ERROR);
                goto bail;
        }
        lower = pdata->functionArguments[0];
        upper = pdata->functionArguments[1];
        pdata->returnPointer = DSDictionaryAlloc();
        glp_init_env();
        /** Data in stack MUST be a case number, if not an error will occur **/
        while (pdata->stack->count > 0)  {
                caseNumber = DSParallelStackPop(pdata->stack);
                if (caseNumber == 0) {
                        continue;
                }
                if (caseNumber > DSDesignSpaceNumberOfCases(pdata->ds)) {
                        DSError(M_DS_WRONG ": Case number out of bounds", A_DS_ERROR);
                        continue;
                }
                aCase = DSDesignSpaceCaseWithCaseNumber(pdata->ds, caseNumber);
                sprintf(string, "%d", caseNumber);//aCase->caseNumber);
                cyclicalCase = DSDesignSpaceCyclicalCaseWithCaseNumber(pdata->ds, caseNumber);
                if (cyclicalCase != NULL) {
                        if (DSCyclicalCaseIsValidAtSlice(cyclicalCase, lower, upper) == true) {
                                DSDictionaryAddValueWithName((DSDictionary*)pdata->returnPointer, string, aCase);
                        }
                } else if (DSCaseIsValidAtSlice(aCase, lower, upper) == true) {
                        DSDictionaryAddValueWithName((DSDictionary*)pdata->returnPointer, string, aCase);
                } else {
                        DSCaseFree(aCase);
                }
        }
        glp_free_env();
bail:
        pthread_exit(NULL);
}
