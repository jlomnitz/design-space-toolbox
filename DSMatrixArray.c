/**
 * \file DSMatrixArray.c
 * \brief Implementation file with functions for dealing with matrix arrays.
 *
 * \details 
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

#include <string.h>
#include "DSMatrixArray.h"
#include "DSMemoryManager.h"


/**
 * \brief Accessor function to retrieve number of matrices in the Matrix array.
 *
 */
#define dsMatrixArrayNumberOfMatrices(x)    ((x)->numberOfMatrices)

/**
 * \brief Memory allocation for a DSMatrixArray.
 *
 * \details Creates a new DSMatrixArray with no matrices.  As matrices are added,
 * the matrix array grows, therefore the matrix array is initialized to 0, with
 * a NULL internal pointer and number of matrices set to 0.
 *
 * \return If the matrix array was created, a new pointer to a DSMatrix is returned.
 * Otherwise, NULL is returned.
 */
extern DSMatrixArray * DSMatrixArrayAlloc(void)
{
        DSMatrixArray * array = NULL;
        array = DSSecureCalloc(sizeof(DSMatrixArray), 1);
        return array;
}

extern DSUInteger DSMatrixArrayNumberOfMatrices(const DSMatrixArray * matrixArray)
{
        return dsMatrixArrayNumberOfMatrices(matrixArray);
}
/**
 * \brief Copies a DSMatrixArray.
 *
 * \details Creates a new DSMatrixArray with the exact same data and contents as
 * some other matrix array. The matrices in the new DSMatrixArray are copies of
 * the matrices in the original matrix array.
 *
 * \param array The DSMatrixArray to be copied.
 *
 * \return If the copy was succesful, a pointer to a copy of the DSMatrixArray is
 * returned. Otherwise, NULL is returned.
 *
 * \see DSMatrixCopy
 */
extern DSMatrixArray * DSMatrixArrayCopy(const DSMatrixArray * array)
{
        DSMatrixArray *copy = NULL;
        DSUInteger i;
        if (array == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixArrayNumberOfMatrices(array) <= 0) {
                DSError(M_DS_WRONG ": No matrices in array", A_DS_WARN);
                goto bail;
        }
        if (DSMatrixArrayInternalPointer(array) == NULL) {
                DSError(M_DS_WRONG ": No matrices in array", A_DS_ERROR);
                goto bail;
        }
        copy = DSMatrixArrayAlloc();
        for (i = 0; i < DSMatrixArrayNumberOfMatrices(array); i++)
                DSMatrixArrayAddMatrix(copy, 
                                       DSMatrixCopy(DSMatrixArrayMatrix(array, i)));
bail:
        return copy;
}

/**
 * \brief Freeing memory for DSMatrixArray.
 *
 * \details Frees the memory associated with a DSMatrixArray data type.  This
 * function is a wrapper for the necessary steps needed to free the internal
 * structure of the DSMatrixArray, this includes calling DSMatrixFree for each
 * of the contained matrices, freeing the internal pointer to the array of
 * matrices, and the DSMatrixArray data type itself.
 *
 * \param array The DSMatrixArray to be freed.
 */
extern void DSMatrixArrayFree(DSMatrixArray *array)
{
        DSUInteger i;
        DSMatrix *matrix;
        if (array == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixArrayInternalPointer(array) != NULL) {
                for (i = 0; i < DSMatrixArrayNumberOfMatrices(array); i++) {
                        matrix = DSMatrixArrayMatrix(array, i);
                        if (matrix != NULL)
                                DSMatrixFree(matrix);
                }
                DSSecureFree(DSMatrixArrayInternalPointer(array));
        }
        DSSecureFree(array);
bail:
        return;
}

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Factory functions
#endif

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Accesor functions
#endif

/**
 * \brief Function to access a matrix in the DSMatrixArray
 *
 * \details This accessor function returns the DSMatrix at the specified index
 * of the DSMatrixArray.  This function is the basic accessor function, and 
 * should always be used to access a matrix in a DSMatrixArray.
 *
 * \param array The DSMatrixArray containing the matrix to be accessed.
 * \param index The DSUInteger specifying the index in the C array of matrices
 *              contained in the DSMatrixArray.
 *
 * \result If the DSMatrix at the specified index was found, the pointer to that
 *         matrix is returned.  Otherwise, NULL is returned.
 */
extern DSMatrix * DSMatrixArrayMatrix(const DSMatrixArray *array, const DSUInteger index)
{
        DSMatrix *matrix = NULL;
        if (array == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixArrayNumberOfMatrices(array) <= index) {
                DSError(M_DS_WRONG ": Index out of bounds", A_DS_ERROR);
                goto bail;
        }
        matrix = DSMatrixArrayInternalPointer(array)[index];
bail:
        return matrix;
}

