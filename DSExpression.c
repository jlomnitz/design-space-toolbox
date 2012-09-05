/**
 * \file DSExpression.c
 * \brief Implementation file with functions for dealing with mathematical
 *        expressions.
 *
 * \details The DSExpression object is used internally to parse mathematical
 *          expressions and to evaluate these expressions using a variable pool.
 *          The DSExpression object supports scalar adition, multiplication, 
 *          powers and real valued functions of real variables.
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
 *
 * \todo Add options to register custom functions.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "DSErrors.h"
#include "DSMemoryManager.h"
#include "DSVariable.h"
#include "DSExpression.h"
#include "DSExpressionTokenizer.h"

#define DS_EXPRESSION_CONSTANT_BRANCH_INDEX     0
#define DS_EXPRESSION_STRING_INIT_LENGTH        1000

extern DSExpression * dsExpressionAllocWithConstant(const double value)
{
        DSExpression *newNode = NULL;
        newNode = DSSecureCalloc(1, sizeof(DSExpression));
        DSExpressionSetConstant(newNode, value);
        newNode->type = DS_EXPRESSION_TYPE_CONSTANT;
        return newNode;
}

extern DSExpression * dsExpressionAllocWithOperator(const char op_code)
{
        DSExpression *newNode = NULL;
        switch (op_code) {
                case '+':
                        newNode = DSSecureCalloc(1, sizeof(DSExpression));
                        DSExpressionSetOperator(newNode, '+');
                        /* First branch reserved for constants */
                        newNode->branches = DSSecureMalloc(sizeof(DSExpression *));
                        newNode->branches[DS_EXPRESSION_CONSTANT_BRANCH_INDEX] = dsExpressionAllocWithConstant(0.0);
                        newNode->numberOfBranches = 1;
                        break;
                case '*':
                        newNode = DSSecureCalloc(1, sizeof(DSExpression));
                        DSExpressionSetOperator(newNode, '*');
                        /* First branch reserved for constants */
                        newNode->branches = DSSecureMalloc(sizeof(DSExpression *));
                        newNode->branches[DS_EXPRESSION_CONSTANT_BRANCH_INDEX] = dsExpressionAllocWithConstant(1.0);
                        newNode->numberOfBranches = 1;
                        break;
                case '^':
                        newNode = DSSecureCalloc(1, sizeof(DSExpression));
                        DSExpressionSetOperator(newNode, op_code);
                        break;
                case '-':
                        DSError(M_DS_WRONG ": DSExpression does not internally use '-' operators", A_DS_ERROR);
                        break;
                case '/':
                        DSError(M_DS_WRONG ": DSExpression does not internally use '/' operators", A_DS_ERROR);
                        break;
                default:
                        DSError(M_DS_WRONG ": DSExpression found unrecognized operator.", A_DS_ERROR);
                        break;
        }
        return  newNode;
}

extern DSExpression * dsExpressionAllocWithVariableName(const char * name)
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
                case DS_EXPRESSION_TYPE_FUNCTION:
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

