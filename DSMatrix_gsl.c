/**
 * \file DSMatrix_gsl.c
 * \brief Implementation file with functions for dealing with matrices using the
 * GNU Scientific Library (gsl).
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

#include <time.h>
#include <stdarg.h>
#include <string.h>

#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>

#include "DSMatrix.h"
#include "DSErrors.h"
#include "DSMemoryManager.h"
#include "DSMatrixArray.h"
#include "DSMatrixTokenizer.h"

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
        aMatrix = DSSecureMalloc(sizeof(DSMatrix *));
        DSMatrixInternalPointer(aMatrix) = NULL;
        DSMatrixSetRows(aMatrix, rows);
        DSMatrixSetColumns(aMatrix, columns);
        DSMatrixInternalPointer(aMatrix) = gsl_matrix_alloc(rows, columns);
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
        aMatrix = DSSecureMalloc(sizeof(DSMatrix *));
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

extern DSMatrix * DSMatrixWithVariablePoolValues(const DSVariablePool *variablePool);



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
                        (rows < DSMatrixTokenRow(current) ?
                         rows = DSMatrixTokenRow(current) : 1);
                        (columns < DSMatrixTokenColumn(current) ?
                         columns = DSMatrixTokenColumn(current) : 1);
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
#pragma mark - Basic Accesor functions
#endif


extern void DSMatrixSetRows(DSMatrix * matrix, const DSUInteger rows)
{
        if (matrix == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixInternalPointer(matrix) != NULL) {
                DSError(M_DS_WRONG, A_DS_ERROR);
                goto bail;
        }
        DSMatrixRows(matrix) = rows;
bail:
        return;
}

extern void DSMatrixSetColumns(DSMatrix * matrix, const DSUInteger columns)
{
        if (matrix == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixInternalPointer(matrix) != NULL) {
                DSError(M_DS_WRONG, A_DS_ERROR);
                goto bail;
        }
        DSMatrixColumns(matrix) = columns;
bail:
        return;
}

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
        value = gsl_matrix_get(DSMatrixInternalPointer(matrix), row, column);
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
        char format[10];
        char data[100];
        if (matrix == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixInternalPointer(matrix) == NULL) {
                DSError(M_DS_MAT_NOINTERNAL, A_DS_WARN);
                goto bail;
        }
        sprintf(format, "%%.%dlf", figures);
        for (i = 0; i < DSMatrixRows(matrix); i++) {
                for (j = 0; j < DSMatrixColumns(matrix); j++) {
                        sprintf(data, format, DSMatrixDoubleValue(matrix, i, j));
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
                DSError("No rows being excluded", A_DS_WARN);
                submatrix = DSMatrixCopy(matrix);
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
                DSError("No rows being excluded", A_DS_WARN);
                submatrix = DSMatrixCopy(matrix);
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
                DSError("No rows being included", A_DS_WARN);
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
                DSError("No columns being included", A_DS_WARN);
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
        tempSubmatrix = DSMatrixSubMatrixExcludingColumns(matrix, numberOfColumns, columns);
        submatrix = DSMatrixSubMatrixExcludingRows(matrix, numberOfRows, rows);
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
        submatrix = DSMatrixSubMatrixIncludingRows(matrix, numberOfRows, rows);
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
                for (j = 0; j < DSMatrixColumns(matrix); j++) {
                        print("%lf%c", DSMatrixDoubleValue(matrix, i, j), 
                              ((j == DSMatrixColumns(matrix)-1) ? '\n' : '\t'));
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
        minValue = gsl_matrix_min(DSMatrixInternalPointer(matrix));
bail:
        return minValue;
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
        maxValue = gsl_matrix_max(DSMatrixInternalPointer(matrix));
bail:
        return maxValue;
}

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Matrix Operations
#endif

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark Arithmetic
#endif

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
        if (rvalue == NULL) {
                matrix = DSMatrixCopy(lvalue);
        } else if (DSMatrixRows(lvalue) != DSMatrixRows(rvalue)) {
                DSError("Matrix rows do not match", A_DS_ERROR);
        } else if (DSMatrixColumns(lvalue) != DSMatrixColumns(rvalue)) {
                DSError("Matrix columns do not match", A_DS_ERROR);
        } else {
                gsl_matrix_sub(DSMatrixInternalPointer(matrix),
                               DSMatrixInternalPointer(rvalue));
        }
bail:
        return matrix;
}

extern DSMatrix * DSMatrixByAddingMatrix(const DSMatrix *lvalue, const DSMatrix *rvalue)
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
        if (rvalue == NULL) {
                matrix = DSMatrixCopy(lvalue);
        } else if (DSMatrixRows(lvalue) != DSMatrixRows(rvalue)) {
                DSError("Matrix rows do not match", A_DS_ERROR);
        } else if (DSMatrixColumns(lvalue) != DSMatrixColumns(rvalue)) {
                DSError("Matrix columns do not match", A_DS_ERROR);
        } else {
                gsl_matrix_add(DSMatrixInternalPointer(matrix),
                               DSMatrixInternalPointer(rvalue));
        }
bail:
        return matrix;
}
extern DSMatrix * DSMatrixByDividingMatrix(const DSMatrix *lvalue, const DSMatrix *rvalue);  /** Multiplication by a matrix inverse or pseudoinverse**/

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
        matrix = DSMatrixAlloc(DSMatrixRows(lvalue), DSMatrixColumns(lvalue));
        gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, 
                       DSMatrixInternalPointer(lvalue), 
                       DSMatrixInternalPointer(rvalue), 0, DSMatrixInternalPointer(matrix));
