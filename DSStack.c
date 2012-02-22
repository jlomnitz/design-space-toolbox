//
//  DSStack.c
//  DesignSpaceToolboxV2
//
//  Created by Jason Lomnitz on 9/27/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>

#include "DSErrors.h"
#include "DSMemoryManager.h"
#include "DSStack.h"

#define DS_STACKSIZE_INCREMENT 10

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



