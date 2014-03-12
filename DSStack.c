/**
 * \file DSStack.h
 * \brief Implementation file with functions for dealing with stack objects.
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

#include "DSErrors.h"
#include "DSMemoryManager.h"
#include "DSStack.h"

#define DS_STACKSIZE_INCREMENT 100

extern DSStack * DSStackAlloc(void)
{
        DSStack * stack = NULL;
        stack = DSSecureCalloc(sizeof(DSStack), 1);
        pthread_mutex_init(&stack->pushpop, NULL);
        return stack;
}

extern void DSStackFreeWithFunction(DSStack *stack, void * function)
{
        void (*freeFunction)(void *) = function;
        if (stack == NULL) {
                DSError(M_DS_NULL ": Design Space Stack is NULL", A_DS_ERROR);
                goto bail;
        }
        while (stack->count > 0) {
                if (freeFunction == NULL)
                        DSStackPop(stack);
                else
                        freeFunction(DSStackPop(stack));
        }
        pthread_mutex_destroy(&stack->pushpop);
        DSSecureFree(stack);
bail:
        return;
}

extern void DSStackFree(DSStack *stack)
{
        if (stack == NULL) {
                DSError(M_DS_NULL ": Design Space Stack is NULL", A_DS_ERROR);
                goto bail;
        }
        DSStackFreeWithFunction(stack, NULL);
bail:
        return;
}

extern void DSStackPush(DSStack *stack, void * object)
{
        pthread_mutex_lock(&stack->pushpop);
        if (stack == NULL) {
                DSError(M_DS_NULL ": Stack to push is NULL", A_DS_ERROR);
                goto bail;
        }
        stack->count++;
        if (stack->count >= stack->size) {
                stack->size += DS_STACKSIZE_INCREMENT;
                if (stack->base == NULL)
                        stack->base = DSSecureMalloc(sizeof(void *)*stack->size);
                else
                        stack->base = DSSecureRealloc(stack->base, sizeof(void *)*stack->size);
        }
        stack->current = stack->base+(stack->count-1);
        *(stack->current) = object;
bail:
        pthread_mutex_unlock(&stack->pushpop);
        return;
}

extern void * DSStackPop(DSStack *stack)
{
        pthread_mutex_lock(&stack->pushpop);
        void * object = NULL;
        if (stack == NULL) {
                DSError(M_DS_NULL ": Stack to pop is NULL", A_DS_ERROR);
                goto bail;
        }
        if (stack->base == NULL || stack->count == 0)
                goto bail;
        object = *(stack->current);
        if (stack->count == stack->size-DS_STACKSIZE_INCREMENT) {
                stack->size -= DS_STACKSIZE_INCREMENT;
                if (stack->count != 1) {
                        stack->base = DSSecureRealloc(stack->base, sizeof(void *)*stack->size);
                }
        }
        stack->count--;
        if (stack->count == 0) {
                DSSecureFree(stack->base);
                stack->base = NULL;
                stack->current = NULL;
        } else {
                stack->current = stack->base+(stack->count-1);
        }
bail:
        pthread_mutex_unlock(&stack->pushpop);
        return object;
}

extern const void * DSStackObjectAtIndex(const DSStack *stack, DSUInteger index)
{
        void * object = NULL;
        if (stack == NULL) {
                DSError(M_DS_STACK_NULL, A_DS_ERROR);
                goto bail;
        }
        if (index >= stack->count) {
                DSError(M_DS_WRONG ": Index is out of bounds", A_DS_ERROR);
                goto bail;
        }
        object = (stack->base)[index];
bail:
        return object;
}

extern const DSUInteger DSStackCount(const DSStack *stack)
{
        DSUInteger count = 0;
        if (stack == NULL) {
                DSError(M_DS_STACK_NULL, A_DS_ERROR);
                goto bail;
        }
        count = stack->count;
bail:
        return count;
}



