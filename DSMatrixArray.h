/**
 * \file DSMatrixArray.h
 * \brief Header file with functions for dealing with matrix arrays.
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

#include "DSTypes.h"
#include "DSMatrix.h"

#ifndef __DS_MATRIX_ARRAY__
#define __DS_MATRIX_ARRAY__

#ifdef __cplusplus
__BEGIN_DECLS
#endif

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, Free and Initialization functions
#endif

extern DSMatrixArray * DSMatrixArrayAlloc(void);
extern DSMatrixArray * DSMatrixArrayCopy(const DSMatrixArray * array);
extern void DSMatrixArrayFree(DSMatrixArray *array);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Factory functions
#endif

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Accesor functions
#endif

extern DSUInteger DSMatrixArrayNumberOfMatrices(const DSMatrixArray * matrixArray);

/**
 * \brief Accessor function to retrieve the pointer to the C matrix array.
 */
#define DSMatrixArrayInternalPointer(x)     ((x)->matrices)

extern DSMatrix * DSMatrixArrayMatrix(const DSMatrixArray *array, const DSUInteger index);
extern void DSMatrixArrayAddMatrix(DSMatrixArray *array, const DSMatrix *matrixToAdd);

#endif

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

extern double DSMatrixArrayDoubleWithIndices(const DSMatrixArray *array, const DSUInteger i, const DSUInteger j, const DSUInteger k);
extern void DSMatrixArrayPrint(const DSMatrixArray * array);

#ifdef __cplusplus
__END_DECLS
#endif

