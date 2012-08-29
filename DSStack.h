/**
 * \file DSStack.h
 * \brief Header file with functions for dealing with stack objects.
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
#include "DSTypes.h"
#include "DSErrors.h"

#ifndef __DS_STACK__
#define __DS_STACK__

#define M_DS_STACK_NULL                 M_DS_NULL ": stack is NULL"

#ifdef __cplusplus
__BEGIN_DECLS
#endif

extern DSStack * DSStackAlloc(void);
extern void DSStackFree(DSStack *stack);
extern void DSStackFreeWithFunction(DSStack *stack, void * function);


extern void DSStackPush(DSStack *stack, void * object);
extern void * DSStackPop(DSStack *stack);

extern const void * DSStackObjectAtIndex(const DSStack *stack, DSUInteger index);

extern const DSUInteger DSStackCount(const DSStack *stack);

#ifdef __cplusplus
__END_DECLS
#endif

#endif
