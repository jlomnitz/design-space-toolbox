//
//  DSSymbolicMatrix.c
//  DesignSpaceToolboxV2
//
//  Created by Jason Lomnitz on 10/9/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include "DSMemoryManager.h"
#include "DSExpression.h"
#include "DSSymbolicMatrix.h"
#include "DSMatrix.h"

#define DSSymbolicMatrixInternalPointer(x)      ((x)->mat)
#define DSSymbolicMatrixInternalRows(x)         ((x)->rows)
#define DSSymbolicMatrixInternalColumns(x)      ((x)->columns)


extern DSSymbolicMatrix * DSSymbolicMatrixAlloc(const DSUInteger rows, const DSUInteger columns)
{
        DSSymbolicMatrix * matrix = NULL;
        DSUInteger i;
        if (rows == 0) {
                DSError(M_DS_WRONG ": Number of rows must be greater than 0", A_DS_ERROR);
                goto bail;
        }
        if (columns == 0) {
                DSError(M_DS_WRONG ": Number of columns must be greater than 0", A_DS_ERROR);
                goto bail;
        }
        matrix = DSSecureCalloc(sizeof(DSSymbolicMatrix), 1);
        DSSymbolicMatrixInternalRows(matrix) = rows;
        DSSymbolicMatrixInternalColumns(matrix) = columns;
        DSSymbolicMatrixInternalPointer(matrix) = DSSecureMalloc(sizeof(DSExpression **)*rows);
        for (i=0; i < DSSymbolicMatrixInternalRows(matrix); i++)
                DSSymbolicMatrixInternalPointer(matrix)[i] = DSSecureCalloc(sizeof(DSExpression *), columns);
        
bail:
        return matrix;
}

extern DSSymbolicMatrix * DSSymbolicMatrixCalloc(const DSUInteger rows, const DSUInteger columns)
{
        return DSSymbolicMatrixAlloc(rows, columns);
}

