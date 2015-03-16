/**
 * \file DSMatrix_gsl.c
 * \brief Implementation file with functions for dealing with matrices using the
 * GNU Scientific Library (gsl).
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

#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_eigen.h>

#include "DSMatrix.h"
#include "DSErrors.h"
#include "DSMemoryManager.h"
#include "DSMatrixArray.h"
#include "DSMatrixTokenizer.h"

#define DSMatrixSetRows(x,y)                    ((x)->rows = (y))
#define DSMatrixSetColumns(x,y)                    ((x)->columns = (y))
//#if defined(__MATRIX_BACK__) && __MATRIX_BACK__ == __MAT_GSL__


#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, Free and Initialization functions
#endif

/**
 * \brief Memory allocation for a DSMatrix using malloc.
 *
 * \details Creates a new matrix of a particular size.  The matrix that is
 * allocated has all the values of the matrix defaulted to 0.  The internal 
 * matrix pointer must be set to NULL; otherwise, the size of the matrix 
 * cannot be changed.
 *
 * \param rows A DSUInteger with the number of rows in the new matrix.
 * \param columns A DSUInteger with the number of columns in the new matrix.
 *
 * \return If the matrix was created, a new pointer to a DSMatrix is returned.
 * Otherwise, NULL is returned.
 *
 */
extern DSMatrix * DSMatrixAlloc(const DSUInteger rows, const DSUInteger columns)
{
        DSMatrix *aMatrix = NULL;
        if (rows == 0 || columns == 0) {
                DSError(M_DS_WRONG, A_DS_WARN);
                goto bail;
        }
        aMatrix = DSSecureMalloc(sizeof(DSMatrix *)*1);
        DSMatrixInternalPointer(aMatrix) = NULL;
        DSMatrixSetRows(aMatrix, rows);
        DSMatrixSetColumns(aMatrix, columns);
        DSMatrixInternalPointer(aMatrix) =gsl_matrix_alloc(rows, columns);
        if (DSMatrixInternalPointer(aMatrix) == NULL)
                DSError(M_DS_MALLOC, A_DS_KILLNOW);
bail:
        return aMatrix;
}

/**
 * \brief Memory allocation for a DSMatrix using calloc.
 *
 * \details Creates a new matrix of a particular size.  The matrix that is
 * allocated has all the values of the matrix defaulted to 0.  The internal 
 * matrix pointer must be set to NULL; otherwise, the size of the matrix 
 * cannot be changed.
 *
 * \param rows A DSUInteger with the number of rows in the new matrix.
 * \param columns A DSUInteger with the number of columns in the new matrix.
 *
 * \return If the matrix was created, a new pointer to a DSMatrix is returned.
 * Otherwise, NULL is returned.
 *
 */
extern DSMatrix * DSMatrixCalloc(const DSUInteger rows, const DSUInteger columns)
{
        DSMatrix *aMatrix = NULL;
        if (rows == 0 || columns == 0) {
                DSError(M_DS_WRONG, A_DS_WARN);
                goto bail;
        }
        aMatrix = DSSecureMalloc(sizeof(DSMatrix *)*1);
        DSMatrixInternalPointer(aMatrix) = NULL;
        DSMatrixSetRows(aMatrix, rows);
        DSMatrixSetColumns(aMatrix, columns);
        DSMatrixInternalPointer(aMatrix) = gsl_matrix_calloc(rows, columns);
        if (DSMatrixInternalPointer(aMatrix) == NULL)
                DSError(M_DS_MALLOC, A_DS_KILLNOW);
bail:
        return aMatrix;
}

/**
 * \brief Copies a DSMatrix.
 *
 * \details Creates a new matrix with the exact same size and contents as some
 * other matrix.  The new matrix is allocated, and thus must be freed.
 *
 * \param original The DSMatrix to be copied.
 *
 * \return If the copy was succesful, a pointer to a copy of the DSMatrix is
 * returned. Otherwise, NULL is returned.
 *
 */
extern DSMatrix * DSMatrixCopy(const DSMatrix *original)
{
        DSMatrix * matrix = NULL;
        if (original == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        matrix = DSMatrixAlloc(DSMatrixRows(original), DSMatrixColumns(original));
       gsl_matrix_memcpy(DSMatrixInternalPointer(matrix), DSMatrixInternalPointer(original));
bail:
        return matrix;
}

/**
 * \brief Freeing memory for DSMatrix.
 *
 * \details Frees the memory associated with a DSMatrix data type.  This
 * function is a wrapper for the necessary steps needed to free the internal
 * structure of the DSMatrix data type.
 *
 * \param matrix The DSMatrix to be freed.
 */
extern void DSMatrixFree(DSMatrix *matrix)
{
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_FATAL);
                goto bail;
        }
        if (DSMatrixInternalPointer(matrix) != NULL)
               gsl_matrix_free(DSMatrixInternalPointer(matrix));
        DSSecureFree(matrix);
bail:
        return;
}


#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Factory functions
#endif

/**
 * \brief Allocates a new DSMatrix as an identity matrix.
 *
 * \details Allocates a square matrix of a specified size, and initializes the
 * diagonal values to 1 and all the other values to 0, creating an identity
 * matrix. The new matrix is therefore an identity matrix.
 *
 * \param size A DSUInteger containing the number of rows and columns in the 
 *             matrix.
 *
 * \return If the identity matrix was succesfully created, a pointer to the 
 * DSMatrix is returned. Otherwise, NULL is returned.
 *
 */
extern DSMatrix * DSMatrixIdentity(const DSUInteger size)
{
        DSMatrix * aMatrix = NULL;
        if (size == 0) {
                DSError(M_DS_WRONG, A_DS_WARN);
                goto bail;
        }
        aMatrix = DSMatrixAlloc(size, size);
       gsl_matrix_set_identity(DSMatrixInternalPointer(aMatrix));
bail:
        return aMatrix;
}

/**
 * \brief Allocates a new DSMatrix with random values between 0 and 1.
 *
 * Allocates a new DSMatrix with a specified size.  The values of each of the 
 * entries in the matrix are randomly selected between 0 and 1.
 *
 * \param rows A DSUInteger with the number of rows in the new matrix.
 * \param columns A DSUInteger with the number of columns in the new matrix.
 *
 * \return If the matrix was created, a new pointer to a DSMatrix is returned.
 * Otherwise, NULL is returned. 
 */
extern DSMatrix * DSMatrixRandomNumbers(const DSUInteger rows, const DSUInteger columns)
{
        DSMatrix * matrix = NULL;
        int i, j;
        matrix = DSMatrixAlloc(rows, columns);
        srand((unsigned int)time(NULL)+rand());
        for (i = 0; i < rows; i++) {
                for (j = 0; j < columns; j++) {
                        DSMatrixSetDoubleValue(matrix, i, j, ((double)rand())/((double)RAND_MAX));
                }
        }
bail:
        return matrix;
}

/**
 * \brief Creates a new matrix by parsing a tab-delimited matrix.
 *
 * This function reads an input string, containing rows delimited by tabs and
 * columns delimited by newlines. This function generates a token stream, and 
 * thus checks the dimensions of the matrix prior to creating it. 
 *
 * \param string A string containing the data to parse.
 *
 * \return A DSMatrix data object with the parsed data. If parsing failed, returns NULL.
 */
extern DSMatrix * DSMatrixByParsingString(const char *string)
{
        DSMatrix * aMatrix = NULL;
        DSUInteger rows, columns, total;
        struct matrix_token *tokens = NULL, *current;
        double *values = NULL;
        if (string == NULL) {
                DSError(M_DS_WRONG ": String to parse is NULL", A_DS_ERROR);
                goto bail;
        }
        if (strlen(string) == 0) {
                DSError(M_DS_WRONG ": String to parse is empty", A_DS_WARN);
                goto bail;                
        }
        tokens = DSMatrixTokenizeString(string);
        if (tokens == NULL) {
                DSError(M_DS_PARSE ": Token stream is NULL", A_DS_ERROR);
                goto bail;
        }
        rows = 0;
        columns = 0;
        current = tokens;
        total = 0;
        while (current) {
                if (DSMatrixTokenType(current) == DS_MATRIX_TOKEN_ERROR) {
                        DSError(M_DS_PARSE ": Unrecognized data", A_DS_ERROR);
                        goto bail;
                }
                if (DSMatrixTokenType(current) == DS_MATRIX_TOKEN_DOUBLE) {
                        if (rows < DSMatrixTokenRow(current))
                            rows = DSMatrixTokenRow(current);
                        if (columns < DSMatrixTokenColumn(current))
                            columns = DSMatrixTokenColumn(current);
                        if (values == NULL) {
                                values = DSSecureMalloc(sizeof(double)*(total+1));
                        } else {
                                values = DSSecureRealloc(values, sizeof(double)*(total+1));
                        }
                        values[total++] = DSMatrixTokenValue(current);
                }
                current = DSMatrixTokenNext(current);
        }
        if (rows == 0 || columns == 0 || total != rows*columns) {
                DSError(M_DS_WRONG ": Data to parse is incorrect", A_DS_WARN);
                goto bail;
        }
        aMatrix = DSMatrixCalloc(rows, columns);
        DSMatrixSetDoubleValues(aMatrix, true, total, values);
bail:
        if (tokens != NULL)
                DSMatrixTokenFree(tokens);
        if (values != NULL)
                DSSecureFree(values);
        return aMatrix;
}


#if defined(__APPLE__) && defined (__MACH__)
#pragma mark Arithmetic (factory)
#endif

