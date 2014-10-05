/**
 * \file DSMatrix.h
 * \brief Header file with functions for dealing with matrices.
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

#include "DSTypes.h"
#include "DSDataSerialization.pb-c.h"


#ifndef __DS_MATRIX__
#define __DS_MATRIX__


/**
 *\addtogroup M_DS_Messages
 * Messages for DSMatrix related errors are M_DS_MAT_NULL, M_DS_MAT_OUTOFBOUNDS
 * and M_DS_MAT_NOINTERNAL.
 */
/*\{*/
#define M_DS_MAT_NULL             "Pointer to matrix is NULL"   //!< Message for a NULL DSMatrix pointer.
#define M_DS_MAT_OUTOFBOUNDS      "Row or column out of bounds" //!< Message for a row or column exceeding matrix bounds.
#define M_DS_MAT_NOINTERNAL       "Matrix data is empty"        //!< Message for a NULL internal matrix structure.
/*\}*/

#define DSMatrixRows(x)             ((x)->rows)
#define DSMatrixColumns(x)          ((x)->columns)

#define DSMatrixInternalPointer(x)  ((x)->mat)

#ifdef __cplusplus
__BEGIN_DECLS
#endif

//enum {
//        __MAT_GSL__,
//        __MAT_CLAPACK__
//};
//
//#ifdef __GSL__
//#define __MATRIX_BACK__ __MAT_GSL__
//#elif defined(__LAPACK__)
//#define __MATRIX_BACK__
//#undef __MATRIX_BACK__
//#endif


#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, Free and Initialization functions
#endif

extern DSMatrix * DSMatrixAlloc(const DSUInteger rows, const DSUInteger columns);
extern DSMatrix * DSMatrixCalloc(const DSUInteger rows, const DSUInteger columns);
extern DSMatrix * DSMatrixCopy(const DSMatrix *original);
extern void DSMatrixFree(DSMatrix *matrix);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Factory functions
#endif

extern DSMatrix * DSMatrixIdentity(const DSUInteger size);
extern DSMatrix * DSMatrixRandomNumbers(const DSUInteger rows, const DSUInteger columns);
extern DSMatrix * DSMatrixByParsingString(const char * string);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark Arithmetic (factory)
#endif

extern DSMatrix * DSMatrixBySubstractingMatrix(const DSMatrix *lvalue, const DSMatrix *rvalue);
extern DSMatrix * DSMatrixByAddingMatrix(const DSMatrix *lvalue, const DSMatrix *rvalue);
extern DSMatrix * DSMatrixByDividingMatrix(const DSMatrix *lvalue, const DSMatrix *rvalue);
extern DSMatrix * DSMatrixByMultiplyingMatrix(const DSMatrix *lvalue, const DSMatrix *rvalue);
extern DSMatrix * DSMatrixByApplyingFunction(const DSMatrix *mvalue, double (*function)(double));

extern DSMatrix * DSMatrixBySubstractingScalar(const DSMatrix *lvalue, const double rvalue);
extern DSMatrix * DSMatrixByAddingScalar(const DSMatrix *lvalue, const double rvalue);
extern DSMatrix * DSMatrixByDividingScalar(const DSMatrix *lvalue, const double rvalue);
extern DSMatrix * DSMatrixByMultiplyingScalar(const DSMatrix *lvalue, const double rvalue);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Basic Accesor functions
#endif

extern double DSMatrixDoubleValue(const DSMatrix *matrix, const DSUInteger row, const DSUInteger column);
extern void DSMatrixSetDoubleValue(DSMatrix *matrix, const DSUInteger row, const DSUInteger column, const double value);
extern void DSMatrixSetDoubleValueAll(DSMatrix *matrix, const double value);
extern void DSMatrixSetDoubleValuesList(DSMatrix *matrix, bool byColumns, DSUInteger numberOfValues, double firstValue, ...);
extern void DSMatrixSetDoubleValues(DSMatrix *matrix, bool byColumns, DSUInteger numberOfValues, double * values);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

extern void DSMatrixRoundToSignificantFigures(DSMatrix *matrix, const unsigned char figures);
extern DSMatrix * DSMatrixSubMatrixExcludingColumnList(const DSMatrix *matrix, 
                                                    const DSUInteger numberOfColumns, 
                                                    const DSUInteger firstColumn, ...);
extern DSMatrix * DSMatrixSubMatrixExcludingColumns(const DSMatrix *matrix,
                                                    const DSUInteger numberOfColumns, 
                                                    const DSUInteger *columns);
extern DSMatrix * DSMatrixSubMatrixExcludingRowList(const DSMatrix *matrix, 
                                                    const DSUInteger numberOfRows, 
                                                    const DSUInteger firstRow, ...);
extern DSMatrix * DSMatrixSubMatrixExcludingRows(const DSMatrix *matrix,
                                                 const DSUInteger numberOfRows,
                                                 const DSUInteger *rows);
extern DSMatrix * DSMatrixSubMatrixIncludingRowList(const DSMatrix *matrix, const DSUInteger numberOfRows, const DSUInteger firstRow, ...);
extern DSMatrix * DSMatrixSubMatrixIncludingRows(const DSMatrix *matrix, const DSUInteger numberOfRows,
                                                 const DSUInteger * rows);
