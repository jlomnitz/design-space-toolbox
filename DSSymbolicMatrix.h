/**
 * \file DSSymbolicMatrix.h
 * \brief Header file with functions for dealing with symbolic matrices.
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

#ifndef __DS_SYMBOLIC_MATRIX__
#define __DS_SYMBOLIC_MATRIX__

#include "DSTypes.h"
#include "DSErrors.h"
#include "DSIO.h"

/**
 *\addtogroup M_DS_Messages
 * Messages for DSMatrix related errors are M_DS_MAT_NULL, M_DS_MAT_OUTOFBOUNDS
 * and M_DS_MAT_NOINTERNAL.
 */
/*\{*/
#define M_DS_SYM_MAT_NULL             "Pointer to symbolic matrix is NULL"   //!< Message for a NULL DSMatrix pointer.
#define M_DS_SYM_MAT_OUTOFBOUNDS      "Row or column out of bounds" //!< Message for a row or column exceeding matrix bounds.
#define M_DS_SYM_MAT_NOINTERNAL       "Matrix data is empty"        //!< Message for a NULL internal matrix structure.
/*\}*/

#ifdef __cplusplus
__BEGIN_DECLS
#endif

extern DSSymbolicMatrix * DSSymbolicMatrixAlloc(const DSUInteger rows, const DSUInteger columns);
extern DSSymbolicMatrix * DSSymbolicMatrixCalloc(const DSUInteger rows, const DSUInteger columns);
extern DSSymbolicMatrix * DSSymbolicMatrixCopy(const DSSymbolicMatrix *original);
extern void DSSymbolicMatrixFree(DSSymbolicMatrix *matrix);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Factory functions
#endif

extern DSSymbolicMatrix * DSSymbolicMatrixIdentity(const DSUInteger size);
extern DSSymbolicMatrix * DSSymbolicMatrixRandomNumbers(const DSUInteger rows, const DSUInteger columns);
extern DSSymbolicMatrix * DSSymbolicMatrixByParsingString(const char * string);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark Arithmetic (factory)
#endif

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Basic Accesor functions
#endif

extern double DSSymbolicMatrixDoubleByEvaluatingExpression(const DSSymbolicMatrix *matrix, const DSUInteger row, const DSUInteger column, const DSVariablePool *variableValues);
extern const DSExpression * DSSymbolicMatrixExpression(const DSSymbolicMatrix *matrix,  const DSUInteger row, const DSUInteger column);
extern void DSSymbolicMatrixSetExpression(DSSymbolicMatrix *matrix, const DSUInteger row, const DSUInteger column, const DSExpression *expr);

extern DSUInteger DSSymbolicMatrixRows(const DSSymbolicMatrix *matrix);
extern DSUInteger DSSymbolicMatrixColumns(const DSSymbolicMatrix *matrix);

extern DSMatrix * DSSymbolicMatrixToNumericalMatrix(const DSSymbolicMatrix *matrix, const DSVariablePool * variables);

#ifdef __cplusplus
__END_DECLS
#endif

#endif