bail:
        return matrix;
}
extern DSMatrix * DSMatrixByApplyingFunction(const DSMatrix *mvalue, double (*function)(double))
{
        DSMatrix * matrix = NULL;
        DSUInteger i, j;
        if (mvalue == NULL || function == NULL) {
                DSError(M_DS_NULL, A_DS_ERROR);
                goto bail;
        }
        matrix = DSMatrixCopy(mvalue);        
        for (i = 0; i < DSMatrixRows(matrix); i++) {
                for (j = 0; j < DSMatrixColumns(matrix); j++) {
                        DSMatrixSetDoubleValue(matrix,
                                               i, j,
                                               function(DSMatrixDoubleValue(matrix,
                                                                            i, j)));
                }
        }
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
        LU = gsl_matrix_alloc(DSMatrixRows(matrix), DSMatrixColumns(matrix));
        gsl_matrix_memcpy(LU, DSMatrixInternalPointer(matrix));
        gsl_linalg_LU_decomp(LU, p, &sign);
        determinant = gsl_linalg_LU_det(LU, sign);
        gsl_permutation_free(p);
        gsl_matrix_free(LU);
bail:
        return determinant;
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
        if (DSMatrixRank(matrix) != DSMatrixRows(matrix))
                goto bail;
        aMatrix = DSMatrixAlloc(DSMatrixRows(matrix), DSMatrixColumns(matrix));
        //        aMatrix = [[[self class] alloc] initWithMatrix:self];
        p = gsl_permutation_alloc(DSMatrixRows(matrix));
        LU = gsl_matrix_alloc(DSMatrixRows(matrix), DSMatrixColumns(matrix));
        gsl_matrix_memcpy(LU, DSMatrixInternalPointer(matrix));
        gsl_linalg_LU_decomp(LU, p, &sign);
        gsl_linalg_LU_invert(LU, p, DSMatrixInternalPointer(matrix));
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

extern DSMatrix * DSMatrixRightNullspace(const DSMatrix *matrix);

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
        LU = gsl_matrix_alloc(DSMatrixRows(A), DSMatrixColumns(A));
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
                                DSMatrixSetDoubleValue(PLU[1], i, j, gsl_matrix_get(LU, i, j));
                        else
                                DSMatrixSetDoubleValue(PLU[2], i, j, gsl_matrix_get(LU, i, j));
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

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Matrix GLPK conversions
#endif

extern double * DSMatrixDataForGLPK(const DSMatrix *matrix);
extern int * DSMatrixRowsForGLPK(const DSMatrix *matrix);
extern int * DSMatrixColumnsForGLPK(const DSMatrix *matrix);

//#endif

