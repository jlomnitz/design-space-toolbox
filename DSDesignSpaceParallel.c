/**
 * \file DSDesignSpaceParallel.c
 * \brief Implementation file with functions for dealing with parallel operatirons used
 * by the design spaces.
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
#include <pthread.h>
#include <glpk.h>
#include "DSDesignSpaceParallel.h"
#include "DSErrors.h"
#include "DSMemoryManager.h"
#include "DSDesignSpace.h"
#include "DSGMASystem.h"
#include "DSSSystem.h"
#include "DSCase.h"
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
                goto bail;
        }
        if (stack->base != NULL)
                DSSecureFree(stack->base);
        DSSecureFree(stack);
        pthread_mutex_destroy(&stack->pushpop);
bail:
        return;
}

extern void DSParallelStackPush(ds_parallelstack_t *stack, const DSUInteger integer)
{
        pthread_mutex_lock(&stack->pushpop);
        if (stack == NULL) {
                DSError(M_DS_NULL ": Stack to push is NULL", A_DS_ERROR);
                goto bail;
        }
        stack->count++;
        if (stack->count >= stack->size) {
                stack->size += PARALLEL_STACK_SIZE_INCREMENT;
                if (stack->base == NULL)
                        stack->base = DSSecureMalloc(sizeof(DSUInteger)*stack->size);
                else
                        stack->base = DSSecureRealloc(stack->base, sizeof(DSUInteger)*stack->size);
        }
        stack->current = stack->base+(stack->count-1);
        *(stack->current) = integer;
bail:
        pthread_mutex_unlock(&stack->pushpop);
        return;
}

extern const DSUInteger DSParallelStackPop(ds_parallelstack_t *stack)
{
        pthread_mutex_lock(&stack->pushpop);
        DSUInteger integer = 0;
        if (stack == NULL) {
                DSError(M_DS_NULL ": Stack to pop is NULL", A_DS_ERROR);
                goto bail;
        }
        if (stack->base == NULL || stack->count == 0)
                goto bail;
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
bail:
        pthread_mutex_unlock(&stack->pushpop);
        return integer;
}

extern void DSParallelStackAddCase(ds_parallelstack_t *stack, DSCase * aCase)
{
        pthread_mutex_lock(&stack->pushpop);
        if (stack == NULL) {
                DSError(M_DS_NULL ": Stack to pop is NULL", A_DS_ERROR);
                goto bail;
        }
        stack->cases[stack->nextIndex++] = aCase;
bail:
        pthread_mutex_unlock(&stack->pushpop);
        return;
}

#include <unistd.h>

extern void * DSParallelWorker(void * pthread_struct)
{
        return DSParallelWorkerCases(pthread_struct);
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
                        aCase = DSCaseWithTermsFromDesignSpace(pdata->ds, termSignature);
//                        pthread_mutex_lock(&workeradd);
                        DSParallelStackAddCase(pdata->stack, aCase);
//                        if (pdata->ds->cases[caseNumber-1] != NULL) {
//                                DSError(M_DS_WRONG ": Case has already been processed", A_DS_ERROR);
//                                DSCaseFree(aCase);
//                        } else {
//                                pdata->ds->cases[caseNumber-1] = aCase;
//                        }
//                        pthread_mutex_unlock(&workeradd);
                        DSSecureFree(termSignature);
                }
        }
bail:
        pthread_exit(NULL);
}

//extern void * DSParallelWorkerCasesSaveToDisk(void * pthread_struct)
//{
//        DSUInteger * termSignature = NULL;
//        struct pthread_struct * pdata = NULL;
//        DSUInteger caseNumber;
//        DSCase *aCase; 
//        char * string = NULL;
//        if (pthread_struct == NULL) {
//                DSError(M_DS_NULL ": Parallel worker data is NULL", A_DS_ERROR);
//                goto bail;
//        }
//        pdata = (struct pthread_struct *)pthread_struct;
//        if (pdata->stack == NULL) {
//                DSError(M_DS_NULL ": Stack in parallel worker is NULL", A_DS_ERROR);
//                goto bail;
//        }
//        if (pdata->ds == NULL) {
//                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        if (pdata->ds->cases == NULL) {
//                DSError(M_DS_NULL ": Case array in design space is NULL", A_DS_ERROR);
//                goto bail;
//        }
//        if (pdata->ds->gma == NULL) {
//                DSError(M_DS_GMA_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        if (pdata->file == NULL) {
//                DSError(M_DS_NOFILE ": Output file is NULL", A_DS_ERROR);
//                goto bail;
//        }
//        /** Data in stack MUST be term signature if not an error will occur **/
//        while (pdata->stack->count > 0)  {
//                caseNumber = DSParallelStackPop(pdata->stack);
//                if (caseNumber == 0)
//                        break;
//                if (caseNumber > DSDesignSpaceNumberOfCases(pdata->ds)) {
//                        DSError(M_DS_WRONG ": Case number out of bounds", A_DS_ERROR);
//                        continue;
//                }
//                if (pdata->ds->cases[caseNumber-1] != NULL)
//                        continue;
//                termSignature = DSCaseSignatureForCaseNumber(caseNumber, pdata->ds->gma);
//                if (termSignature != NULL) {
//                        aCase = DSCaseWithTermsFromDesignSpace(pdata->ds, termSignature);
//                        if (pdata->ds->cases[caseNumber-1] != NULL) {
//                                DSError(M_DS_WRONG ": Case has already been processed", A_DS_ERROR);
//                                DSCaseFree(aCase);
//                        } else {
//                                string = DSCaseStringInJSONFormat(aCase);
//                                pthread_mutex_lock(&iomutex);
//                                fprintf(pdata->file, "%s,", string);
//                                pthread_mutex_unlock(&iomutex);
//                                DSSecureFree(string);
//                                string = NULL;
//                                DSCaseFree(aCase);
//                        }
//                        DSSecureFree(termSignature);
//                }
//        }
//bail:
//        pthread_exit(NULL);
//}
//
//extern void * DSParallelWorkerValidity(void * pthread_struct)
//{
//        /* GLPK is not currently thread safe */
//        struct pthread_struct * pdata = NULL;
//        DSUInteger caseNumber;
//        const DSCase *aCase;
//        if (pthread_struct == NULL) {
//                DSError(M_DS_NULL ": Parallel worker data is NULL", A_DS_ERROR);
//                goto bail;
//        }
//        pdata = (struct pthread_struct *)pthread_struct;
//        if (pdata->stack == NULL) {
//                DSError(M_DS_NULL ": Stack in parallel worker is NULL", A_DS_ERROR);
//                goto bail;
//        }
//        if (pdata->ds == NULL) {
//                DSError(M_DS_DESIGN_SPACE_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        if (pdata->ds->cases == NULL) {
//                DSError(M_DS_NULL ": Case array in design space is NULL", A_DS_ERROR);
//                goto bail;
//        }
//        if (pdata->ds->gma == NULL) {
//                DSError(M_DS_GMA_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        /** Data in stack MUST be a case number, if not an error will occur **/
//        while (pdata->stack->count > 0)  {
//                caseNumber = DSParallelStackPop(pdata->stack);
//                if (caseNumber == 0) {
//                        DSError(M_DS_WRONG ": Case number is 0", A_DS_ERROR);
//                        continue;
//                }
//                if (caseNumber > DSDesignSpaceNumberOfCases(pdata->ds)) {
//                        DSError(M_DS_WRONG ": Case number out of bounds", A_DS_ERROR);
//                        continue;
//                }
//                aCase = DSDesignSpaceCaseWithCaseNumber(pdata->ds, caseNumber);
//                if (DSCaseIsValid(aCase) == true) {
//                        // Add case validity
//                }
////                pdata->ds->validCases[caseNumber-1] = DSCaseIsValid(aCase);
//        }
//bail:
//        pthread_exit(NULL);
//}

