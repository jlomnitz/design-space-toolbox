

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
extern void * DSSecureMalloc(size_t size)
{
        void * data = malloc(size);
        if (data == NULL) {
                DSError(M_DS_MALLOC, A_DS_FATAL);
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
extern void * DSSecureCalloc(size_t count, size_t size)
{
        void *data = calloc(count, size);
        if (data == NULL) {
                DSError(M_DS_MALLOC, A_DS_FATAL);
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
extern void * DSSecureRealloc(void *ptr, size_t size)
{
        void *data;
        if (ptr == NULL) {
                DSError(M_DS_NULL ": Defaulting to DSSecureMalloc", A_DS_WARN);
                data = DSSecureMalloc(size);
        } else {
                data = realloc(ptr, size);
        }
        if (data == NULL) {
                DSError(M_DS_MALLOC, A_DS_FATAL);
        }
        return data;
}

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
extern void DSSecureFree(void * ptr)
{
        if (ptr == NULL)
                DSError(M_DS_NULL, A_DS_ERROR);
        else
                free(ptr);
}



