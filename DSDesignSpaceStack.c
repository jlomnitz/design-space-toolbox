//
//  DSDesignSpaceStack.c
//  DesignSpaceToolboxV2
//
//  Created by Jason Lomnitz on 9/27/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>

#include "DSErrors.h"
#include "DSMemoryManager.h"
#include "DSDesignSpace.h"
#include "DSDesignSpaceStack.h"

#define DS_DESIGN_SPACE_STACKSIZE_INCREMENT 10

extern DSDesignSpaceStack * DSDesignSpaceStackAlloc(void)
{
        DSDesignSpaceStack * stack = NULL;
        stack = DSSecureCalloc(sizeof(DSDesignSpaceStack), 1);
        pthread_mutex_init(&stack->pushpop, NULL);
        return stack;
}

extern void DSDesignSpaceStackFree(DSDesignSpaceStack *stack)
{
        if (stack == NULL) {
                DSError(M_DS_NULL ": Design Space Stack is NULL", A_DS_ERROR);
                goto bail;
        }
        while (stack->count > 0) {
                DSDesignSpaceFree(DSDesignSpaceStackPop(stack));
        }
        pthread_mutex_destroy(&stack->pushpop);
        DSSecureFree(stack);
bail:
        return;
}

extern void DSDesignSpaceStackPush(DSDesignSpaceStack *stack, DSDesignSpace * ds)
{
        pthread_mutex_lock(&stack->pushpop);
        if (stack == NULL) {
                DSError(M_DS_NULL ": Stack to push is NULL", A_DS_ERROR);
                goto bail;
        }
        stack->count++;
        if (stack->count >= stack->size) {
                stack->size += DS_DESIGN_SPACE_STACKSIZE_INCREMENT;
                if (stack->base == NULL)
                        stack->base = DSSecureMalloc(sizeof(void *)*stack->size);
                else
                        stack->base = DSSecureRealloc(stack->base, sizeof(void *)*stack->size);
        }
        stack->current = stack->base+(stack->count-1);
        *((DSDesignSpace **)(stack->current)) = ds;
bail:
        pthread_mutex_unlock(&stack->pushpop);
        return;
}

extern DSDesignSpace * DSDesignSpaceStackPop(DSDesignSpaceStack *stack)
{
        pthread_mutex_lock(&stack->pushpop);
        DSDesignSpace * ds = NULL;
        if (stack == NULL) {
                DSError(M_DS_NULL ": Stack to pop is NULL", A_DS_ERROR);
                goto bail;
        }
        if (stack->base == NULL || stack->count == 0)
                goto bail;
        ds = *((DSDesignSpace **)(stack->current));
        if (stack->count == stack->size-DS_DESIGN_SPACE_STACKSIZE_INCREMENT) {
                stack->size -= DS_DESIGN_SPACE_STACKSIZE_INCREMENT;
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
        return ds;
}

extern const DSDesignSpace * DSDesignSpaceStackDesignSpaceAtIndex(const DSDesignSpaceStack *stack, DSUInteger index)
{
        DSDesignSpace * ds = NULL;
        if (stack == NULL) {
                DSError(M_DS_STACK_NULL, A_DS_ERROR);
                goto bail;
        }
        if (index >= stack->count) {
                DSError(M_DS_WRONG ": Index is out of bounds", A_DS_ERROR);
                goto bail;
        }
        ds = ((DSDesignSpace **)stack->base)[index];
bail:
        return ds;
}

extern const DSUInteger DSDesignSpaceStackCount(const DSDesignSpaceStack *stack)
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