extern DSExpression * DSExpressionCopy(const DSExpression * expression)
{
        DSExpression * root = NULL;
        char * string = NULL;
        if (expression == NULL) {
                DSError(M_DS_NULL ": Expression to copy is NULL", A_DS_ERROR);
                goto bail;
        }
        string = DSExpressionAsString(expression);
        root = DSExpressionByParsingString(string);
        DSSecureFree(string);
bail:
        return root;
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

extern DSExpression * DSExpressionAddExpressions(DSExpression *lvalue, DSExpression *rvalue)
{
        DSExpression * newRoot = NULL;
        if (lvalue == NULL && rvalue == NULL) {
                DSError(M_DS_NULL ": Expression is NULL", A_DS_ERROR);
                goto bail;
        }
        if (lvalue == NULL) {
                newRoot = rvalue;
                goto bail;
        }
        if (rvalue == NULL) {
                newRoot = lvalue;
                goto bail;
        }
        newRoot = dsExpressionAllocWithOperator('+');
        DSExpressionAddBranch(newRoot, lvalue);
        DSExpressionAddBranch(newRoot, rvalue);
bail:
        return newRoot;
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
        if (DSExpressionType(expression) != DS_EXPRESSION_TYPE_OPERATOR) {
                DSError(M_DS_WRONG ": Expression root is not an operator", A_DS_ERROR);
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

static void dsExpressionAddBranchToFunction(DSExpression *expression, DSExpression *branch)
{
        if (expression == NULL) {
                DSError(M_DS_NULL ": Expression root is NULL", A_DS_ERROR);
                goto bail;
        }
        if (branch == NULL) {
                DSError(M_DS_NULL ": Branch being added is NULL", A_DS_ERROR);
                goto bail;
        }
        expression->branches = DSSecureMalloc(sizeof(DSExpression *)*(DSExpressionNumberOfBranches(expression)+1));
        expression->branches[DSExpressionNumberOfBranches(expression)] = branch;
        expression->numberOfBranches++;
        expression->type = DS_EXPRESSION_TYPE_FUNCTION;
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
        if (DSExpressionType(expression) == DS_EXPRESSION_TYPE_VARIABLE) {
                dsExpressionAddBranchToFunction(expression, branch);
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
        if (DSExpressionType(branch)  == DS_EXPRESSION_TYPE_VARIABLE || 
            DSExpressionType(branch) == DS_EXPRESSION_TYPE_FUNCTION) {
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


#if defined(__APPLE__) && defined (__MACH__)
#pragma mark - Utility functions
#endif

#define ds_function_index_log    0
#define ds_function_index_ln     1
#define ds_function_index_log10  2
#define ds_function_index_cos    3
#define ds_function_index_sin    4

static double dsExpressionEvaluateMathematicalFunction(const DSExpression *function, const DSVariablePool * pool)
{
        double value, eval = NAN;
        DSVariablePool *functionNames = NULL;
        if (function == NULL) {
                DSError(M_DS_NULL ": Expression node is null", A_DS_ERROR);
                goto bail;
        }
        if (DSExpressionType(function) != DS_EXPRESSION_TYPE_FUNCTION) {
                DSError(M_DS_WRONG ": Expression node must be a function", A_DS_ERROR);
                goto bail;
        }
        functionNames = DSVariablePoolByParsingString("log : 1, ln : 1, log10 : 1, cos : 1, sin : 1");
        if (DSVariablePoolHasVariableWithName(functionNames, DSExpressionVariable(function)) == false) {
                DSError(M_DS_WRONG ": Function name not recognized", A_DS_ERROR);
                goto bail;
        }
        value = DSExpressionEvaluateWithVariablePool(DSExpressionBranchAtIndex(function, 0), pool);
        switch (DSVariablePoolIndexOfVariableWithName(functionNames, DSExpressionVariable(function))) {
                case ds_function_index_ln:
                        eval = log(value);
                        break;
                case ds_function_index_log:
                case ds_function_index_log10:
                        eval = log10(value);
                        break;
                case ds_function_index_cos:
                        eval = cos(value);
                        break;
                case ds_function_index_sin:
                        eval = sin(value);
                        break;
                default:
                        break;
        }
bail:
        if (functionNames != NULL)
                DSVariablePoolFree(functionNames);
        return eval;
}

extern double DSExpressionEvaluateWithVariablePool(const DSExpression *expression, const DSVariablePool *pool)
{
        double value = NAN;
        DSVariable *variable = NULL;
        DSUInteger i;
        if (expression == NULL) {
                DSError(M_DS_NULL ": Expression is NULL", A_DS_ERROR);
                goto bail;
        }
        switch (DSExpressionType(expression)) {
                case DS_EXPRESSION_TYPE_VARIABLE:
                        if (pool != NULL) {
                                if (DSVariablePoolHasVariableWithName(pool, DSExpressionVariable(expression)) == true) {
                                        variable = DSVariablePoolVariableWithName(pool, DSExpressionVariable(expression));
                                        value = DSVariableValue(variable);
                                } else {
                                        DSError(M_DS_WRONG ": Variable pool does not have variable.", A_DS_ERROR);
                                }
                        } else {
                                DSError(M_DS_VAR_NULL, A_DS_ERROR);
                        }
                        
                        break;
                case DS_EXPRESSION_TYPE_CONSTANT:
                        value = DSExpressionConstant(expression);
                        break;
                case DS_EXPRESSION_TYPE_FUNCTION:
                        value=dsExpressionEvaluateMathematicalFunction(expression, pool);
                        break;
                case DS_EXPRESSION_TYPE_OPERATOR:
                        switch (DSExpressionOperator(expression)) {
                                case '+':
                                        value = 0;
                                        for (i = 0; i < DSExpressionNumberOfBranches(expression); i++)
                                                value += DSExpressionEvaluateWithVariablePool(DSExpressionBranchAtIndex(expression, i), pool);
                                        break;
                                case '*':
                                        value = 1;
                                        for (i = 0; i < DSExpressionNumberOfBranches(expression); i++)
                                                value *= DSExpressionEvaluateWithVariablePool(DSExpressionBranchAtIndex(expression, i), pool);
                                        break;
                                case '^':
                                        value = pow(DSExpressionEvaluateWithVariablePool(DSExpressionBranchAtIndex(expression, 0), pool),
                                                    DSExpressionEvaluateWithVariablePool(DSExpressionBranchAtIndex(expression, 1), pool));
                                        break;
                                default:
                                        break;
                        }
                        break;
                default:
                        break;
        }
bail:
        return value;
}

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

static void dsExpressionToStringInternal(const DSExpression *current, char ** string, DSUInteger *length)
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
                                if (i == 0 && DSExpressionOperator(current) == '*' && constant == 0.0) {
                                        strncat(*string, "0", *length-strlen(*string));
                                        break;
                                }
                                if (i == 0 && DSExpressionOperator(current) == '*' && constant == -1.0) {
                                        strncat(*string, "-", *length-strlen(*string));
                                        continue;
                                }
                                branch = DSExpressionBranchAtIndex(current, i);
                                if (DSExpressionType(branch) == DS_EXPRESSION_TYPE_OPERATOR &&
                                    operatorIsLowerPrecedence(DSExpressionOperator(current), DSExpressionOperator(branch)))
                                        strncat(*string, "(", *length-strlen(*string));
                                dsExpressionToStringInternal(branch, string, length);
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
                case DS_EXPRESSION_TYPE_FUNCTION:
                        sprintf(temp, "%s(", DSExpressionVariable(current));
                        if (strlen(*string)+strlen(temp) >= *length) {
                                length += DS_EXPRESSION_STRING_INIT_LENGTH;
                                *string = DSSecureRealloc(string, sizeof(char)**length);
                        }
                        strncat(*string, temp, *length-strlen(*string));
                        dsExpressionToStringInternal(DSExpressionBranchAtIndex(current, 0), string, length);
                        strncat(*string, ")", *length-strlen(*string));
                        temp[0] = '\0';
                        break;
                default:
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
        dsExpressionToStringInternal(expression, &string, &length);
bail:
        return string;
}

static void expressionToTroffStringInternal(const DSExpression *current, char ** string, DSUInteger *length)
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
                                if (i == 0 && DSExpressionOperator(current) == '*' && constant == 0.0) {
                                        strncat(*string, "0", *length-strlen(*string));
                                        break;
                                }
                                if (i == 0 && DSExpressionOperator(current) == '*' && constant == -1.0) {
                                        strncat(*string, "-", *length-strlen(*string));
                                        continue;
                                }
                                branch = DSExpressionBranchAtIndex(current, i);
                                if (DSExpressionType(branch) == DS_EXPRESSION_TYPE_OPERATOR &&
                                    operatorIsLowerPrecedence(DSExpressionOperator(current), DSExpressionOperator(branch)))
                                        strncat(*string, "(", *length-strlen(*string));
                                expressionToTroffStringInternal(branch, string, length);
                                if (DSExpressionType(branch) == DS_EXPRESSION_TYPE_OPERATOR &&
                                    operatorIsLowerPrecedence(DSExpressionOperator(current), DSExpressionOperator(branch)))
                                        strncat(*string, ")", *length-strlen(*string));
                                if (i < DSExpressionNumberOfBranches(current)-1) {
                                        if (DSExpressionOperator(current) == '+')
                                                sprintf(temp, " %c ", DSExpressionOperator(current));
                                        else if (DSExpressionOperator(current) == '^')
                                                sprintf(temp, " sup ");
                                        else
                                                sprintf(temp, " ~ ");
                                        strncat(*string,  temp, *length-strlen(*string));
                                        temp[0] = '\0';
                                }
                        }
                        break;
                case DS_EXPRESSION_TYPE_FUNCTION:
                        sprintf(temp, "%s(", DSExpressionVariable(current));
                        if (strlen(*string)+strlen(temp) >= *length) {
                                length += DS_EXPRESSION_STRING_INIT_LENGTH;
                                *string = DSSecureRealloc(string, sizeof(char)**length);
                        }
                        strncat(*string, temp, *length-strlen(*string));
                        expressionToTroffStringInternal(DSExpressionBranchAtIndex(current, 0), string, length);
                        strncat(*string, ")", *length-strlen(*string));
                        temp[0] = '\0';
                        break;
                default:
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

extern char * DSExpressionAsTroffString(const DSExpression *expression)
{
        DSUInteger length = DS_EXPRESSION_STRING_INIT_LENGTH;
        char * string = NULL;
        if (expression == NULL) {
                DSError(M_DS_NULL ": Node to print is nil", A_DS_ERROR);
                goto bail;
        }
        string = DSSecureCalloc(sizeof(char), length);
        expressionToTroffStringInternal(expression, &string, &length);
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
        if (strlen(string) != 0) {
                if (DSPrintf == NULL) {
                        printf("%s\n", string);
                } else {
                        DSPrintf("%s\n", string);
                }
        } else {
                if (DSPrintf == NULL) {
                        printf("0\n");
                } else {
                        DSPrintf("0\n");
                }
        }
        DSSecureFree(string);
bail:
        return;
}




