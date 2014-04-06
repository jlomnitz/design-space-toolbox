/**
 * \file DSDesignSpaceParallel.h
 * \brief Header file with functions for dealing with parallel operatirons used
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

#include <pthread.h>
#include "DSTypes.h"

#ifndef __DS_DESIGN_SPACE_PARALLEL__
#define __DS_DESIGN_SPACE_PARALLEL__

/**
 * \brief Stack object used by the worker threads.
 *
 * \details This structure is a stack of case numbers indicating the DSCases that
 * need to be processed, and each pthread_t used for processing cases and
 * determining validity (currently disabled due to the non re-entrant GLPK) must
 * have access to a ds_parallelstack_t.
 *
 * \note One stack should be created per thread, to avoid one thread blocking
 * another during popping and pushing operations.  A single stack could be used,
 * as the parallel stacks are thread safe, and under some conditions might
 * be more efficient as all the threads in the thread pool will remain active 
 * until all cases have been processed. Currently, the number of cases to be 
 * processed by a thread are determined prior to launching the threads, and each
 * thread has an equal number of cases to process. If a thread has many invalid
 * cases, it may finish all of its cases before the other threads, and thus 
 * it is possible for the system to make less use of multiple processors.  To 
 * avoid this situation, more threads than processors can be used or a single 
 * shared stack could be used.
 */
typedef struct {
        DSUInteger * base;       //!< The pointer to the array of DSUIntegers storing the case numbers.
        DSUInteger * current;    //!< A pointer to the top of the stack.
        DSUInteger count;        //!< The number of elements in the stack.
        DSUInteger size;         //!< The current size of the base array.
        DSUInteger nextIndex;    //!< The index of the current case.
        DSCase ** cases;         //!< The array of cases processed.
        pthread_mutex_t pushpop; //!< The mutex used when pushing and popping data from the stack.
} ds_parallelstack_t;

/**
 * \brief Data structure passed to a pthread.
 *
 * \details This data structure has two fields, one is a pointer to a 
 * ds_parallelstack_t object; this stack containes a stack of case numbers
 * to be processed in parallel.  Each stack is not designed to be accessed
 * concurrently, but should still be thread safe.
 */
struct pthread_struct {
        ds_parallelstack_t * stack;
        DSDesignSpace * ds;
        DSUInteger numberOfArguments;
        void ** functionArguments;
        void * returnPointer;
};

extern void DSParallelInitMutexes(void);

extern ds_parallelstack_t * DSParallelStackAlloc(void);
extern void DSParallelStackFree(ds_parallelstack_t *stack);

extern void DSParallelStackPush(ds_parallelstack_t *stack, const DSUInteger number);
extern const DSUInteger DSParallelStackPop(ds_parallelstack_t *stack);
extern void DSParallelStackAddCase(ds_parallelstack_t *stack, DSCase * aCase);

extern void * DSParallelWorkerCases(void * pthread_struct);
extern void * DSParallelWorkerCyclicalCases(void * pthread_struct);
extern void * DSParallelWorkerCasesSaveToDisk(void * pthread_struct);

extern void * DSParallelWorkerValidity(void * pthread_struct);
extern void * DSParallelWorkerValiditySlice(void * pthread_struct);


#endif