extern DSMatrix * DSMatrixSubMatrixIncludingColumnList(const DSMatrix *matrix, const DSUInteger numberOfColumns, const DSUInteger firstColumn, ...);
extern DSMatrix * DSMatrixSubMatrixExcludingRowAndColumnList(const DSMatrix *matrix,
                                                             const DSUInteger numberOfRows,
                                                             const DSUInteger numberOfColumns,
                                                             const DSUInteger firstRow, ...);
extern DSMatrix * DSMatrixSubMatrixExcludingRowsAndColumns(const DSMatrix *matrix,
                                                           const DSUInteger numberOfRows,
                                                           const DSUInteger numberOfColumns,
                                                           const DSUInteger *rows,
                                                           const DSUInteger *columns);
extern DSMatrix * DSMatrixSubMatrixIncludingColumns(const DSMatrix *matrix, 
                                                    const DSUInteger numberOfColumns, 
                                                    const DSUInteger *columns);
extern DSMatrix * DSMatrixSubMatrixIncludingRowAndColumnList(const DSMatrix *matrix,
                                                             const DSUInteger numberOfRows,
                                                             const DSUInteger numberOfColumns,
                                                             const DSUInteger firstRow, ...);
extern DSMatrix * DSMatrixSubMatrixIncludingRowsAndColumns(const DSMatrix *matrix,
                                                           const DSUInteger numberOfRows,
                                                           const DSUInteger numberOfColumns,
                                                           const DSUInteger *rows,
                                                           const DSUInteger *columns);
extern DSMatrix * DSMatrixAppendMatrices(const DSMatrix *firstMatrix, 
                                         const DSMatrix *secondMatrix,
                                         const bool byColumn);
extern void DSMatrixSwitchRows(DSMatrix *matrix, const DSUInteger rowA, const DSUInteger rowB);
extern void DSMatrixSwitchColumns(DSMatrix *matrix, const DSUInteger columnA, const DSUInteger columnB);
extern void DSMatrixClearRow(DSMatrix *matrix, const DSUInteger row);
extern void DSMatrixClearColumns(DSMatrix *matrix, const DSUInteger column);

extern DSMatrix * DSMatrixWithUniqueRows(const DSMatrix *matrix);

extern void DSMatrixPrint(const DSMatrix *matrix);


#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Matrix Property Querying
#endif

extern bool DSMatrixIsIdentity(const DSMatrix *matrix);
extern bool DSMatrixIsSquare(const DSMatrix *matrix);
extern DSUInteger DSMatrixRank(const DSMatrix *matrix);
extern double minimumValue(const DSMatrix *matrix, const bool shouldExcludeZero);
extern double maximumValue(const DSMatrix *matrix, const bool shouldExcludeZero);
extern complex double DSMatrixDominantEigenvalue(const DSMatrix *matrix);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Matrix Operations
#endif

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark Arithmetic
#endif

extern void DSMatrixSubstractByMatrix(DSMatrix *addTo, const DSMatrix *addBy);
extern void DSMatrixAddByMatrix(DSMatrix *addTo, const DSMatrix *addBy);

extern void DSMatrixApplyFunction(DSMatrix *matrix, double (*function)(double));

extern void DSMatrixMultiplyByScalar(DSMatrix *matrix, const double value);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark Linear Algebra
#endif

extern double DSMatrixDeterminant(const DSMatrix *matrix); 
extern double DSMatrixMinor(const DSMatrix *matrix,
                            const DSUInteger row,
                            const DSUInteger column);


extern DSMatrix * DSMatrixTranspose(const DSMatrix *matrix);
extern DSMatrix * DSMatrixInverse(const DSMatrix *matrix);
extern DSMatrixArray * DSMatrixSVD(const DSMatrix *matrix);
extern DSMatrixArray * DSMatrixQRD(const DSMatrix *matrix);
extern DSMatrix * DSMatrixRightNullspace(const DSMatrix *matrix);
extern DSMatrix * DSMatrixLeftNullspace(const DSMatrix *matrix);
extern DSMatrix * DSMatrixIdenticalRows(const DSMatrix * matrix);

extern DSMatrixArray * DSMatrixPLUDecomposition(const DSMatrix *matrix);

extern DSMatrix * DSMatrixCharacteristicPolynomialCoefficients(const DSMatrix * matrix);

extern DSMatrix * DSMatrixUndeterminedCoefficientsRnMatrixForSize(const DSUInteger matrixSize);
extern DSMatrix * DSMatrixCharacteristicPolynomialUndeterminedCoefficients(const DSMatrix * matrix, const DSMatrix * Rn);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Matrix GLPK conversions
#endif

extern double * DSMatrixDataForGLPK(const DSMatrix *matrix);
extern int * DSMatrixRowsForGLPK(const DSMatrix *matrix);
extern int * DSMatrixColumnsForGLPK(const DSMatrix *matrix);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Matrix GLPK conversions
#endif

extern DSMatrixMessage * DSMatrixEncode(const DSMatrix * matrix);
extern DSMatrix * DSMatrixDecode(size_t length, const void *);


#endif

#ifdef __cplusplus
__END_DECLS
#endif
