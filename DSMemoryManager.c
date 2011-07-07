/**
 * \file DSMemoryManager.c
 * \brief implementation file with functions for secure memory management.
 *
 * \details
 * This file specifies the design space standard for error handling.
 * Contained here are the necessary macros and functions to succesfully report
 * the errors throughout the design space library.
 *
 * Copyright (C) 2010 Jason Lomnitz.\n\n
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
#include <stdlib.h>

#include "DSMemoryManager.h"

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
extern void * DSSecureMalloc(DSInteger size)
{
        void * data = malloc(size);
        if (data == NULL) {
                DSError(M_DS_MALLOC, A_DS_KILLNOW);
        }
        return data;
}

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
extern void * DSSecureCalloc(DSInteger count, DSInteger size)
{
        void *data = calloc(count, size);
        if (data == NULL) {
                DSError(M_DS_MALLOC, A_DS_KILLNOW);
        }
        return data;
}

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
extern void * DSSecureRealloc(void *ptr, DSInteger size)
{
        void *data;
        if (ptr == NULL) {
                DSError(M_DS_NULL ": Defaulting to DSSecureMalloc", A_DS_WARN);
                data = DSSecureMalloc(size);
        } else {
                data = realloc(ptr, size);
        }
        if (data == NULL) {
                DSError(M_DS_MALLOC, A_DS_KILLNOW);
        }
        return data;
}