extern DSSymbolicMatrix * DSSymbolicMatrixCopy(const DSSymbolicMatrix *original)
{
        DSSymbolicMatrix * new = NULL;
        DSUInteger i, j;
        if (original == NULL) {
                DSError(M_DS_SYM_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        new = DSSymbolicMatrixAlloc(DSSymbolicMatrixRows(original), DSSymbolicMatrixColumns(original));
        if (new == NULL)
                goto bail;
        for (i = 0; i < DSSymbolicMatrixInternalRows(new); i++)
                for (j = 0; j < DSSymbolicMatrixInternalColumns(new); j++)
                        DSSymbolicMatrixSetExpression(new, i, j, DSSymbolicMatrixExpression(original, i, j));
bail:
        return new;
}

extern void DSSymbolicMatrixFree(DSSymbolicMatrix *matrix);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Factory functions
#endif

extern DSSymbolicMatrix * DSSymbolicMatrixIdentity(const DSUInteger size)
{
        DSUInteger i;
        DSSymbolicMatrix * matrix = NULL;
        DSExpression * expr = NULL;
        if (size <= 0) {
                DSError(M_DS_WRONG ": Symbolic matrix dimensions are less than zero", A_DS_ERROR);
                goto bail;
        }
        expr = DSExpressionByParsingString("1");
        matrix = DSSymbolicMatrixAlloc(size, size);
        for (i = 0; i < size; i++)
                DSSymbolicMatrixSetExpression(matrix, i, i, expr);
        DSExpressionFree(expr);
bail:
        return matrix;
}

extern DSSymbolicMatrix * DSSymbolicMatrixRandomNumbers(const DSUInteger rows, const DSUInteger columns)
{
//        DSUInteger i, j;
        DSSymbolicMatrix * matrix = NULL;
//        DSExpression *expr;
        if (rows == 0 || columns == 0) {
                DSError(M_DS_WRONG ": Symbolic matrix dimensions are less than zero", A_DS_ERROR);
                goto bail;
        }
        DSError(M_DS_NOT_IMPL, A_DS_WARN);
        goto bail;
bail:
        return matrix;
}

extern DSSymbolicMatrix * DSSymbolicMatrixByParsingString(const char * string);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark Arithmetic (factory)
#endif

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Basic Accesor functions
#endif

extern double DSSymbolicMatrixDoubleByEvaluatingExpression(const DSSymbolicMatrix *matrix, const DSUInteger row, const DSUInteger column, const DSVariablePool *variableValues)
{
        DSExpression * expr = NULL;
        double value = NAN;
        if (matrix == NULL) {
                DSError(M_DS_SYM_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (row >= DSSymbolicMatrixRows(matrix)) {
                DSError(M_DS_SYM_MAT_OUTOFBOUNDS ": Row out of bounds", A_DS_ERROR);
                goto bail;
        }
        if (column >= DSSymbolicMatrixColumns(matrix)) {
                DSError(M_DS_SYM_MAT_OUTOFBOUNDS ": Column out of bounds", A_DS_ERROR);
                goto bail;
        }
        expr = DSSymbolicMatrixInternalPointer(matrix)[row][column];
        if (expr == NULL) {
                value = 0;
                goto bail;
        }
        value = DSExpressionEvaluateWithVariablePool(expr, variableValues);
bail:
        return value;
}

extern const DSExpression * DSSymbolicMatrixExpression(const DSSymbolicMatrix *matrix,  const DSUInteger row, const DSUInteger column)
{
        DSExpression * expr = NULL;
        if (matrix == NULL) {
                DSError(M_DS_SYM_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (row >= DSSymbolicMatrixRows(matrix)) {
                DSError(M_DS_SYM_MAT_OUTOFBOUNDS ": Row out of bounds", A_DS_ERROR);
                goto bail;
        }
        if (column >= DSSymbolicMatrixColumns(matrix)) {
                DSError(M_DS_SYM_MAT_OUTOFBOUNDS ": Column out of bounds", A_DS_ERROR);
                goto bail;
        }
        expr = DSSymbolicMatrixInternalPointer(matrix)[row][column];
bail:
        return expr;
}

extern void DSSymbolicMatrixSetExpression(DSSymbolicMatrix *matrix, const DSUInteger row, const DSUInteger column, const DSExpression *expr)
{
        DSExpression **matPosition = NULL;
        if (matrix == NULL) {
                DSError(M_DS_SYM_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        if (row >= DSSymbolicMatrixRows(matrix)) {
                DSError(M_DS_SYM_MAT_OUTOFBOUNDS ": Row out of bounds", A_DS_ERROR);
                goto bail;
        }
        if (column >= DSSymbolicMatrixColumns(matrix)) {
                DSError(M_DS_SYM_MAT_OUTOFBOUNDS ": Column out of bounds", A_DS_ERROR);
                goto bail;
        }
        matPosition = &(DSSymbolicMatrixInternalPointer(matrix)[row][column]);
        if (expr == NULL && *matPosition == NULL) {
                goto bail;
        }
        if (expr == NULL && *matPosition != NULL) {
                DSExpressionFree(*matPosition);
                *matPosition = NULL;
                goto bail;
        }
        if (*matPosition != NULL)
                DSExpressionFree(*matPosition);
        *matPosition = DSExpressionCopy(expr);
bail:
        return;
}

extern DSUInteger DSSymbolicMatrixRows(const DSSymbolicMatrix *matrix)
{
        DSUInteger rows = 0;
        if (matrix == NULL) {
                DSError(M_DS_SYM_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        rows = DSSymbolicMatrixInternalRows(matrix);
bail:
        return rows;
}

extern DSUInteger DSSymbolicMatrixColumns(const DSSymbolicMatrix *matrix)
{
        DSUInteger columns = 0;
        if (matrix == NULL) {
                DSError(M_DS_SYM_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        columns = DSSymbolicMatrixInternalRows(matrix);
bail:
        return columns;
}

extern DSMatrix * DSSymbolicMatrixToNumericalMatrix(const DSSymbolicMatrix *matrix, const DSVariablePool * variables)
{
        DSMatrix *numericalMatrix = NULL;
        DSUInteger i, j;
        if (matrix == NULL) {
                DSError(M_DS_SYM_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        numericalMatrix = DSMatrixAlloc(DSSymbolicMatrixRows(matrix), DSSymbolicMatrixColumns(matrix));
        if (numericalMatrix == NULL) {
                goto bail;
        }
        for (i = 0; i < DSSymbolicMatrixRows(matrix); i++) {
                for (j = 0; j < DSSymbolicMatrixColumns(matrix); j++) {
                        DSMatrixSetDoubleValue(numericalMatrix, i, j, 
                                               DSSymbolicMatrixDoubleByEvaluatingExpression(matrix, i, j, variables));
                }
        }
bail:
        return numericalMatrix;
}


