/**
 * \file DSMatrix.h
 * \brief Header file with functions for dealing with matrices.
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

#include "DSTypes.h"

#ifndef __DS_MATRIX__
#define __DS_MATRIX__




#define M_DS_MAT_OUTOFBOUNDS      "Row or column out of bounds"
#define M_DS_MAT_NOINTERNAL       "Matrix data is empty"

#define DSMatrixRows(x)             ((x)->rows)
#define DSMatrixColumns(x)          ((x)->columns)

#define DSMatrixInternalPointer(x)  ((x)->mat)

#ifdef __cplusplus
__BEGIN_DECLS
#endif

enum {
        __MAT_GSL__,
        __MAT_LAPACK__
};

#ifdef __GSL__
#define __MATRIX_BACK__ __MAT_GSL__
#elif defined(__LAPACK__)                /** Should provide support for other numerical libraries  **/
#define __MATRIX_BACK__
#undef __MATRIX_BACK__    /** LAPACK not currently supported **/
#endif


#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, Free and Initialization functions
#endif

extern DSMatrix * DSMatrixAlloc(DSUInteger rows, DSUInteger columns);
extern DSMatrix * DSMatrixCopy(DSMatrix *original);
extern void DSMatrixInitializeWithValue(DSMatrix *matrix, double value);
extern void DSMatrixFree(DSMatrix *matrix);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Factory functions
#endif

extern DSMatrix * DSMatrixIdentity(DSUInteger size);
extern DSMatrix * DSMatrixRandomNumbers(DSUInteger rows, DSUInteger columns);
extern DSMatrix * DSMatrixWithVariablePoolValues(void *variablePool);



#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Basic Accesor functions
#endif

extern double DSMatrixDoubleValue(DSMatrix *matrix, DSUInteger row, DSUInteger column);

extern void DSMatrixSetRows(DSMatrix * matrix, DSUInteger rows);
extern void DSMatrixSetColumns(DSMatrix * matrix, DSUInteger columns);
extern void DSMatrixSetDoubleValue(DSMatrix *matrix, DSUInteger row, DSUInteger column, double value);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

extern void DSMatrixRoundToSignificantFigures(DSMatrix *matrix, unsigned char figures);

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
extern DSMatrix * DSMatrixAppendMatrices(DSMatrix *firstMatrix, 
                                         DSMatrix *secondMatrix,
                                         bool byColumn);
extern void DSMatrixSwitchRows(DSMatrix *matrix, DSUInteger rowA, DSUInteger rowB);
extern void DSMatrixSwitchColumns(DSMatrix *matrix, DSUInteger columnA, DSUInteger columnB);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Matrix Property Querying
#endif

extern bool DSMatrixIsIdentity(DSMatrix *matrix);
extern bool DSMatrixIsSquare(DSMatrix *matrix);
extern DSUInteger DSMatrixRank(DSMatrix *matrix);
extern double minimumValue(DSMatrix *matrix, bool shouldExcludeZero);
extern double maximumValue(DSMatrix *matrix, bool shouldExcludeZero);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Matrix Operations
#endif

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark Arithmetic
#endif

extern DSMatrix * DSMatrixBySubstractingMatrix(DSMatrix *lvalue, DSMatrix *rvalue);
extern DSMatrix * DSMatrixByAddingMatrix(DSMatrix *lvalue, DSMatrix *rvalue);
extern DSMatrix * DSMatrixByDividingMatrix(DSMatrix *lvalue, DSMatrix *rvalue);
extern DSMatrix * DSMatrixByMultiplyingMatrix(DSMatrix *lvalue, DSMatrix *rvalue);
extern DSMatrix * DSMatrixByApplyingFunction(DSMatrix *matrix, double (*function)(double));

extern DSMatrix * DSMatrixBySubstractingScalar(DSMatrix *lvalue, double rvalue);
extern DSMatrix * DSMatrixByAddingScalar(DSMatrix *lvalue, double rvalue);
extern DSMatrix * DSMatrixByDividingScalar(DSMatrix *lvalue, double rvalue);
extern DSMatrix * DSMatrixByMultiplyingScalar(DSMatrix *lvalue, double rvalue);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark Linear Algebra
#endif

extern double DSMatrixDeterminant(DSMatrix *matrix); 

extern DSMatrix * DSMatrixTranspose(DSMatrix *matrix);
extern DSMatrix * DSMatrixInverse(DSMatrix *matrix);
extern DSMatrix * DSMatrixRightNullspace(DSMatrix *matrix);

extern DSMatrix ** DSMatrixPLUDecomposition(DSMatrix *matrix);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Matrix GLPK conversions
#endif

extern double * DSMatrixDataForGLPK(DSMatrix *matrix);
extern int * DSMatrixRowsForGLPK(DSMatrix *matrix);
extern int * DSMatrixColumnsForGLPK(DSMatrix *matrix);

#endif


/*

 
 To be implemented!
 -(double)minorWithRow:(NSUInteger)aRow andColumn:(NSUInteger)aCol;
 -(id)adjoint;
 -(id)cofactorMatrix;
 
 */

#ifdef __cplusplus
__END_DECLS
#endif