/**
 * \brief Create a new DSMatrix object by substracting a matrix from another.
 *
 * \details This function takes two matrices of the same dimensions, and substracts
 * the ij element of the rvalue matrix to the ij element of the lvalue matrix. This
 * function assumes constant matrices, and thus does not modify either of the 
 * inputs, but instead creates a copy of the minuend operand matrix, and called
 * DSMatrixSubstractByMatrix() with the copy as the new minuend.
 *
 * \param lvalue The DSMatrix object that is the minuend. 
 * \param rvalue The DSMatrix object that is the subtrahend.
 *
 * \return If the substraction operation was succesful, the function returns a pointer
 *         to the newly allocated difference matrix. Otherwise, NULL is returned.
 *
 * \see DSMatrixSubstractByMatrix()
 */
extern DSMatrix * DSMatrixBySubstractingMatrix(const DSMatrix *lvalue, const DSMatrix *rvalue)
{
        DSMatrix * matrix = NULL;
        if (lvalue == NULL && rvalue == NULL) {
                DSError("lvalue and rvalue are null", A_DS_WARN);
                goto bail;
        }
        if (lvalue == NULL) {
                matrix = DSMatrixByMultiplyingScalar(rvalue, -1.0);
                goto bail;
        }
        matrix = DSMatrixCopy(lvalue);
        DSMatrixSubstractByMatrix(matrix, rvalue);
bail:
        return matrix;
}

/**
 * \brief Create a new DSMatrix object by adding a matrix to another.
 *
 * \details This function takes two matrices of the same dimensions, and adds
 * the ij element of the rvalue matrix to the ij element of the lvalue matrix. This
 * function assumes constant matrices, and thus does not modify either of the 
 * inputs, but instead creates a copy of the first operand matrix, and calls
 * DSMatrixAddByMatrix(), using the copy as the first operand.
 *
 * \param lvalue The first DSMatrix object to be added.
 * \param rvalue The second DSMatrix object to be added.
 *
 * \return If the addition operation was succesful, the function returns a pointer
 *         to the newly allocated matrix. Otherwise, NULL is returned.
 *
 * \see DSMatrixAddByMatrix()
 */
extern DSMatrix * DSMatrixByAddingMatrix(const DSMatrix *lvalue, const DSMatrix *rvalue)
{
        DSMatrix * matrix = NULL;
        if (lvalue == NULL && rvalue == NULL) {
                DSError("lvalue and rvalue are null", A_DS_WARN);
                goto bail;
        }
        if (lvalue == NULL) {
                DSError(M_DS_WRONG ": lvalue matrix is NULL", A_DS_WARN);
                matrix = DSMatrixByMultiplyingScalar(rvalue, -1.0);
                goto bail;
        }
        matrix = DSMatrixCopy(lvalue);
        DSMatrixAddByMatrix(matrix, rvalue);
bail:
        return matrix;
}

extern DSMatrix * DSMatrixByDividingMatrix(const DSMatrix *lvalue, const DSMatrix *rvalue)
{
        DSError(M_DS_NOT_IMPL, A_DS_ERROR);
        return NULL;
}

extern DSMatrix * DSMatrixByMultiplyingMatrix(const DSMatrix *lvalue, const DSMatrix *rvalue)
{
        DSMatrix * matrix = NULL;
        if (lvalue == NULL || rvalue == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixColumns(lvalue) != DSMatrixRows(rvalue)) {
                DSError("Matrix dimensions do not match", A_DS_ERROR);
                goto bail;
        }
        matrix = DSMatrixAlloc(DSMatrixRows(lvalue), DSMatrixColumns(rvalue));
        gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, 
                       DSMatrixInternalPointer(lvalue), 
                       DSMatrixInternalPointer(rvalue), 0, DSMatrixInternalPointer(matrix));
bail:
        return matrix;
}

extern DSMatrix * DSMatrixByApplyingFunction(const DSMatrix *mvalue, double (*function)(double))
{
        DSMatrix * matrix = NULL;
        if (mvalue == NULL || function == NULL) {
                DSError(M_DS_NULL, A_DS_ERROR);
                goto bail;
        }
        matrix = DSMatrixCopy(mvalue);        
        DSMatrixApplyFunction(matrix, function);
bail:
        return matrix;
}


extern DSMatrix * DSMatrixBySubstractingScalar(const DSMatrix *lvalue, const double rvalue)
{
        DSMatrix * matrix = NULL;
        if (lvalue == NULL) {
                DSError(M_DS_NULL, A_DS_ERROR);
                goto bail;
        }
        matrix = DSMatrixCopy(lvalue);        
        if (rvalue == 0.0) {
                goto bail;
        }
       gsl_matrix_add_constant(DSMatrixInternalPointer(matrix), -rvalue);
bail:
        return matrix;
}

extern DSMatrix * DSMatrixByAddingScalar(const DSMatrix *lvalue, const double rvalue)
{
        DSMatrix * matrix = NULL;
        if (lvalue == NULL) {
                DSError(M_DS_NULL, A_DS_ERROR);
                goto bail;
        }
        matrix = DSMatrixCopy(lvalue);        
        if (rvalue == 0.0) {
                goto bail;
        }
       gsl_matrix_add_constant(DSMatrixInternalPointer(matrix), rvalue);
bail:
        return matrix;
}

