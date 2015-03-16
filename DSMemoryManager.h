/**
 * \file DSMemoryManager.h
 * \brief Header file with functions for secure memory allocation.
 *
 * \details 
 * This file specifies the design space standard for error handling.
 * Contained here are the necessary macros and functions to succesfully report
 * the errors throughout the design space library.
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

#include "DSErrors.h"

#ifndef __DS_MEMORY_MANAGER__
#define __DS_MEMORY_MANAGER__

#ifdef __cplusplus
__BEGIN_DECLS
#endif

/**
 * \brief Function to securely allocate data using malloc.
 *
 * This function is a secure malloc function which checks the allocated
 * pointer.  If the data pointer is null, indicative of errors allocating memory,
 * the function issues a fatal error.
 *
 * \param size A DSUInteger specifying the size of memory being allocated.
 * \return A pointer to the allocated data.
 */
extern void * DSSecureMalloc(size_t size);

/**
 * \brief Function to securely allocate data using calloc.
 *
 * This function is a secure calloc function which checks the allocated
 * pointer.  If the data pointer is null, indicative of errors allocating memory,
 * the function issues a fatal error.
 *
 * \param count A DSUInteger specifying the number of memory blocks being allocated.
 * \param size The memory size of each block being allocated.
 * \return A pointer to the allocated data.
 */
extern void * DSSecureCalloc(size_t count, size_t size);

/**
 * \brief Function to securely allocate data using realloc.
 *
 * This function is a secure realloc function which checks the allocated
 * pointer.  If the data pointer is null, indicative of errors allocating memory,
 * the function issues a fatal error. This function calls malloc in case that
 * pointer to be reallocated is NULL.
 *
 * \param count A DSUInteger specifying the number of memory blocks being allocated.
 * \param size The memory size of each block being allocated.
 * \return A pointer to the allocated data.
 */
extern void * DSSecureRealloc(void *ptr, size_t size);

/**
 * \brief Function to securely free data.
 *
 * This function is a secure free function which checks the data pointer.
 * If the data pointer is null, indicative of errors when freeing memory,
 * the function issues a fatal error. This function calls malloc in case that
 * pointer to be reallocated is NULL.
 *
 * \param count A DSUInteger specifying the number of memory blocks being allocated.
 * \param size The memory size of each block being allocated.
 * \return A pointer to the allocated data.
 */
extern void DSSecureFree(void * ptr);

#ifdef __cplusplus
__END_DECLS
#endif

#endif




