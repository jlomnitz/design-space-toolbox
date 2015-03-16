

#include <stdio.h>
#include <stdlib.h>

#include "DSMemoryManager.h"


extern void * DSSecureMalloc(size_t size)
{
        void * data = malloc(size);
        if (data == NULL) {
                DSError(M_DS_MALLOC, A_DS_FATAL);
        }
        return data;
}

extern void * DSSecureCalloc(size_t count, size_t size)
{
        void *data = calloc(count, size);
        if (data == NULL) {
                DSError(M_DS_MALLOC, A_DS_FATAL);
        }
        return data;
}

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

extern void DSSecureFree(void * ptr)
{
        if (ptr == NULL)
                DSError(M_DS_NULL, A_DS_ERROR);
        else
                free(ptr);
}