extern DSMatrix * DSMatrixByDividingScalar(const DSMatrix *lvalue, const double rvalue)
{
        DSMatrix * matrix = NULL;
        if (lvalue == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        matrix = DSMatrixCopy(lvalue);        
        if (rvalue == 0.0) {
                DSMatrixSetDoubleValueAll(matrix, INFINITY);
                goto bail;
        }
       gsl_matrix_scale(DSMatrixInternalPointer(matrix), 1.0/rvalue);
bail:
        return matrix;
}

extern DSMatrix * DSMatrixByMultiplyingScalar(const DSMatrix *lvalue, const double rvalue)
{
        DSMatrix * matrix = NULL;
        if (lvalue == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        matrix = DSMatrixCopy(lvalue);        
        if (rvalue == 0.0) {
                DSMatrixSetDoubleValueAll(matrix, 0.0);
                goto bail;
        }
       gsl_matrix_scale(DSMatrixInternalPointer(matrix), rvalue);
bail:
        return matrix;
}


#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Basic Accesor functions
#endif

//
//extern void DSMatrixSetRows(DSMatrix * matrix, const DSUInteger rows)
//{
//        if (matrix == NULL) {
//                DSError(M_DS_NULL, A_DS_WARN);
//                goto bail;
//        }
//        if (DSMatrixInternalPointer(matrix) != NULL) {
//                DSError(M_DS_WRONG, A_DS_ERROR);
//                goto bail;
//        }
//        DSMatrixRows(matrix) = rows;
//bail:
//        return;
//}
//
//extern void DSMatrixSetColumns(DSMatrix * matrix, const DSUInteger columns)
//{
//        if (matrix == NULL) {
//                DSError(M_DS_NULL, A_DS_WARN);
//                goto bail;
//        }
//        if (DSMatrixInternalPointer(matrix) != NULL) {
//                DSError(M_DS_WRONG, A_DS_ERROR);
//                goto bail;
//        }
//        DSMatrixColumns(matrix) = columns;
//bail:
//        return;
//}

/**
 * \brief Returns the element of the DSMatrix specified by a row and column.
 *
 * \details Returns an element of the matrix, with indices i and j
 * starting at 0.
 *
 * \param matrix The DSMatrix whose elements will be accessed.
 * \param row A DSUInteger specifying the row coordinate of the element to be
 *            accessed.
 * \param column A DSUInteger specifying the column coordinate of the element to
 *               be accessed.
 *
 * \result If the value was succesfully retrieved, the double value contained at
 *         the row and column coordinate of the DSMatrix is returned. Otherwise,
 *         NaN is returned.
 */
extern double DSMatrixDoubleValue(const DSMatrix *matrix, const DSUInteger row, const DSUInteger column)
{
        double value = NAN;
        if (matrix == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixInternalPointer(matrix) == NULL) {
                DSError(M_DS_MAT_NOINTERNAL, A_DS_WARN);
                goto bail;
        }
        if (row >= DSMatrixRows(matrix) || column >= DSMatrixColumns(matrix)) {
                DSError(M_DS_MAT_OUTOFBOUNDS, A_DS_ERROR);
                goto bail;
        }
        value =gsl_matrix_get(DSMatrixInternalPointer(matrix), row, column);
bail:
        return value;
}

extern void DSMatrixSetDoubleValue(DSMatrix *matrix, const DSUInteger row, const DSUInteger column, const double value)
{
        if (matrix == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixInternalPointer(matrix) == NULL) {
                DSError(M_DS_MAT_NOINTERNAL, A_DS_WARN);
                goto bail;
        }
        if (row >= DSMatrixRows(matrix) || column >= DSMatrixColumns(matrix)) {
                DSError(M_DS_MAT_OUTOFBOUNDS, A_DS_ERROR);
                goto bail;
        }
       gsl_matrix_set(DSMatrixInternalPointer(matrix), row, column, value);
bail:
        return;
}

extern void DSMatrixSetDoubleValuesList(DSMatrix *matrix, bool byColumns, DSUInteger numberOfValues, double firstValue, ...)
{
        double *values = NULL;
        DSUInteger i;
        va_list ap;
	va_start(ap, firstValue);
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (numberOfValues != DSMatrixColumns(matrix)*DSMatrixRows(matrix)) {
                DSError(M_DS_WRONG ": Number of values must match number of entries in matrix", A_DS_ERROR);
                goto bail;
        }
        values = DSSecureMalloc(sizeof(double)*numberOfValues);
        values[0] = firstValue;
        for (i = 1; i < numberOfValues; i++)
                values[i] = (double)va_arg(ap, double);
        DSMatrixSetDoubleValues(matrix, byColumns, numberOfValues, values);
        DSSecureFree(values);
bail:
        return;
}


extern void DSMatrixSetDoubleValues(DSMatrix *matrix, bool byColumns, DSUInteger numberOfValues, double * values)
{
        DSUInteger i;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (numberOfValues != DSMatrixColumns(matrix)*DSMatrixRows(matrix)) {
                DSError(M_DS_WRONG ": Number of values must match number of entries in matrix", A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < numberOfValues; i++) {
                if (byColumns == true) {
                        DSMatrixSetDoubleValue(matrix, i / DSMatrixColumns(matrix),
                                               i % DSMatrixColumns(matrix), values[i]);
                } else {
                        DSMatrixSetDoubleValue(matrix, i % DSMatrixRows(matrix),
                                               i / DSMatrixRows(matrix), values[i]);
                }
        }
bail:
        return;
        
}

/**
 * \brief Sets all the values of a matrix to a value.
 *
 * \details This function does not allocate the necessary memory; instead it 
 * goes through all the rows and columns of the matrix, assigning them the
 * specified value.
 *
 * \param matrix The DSMatrix that will be assigned the value.
 * \param value The double variable whose value will be assigned.
 *
 */
extern void DSMatrixSetDoubleValueAll(DSMatrix *matrix, const double value)
{
        if (matrix == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixInternalPointer(matrix) == NULL) {
                DSError(M_DS_MAT_NOINTERNAL, A_DS_WARN);
                goto bail;
        }
       gsl_matrix_set_all(DSMatrixInternalPointer(matrix), value);
bail:
        return;
}

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

extern void DSMatrixRoundToSignificantFigures(DSMatrix *matrix, const unsigned char figures)
{
        DSUInteger i, j;
        double value;
        char data[100];
        if (matrix == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixInternalPointer(matrix) == NULL) {
                DSError(M_DS_MAT_NOINTERNAL, A_DS_WARN);
                goto bail;
        }
        //        sprintf(format, "%%.%dlf", figures);
        for (i = 0; i < DSMatrixRows(matrix); i++) {
                for (j = 0; j < DSMatrixColumns(matrix); j++) {
                        sprintf(data, "%.*lf", figures, DSMatrixDoubleValue(matrix, i, j));
                        sscanf(data, "%lf", &value);
                        DSMatrixSetDoubleValue(matrix, i, j, value);
                }
        }
bail:
        return;
}

extern DSMatrix * DSMatrixSubMatrixExcludingRowList(const DSMatrix *matrix, const DSUInteger numberOfRows, const DSUInteger firstRow, ...)
{
        unsigned int i;
        DSUInteger *rowsToExclude = NULL;
        DSMatrix * submatrix = NULL;
        va_list ap;
	va_start(ap, firstRow);
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (numberOfRows == 0) {
                DSError("No rows being excluded", A_DS_WARN);
                submatrix = DSMatrixCopy(matrix);
                goto bail;
        }
        rowsToExclude = DSSecureMalloc(sizeof(DSUInteger)*numberOfRows);
        rowsToExclude[0] = firstRow;
        for (i = 1; i < numberOfRows; i++)
                rowsToExclude[i] = va_arg(ap, DSUInteger);
        submatrix = DSMatrixSubMatrixExcludingRows(matrix, numberOfRows, rowsToExclude);
        DSSecureFree(rowsToExclude);
        va_end(ap);
bail:
        return submatrix;
}

extern DSMatrix * DSMatrixSubMatrixExcludingRows(const DSMatrix *matrix, const DSUInteger numberOfRows, const DSUInteger *rows)
{
        DSUInteger i, j, k;
        DSUInteger *rowsToInclude = NULL;
        DSMatrix * submatrix = NULL;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (numberOfRows == 0) {
                submatrix = DSMatrixCopy(matrix);
                goto bail;
        }
        if (numberOfRows == DSMatrixRows(matrix)) {
                submatrix = NULL;
                goto bail;
        }
        if (rows == NULL) {
                DSError("Null array for rows", A_DS_WARN);
                goto bail;
        }
        rowsToInclude = DSSecureMalloc(sizeof(DSUInteger)*(DSMatrixRows(matrix)-numberOfRows));
        k = 0;
        for (i = 0; i < DSMatrixRows(matrix); i++) {
                for (j = 0; j < numberOfRows; j++)
                        if (rows[j] == i)
                                break;
                if (j == numberOfRows)
                        rowsToInclude[k++] = i;
                if (k >= DSMatrixRows(matrix)-numberOfRows)
                        rowsToInclude = DSSecureRealloc(rowsToInclude, sizeof(DSUInteger)*(k+1));
        }
        submatrix = DSMatrixSubMatrixIncludingRows(matrix, k, rowsToInclude);
        DSSecureFree(rowsToInclude);
bail:
        return submatrix;
}

extern DSMatrix * DSMatrixSubMatrixExcludingColumnList(const DSMatrix *matrix, const DSUInteger numberOfColumns, const DSUInteger firstColumn, ...)
{
        unsigned int i;
        DSUInteger *columnsToExclude = NULL;
        DSMatrix * submatrix = NULL;
        va_list ap;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (numberOfColumns == 0) {
                DSError("No columns being excluded", A_DS_WARN);
                submatrix = DSMatrixCopy(matrix);
                goto bail;
        }
        va_start(ap, firstColumn);
        columnsToExclude = DSSecureMalloc(sizeof(DSUInteger)*numberOfColumns);
        columnsToExclude[0] = firstColumn;
        for (i = 1; i < numberOfColumns; i++)
                columnsToExclude[i] = va_arg(ap, DSUInteger);
        submatrix = DSMatrixSubMatrixExcludingColumns(matrix, numberOfColumns, columnsToExclude);
        DSSecureFree(columnsToExclude);
        va_end(ap);
bail:
        return submatrix;
}

extern DSMatrix * DSMatrixSubMatrixExcludingColumns(const DSMatrix *matrix, const DSUInteger numberOfColumns, const DSUInteger *columns)
{
        DSMatrix * submatrix = NULL;
        DSMatrix *transpose, *subTranspose;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (numberOfColumns == 0) {
                submatrix = DSMatrixCopy(matrix);
                goto bail;
        }
        if (numberOfColumns == DSMatrixColumns(matrix)) {
                submatrix = NULL;
                goto bail;
        }
        transpose = DSMatrixTranspose(matrix);
        subTranspose = DSMatrixSubMatrixExcludingRows(transpose, numberOfColumns, columns);
        submatrix = DSMatrixTranspose(subTranspose);
        DSMatrixFree(transpose);
        DSMatrixFree(subTranspose);
bail:
        return submatrix;
}

extern DSMatrix * DSMatrixSubMatrixIncludingRowList(const DSMatrix *matrix, const DSUInteger numberOfRows, const DSUInteger firstRow, ...)
{
        unsigned int i;
        DSUInteger *rowsToInclude = NULL;
        DSMatrix * submatrix = NULL;
        va_list ap;
	va_start(ap, firstRow);
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (numberOfRows == 0) {
                DSError("No rows being included", A_DS_WARN);
                goto bail;
        }
        rowsToInclude = DSSecureMalloc(sizeof(DSUInteger)*numberOfRows);
        rowsToInclude[0] = firstRow;
        for (i = 1; i < numberOfRows; i++)
                rowsToInclude[i] = va_arg(ap, DSUInteger);
        submatrix = DSMatrixSubMatrixIncludingRows(matrix, numberOfRows, rowsToInclude);
        DSSecureFree(rowsToInclude);
        va_end(ap);
bail:
        return submatrix;
}

extern DSMatrix * DSMatrixSubMatrixIncludingRows(const DSMatrix *matrix, const DSUInteger numberOfRows,
                                                 const DSUInteger * rows)
{
        DSUInteger i, j;
        DSMatrix *submatrix = NULL;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (numberOfRows == 0) {
                goto bail;
        }
        if (rows == NULL) {
                DSError("Null array for rows", A_DS_WARN);
                goto bail;
        }
        submatrix = DSMatrixAlloc(numberOfRows, DSMatrixColumns(matrix));
        for (i = 0; i < numberOfRows; i++) {
                for (j = 0; j < DSMatrixColumns(matrix); j++) {
                        DSMatrixSetDoubleValue(submatrix, i, j,
                                               DSMatrixDoubleValue(matrix, rows[i], j));
                }
        }
bail:
        return submatrix;
}

extern DSMatrix * DSMatrixSubMatrixIncludingColumnList(const DSMatrix *matrix, const DSUInteger numberOfColumns, const DSUInteger firstColumn, ...)
{
        unsigned int i;
        DSUInteger *columnsToInclude = NULL;
        DSMatrix * submatrix = NULL;
        va_list ap;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (numberOfColumns == 0) {
                goto bail;
        }
        va_start(ap, firstColumn);
        columnsToInclude = DSSecureMalloc(sizeof(DSUInteger)*numberOfColumns);
        columnsToInclude[0] = firstColumn;
        for (i = 1; i < numberOfColumns; i++)
                columnsToInclude[i] = va_arg(ap, DSUInteger);
        submatrix = DSMatrixSubMatrixIncludingColumns(matrix, numberOfColumns, columnsToInclude);
        DSSecureFree(columnsToInclude);
        va_end(ap);
bail:
        return submatrix;
}

extern DSMatrix * DSMatrixSubMatrixIncludingColumns(const DSMatrix *matrix, 
                                                    const DSUInteger numberOfColumns, 
                                                    const DSUInteger *columns)
{
        DSMatrix * submatrix = NULL;
        DSMatrix *transpose, *subTranspose;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (numberOfColumns == 0) {
                DSError("No columns being included", A_DS_WARN);
                goto bail;
        }
        transpose = DSMatrixTranspose(matrix);
        subTranspose = DSMatrixSubMatrixIncludingRows(transpose, numberOfColumns, columns);
        submatrix = DSMatrixTranspose(subTranspose);
        DSMatrixFree(transpose);
        DSMatrixFree(subTranspose);
bail:
        return submatrix;
}

extern DSMatrix * DSMatrixSubMatrixExcludingRowAndColumnList(const DSMatrix *matrix,
                                                             const DSUInteger numberOfRows,
                                                             const DSUInteger numberOfColumns,
                                                             const DSUInteger firstRow, ...)
{
        DSMatrix *submatrix = NULL, *tempSubmatrix;
        DSUInteger i, *rowsToExclude, *columnsToExclude;
        va_list ap;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (numberOfColumns == 0 && numberOfRows == 0) {
                DSError("No columns and rows being included", A_DS_WARN);
                goto bail;
        }
        va_start(ap, firstRow);
        rowsToExclude = DSSecureMalloc(sizeof(DSUInteger)*numberOfRows);
        columnsToExclude = DSSecureMalloc(sizeof(DSUInteger)*numberOfColumns);
        rowsToExclude[0] = firstRow;
        for (i = 1; i < numberOfRows; i++)
                rowsToExclude[i] = va_arg(ap, DSUInteger);
        for (i = 0; i < numberOfColumns; i++)
                columnsToExclude[i] = va_arg(ap, DSUInteger);
        tempSubmatrix = DSMatrixSubMatrixExcludingColumns(matrix, numberOfColumns, columnsToExclude);
        submatrix = DSMatrixSubMatrixExcludingRows(tempSubmatrix, numberOfRows, rowsToExclude);
        DSMatrixFree(tempSubmatrix);
        DSSecureFree(rowsToExclude);
        DSSecureFree(columnsToExclude);
bail:
        return submatrix;
}

extern DSMatrix * DSMatrixSubMatrixExcludingRowsAndColumns(const DSMatrix *matrix,
                                                           const DSUInteger numberOfRows,
                                                           const DSUInteger numberOfColumns,
                                                           const DSUInteger *rows,
                                                           const DSUInteger *columns)
{
        DSMatrix *submatrix = NULL, *tempSubmatrix;
        if (numberOfRows == DSMatrixRows(matrix) || numberOfColumns == DSMatrixColumns(matrix)) {
                goto bail;
        }
        tempSubmatrix = DSMatrixSubMatrixExcludingColumns(matrix, numberOfColumns, columns);
        submatrix = DSMatrixSubMatrixExcludingRows(tempSubmatrix, numberOfRows, rows);
        DSMatrixFree(tempSubmatrix);
bail:
        return submatrix;
}

extern DSMatrix * DSMatrixSubMatrixIncludingRowAndColumnList(const DSMatrix *matrix,
                                                           const DSUInteger numberOfRows,
                                                           const DSUInteger numberOfColumns,
                                                           const DSUInteger firstRow, ...)
{
        DSMatrix *submatrix = NULL, *tempSubmatrix;
        DSUInteger i, *rowsToInclude, *columnsToInclude;
        va_list ap;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (numberOfColumns == 0 && numberOfRows == 0) {
                DSError("No columns and rows being included", A_DS_WARN);
                goto bail;
        }
        va_start(ap, firstRow);
        rowsToInclude = DSSecureMalloc(sizeof(DSUInteger)*numberOfRows);
        columnsToInclude = DSSecureMalloc(sizeof(DSUInteger)*numberOfColumns);
        rowsToInclude[0] = firstRow;
        for (i = 1; i < numberOfRows; i++)
                rowsToInclude[i] = va_arg(ap, DSUInteger);
        for (i = 0; i < numberOfColumns; i++)
                columnsToInclude[i] = va_arg(ap, DSUInteger);
        tempSubmatrix = DSMatrixSubMatrixIncludingColumns(matrix, numberOfColumns, columnsToInclude);
        submatrix = DSMatrixSubMatrixIncludingRows(tempSubmatrix, numberOfRows, rowsToInclude);
        DSMatrixFree(tempSubmatrix);
        DSSecureFree(rowsToInclude);
        DSSecureFree(columnsToInclude);
bail:
        return submatrix;
}

extern DSMatrix * DSMatrixSubMatrixIncludingRowsAndColumns(const DSMatrix *matrix,
                                                           const DSUInteger numberOfRows,
                                                           const DSUInteger numberOfColumns,
                                                           const DSUInteger *rows,
                                                           const DSUInteger *columns)
{
        DSMatrix *submatrix = NULL, *tempSubmatrix;
        tempSubmatrix = DSMatrixSubMatrixIncludingColumns(matrix, numberOfColumns, columns);
        submatrix = DSMatrixSubMatrixIncludingRows(tempSubmatrix, numberOfRows, rows);
        DSMatrixFree(tempSubmatrix);
bail:
        return submatrix;
}

extern DSMatrix * DSMatrixAppendMatrices(const DSMatrix *firstMatrix, 
                                         const DSMatrix *secondMatrix,
                                         const bool byColumn)
{
        DSMatrix *matrix = NULL, *current;
        DSUInteger i, j, k, l;
        if (firstMatrix == NULL || secondMatrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (byColumn == true && DSMatrixRows(firstMatrix) != DSMatrixRows(secondMatrix)) {
                DSError(M_DS_WRONG ": Number of rows does not match", A_DS_ERROR);
                goto bail;
        } else if (byColumn == false && DSMatrixColumns(firstMatrix) != DSMatrixColumns(secondMatrix)) {
                DSError(M_DS_WRONG ": Number of columns does not match", A_DS_ERROR);
                goto bail;
        }
        if (byColumn == true) {
                matrix = DSMatrixAlloc(DSMatrixRows(firstMatrix),
                                       DSMatrixColumns(firstMatrix) + DSMatrixColumns(secondMatrix));
        } else {
                matrix = DSMatrixAlloc(DSMatrixRows(firstMatrix) + DSMatrixRows(secondMatrix),
                                       DSMatrixColumns(firstMatrix));
        }
        for (i = 0; i < DSMatrixRows(matrix); i++) {
                for (j = 0; j < DSMatrixColumns(matrix); j++) {
                        current = (DSMatrix *)firstMatrix;
                        k = i;
                        l = j;
                        if (byColumn == true && j >= DSMatrixColumns(firstMatrix)) {
                                l = j - DSMatrixColumns(firstMatrix);
                                current = (DSMatrix *)secondMatrix;
                        } else if (byColumn == false && i >= DSMatrixRows(firstMatrix)) {
                                k = i - DSMatrixRows(firstMatrix);
                                current = (DSMatrix *)secondMatrix;
                        }
                        DSMatrixSetDoubleValue(matrix, i, j,
                                               DSMatrixDoubleValue(current, k, l));
                }
        }
        
bail:
        return matrix;
}

extern void DSMatrixSwitchRows(DSMatrix *matrix, const DSUInteger rowA, const DSUInteger rowB)
{
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (rowA >= DSMatrixRows(matrix) || rowB >= DSMatrixRows(matrix)) {
                DSError(M_DS_MAT_OUTOFBOUNDS, A_DS_WARN);
                goto bail;
        }
       gsl_matrix_swap_rows(DSMatrixInternalPointer(matrix), rowA, rowB);
bail:
        return;
}

extern void DSMatrixClearRow(DSMatrix *matrix, const DSUInteger row)
{
        DSUInteger i;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (row >= DSMatrixRows(matrix)) {
                DSError(M_DS_MAT_OUTOFBOUNDS, A_DS_WARN);
                goto bail;
        }
        for (i = 0; i < DSMatrixColumns(matrix); i++) {
                DSMatrixSetDoubleValue(matrix, row, i, 0.0f);
        }
bail:
        return;
}

extern void DSMatrixClearColumns(DSMatrix *matrix, const DSUInteger column)
{
        DSUInteger i;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (column >= DSMatrixColumns(matrix)) {
                DSError(M_DS_MAT_OUTOFBOUNDS, A_DS_WARN);
                goto bail;
        }
        for (i = 0; i < DSMatrixRows(matrix); i++) {
                DSMatrixSetDoubleValue(matrix, i, column, 0.0f);
        }
bail:
        return;
}

extern void DSMatrixSwitchColumns(DSMatrix *matrix, const DSUInteger columnA, const DSUInteger columnB)
{
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (columnA >= DSMatrixColumns(matrix) || columnB >= DSMatrixColumns(matrix)) {
                DSError(M_DS_MAT_OUTOFBOUNDS, A_DS_WARN);
                goto bail;
        }
       gsl_matrix_swap_columns(DSMatrixInternalPointer(matrix), columnA, columnB);
bail:
        return;
}

extern DSMatrix * DSMatrixWithUniqueRows(const DSMatrix *matrix)
{
        DSMatrix *newMatrix = NULL;
        bool * rowsToRemove = NULL;
        DSUInteger * indexRowsToRemove;
        DSUInteger i, j, k, numberToRemove;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        numberToRemove = 0;
        rowsToRemove = DSSecureCalloc(sizeof(bool), DSMatrixRows(matrix));
        for (i = 0; i < DSMatrixRows(matrix)-1; i++) {
                if (rowsToRemove[i] == true)
                        continue;
                for (j = i+1; j < DSMatrixRows(matrix); j++) {
                        if (rowsToRemove[j] == true)
                                continue;
                        for (k = 0; k < DSMatrixColumns(matrix); k++)
                                if (gsl_matrix_get(DSMatrixInternalPointer(matrix), i, k) !=
                                   gsl_matrix_get(DSMatrixInternalPointer(matrix), j, k))
                                        break;
                        if (k == DSMatrixColumns(matrix)) {
                                rowsToRemove[j] = true;
                                numberToRemove++;
                        }
                }
        }
        j = 0;
        if (numberToRemove != 0) {
                indexRowsToRemove = DSSecureCalloc(sizeof(DSUInteger), numberToRemove);
                for (i = 0; i < DSMatrixRows(matrix); i++) {
                        if (rowsToRemove[i] == true)
                                indexRowsToRemove[j++] = i;
                }
                newMatrix = DSMatrixSubMatrixExcludingRows(matrix, numberToRemove, indexRowsToRemove);
                DSSecureFree(indexRowsToRemove);
        }
        DSSecureFree(rowsToRemove);
bail:
        return newMatrix;
}

extern void DSMatrixPrint(const DSMatrix *matrix)
{
        int (*print)(const char *, ...);
        DSUInteger i, j;
        if (matrix == NULL) {
                goto bail;
        }
        if (DSPrintf == NULL)
                print = printf;
        else
                print = DSPrintf;
        for (i = 0; i < DSMatrixRows(matrix); i++) {
                printf("[");
                for (j = 0; j < DSMatrixColumns(matrix); j++) {
                        print("%lf", DSMatrixDoubleValue(matrix, i, j), 
                              ((j == DSMatrixColumns(matrix)-1) ? "]" : ", "));
                        if (j == DSMatrixColumns(matrix)-1 && i == DSMatrixRows(matrix)-1) {
                                printf("]\n");
                        } else if (j == DSMatrixColumns(matrix)-1) {
                                printf("],\n");
                        } else {
                                printf(", ");
                        }
                }
        }
bail:
        return;
}

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Matrix Property Querying
#endif

extern bool DSMatrixIsIdentity(const DSMatrix *matrix)
{
        DSUInteger i, j;
        bool isIdentity = false;
        if (matrix == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        for (i = 0; i < DSMatrixRows(matrix); i++) {
                for (j = 0; j < DSMatrixColumns(matrix); j++) {
                        if (DSMatrixDoubleValue(matrix, i, j) != (i == j))
                                goto bail;
                }
        }
        isIdentity = true;
bail:
        return isIdentity;
}

extern bool DSMatrixIsSquare(const DSMatrix *matrix)
{
        bool isSquare = false;
        if (matrix == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixRows(matrix) == DSMatrixColumns(matrix))
                isSquare = true;
bail:
        return isSquare;
}

__deprecated static DSUInteger DSMatrixRankSquareMatrix(const DSMatrix *matrix)
{
        DSUInteger rank = 0;
        DSMatrix *subMatrix = NULL;
        double determinant;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        determinant = DSMatrixDeterminant(matrix);
        if (determinant == 0.0 && DSMatrixRows(matrix) <= 1) {
                goto bail;
        }
        if (determinant == 0.0) {
                subMatrix = DSMatrixSubMatrixExcludingRowAndColumnList(matrix, 1, 1, DSMatrixRows(matrix)-1, DSMatrixColumns(matrix)-1);
                rank = DSMatrixRankSquareMatrix(subMatrix);
                DSMatrixFree(subMatrix);
        } else {
                rank = DSMatrixRows(matrix);
        }
bail:
        return rank;
}

__deprecated static DSUInteger DSMatrixRankNonSquareMatrix(const DSMatrix *matrix)
{
        DSUInteger rank = 0, i;
        DSMatrixArray * SVD = NULL;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixRows(matrix) < DSMatrixColumns(matrix)) {
                DSError("Cannot get rank: more columns than rows", A_DS_WARN);
                goto bail;
        }
        SVD = DSMatrixSVD(matrix);
        if (SVD == NULL) {
                DSError("Singular value decomposition is null", A_DS_ERROR);
                goto bail;
        }
        if (DSMatrixArrayMatrix(SVD, 0) == NULL) {
                DSError(M_DS_MAT_NULL ": S matrix", A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < DSMatrixColumns(DSMatrixArrayMatrix(SVD, 0)); i++) {
                rank += (DSMatrixDoubleValue(DSMatrixArrayMatrix(SVD, 0), 0, i) != 0);
        }
        DSMatrixArrayFree(SVD);
bail:
        return rank;
}

extern DSUInteger DSMatrixRank(const DSMatrix *matrix)
{
        DSUInteger rank = 0, i;
        DSMatrixArray * SVD = NULL;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixRows(matrix) < DSMatrixColumns(matrix)) {
                DSError("Cannot get rank: more columns than rows", A_DS_WARN);
                goto bail;
        }
        SVD = DSMatrixSVD(matrix);
        if (SVD == NULL) {
                DSError("Singular value decomposition is null", A_DS_ERROR);
                goto bail;
        }
        if (DSMatrixArrayMatrix(SVD, 0) == NULL) {
                DSError(M_DS_MAT_NULL ": S matrix", A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < DSMatrixColumns(DSMatrixArrayMatrix(SVD, 0)); i++) {
                rank += (DSMatrixDoubleValue(DSMatrixArrayMatrix(SVD, 0), 0, i) != 0);
        }
        DSMatrixArrayFree(SVD);
bail:
        return rank;
}

extern double minimumValue(const DSMatrix *matrix, const bool shouldExcludeZero)
{
        double minValue = NAN;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixInternalPointer(matrix) == NULL) {
                DSError(M_DS_MAT_NOINTERNAL, A_DS_ERROR);
                goto bail;
        }
        minValue =gsl_matrix_min(DSMatrixInternalPointer(matrix));
bail:
        return minValue;
}

extern double complex DSMatrixDominantEigenvalue(const DSMatrix *matrix)
{
        DSMatrix * copy = NULL;
        double complex eigenValue = 0+0i;
        gsl_vector_complex *eval;
        gsl_matrix_complex *evec;
        gsl_eigen_nonsymmv_workspace * w;
        gsl_complex gslcomplex;
        
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSMatrixRows(matrix) != DSMatrixColumns(matrix)) {
                DSError(M_DS_WRONG ": DSMatrix is not a square matrix", A_DS_ERROR);
                goto bail;
        }
        copy = DSMatrixCopy(matrix);
        eval = gsl_vector_complex_alloc (DSMatrixRows(copy));
        evec = gsl_matrix_complex_alloc (DSMatrixRows(copy), DSMatrixColumns(copy));
        w = gsl_eigen_nonsymmv_alloc(DSMatrixRows(copy));
        
        gsl_eigen_nonsymmv(DSMatrixInternalPointer(copy), eval, evec, w);
        gsl_eigen_nonsymmv_sort(eval, evec, GSL_EIGEN_SORT_ABS_ASC);
        gslcomplex = gsl_vector_complex_get(eval, 0);
        eigenValue = GSL_REAL(gslcomplex)+GSL_IMAG(gslcomplex)*1i;
        printf("%lf+%lfi\n", creal(eigenValue), cimag(eigenValue));
        gsl_vector_complex_free(eval);
        gsl_matrix_complex_free(evec);
        gsl_eigen_nonsymmv_free(w);
        DSMatrixFree(copy);
bail:
        return eigenValue;
}

extern gsl_vector_complex * DSMatrixEigenvalues(const DSMatrix *matrix)
{
        DSMatrix * copy = NULL;
        gsl_vector_complex *eval = NULL;
        gsl_matrix_complex *evec;
        gsl_eigen_nonsymmv_workspace * w;
        
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSMatrixRows(matrix) != DSMatrixColumns(matrix)) {
                DSError(M_DS_WRONG ": DSMatrix is not a square matrix", A_DS_ERROR);
                goto bail;
        }
        copy = DSMatrixCopy(matrix);
        eval = gsl_vector_complex_alloc (DSMatrixRows(copy));
        evec = gsl_matrix_complex_alloc (DSMatrixRows(copy), DSMatrixColumns(copy));
        w = gsl_eigen_nonsymmv_alloc(DSMatrixRows(copy));
        
        gsl_eigen_nonsymmv(DSMatrixInternalPointer(copy), eval, evec, w);
        gsl_eigen_nonsymmv_sort(eval, evec, GSL_EIGEN_SORT_ABS_ASC);
        gsl_matrix_complex_free(evec);
        gsl_eigen_nonsymmv_free(w);
        DSMatrixFree(copy);
bail:
        return eval;
}

extern double maximumValue(const DSMatrix *matrix, const bool shouldExcludeZero)
{
        double maxValue = NAN;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixInternalPointer(matrix) == NULL) {
                DSError(M_DS_MAT_NOINTERNAL, A_DS_ERROR);
                goto bail;
        }
        maxValue =gsl_matrix_max(DSMatrixInternalPointer(matrix));
bail:
        return maxValue;
}

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Matrix Operations
#endif

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark Arithmetic
#endif

extern void DSMatrixAddByMatrix(DSMatrix *addTo, const DSMatrix *addBy)
{
        if (addTo == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (addBy == NULL) {
                DSError(M_DS_MAT_NULL ": Adding by NULL", A_DS_WARN);
                goto bail;
        }
        if (DSMatrixRows(addTo) != DSMatrixRows(addBy)) {
                DSError("Matrix rows do not match", A_DS_ERROR);
        } else if (DSMatrixColumns(addTo) != DSMatrixColumns(addBy)) {
                DSError("Matrix columns do not match", A_DS_ERROR);
        } else {
               gsl_matrix_add(DSMatrixInternalPointer(addTo),
                               DSMatrixInternalPointer(addBy));
        }
bail:
        return;
}

extern void DSMatrixSubstractByMatrix(DSMatrix *addTo, const DSMatrix *addBy)
{
        if (addTo == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (addBy == NULL) {
                DSError(M_DS_MAT_NULL ": Adding by NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSMatrixRows(addTo) != DSMatrixRows(addBy)) {
                DSError("Matrix rows do not match", A_DS_ERROR);
        } else if (DSMatrixColumns(addTo) != DSMatrixColumns(addBy)) {
                DSError("Matrix columns do not match", A_DS_ERROR);
        } else {
               gsl_matrix_sub(DSMatrixInternalPointer(addTo),
                               DSMatrixInternalPointer(addBy));
        }
bail:
        return;
}

extern void DSMatrixApplyFunction(DSMatrix *matrix, double (*function)(double)) {
        DSUInteger i, j;
        if (matrix == NULL || function == NULL) {
                DSError(M_DS_NULL, A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < DSMatrixRows(matrix); i++) {
                for (j = 0; j < DSMatrixColumns(matrix); j++) {
                        DSMatrixSetDoubleValue(matrix,
                                               i, j,
                                               function(DSMatrixDoubleValue(matrix,
                                                                            i, j)));
                }
        }
bail:
        return;
}


extern void DSMatrixMultiplyByScalar(DSMatrix *matrix, const double value)
{
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
       gsl_matrix_scale(DSMatrixInternalPointer(matrix), value);
bail:
        return;
}

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark Linear Algebra
#endif

extern double DSMatrixDeterminant(const DSMatrix *matrix)
{
       gsl_matrix *LU;
        gsl_permutation *p;
        int sign;
        double determinant = NAN;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixIsSquare(matrix) == false) {
                DSError("Determinant of rectangular matrix undefined", A_DS_WARN);
                goto bail;
        }
        p = gsl_permutation_alloc(DSMatrixRows(matrix));
        LU =gsl_matrix_alloc(DSMatrixRows(matrix), DSMatrixColumns(matrix));
       gsl_matrix_memcpy(LU, DSMatrixInternalPointer(matrix));
        gsl_linalg_LU_decomp(LU, p, &sign);
        determinant = gsl_linalg_LU_det(LU, sign);
        gsl_permutation_free(p);
       gsl_matrix_free(LU);
bail:
        return determinant;
}


extern double DSMatrixMinor(const DSMatrix *matrix,
                            const DSUInteger row,
                            const DSUInteger column)
{
        double minor = NAN;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (row >= DSMatrixRows(matrix)) {
                DSError(M_DS_WRONG, A_DS_ERROR);
                goto bail;
        }
        if (column >= DSMatrixColumns(matrix)) {
                DSError(M_DS_WRONG, A_DS_ERROR);
                goto bail;
        }
        DSMatrix * submatrix = DSMatrixSubMatrixExcludingRowAndColumnList(matrix,
                                                                          1,
                                                                          1,
                                                                          row,
                                                                          column);
        minor = DSMatrixDeterminant(submatrix);
        DSMatrixFree(submatrix);
bail:
        return minor;
}

extern DSMatrix * DSMatrixTranspose(const DSMatrix *matrix)
{
        DSMatrix *transpose = NULL;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        transpose = DSMatrixAlloc(DSMatrixColumns(matrix), DSMatrixRows(matrix));
        gsl_matrix_transpose_memcpy(DSMatrixInternalPointer(transpose), 
                                    DSMatrixInternalPointer(matrix));
bail:
        return transpose;
}

extern DSMatrix * DSMatrixInverse(const DSMatrix *matrix)
{
       gsl_matrix *LU;
        gsl_permutation *p;
        DSMatrix *aMatrix = NULL;
        int sign;
        if (matrix == NULL)
                goto bail;
        if (DSMatrixIsSquare(matrix) == false)
                goto bail;
        if (fabs(DSMatrixDeterminant(matrix)) < 1E-14) {
                DSError("Matrix to invert is singular", A_DS_NOERROR);
                goto bail;
        }
        aMatrix = DSMatrixAlloc(DSMatrixRows(matrix), DSMatrixColumns(matrix));
        p = gsl_permutation_alloc(DSMatrixRows(matrix));
        LU =gsl_matrix_alloc(DSMatrixRows(matrix), DSMatrixColumns(matrix));
       gsl_matrix_memcpy(LU, DSMatrixInternalPointer(matrix));
        gsl_linalg_LU_decomp(LU, p, &sign);
        gsl_linalg_LU_invert(LU, p, DSMatrixInternalPointer(aMatrix));
        gsl_permutation_free(p);
       gsl_matrix_free(LU);
bail:
        return aMatrix;
}

extern DSMatrixArray * DSMatrixSVD(const DSMatrix *matrix)
{
        DSMatrix *S, *U, *V;
        DSMatrixArray *array = NULL;
        DSUInteger i;
        gsl_matrix *u, *v;
        gsl_vector *w, *s;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        array = DSMatrixArrayAlloc();
        S = DSMatrixAlloc(1, DSMatrixColumns(matrix));
        U = DSMatrixCopy(matrix);
        V = DSMatrixAlloc(DSMatrixColumns(matrix), DSMatrixColumns(matrix));
        u = DSMatrixInternalPointer(U);
        v = DSMatrixInternalPointer(V);
        s = gsl_vector_alloc(DSMatrixColumns(matrix));
        w = gsl_vector_alloc(DSMatrixColumns(matrix));
        gsl_linalg_SV_decomp(u, v, s, w);
        for (i = 0; i < DSMatrixColumns(matrix); i++)
                DSMatrixSetDoubleValue(S, 0, i, gsl_vector_get(s, i));
        DSMatrixRoundToSignificantFigures(S, 14);
        gsl_vector_free(w);
        gsl_vector_free(s);
        DSMatrixArrayAddMatrix(array, S);
        DSMatrixArrayAddMatrix(array, U);
        DSMatrixArrayAddMatrix(array, V);
bail:
        return array;
        
}

extern DSMatrixArray * DSMatrixQRD(const DSMatrix *matrix)
{
        DSMatrix *QR, *Q, *R;
        DSMatrixArray *array = NULL;
        DSUInteger min;
        gsl_matrix *qr, *q, *r;
        gsl_vector *tau;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        array = DSMatrixArrayAlloc();
        QR = DSMatrixCopy(matrix);
        Q = DSMatrixCalloc(DSMatrixRows(matrix), DSMatrixRows(matrix));
        R = DSMatrixCalloc(DSMatrixRows(matrix), DSMatrixColumns(matrix));
        qr = QR->mat;
        q = Q->mat;
        r = R->mat;
        min = (DSMatrixRows(matrix) < DSMatrixColumns(matrix) ? DSMatrixRows(matrix) : DSMatrixColumns(matrix));
        tau = gsl_vector_alloc(min);
        gsl_linalg_QR_decomp(qr, tau);
        gsl_linalg_QR_unpack(qr, tau, q, r);
        DSMatrixArrayAddMatrix(array, Q);
        DSMatrixArrayAddMatrix(array, R);
        gsl_vector_free(tau);
        DSMatrixFree(QR);
bail:
        return array;
        
}


/*
 DSTNumericalMatrix *nullspace = nil;
 gsl_matrix *tempMatrix;
 NSUInteger cols, i, j, k;
 if (matrix == NULL)
 goto bail;
 [self calculateSingularValueDecomposition];
 if (U == NULL || S == NULL || V == NULL || w == NULL)
 goto bail;
 cols = DST_MATRIX_COLUMNS(matrix) - [self rank];
 if (cols == 0)
 goto bail;
 tempMatrix = gsl_matrix_alloc(DST_MATRIX_ROWS(matrix), cols);
 j = 0;
 for (i = 0; i < DST_MATRIX_ROWS(V); i++) {
 if (fabs(S->data[i*S->stride]) > tolerance)
 continue;
 for (k = 0; k < DST_MATRIX_COLUMNS(V); k++) {
 tempMatrix->data[j*tempMatrix->tda+k] = V->data[k*V->tda+i];
 }
 j++;
 }
 nullspace = [[DSTNumericalMatrix alloc] init];
 [nullspace setRows:DST_MATRIX_ROWS(tempMatrix) columns:DST_MATRIX_COLUMNS(tempMatrix)];
 gsl_matrix_memcpy([nullspace matrix], tempMatrix);
 gsl_matrix_free(tempMatrix);
 [nullspace autorelease];
 bail:
 return nullspace;
 */

static DSMatrix * dsMatrixRightNullspaceMLTN(const DSMatrix *matrix)
{
        DSMatrix *nullspace = NULL, *Q = NULL, *R = NULL, *Rt;
        DSMatrixArray * QRD = NULL;
        DSUInteger i, j , columns, rank = 0, * include;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        QRD = DSMatrixQRD(matrix);
        if (QRD == NULL) {
                DSError(M_DS_NULL ": QR decomposition failed", A_DS_ERROR);
                goto bail;
        }
        Q = DSMatrixArrayMatrix(QRD, 0);
        R = DSMatrixArrayMatrix(QRD, 1);
        Rt = DSMatrixTranspose(R);
        rank = DSMatrixRank(Rt);
        columns = DSMatrixColumns(matrix) - rank;
        include = DSSecureCalloc(sizeof(DSUInteger), columns);
        j = 0;
        for (i = rank; i < DSMatrixColumns(Q); i++) {
                include[j++] = i;
        }
        nullspace = DSMatrixSubMatrixIncludingColumns(Q, columns, include);
bail:
        if (QRD != NULL)
                DSMatrixArrayFree(QRD);
        return nullspace;
}

extern DSMatrix * DSMatrixRightNullspace(const DSMatrix *matrix)
{
        DSMatrix *nullspace = NULL, *V = NULL, *S = NULL;
        DSMatrixArray * svd = NULL;
        DSUInteger i, j , k, columns, rank = 0;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSMatrixRows(matrix) < DSMatrixColumns(matrix)) {
                nullspace = dsMatrixRightNullspaceMLTN(matrix);
                goto bail;
        }
        svd = DSMatrixSVD(matrix);
        if (svd == NULL) {
                DSError(M_DS_NULL ": Singular Value decomposition failed", A_DS_ERROR);
                goto bail;
        }
        S = DSMatrixArrayMatrix(svd, 0);
        V = DSMatrixArrayMatrix(svd, 2);
        if (S == NULL) {
                DSError(M_DS_MAT_NULL ": S matrix is NULL", A_DS_ERROR);
                goto bail;
        }
        if (V == NULL) {
                DSError(M_DS_MAT_NULL ": V matrix is NULL", A_DS_ERROR);
                goto bail;                
        }
        for (i = 0; i < DSMatrixColumns(S); i++)
                rank += (fabs(DSMatrixDoubleValue(S, 0, i)) >= 1E-14);
        columns = DSMatrixColumns(matrix) - rank;
        if (columns == 0)
                goto bail;
        nullspace = DSMatrixCalloc(DSMatrixRows(V), columns);
        j = 0;
        for (i = 0; i < DSMatrixColumns(S); i++) {
                if (fabs(DSMatrixDoubleValue(S, 0, i)) >= 1E-14)
                    continue;
                for (k = 0; k < DSMatrixRows(V); k++) {
                        DSMatrixSetDoubleValue(nullspace, k, j, DSMatrixDoubleValue(V, k, i));
                }
                j++;
        }
bail:
        if (svd != NULL)
                DSMatrixArrayFree(svd);

        return nullspace;
}

extern DSMatrix * DSMatrixRightNullspaceMLTN(const DSMatrix *matrix)
{
        DSMatrix *nullspace = NULL, *V = NULL, *S = NULL;
        DSMatrixArray * svd = NULL;
        DSUInteger i, j , k, columns, rank = 0;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        svd = DSMatrixSVD(matrix);
        if (svd == NULL) {
                DSError(M_DS_NULL ": Singular Value decomposition failed", A_DS_ERROR);
                goto bail;
        }
        S = DSMatrixArrayMatrix(svd, 0);
        V = DSMatrixArrayMatrix(svd, 2);
        if (S == NULL) {
                DSError(M_DS_MAT_NULL ": S matrix is NULL", A_DS_ERROR);
                goto bail;
        }
        if (V == NULL) {
                DSError(M_DS_MAT_NULL ": V matrix is NULL", A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < DSMatrixColumns(S); i++)
                rank += (fabs(DSMatrixDoubleValue(S, 0, i)) >= 1E-14);
        columns = DSMatrixColumns(matrix) - rank;
        if (columns == 0)
                goto bail;
        nullspace = DSMatrixCalloc(DSMatrixRows(V), columns);
        j = 0;
        for (i = 0; i < DSMatrixColumns(S); i++) {
                if (fabs(DSMatrixDoubleValue(S, 0, i)) >= 1E-14)
                        continue;
                for (k = 0; k < DSMatrixRows(V); k++) {
                        DSMatrixSetDoubleValue(nullspace, k, j, DSMatrixDoubleValue(V, k, i));
                }
                j++;
        }
bail:
        if (svd != NULL)
                DSMatrixArrayFree(svd);
        
        return nullspace;
}


extern DSMatrix * DSMatrixLeftNullspace(const DSMatrix *matrix)
{
        DSMatrix *nullspace = NULL;
        DSMatrix *transpose = NULL;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        transpose = DSMatrixTranspose(matrix);
        if (transpose != NULL) {
                nullspace = DSMatrixRightNullspace(transpose);
                DSMatrixFree(transpose);
        }
bail:
        return nullspace;
}

//extern DSMatrix * DSMatrixIdenticalRows(const DSMatrix * matrix)
//{
//        DSMatrix *problematic = NULL;
//        bool foundIdentity;
//        DSMatrix *nullspace = NULL;
//        DSUInteger i, j, k, identities;
//        double lvalue, rvalue;
//        if (matrix == NULL) {
//                DSError(M_DS_MAT_NULL, A_DS_ERROR);
//                goto bail;
//        }
//        identities = 0;
//        nullspace = DSMatrixCalloc(DSMatrixRows(matrix), DSMatrixColumns(matrix));
//        for (i = 0; i < DSMatrixRows(matrix); i++) {
//                for (k = 0; k < identities; k++) {
//                        if (DSMatrixDoubleValue(nullspace, i, k) == 1.0f) {
//                                break;
//                        }
//                }
//                if (k != identities)
//                        continue;
//                foundIdentity = false;
//                for (k = i+1; k < DSMatrixRows(matrix); k++) {
//                        for (j = 0; j < DSMatrixColumns(matrix); j++) {
//                                lvalue = DSMatrixDoubleValue(matrix, i, j);
//                                rvalue = DSMatrixDoubleValue(matrix, k, j);
//                                if (fabs(lvalue-rvalue) > 1e-14)
//                                        break;
//                        }
//                        if (j == DSMatrixColumns(matrix)) {
//                                DSMatrixSetDoubleValue(nullspace, i, identities, 1.0f);
//                                DSMatrixSetDoubleValue(nullspace, k, identities, 1.0f);
//                                foundIdentity = true;
//                        }
//                }
//                if (foundIdentity)
//                        identities++;
//        }
//        if (identities == 0) {
//                goto bail;
//        }
//        problematic = DSMatrixCalloc(DSMatrixRows(matrix), identities);
//        for (i = 0; i < DSMatrixRows(matrix); i++) {
//                for (k = 0; k < identities; k++) {
//                        DSMatrixSetDoubleValue(problematic, i, k, DSMatrixDoubleValue(nullspace, i, k));
//                }
//        }
//bail:
//        if (nullspace != NULL)
//                DSMatrixFree(nullspace);
//        return problematic;
//}

extern DSMatrix * DSMatrixIdenticalRows(const DSMatrix * matrix)
{
        DSMatrix *identityMatrix = NULL, * current, * found;
        bool foundIdentity, foundOne = false;
        DSUInteger i, j, k, identities, * columns;
        double lvalue, rvalue;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        identities = 0;
        found = DSMatrixCalloc(DSMatrixRows(matrix), 1);
        current = DSMatrixCalloc(DSMatrixRows(matrix), DSMatrixRows(matrix));
        columns = DSSecureCalloc(sizeof(DSUInteger), DSMatrixColumns(matrix));
        for (i = 0; i < DSMatrixRows(matrix); i++) {
                foundOne = false;
                for (j = i+1; j < DSMatrixRows(matrix); j++) {
                        if (DSMatrixDoubleValue(found, j, 0) != 0.0)
                                continue;
                        foundIdentity = false;
                        for (k = 0; k < DSMatrixColumns(matrix); k++) {
                                lvalue = DSMatrixDoubleValue(matrix, i, k);
                                rvalue = DSMatrixDoubleValue(matrix, j, k);
                                if (fabs(lvalue-rvalue) > 1e-13) {
                                        foundIdentity = false;
                                        break;
                                } else if (fabs(rvalue) < 1e-13) {
                                        continue;
                                } else {
                                        foundIdentity = true;
                                }
                        }
                        if (foundIdentity == true) {
                                DSMatrixSetDoubleValue(current, i, identities, 1.);
                                DSMatrixSetDoubleValue(current, j, identities, 1.);
                                DSMatrixSetDoubleValue(found, i, 0, 1.);
                                DSMatrixSetDoubleValue(found, j, 0, 1.);
                                foundOne = true;
                        }
                }
                if (foundOne == true) {
                        columns[identities] = identities;
                        identities++;
                }
        }
        if (identities > 0) {
                if (identities == DSMatrixColumns(current)) {
                        identityMatrix = current;
                        current = NULL;
                } else {
                        identityMatrix = DSMatrixSubMatrixIncludingColumns(current, identities, columns);
                }

        }
        if (current != NULL)
                DSMatrixFree(current);
        DSSecureFree(columns);
        DSMatrixFree(found);
bail:
        return identityMatrix;
}

/**
 * \brief Creates a LU decomposition and returns the permutation matrix.
 *
 * \details This function creates a LU decomposition of a DSMatrix A.  This
 * function creates an array of three matrices: a DSMatrix P, a DSMatrix L and
 * a DSMatrix U; where \f$ P A = L U \f$.
 *
 * \param A A DSMatrix containing the matrix to be decomposed.
 *
 */
extern DSMatrixArray * DSMatrixPLUDecomposition(const DSMatrix *A)
{
        DSMatrix **PLU = NULL;
        DSMatrixArray *array = NULL;
        DSUInteger i, j;
       gsl_matrix *LU;
        gsl_permutation *p = NULL;
        int sign;
        if (A == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixIsSquare(A) == false) {
                DSError("Determinant of rectangular matrix undefined", A_DS_WARN);
                goto bail;
        }
        p = gsl_permutation_alloc(DSMatrixRows(A));
        LU =gsl_matrix_alloc(DSMatrixRows(A), DSMatrixColumns(A));
       gsl_matrix_memcpy(LU, DSMatrixInternalPointer(A));
        gsl_linalg_LU_decomp(LU, p, &sign);
        PLU = DSSecureCalloc(sizeof(DSMatrix *), 3);
        PLU[0] = DSMatrixCalloc(DSMatrixRows(A), DSMatrixColumns(A));
        PLU[1] = DSMatrixIdentity(DSMatrixRows(A));
        PLU[2] = DSMatrixCalloc(DSMatrixRows(A), DSMatrixColumns(A));

        for (i = 0; i < DSMatrixRows(A); i++) {
                DSMatrixSetDoubleValue(PLU[0], i, (DSUInteger)gsl_permutation_get(p, i), 1.0);
                for (j = 0; j < DSMatrixColumns(A); j++) {
                        if (i > j)
                                DSMatrixSetDoubleValue(PLU[1], i, j,gsl_matrix_get(LU, i, j));
                        else
                                DSMatrixSetDoubleValue(PLU[2], i, j,gsl_matrix_get(LU, i, j));
                }
        }
        gsl_permutation_free(p);
       gsl_matrix_free(LU);
        array = DSMatrixArrayAlloc();
        DSMatrixArrayAddMatrix(array, PLU[0]);
        DSMatrixArrayAddMatrix(array, PLU[1]);
        DSMatrixArrayAddMatrix(array, PLU[2]);
        DSSecureFree(PLU);
bail:
        return array;
}

extern DSMatrix * DSMatrixCharacteristicPolynomialCoefficients(const DSMatrix * matrix)
{
        DSMatrix * coefficients = NULL;
        coefficients = DSMatrixCharacteristicPolynomialUndeterminedCoefficients(matrix, NULL);
        
bail:
        return coefficients;
}

extern DSMatrix * DSMatrixUndeterminedCoefficientsRnMatrixForSize(const DSUInteger matrixSize)
{
        DSMatrix * Sn = NULL;
        DSMatrix * Rn = NULL;
        DSUInteger i, j;
        if (matrixSize == 1) {
                goto bail;
        }
        Sn = DSMatrixAlloc(matrixSize-1, matrixSize-1);
        for (i = 0; i < matrixSize-1; i++) {
                for (j = 0; j < matrixSize-1; j++) {
                        DSMatrixSetDoubleValue(Sn,
                                               i,
                                               j, pow(i+1, matrixSize-(j+1)));
                }
        }
        Rn = DSMatrixInverse(Sn);
        DSMatrixFree(Sn);
bail:
        return Rn;
}

extern DSMatrix * DSMatrixUndeterminedCoefficientsDArrayForMatrix(const DSMatrix *matrix)
{
        DSMatrix * D = NULL;
        DSMatrix * d = NULL;
        DSMatrix * identity = NULL;
        DSMatrix * jI_A = NULL;
        DSUInteger i;
        DSUInteger matrixSize;
        double value;
        if (DSMatrixIsSquare(matrix) == false) {
                DSError(M_DS_WRONG ": matrix is not square", A_DS_ERROR);
                goto bail;
        }
        matrixSize = DSMatrixRows(matrix);
        identity = DSMatrixIdentity(matrixSize);
        D = DSMatrixAlloc(matrixSize-1, 1);
        d = DSMatrixAlloc(matrixSize, 1);
        for (i = 0; i < matrixSize; i++) {
                jI_A = DSMatrixByMultiplyingScalar(identity, 1.0f*i);
                DSMatrixSubstractByMatrix(jI_A, matrix);
                DSMatrixSetDoubleValue(d, i, 0, DSMatrixDeterminant(jI_A));
                DSMatrixFree(jI_A);
        }
        for (i = 0; i < matrixSize-1; i++) {
                value = DSMatrixDoubleValue(d, i+1, 0)-DSMatrixDoubleValue(d, 0, 0)-pow(i+1, matrixSize);
                DSMatrixSetDoubleValue(D, i, 0, value);
        }
        DSMatrixFree(d);
        DSMatrixFree(identity);
bail:
        return D;
}

static DSMatrix * dsMatrixCharacteristicPolynomialUndeterminedCoefficientOneRow(const DSMatrix *matrix)
{
        DSMatrix * coefficients = NULL;
        coefficients = DSMatrixAlloc(1, DSMatrixRows(matrix)+1);
        DSMatrixSetDoubleValue(coefficients, 0, 0, 1.);
        DSMatrixSetDoubleValue(coefficients, 0, 1, -DSMatrixDoubleValue(matrix, 0, 0));
        return coefficients;
}
/**
 * Uses method of undetermined coefficients to find the coefficients of a 
 * characteristic polynomial.
 */
extern DSMatrix * DSMatrixCharacteristicPolynomialUndeterminedCoefficients(const DSMatrix * matrix, const DSMatrix * Rn)
{
        DSMatrix * coefficients = NULL;
        DSMatrix * D = NULL;
        DSMatrix * Rn_internal = NULL;
        DSMatrix * temp = NULL;
        DSUInteger i;
        bool mustDealloc = false;
        DSMatrix * mA = NULL;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSMatrixIsSquare(matrix) == false) {
                DSError(M_DS_WRONG ": Matrix must be square", A_DS_ERROR);
                goto bail;
        }
        if (DSMatrixRows(matrix) == 1) {
                coefficients = dsMatrixCharacteristicPolynomialUndeterminedCoefficientOneRow(matrix);
                goto bail;
        }
        if (Rn != NULL) {
                Rn_internal = (DSMatrix *)Rn;
        } else {
                Rn_internal = DSMatrixUndeterminedCoefficientsRnMatrixForSize(DSMatrixRows(matrix));
                mustDealloc = true;
        }
        if (DSMatrixIsSquare(Rn_internal) == false) {
                DSError(M_DS_WRONG "Rn matrix is not square", A_DS_ERROR);
                goto bail;
        }
        if (DSMatrixRows(matrix) != DSMatrixRows(Rn_internal)+1) {
                DSError(M_DS_MAT_OUTOFBOUNDS ": matrix and Rn matrix of different sizes" , A_DS_ERROR);
                goto bail;
        }
        mA = DSMatrixByMultiplyingScalar(matrix, -1.);
        D = DSMatrixUndeterminedCoefficientsDArrayForMatrix(matrix);
        temp = DSMatrixByMultiplyingMatrix(Rn_internal, D);
        coefficients = DSMatrixAlloc(1, DSMatrixRows(matrix)+1);
        DSMatrixSetDoubleValue(coefficients, 0, 0, 1.);
        for (i = 0; i < DSMatrixRows(temp); i++) {
                DSMatrixSetDoubleValue(coefficients, 0, i+1,DSMatrixDoubleValue(temp, i, 0));
        }
        DSMatrixSetDoubleValue(coefficients, 0, i+1, DSMatrixDeterminant(mA));
        DSMatrixFree(temp);
        DSMatrixFree(D);
        DSMatrixFree(mA);
bail:
        if (mustDealloc == true)
                DSMatrixFree(Rn_internal);
        return coefficients;
}

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Matrix GLPK conversions
#endif

extern double * DSMatrixDataForGLPK(const DSMatrix *matrix)
{
        double *data = NULL;
        DSUInteger i, j;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        data = calloc(sizeof(double),DSMatrixRows(matrix)*DSMatrixColumns(matrix)+1);
        for (i = 0; i < DSMatrixRows(matrix); i++) {
                for (j = 0; j < DSMatrixColumns(matrix); j++) {
                        data[1+i*DSMatrixColumns(matrix)+j] = DSMatrixDoubleValue(matrix, i, j);
                }
        }
bail:
        return data;
}

extern int * DSMatrixRowsForGLPK(const DSMatrix *matrix)
{
        int *rows = NULL;
        DSUInteger i;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        rows = calloc(sizeof(int), DSMatrixRows(matrix)*DSMatrixColumns(matrix)+1);
        for (i = 1; i <= DSMatrixRows(matrix)*DSMatrixColumns(matrix); i++) {
                rows[i] = ((i-1) / DSMatrixColumns(matrix))+1;
        }
bail:
        return rows;
}

extern int * DSMatrixColumnsForGLPK(const DSMatrix *matrix)
{
        int *columns = NULL;
        DSUInteger i;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        columns = calloc(sizeof(int), DSMatrixRows(matrix)*DSMatrixColumns(matrix)+1);
        for (i = 1; i <= DSMatrixRows(matrix)*DSMatrixColumns(matrix); i++) {
                columns[i] = ((i-1) % DSMatrixColumns(matrix))+1;
        }
bail:
        return columns;
}

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Data Serialization
#endif


extern DSMatrixMessage * DSMatrixEncode(const DSMatrix * matrix)
{
        DSMatrixMessage * message = NULL;
        DSUInteger i;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        message = DSSecureMalloc(sizeof(DSMatrixMessage)*1);
        dsmatrix_message__init(message);
        message->rows = DSMatrixRows(matrix);
        message->columns = DSMatrixColumns(matrix);
        message->n_values = message->rows*message->columns;
        message->values = DSSecureMalloc(sizeof(double)*message->n_values);
        for (i = 0; i < message->n_values; i++) {
                message->values[i] = DSMatrixDoubleValue(matrix,
                                                        i / DSMatrixColumns(matrix),
                                                        i % DSMatrixColumns(matrix));
        }
bail:
        return message;
}

extern DSMatrix * DSMatrixFromMatrixMessage(const DSMatrixMessage * message)
{
        DSMatrix * matrix = NULL;
        DSUInteger i, j;
        if (message == NULL) {
                printf("message is NULL\n");
                goto bail;
        }
        matrix = DSMatrixAlloc(message->rows, message->columns);
        for (i = 0; i < DSMatrixRows(matrix); i++) {
                for (j = 0; j < DSMatrixColumns(matrix); j++) {
                        DSMatrixSetDoubleValue(matrix, i, j, message->values[i*DSMatrixColumns(matrix)+j]);
                }
        }
bail:
        return matrix;
}

extern DSMatrix * DSMatrixDecode(size_t length, const void * buffer)
{
        DSMatrix * matrix = NULL;
        DSMatrixMessage * message;
        message = dsmatrix_message__unpack(NULL, length, buffer);
        matrix = DSMatrixFromMatrixMessage(message);
        dsmatrix_message__free_unpacked(message, NULL);
bail:
        return matrix;
}