/**
 * \brief Function to add a new matrix to the DSMatrixArray.
 *
 * \details This function is the standard mechanism to add a DSMatrix to a
 * DSMatrixArray.  This function allocates the necessary space in the internal
 * C array, and adds the DSMatrix to the end of the array.  Once added to the
 * matrix array, the memory is managed by the matrix array and is freed upon
 * calling DSMatrixArrayFree.
 *
 * \param array The DSMatrixArray that will have a new matrix added.
 * \param matrixToAdd The DSMatrix to be added to the matrix array.
 */
extern void DSMatrixArrayAddMatrix(DSMatrixArray *array, const DSMatrix *matrixToAdd)
{
        if (array == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixArrayNumberOfMatrices(array) != 0 && DSMatrixArrayInternalPointer(array) == NULL) {
                DSError(M_DS_WRONG ": Number of matrices in array", A_DS_ERROR);
                goto bail;
        }
        dsMatrixArrayNumberOfMatrices(array)++;
        if (DSMatrixArrayNumberOfMatrices(array) == 1) {
                DSMatrixArrayInternalPointer(array) = DSSecureMalloc(sizeof(DSMatrix *)*DSMatrixArrayNumberOfMatrices(array));
        } else {
                DSMatrixArrayInternalPointer(array) = DSSecureRealloc(DSMatrixArrayInternalPointer(array), 
                                                                      sizeof(DSMatrix *)*DSMatrixArrayNumberOfMatrices(array));
        }
        DSMatrixArrayInternalPointer(array)[DSMatrixArrayNumberOfMatrices(array)-1] = (DSMatrix *)matrixToAdd;
bail:
        return;
}

extern double DSMatrixArrayDoubleWithIndices(const DSMatrixArray *array, const DSUInteger i, const DSUInteger j, const DSUInteger k)
{
        double value = NAN;
        DSMatrix *current = NULL;
        if (array == NULL) {
                DSError(M_DS_NULL ": Matrix array is NULL", A_DS_ERROR);
                goto bail;
        }
        if (i >= DSMatrixArrayNumberOfMatrices(array)) {
                DSError("Matrix array matrix out of bounds", A_DS_ERROR);
                goto bail;
        }
        current = DSMatrixArrayMatrix(array, i);
        if (current == NULL) {
                DSError(M_DS_MAT_NULL ": Matrix at specified index is NULL", A_DS_ERROR);
                goto bail;
        }
        if (j >= DSMatrixRows(current) || k >= DSMatrixColumns(current)) {
                DSError(M_DS_MAT_OUTOFBOUNDS, A_DS_ERROR);
                goto bail;
        }
        value = DSMatrixDoubleValue(current, j, k);
bail:
        return value;
}

void DSMatrixArrayPrint(const DSMatrixArray * array)
{
        int (*print)(const char *, ...);
        DSUInteger i;
        if (array == NULL) {
                DSError(M_DS_NULL ": Matrix array to print is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSPrintf == NULL)
                print = printf;
        else
                print = DSPrintf;
        for (i = 0; i < DSMatrixArrayNumberOfMatrices(array); i++) {
                print("[:,:,%i] = \n\n", i);
                DSMatrixPrint(DSMatrixArrayMatrix(array, i));
                print("\n");
        }
bail:
        return;
}


#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Data Serialization
#endif


extern DSMatrixArrayMessage * DSMatrixArrayEncode(const DSMatrixArray * matrixArray)
{
        DSMatrixArrayMessage * message = NULL;
        DSUInteger i;
        if (matrixArray == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        message = DSSecureMalloc(sizeof(DSMatrixArrayMessage)*1);
        dsmatrix_array_message__init(message);
        message->n_matrices = DSMatrixArrayNumberOfMatrices(matrixArray);
        message->matrices = DSSecureMalloc(sizeof(DSMatrixMessage)*message->n_matrices);
        for (i = 0; i < message->n_matrices; i++) {
                message->matrices[i] = DSMatrixEncode(DSMatrixArrayMatrix(matrixArray, i));
        }
bail:
        return message;
}

extern DSMatrixArray * DSMatrixArrayFromMatrixArrayMessage(const DSMatrixArrayMessage * message)
{
        DSMatrixArray * matrixArray = NULL;
        DSUInteger i;
        if (message == NULL) {
                printf("message is NULL\n");
                goto bail;
        }
        matrixArray = DSMatrixArrayAlloc();
        for (i = 0; i < message->n_matrices; i++) {
                DSMatrixArrayAddMatrix(matrixArray, DSMatrixFromMatrixMessage(message->matrices[i]));
        }
bail:
        return matrixArray;
}

extern DSMatrixArray * DSMatrixArrayDecode(size_t length, const void * buffer)
{
        DSMatrixArray * matrixArray = NULL;
        DSMatrixArrayMessage * message;
        message = dsmatrix_array_message__unpack(NULL, length, buffer);
        matrixArray = DSMatrixArrayFromMatrixArrayMessage(message);
        dsmatrix_array_message__free_unpacked(message, NULL);
bail:
        return matrixArray;
}

