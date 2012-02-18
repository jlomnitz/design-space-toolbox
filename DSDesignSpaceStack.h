//
//  DSDesignSpaceStack.h
//  DesignSpaceToolboxV2
//
//  Created by Jason Lomnitz on 9/27/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "DSTypes.h"
#include "DSErrors.h"

#ifndef __DS_DESIGN_SPACE_STACK__
#define __DS_DESIGN_SPACE_STACK__

#define M_DS_STACK_NULL                 M_DS_NULL ": Design space stack is NULL"
extern DSDesignSpaceStack * DSDesignSpaceStackAlloc(void);
extern void DSDesignSpaceStackFree(DSDesignSpaceStack *stack);

extern void DSDesignSpaceStackPush(DSDesignSpaceStack *stack, DSDesignSpace * ds);
extern DSDesignSpace * DSDesignSpaceStackPop(DSDesignSpaceStack *stack);

extern const DSDesignSpace * DSDesignSpaceStackDesignSpaceAtIndex(const DSDesignSpaceStack *stack, DSUInteger index);

extern const DSUInteger DSDesignSpaceStackCount(const DSDesignSpaceStack *stack);

#endif
