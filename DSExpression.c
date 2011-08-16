/**
 * \file DSExpression.c
 * \brief Implementation file with functions for dealing with mathematical
 *        expressions.
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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "DSExpression.h"
#include "DSErrors.h"
#include "DSMemoryManager.h"
#include "DSExpressionTokenizer.h"

#define DS_EXPRESSION_CONSTANT_BRANCH     0
#define DS_EXPRESSION_STRING_INIT_LENGTH  1000

extern DSExpression * DSExpressionAllocWithOperator(const char op_code)
{
        DSExpression *newNode = NULL;
        switch (op_code) {
                case '+':
                        newNode = DSSecureCalloc(1, sizeof(DSExpression));
                        DSExpressionSetOperator(newNode, '+');
                        /* First branch reserved for constants */
                        newNode->branches = DSSecureMalloc(sizeof(DSExpression *)*(DSExpressionNumberOfBranches(newNode)+1));
                        newNode->branches[DSExpressionNumberOfBranches(newNode)] = DSExpressionAllocWithConstant(0.0);
                        newNode->numberOfBranches++;
                        break;
                case '-':
                        DSError(M_DS_WRONG ": DSExpression does not internally use '-' operators", A_DS_ERROR);
                        break;
                case '*':
                        newNode = DSSecureCalloc(1, sizeof(DSExpression));
                        DSExpressionSetOperator(newNode, '*');
                        /* First branch reserved for constants */
                        newNode->branches = DSSecureMalloc(sizeof(DSExpression *)*(DSExpressionNumberOfBranches(newNode)+1));
                        newNode->branches[DSExpressionNumberOfBranches(newNode)] = DSExpressionAllocWithConstant(1.0);
                        newNode->numberOfBranches++;
                        break;
                case '/':
                        DSError(M_DS_WRONG ": DSExpression does not internally use '/' operators", A_DS_ERROR);
                        break;
                case '^':
                        newNode = DSSecureCalloc(1, sizeof(DSExpression));
                        DSExpressionSetOperator(newNode, '^');
                        break;
                default:
                        break;
        }
        return  newNode;
}

extern DSExpression * DSExpressionAllocWithConstant(const double value)
{
        DSExpression *newNode = NULL;
        newNode = DSSecureCalloc(1, sizeof(DSExpression));
        DSExpressionSetConstant(newNode, value);
        return newNode;
}

extern DSExpression * DSExpressionAllocWithVariableName(const char * name)
{
        DSExpression *newNode = NULL;
        if (name == NULL) {
                DSError(M_DS_WRONG ": name of variable is NULL", A_DS_ERROR);
                goto bail;
        }
        if (strlen(name) == 0) {
                DSError(M_DS_WRONG ": name of variable is empty", A_DS_ERROR);
                goto bail;
        }
        newNode = DSSecureCalloc(1, sizeof(DSExpression));
        DSExpressionSetVariable(newNode, strdup(name));
bail:
        return newNode;       
}

