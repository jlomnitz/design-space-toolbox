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
#define DS_EXPRESSION_TYPE_FUNCTION         4


#define DSExpressionSetOperator(x, y)       ((x->node.op_code) = y, (x->type = DS_EXPRESSION_TYPE_OPERATOR))
#define DSExpressionSetVariable(x, y)       ((x->node.variable) = y, (x->type = DS_EXPRESSION_TYPE_VARIABLE))
#define DSExpressionSetConstant(x, y)       ((x->node.constant) = y, (x->type = DS_EXPRESSION_TYPE_CONSTANT))

#define DSExpressionType(x)                 (x->type)
#define DSExpressionNumberOfBranches(x)     (x->numberOfBranches)
#define DSExpressionBranchAtIndex(x, y)     ((y < DSExpressionNumberOfBranches(x)) ? x->branches[y] : NULL)
#define DSExpressionOperator(x)             ((x->type == DS_EXPRESSION_TYPE_OPERATOR) ? x->node.op_code : '?')
#define DSExpressionVariable(x)             ((x->type == DS_EXPRESSION_TYPE_VARIABLE || x->type == DS_EXPRESSION_TYPE_FUNCTION) ? x->node.variable : NULL)
#define DSExpressionConstant(x)             ((x->type == DS_EXPRESSION_TYPE_CONSTANT) ? x->node.constant : NAN)


#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, Free and Initialization functions
#endif

extern void DSExpressionFree(DSExpression *root);

extern DSExpression * DSExpressionCopy(const DSExpression * expression);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Factory functions
#endif

extern DSExpression * DSExpressionByParsingString(const char *string);
extern DSExpression * DSExpressionAddExpressions(DSExpression *lvalue, DSExpression *rvalue);
extern DSExpression * DSExpressionByCompressingConstantVariables(const DSExpression *expression, const DSVariablePool * assumedConstant);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Expression properties
#endif


#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

extern double DSExpressionEvaluateWithVariablePool(const DSExpression *expression, const DSVariablePool *pool);
extern DSExpression * DSExpressionEquationLHSExpression(const DSExpression *expression);
extern DSExpression * DSExpressionEquationRHSExpression(const DSExpression *expression);
extern DSVariablePool * DSExpressionVariablesInExpression(const DSExpression * expression);

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

extern char * DSExpressionAsString(const DSExpression *expression);
extern char * DSExpressionAsTroffString(const DSExpression *expression);
extern void DSExpressionPrint(const DSExpression *expression);



#endif
