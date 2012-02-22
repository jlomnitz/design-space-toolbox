//
//  DSStack.h
//  DesignSpaceToolboxV2
//
//  Created by Jason Lomnitz on 9/27/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "DSTypes.h"
#include "DSErrors.h"

#ifndef __DS_STACK__
#define __DS_STACK__

#define M_DS_STACK_NULL                 M_DS_NULL ": stack is NULL"

extern DSStack * DSStackAlloc(void);
extern void DSStackFree(DSStack *stack);
extern void DSStackFreeWithFunction(DSStack *stack, void * function);


extern void DSStackPush(DSStack *stack, void * object);
extern void * DSStackPop(DSStack *stack);

extern const void * DSStackObjectAtIndex(const DSStack *stack, DSUInteger index);

extern const DSUInteger DSStackCount(const DSStack *stack);

#endif
