/**
 * \file DSExpression.h
 * \brief Header file with functions for dealing with mathematical expressions.
 *
 * \details The mathematical expressions are converted into a form similar to 
 * the model used in MUPAD.  Internally, only three operators are used: '+', '*'
 * and '^'.  The '-' operator is converted, such that \$A-B\$ would actually
 * be \$A+B*(-1)\$ and the '/' operator is converted such that \$A/B\$ would
 * actually be \$A*B^-1\$.  The '*' and '+' operators must have at least two
 * branches, but may have any number of branches. The first branch for these
 * operators is reserved for constant values, such that a+b is actually 0+a+b,
 * and a*b is actually 1*a*b.  This canonical form is used to speed up the 
 * processing of mathematical expressions when converting them to matrices for
 * the GMA and SSystem.  The '^' must have only two branches.
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

#ifndef __DS_EXPRESSION__
#define __DS_EXPRESSION__

#define DS_EXPRESSION_TYPE_UNDEFINED        0
#define DS_EXPRESSION_TYPE_OPERATOR         1
#define DS_EXPRESSION_TYPE_CONSTANT         2
#define DS_EXPRESSION_TYPE_VARIABLE         3


#define DSExpressionSetOperator(x, y)       ((x->node.op_code) = y, (x->type = DS_EXPRESSION_TYPE_OPERATOR))
#define DSExpressionSetVariable(x, y)       ((x->node.variable) = y, (x->type = DS_EXPRESSION_TYPE_VARIABLE))
#define DSExpressionSetConstant(x, y)       ((x->node.constant) = y, (x->type = DS_EXPRESSION_TYPE_CONSTANT))

#define DSExpressionType(x)                 (x->type)
#define DSExpressionNumberOfBranches(x)     (x->numberOfBranches)
#define DSExpressionBranchAtIndex(x, y)     ((y < DSExpressionNumberOfBranches(x)) ? x->branches[y] : NULL)
#define DSExpressionOperator(x)             ((x->type == DS_EXPRESSION_TYPE_OPERATOR) ? x->node.op_code : '?')
#define DSExpressionVariable(x)             ((x->type == DS_EXPRESSION_TYPE_VARIABLE) ? x->node.variable : NULL)
#define DSExpressionConstant(x)             ((x->type == DS_EXPRESSION_TYPE_CONSTANT) ? x->node.constant : NAN)


#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, Free and Initialization functions
#endif

extern DSExpression * DSExpressionAllocWithOperator(const char op_code);
extern DSExpression * DSExpressionAllocWithConstant(const double value);
extern DSExpression * DSExpressionAllocWithVariableName(const char * name);
extern void DSExpressionFree(DSExpression *root);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Factory functions
#endif

extern DSExpression * DSExpressionByParsingString(const char *string);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Expression properties
#endif

/** These functions might be removed **/
extern DSUInteger DSExpressionNumberOfTerms(const DSExpression *expression);
extern DSUInteger DSExpressionNumberOfPositiveTerms(const DSExpression *expression);
extern DSUInteger DSExpressionNumberOfNegativeTerms(const DSExpression *expression);

extern const DSExpression * DSExpressionTermAtIndex(const DSExpression * expression);
extern const DSExpression * DSExpressionPositiveTermAtIndex(const DSExpression * expression, const DSUInteger index);
extern const DSExpression * DSExpressionNegativeTermAtIndex(const DSExpression * expression, const DSUInteger index);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

extern char * DSExpressionAsString(const DSExpression *expression);
extern void DSExpressionPrint(const DSExpression *expression);



#endif