extern void DSExpressionFree(DSExpression *root)
{
        DSUInteger i;
        if (root == NULL) {
                DSError(M_DS_NULL ": Expression to free is NULL", A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < DSExpressionNumberOfBranches(root); i++) {
                DSExpressionFree(DSExpressionBranchAtIndex(root, i));
        }
        if (root->branches != NULL)
                DSSecureFree(root->branches);
        switch (DSExpressionType(root)) {
                case DS_EXPRESSION_TYPE_VARIABLE:
                        DSSecureFree(DSExpressionVariable(root));
                        break;
                default:
                        break;
        }
        DSSecureFree(root);
bail:
        return;
}

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Factory functions
#endif

extern DSExpression * DSExpressionByParsingString(const char *string)
{
        DSExpression *root = NULL;
        void *parser = NULL;
        struct expression_token *tokens, *current;
        parse_expression_s parsed;
        if (string == NULL) {
                DSError(M_DS_WRONG ": String to parse is NULL", A_DS_ERROR);
                goto bail;
        }
        if (strlen(string) == 0) {
                DSError(M_DS_WRONG ": String to parse is empty", A_DS_WARN);
                goto bail;                
        }
        tokens = DSExpressionTokenizeString(string);
        if (tokens == NULL) {
                DSError(M_DS_PARSE ": Token stream is NULL", A_DS_ERROR);
                goto bail;
        }
        parser = DSExpressionParserAlloc(DSSecureMalloc);
        current = tokens;
        parsed.wasSuccesful = true;
        while (current != NULL) {
                if (DSExpressionTokenType(current) == DS_EXPRESSION_TOKEN_START) {
                        current = DSExpressionTokenNext(current);
                        continue;
                }
                DSExpressionParser(parser, 
                                   DSExpressionTokenType(current), 
                                   (DSExpression *)current,
                                   &parsed);
                current = DSExpressionTokenNext(current);
        }
        DSExpressionParser(parser, 
                           0, 
                           NULL,
                           &parsed);
        DSExpressionParserFree(parser, DSSecureFree);
        DSExpressionTokenFree(tokens);
        if (parsed.wasSuccesful == true)
                root = parsed.root;
        else
                DSExpressionFree(parsed.root);
bail:
        return root;  
}


#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Branch adding functions
#endif

static void DSExpressionAddNonConstantBranch(DSExpression *expression, DSExpression *branch)
{
        if (expression == NULL) {
                DSError(M_DS_NULL ": Expression root is NULL", A_DS_ERROR);
                goto bail;
        }
        if (branch == NULL) {
                DSError(M_DS_NULL ": Branch being added is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSExpressionType(expression) != DS_EXPRESSION_TYPE_OPERATOR) {
                DSError(M_DS_WRONG ": Expression root is not an operator", A_DS_ERROR);
                goto bail;
        }
        if (expression->branches == NULL)
                expression->branches = DSSecureMalloc(sizeof(DSExpression *)*(DSExpressionNumberOfBranches(expression)+1));
        else
                expression->branches = DSSecureRealloc(expression->branches, 
                                                       sizeof(DSExpression *)*(DSExpressionNumberOfBranches(expression)+1));
        expression->branches[DSExpressionNumberOfBranches(expression)] = branch;
        expression->numberOfBranches++;
bail:
        return;
}

static void DSExpressionAddConstantBranch(DSExpression *expression, DSExpression *branch)
{
        double constant;
        if (expression == NULL) {
                DSError(M_DS_NULL ": Expression root is NULL", A_DS_ERROR);
                goto bail;
        }
        if (branch == NULL) {
                DSError(M_DS_NULL ": Branch being added is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSExpressionType(branch) != DS_EXPRESSION_TYPE_CONSTANT) {
                DSError(M_DS_WRONG ": branch expression is not a constant", A_DS_ERROR);
                goto bail;
        }
        constant = DSExpressionConstant(branch);
        switch (DSExpressionOperator(expression)) {
                case '+':
                        DSExpressionSetConstant(DSExpressionBranchAtIndex(expression, 0),
                                                DSExpressionConstant(DSExpressionBranchAtIndex(expression, 0))+constant);
                        DSExpressionFree(branch);
                        break;
                case '*':
                        DSExpressionSetConstant(DSExpressionBranchAtIndex(expression, 0),
                                                DSExpressionConstant(DSExpressionBranchAtIndex(expression, 0))*constant);
                        DSExpressionFree(branch);
                        break;
                case '^':
                        DSExpressionAddNonConstantBranch(expression, branch);
                        break;
                default:
                        break;
        }
bail:
        return;
}

extern void DSExpressionAddBranch(DSExpression *expression, DSExpression *branch)
{
        DSUInteger i;
        char expressionOperator = '?';
        if (expression == NULL) {
                DSError(M_DS_NULL ": Expression root is NULL", A_DS_ERROR);
                goto bail;
        }
        if (branch == NULL) {
                DSError(M_DS_NULL ": Branch being added is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSExpressionType(expression) != DS_EXPRESSION_TYPE_OPERATOR) {
                DSError(M_DS_WRONG ": Adding branch to non-operator expression", A_DS_ERROR);
                goto bail;
        }
        if (DSExpressionType(branch) == DS_EXPRESSION_TYPE_UNDEFINED) {
                DSError(M_DS_WRONG ": branch expression type is undefined", A_DS_ERROR);
                goto bail;
        }
        if (DSExpressionType(branch)  == DS_EXPRESSION_TYPE_VARIABLE) {
                DSExpressionAddNonConstantBranch(expression, branch);
                goto bail;
        }
        if (DSExpressionType(branch) == DS_EXPRESSION_TYPE_CONSTANT) {
                DSExpressionAddConstantBranch(expression, branch);
                goto bail;
        }
        if (DSExpressionNumberOfBranches(branch) < 2) {
                DSError(M_DS_WRONG ": branch has insufficient branches", A_DS_ERROR);
                goto bail;
        }
        expressionOperator = DSExpressionOperator(branch);
        switch (DSExpressionOperator(expression)) {
                case '+':
                        if (expressionOperator == '+') {
                                for (i = 0; i < DSExpressionNumberOfBranches(branch); i++)
                                        DSExpressionAddBranch(expression, DSExpressionBranchAtIndex(branch, i));
                                branch->numberOfBranches = 0;
                                DSExpressionFree(branch);
                        } else {
                                DSExpressionAddNonConstantBranch(expression, branch);
                        }
                        break;
                case '*':
                        if (expressionOperator == '*') {
                                for (i = 0; i < DSExpressionNumberOfBranches(branch); i++)
                                        DSExpressionAddBranch(expression, DSExpressionBranchAtIndex(branch, i));
                                branch->numberOfBranches = 0;
                                DSExpressionFree(branch);
                        } else {
                                DSExpressionAddNonConstantBranch(expression, branch);
                        }
                        break;
                case '^':
                        DSExpressionAddNonConstantBranch(expression, branch);
                        break;
                default:
                        DSError(M_DS_WRONG ": Operator for expression root is undefined", A_DS_ERROR);
                        break;
        }
bail:
        return;
}

#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Expression properties
#endif

extern DSUInteger DSExpressionNumberOfTerms(const DSExpression *expression)
{
        DSUInteger numberOfTerms = 0;
        if (expression == NULL) {
                DSError(M_DS_WRONG ": Expression is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSExpressionType(expression) == DS_EXPRESSION_TYPE_UNDEFINED) {
                DSError(M_DS_NOFORMAT ": Expression type is undefined", A_DS_ERROR);
                goto bail;
        }
        if (DSExpressionType(expression) == DS_EXPRESSION_TYPE_CONSTANT) {
                if (DSExpressionConstant(expression) != 0.0)
                        numberOfTerms = 1;
                goto bail;
        }
        if (DSExpressionType(expression) == DS_EXPRESSION_TYPE_VARIABLE) {
                numberOfTerms = 1;
                goto bail;
        }
        switch (DSExpressionOperator(expression)) {
                case '*':
                        if (DSExpressionConstant(DSExpressionBranchAtIndex(expression, 0)) != 0.0)
                                numberOfTerms = 1;
                        break;
                case '^':
                        if (DSExpressionType(DSExpressionBranchAtIndex(expression, 0)) == DS_EXPRESSION_TYPE_VARIABLE)
                                numberOfTerms = 1;
                        else if (DSExpressionType(DSExpressionBranchAtIndex(expression, 0)) == DS_EXPRESSION_TYPE_CONSTANT &&
                                 DSExpressionConstant(DSExpressionBranchAtIndex(expression, 0)) != 0.0)
                                numberOfTerms = 1;
                        else if (DSExpressionType(DSExpressionBranchAtIndex(expression, 0)) == DS_EXPRESSION_TYPE_OPERATOR &&
                                 DSExpressionOperator(DSExpressionBranchAtIndex(expression, 0)) != '+')
                                numberOfTerms = DSExpressionNumberOfTerms(DSExpressionBranchAtIndex(expression, 0));
                        else
                                DSError("Expression too complicated to accurately calculate number of terms", A_DS_WARN);
                        break;
                case '+':
                        numberOfTerms = DSExpressionNumberOfBranches(expression);
                        if (DSExpressionConstant(DSExpressionBranchAtIndex(expression, DS_EXPRESSION_CONSTANT_BRANCH)) == 0.0)
                                numberOfTerms--;
                        break;
                default:
                        DSError(M_DS_NOFORMAT ": Operator type is not defined", A_DS_ERROR);
                        break;
        }
bail:
        return numberOfTerms;
}

extern DSUInteger DSExpressionNumberOfPositiveTerms(const DSExpression *expression)
{
        DSUInteger i, numberOfTerms = 0;
        if (expression == NULL) {
                DSError(M_DS_WRONG ": Expression is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSExpressionType(expression) == DS_EXPRESSION_TYPE_UNDEFINED) {
                DSError(M_DS_NOFORMAT ": Expression type is undefined", A_DS_ERROR);
                goto bail;
        }
        if (DSExpressionType(expression) == DS_EXPRESSION_TYPE_CONSTANT) {
                if (DSExpressionConstant(expression) > 0.0)
                        numberOfTerms = 1;
                goto bail;
        }
        if (DSExpressionType(expression) == DS_EXPRESSION_TYPE_VARIABLE) {
                numberOfTerms = 1;
                goto bail;
        }
        switch (DSExpressionOperator(expression)) {
                case '*':
                        if (DSExpressionConstant(DSExpressionBranchAtIndex(expression, 0)) > 0.0)
                                numberOfTerms = 1;
                        break;
                case '^':
                        if (DSExpressionType(DSExpressionBranchAtIndex(expression, 0)) != DS_EXPRESSION_TYPE_OPERATOR) {
                                numberOfTerms = DSExpressionNumberOfPositiveTerms(DSExpressionBranchAtIndex(expression, 0));
                        } else if (DSExpressionOperator(DSExpressionBranchAtIndex(expression, 0)) != '+') {
                                if (DSExpressionType(DSExpressionBranchAtIndex(expression, 1)) == DS_EXPRESSION_TYPE_CONSTANT &&
                                    !fmod(DSExpressionConstant(DSExpressionBranchAtIndex(expression, 1)), 2)) {
                                        numberOfTerms++;
                                } else {
                                        numberOfTerms = DSExpressionNumberOfPositiveTerms(DSExpressionBranchAtIndex(expression, 0));
                                        DSError("Expression too complicated to accurately calculate number of terms", A_DS_WARN);
                                }
                        }
                        break;
                case '+':
                        for (i = 0; i < DSExpressionNumberOfBranches(expression); i++)
                                numberOfTerms += DSExpressionNumberOfPositiveTerms(DSExpressionBranchAtIndex(expression, i));
                        break;
                default:
                        DSError(M_DS_NOFORMAT ": Operator type is not defined", A_DS_ERROR);
                        break;
        }
bail:
        return numberOfTerms;
}

extern DSUInteger DSExpressionNumberOfNegativeTerms(const DSExpression *expression)
{
        DSUInteger i, numberOfTerms = 0;
        if (expression == NULL) {
                DSError(M_DS_WRONG ": Expression is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSExpressionType(expression) == DS_EXPRESSION_TYPE_UNDEFINED) {
                DSError(M_DS_NOFORMAT ": Expression type is undefined", A_DS_ERROR);
                goto bail;
        }
        if (DSExpressionType(expression) == DS_EXPRESSION_TYPE_CONSTANT) {
                if (DSExpressionConstant(expression) < 0.0)
                        numberOfTerms = 1;
                goto bail;
        }
        if (DSExpressionType(expression) == DS_EXPRESSION_TYPE_VARIABLE) {
                goto bail;
        }
        switch (DSExpressionOperator(expression)) {
                case '*':
                        if (DSExpressionConstant(DSExpressionBranchAtIndex(expression, 0)) < 0.0)
                                numberOfTerms = 1;
                        break;
                case '^':
                        if (DSExpressionType(DSExpressionBranchAtIndex(expression, 1)) == DS_EXPRESSION_TYPE_CONSTANT &&
                            !fmod(DSExpressionConstant(DSExpressionBranchAtIndex(expression, 1)), 2.0)) {
                                break;
                        }
                        if (DSExpressionType(DSExpressionBranchAtIndex(expression, 0)) != DS_EXPRESSION_TYPE_OPERATOR) {
                                numberOfTerms = DSExpressionNumberOfNegativeTerms(DSExpressionBranchAtIndex(expression, 0));
                        } else if (DSExpressionOperator(DSExpressionBranchAtIndex(expression, 0)) != '+') {
                                numberOfTerms = DSExpressionNumberOfNegativeTerms(DSExpressionBranchAtIndex(expression, 0));
                                DSError("Expression too complicated to accurately calculate number of terms", A_DS_WARN);
                        }
                        break;
                case '+':
                        for (i = 0; i < DSExpressionNumberOfBranches(expression); i++)
                                numberOfTerms += DSExpressionNumberOfNegativeTerms(DSExpressionBranchAtIndex(expression, i));
                        break;
                default:
                        DSError(M_DS_NOFORMAT ": Operator type is not defined", A_DS_ERROR);
                        break;
        }
bail:
        return numberOfTerms;
}

extern const DSExpression * DSExpressionTermAtIndex(const DSExpression * expression);
extern const DSExpression * DSExpressionPositiveTermAtIndex(const DSExpression * expression, const DSUInteger index);
extern const DSExpression * DSExpressionNegativeTermAtIndex(const DSExpression * expression, const DSUInteger index);


#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

static bool operatorIsLowerPrecedence(char op1, char op2)
{
        bool isLower = false;
        switch (op1) {
                case '^':
                        if (op2 == '+' || op2 == '*')
                                isLower = true;   
                        break;
                case '*':
                        if (op2 == '+')
                                isLower = true;
                        break;
                default:
                        break;
        }
        return isLower;
}

static void expressionToStringInternal(const DSExpression *current, char ** string, DSUInteger *length)
{
        DSUInteger i;
        DSExpression * branch;
        double constant;
        char temp[100] = {'\0'};
        if (current == NULL) {
                DSError(M_DS_NULL ": Node to print is nil", A_DS_ERROR);
                goto bail;
        }
        switch (DSExpressionType(current)) {
                case DS_EXPRESSION_TYPE_CONSTANT:
                        sprintf(temp, "%lf", DSExpressionConstant(current));
                        break;
                case DS_EXPRESSION_TYPE_VARIABLE:
                        sprintf(temp, "%s", DSExpressionVariable(current));
                        break;
                case DS_EXPRESSION_TYPE_OPERATOR:
                        constant = DSExpressionConstant(DSExpressionBranchAtIndex(current, 0)); 
                        for (i=0; i < DSExpressionNumberOfBranches(current); i++) {
                                if (i == 0 && DSExpressionOperator(current) == '+' && constant == 0.0)
                                        continue;
                                if (i == 0 && DSExpressionOperator(current) == '*' && constant == 1.0)
                                        continue;
                                if (i == 0 && DSExpressionOperator(current) == '*' && constant == -1.0) {
                                        strncat(*string, "-", *length-strlen(*string));
                                        continue;
                                }
                                branch = DSExpressionBranchAtIndex(current, i);
                                if (DSExpressionType(branch) == DS_EXPRESSION_TYPE_OPERATOR &&
                                    operatorIsLowerPrecedence(DSExpressionOperator(current), DSExpressionOperator(branch)))
                                        strncat(*string, "(", *length-strlen(*string));
                                expressionToStringInternal(DSExpressionBranchAtIndex(current, i), string, length);
                                if (DSExpressionType(branch) == DS_EXPRESSION_TYPE_OPERATOR &&
                                    operatorIsLowerPrecedence(DSExpressionOperator(current), DSExpressionOperator(branch)))
                                        strncat(*string, ")", *length-strlen(*string));
                                if (i < DSExpressionNumberOfBranches(current)-1) {
                                        sprintf(temp, "%c", DSExpressionOperator(current));
                                        strncat(*string,  temp, *length-strlen(*string));
                                        temp[0] = '\0';
                                }
                        }
                        break;
        }
        if (strlen(*string)+strlen(temp) >= *length) {
                length += DS_EXPRESSION_STRING_INIT_LENGTH;
                *string = DSSecureRealloc(string, sizeof(char)**length);
        }
        strncat(*string,  temp, *length-strlen(*string));
bail:
        return;
}

extern char * DSExpressionAsString(const DSExpression *expression)
{
        DSUInteger length = DS_EXPRESSION_STRING_INIT_LENGTH;
        char * string = NULL;
        if (expression == NULL) {
                DSError(M_DS_NULL ": Node to print is nil", A_DS_ERROR);
                goto bail;
        }
        string = DSSecureCalloc(sizeof(char), length);
        expressionToStringInternal(expression, &string, &length);
        if (strlen(string) != 0)
                string = DSSecureRealloc(string, sizeof(char)*(strlen(string)+1));
bail:
        return string;
}

extern void DSExpressionPrint(const DSExpression *expression)
{
        char *string = NULL;
        if (expression == NULL) {
                DSError(M_DS_NULL ": Node to print is nil", A_DS_ERROR);
                goto bail;
        }
        string = DSExpressionAsString(expression);
        if (string == NULL)
                goto bail;
        if (DSPrintf == NULL) {
                printf("%s\n", string);
        } else {
                DSPrintf("%s\n", string);
        }
        DSSecureFree(string);
bail:
        return;
}




