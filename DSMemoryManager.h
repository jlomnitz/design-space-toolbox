/**
 * \file DSMemoryManager.h
 * \brief Header file with functions for secure memory allocation.
 *
 * \details 
 * This file specifies the design space standard for error handling.
 * Contained here are the necessary macros and functions to succesfully report
 * the errors throughout the design space library.
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

#include "DSErrors.h"

#ifndef __DS_MEMORY_MANAGER__
#define __DS_MEMORY_MANAGER__

#ifdef __cplusplus
__BEGIN_DECLS
#endif

extern void * DSSecureMalloc(DSInteger size);
extern void * DSSecureCalloc(DSInteger count, DSInteger size);
extern void * DSSecureRealloc(void *ptr, DSInteger size);
extern void DSSecureFree(void * ptr);

#ifdef __cplusplus
__END_DECLS
#endif

#endif




