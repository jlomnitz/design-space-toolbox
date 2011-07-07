/**
 * \file DSMatrix.c
 * \brief Implementation file with functions for dealing with matrices using the
 * GNU Scientific Library (gsl).
 *
 * \details 
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

#include <time.h>
#include <gsl/gsl_linalg.h>
#include "DSMatrix.h"
#include "DSErrors.h"

#if defined(__MATRIX_BACK__) && __MATRIX_BACK__ == __MAT_GSL__


/**
 * \brief Memory allocation for DSMatrix data.
 *
 * \details Creates a new matrix of a particular size.  The matrix that is
 * allocated has all the values of the matrix defaulted to 0.  
 */
extern DSMatrix * DSMatrixAlloc(DSUInteger rows, DSUInteger columns)
{
        DSMatrix *aMatrix = NULL;
        if (rows == 0 || columns == 0) {
                DSError(M_DS_WRONG, A_DS_WARN);
                goto bail;
        }
        aMatrix = malloc(sizeof(DSMatrix *));
        DSMatrixSetRows(aMatrix, rows);
        DSMatrixSetColumns(aMatrix, columns);
        DSMatrixInternalPointer(aMatrix) = gsl_matrix_calloc(rows, columns);
        if (DSMatrixInternalPointer(aMatrix) == NULL)
                DSError(M_DS_MALLOC, A_DS_KILLNOW);
bail:
        return aMatrix;
}

extern void DSMatrixInitializeWithValue(DSMatrix *matrix, double value)
{
        DSUInteger i, j;
        if (matrix == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixInternalPointer(matrix) == NULL) {
                DSError(M_DS_MAT_NOINTERNAL, A_DS_WARN);
                goto bail;
        }
        for (i = 0; i < DSMatrixRows(matrix); i++) {
                for (j = 0; j < DSMatrixColumns(matrix); j++) {
                        DSMatrixSetDoubleValue(matrix, i, j, value);
                }
        }
bail:
        return;
}

extern void DSMatrixFree(DSMatrix *matrix)
{
        if (matrix == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        if (DSMatrixInternalPointer(matrix) != NULL)
                gsl_matrix_free(DSMatrixInternalPointer(matrix));
        free(matrix);
bail:
        return;
}


extern DSMatrix * DSMatrixIdentity(DSUInteger size)
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

extern DSMatrix * DSMatrixRandomNumbers(DSUInteger rows, DSUInteger columns)
{
        DSMatrix * matrix = NULL;
        int i, j;
        matrix = DSMatrixAlloc(rows, columns);
        srand((unsigned int)time(NULL)+rand());
        for (i = 0; i < rows; i++) {
                for (j = 0; j < columns; j++) {
                        DSMatrixSetDoubleValue(matrix, i, j, rand()/RAND_MAX);
                }
        }
bail:
        return matrix;
}

extern DSMatrix * DSMatrixCopy(DSMatrix *original)
{
        DSMatrix * matrix = NULL;
        DSUInteger i, j;
        double value;
        if (original == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        matrix = DSMatrixAlloc(DSMatrixRows(original), DSMatrixColumns(original));
        for (i = 0; i < DSMatrixRows(original); i++) {
                for (j = 0; j < DSMatrixColumns(original); j++) {
                        value = DSMatrixDoubleValue(matrix, i, j);
                        DSMatrixSetDoubleValue(matrix, i, j, value);
                }
        }
bail:
        return matrix;
}

extern DSMatrix * DSMatrixWithVariablePoolValues(void *variablePool);

extern double DSMatrixDoubleValue(DSMatrix *matrix, DSUInteger row, DSUInteger column)
{
        double value = 0;
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

extern void DSMatrixSetDoubleValue(DSMatrix *matrix, DSUInteger row, DSUInteger column, double value)
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

extern void DSMatrixRoundToSignificantFigures(DSMatrix *matrix, unsigned char figures)
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

extern double * DSMatrixDataForGLPK(DSMatrix *matrix);
extern int * DSMatrixRowsForGLPK(DSMatrix *matrix);
extern int * DSMatrixColumnsForGLPK(DSMatrix *matrix);


extern DSMatrix * DSMatrixSubMatrixIncludingRows(DSUInteger numberOfRows,
                                                 DSUInteger firstRow,
                                                 ...);
extern DSMatrix * DSMatrixSubMatrixIncludingColumns(DSUInteger numberOfCoumns, 
                                                    DSUInteger firstColumn, 
                                                    ...);
extern DSMatrix * DSMatrixSubMatrixIncludingRowsAndColumns(DSUInteger numberRows,
                                                           DSUInteger numberColumns,
                                                           DSUInteger firstRow,
                                                           ...);

extern DSMatrix * DSMatrixBySubstractingMatrix(DSMatrix *lvalue, DSMatrix *rvalue);
extern DSMatrix * DSMatrixByAddingMatrix(DSMatrix *lvalue, DSMatrix *rvalue);
extern DSMatrix * DSMatrixByDividingMatrix(DSMatrix *lvalue, DSMatrix *rvalue);
extern DSMatrix * DSMatrixByMultiplyingMatrix(DSMatrix *lvalue, DSMatrix *rvalue);
extern DSMatrix * DSMatrixByApplyingFunction(DSMatrix *matrix, double (*function)(double));

extern DSMatrix * DSMatrixBySubstractingScalar(DSMatrix *lvalue, double rvalue);
extern DSMatrix * DSMatrixByAddingScalar(DSMatrix *lvalue, double rvalue);
extern DSMatrix * DSMatrixByDividingScalar(DSMatrix *lvalue, double rvalue);
extern DSMatrix * DSMatrixByMultiplyingScalar(DSMatrix *lvalue, double rvalue);

extern bool DSMatrixIsIdentity(DSMatrix *matrix);
extern bool DSMatrixIsSquare(DSMatrix *matrix);
extern DSUInteger DSMatrixRank(DSMatrix *matrix);
extern double minimumValue(DSMatrix *matrix, bool shouldExcludeZero);
extern double maximumValue(DSMatrix *matrix, bool shouldExcludeZero);
extern double DSMatrixDeterminant(DSMatrix *matrix);

extern DSMatrix * DSMatrixAppendMatrices(DSMatrix *firstMatrix, 
                                         DSMatrix *secondMatrix,
                                         bool byColumn);
extern void DSMatrixSwitchRows(DSMatrix *matrix, DSUInteger rowA, DSUInteger rowB);
extern void DSMatrixSwitchColumns(DSMatrix *matrix, DSUInteger columnA, DSUInteger columnB); 

extern DSMatrix * DSMatrixTranspose(DSMatrix *matrix);
extern DSMatrix * DSMatrixInverse(DSMatrix *matrix);
extern DSMatrix * DSMatrixRightNullspace(DSMatrix *matrix);
extern DSMatrix ** DSMatrixPLUDecomposition(DSMatrix *matrix);      

#endif

