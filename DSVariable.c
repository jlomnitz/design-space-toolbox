/**
 * \file DSVariable.c
 * \brief Implementation file with functions for dealing with variables.
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


#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "DSMemoryManager.h"
#include "DSErrors.h"
#include "DSVariable.h"
#include "DSVariableTokenizer.h"


#if defined(__APPLE__) && defined(__MACH__)
#pragma mark - Symbol Variables
#endif


/**
 * \brief Creates a new DSVariable with INFINITY as a default value.
 * 
 * This function may
 * be used throughout, in order to create new variables consistently and 
 * portably.  As variables are allocated individually, it is important to 
 * not that they should be released with the accesory method.
 * \param name A string with which to identify the DSVariable.
 * \return The pointer to the newly allocated DSVariable.
 * \see DSVariable
 * \see DSVariableFree
 */
extern DSVariable *DSVariableAlloc(const char *name)
{
        DSVariable *var;
        if (name == NULL) {
                DSError(M_DS_NULL ": Name is a NULL pointer", A_DS_ERROR);
                goto bail;
        }
        if (strlen(name) == 0) {
                DSError(M_DS_WRONG ": Name is empty", A_DS_WARN);
                goto bail;
        }
        var = DSSecureMalloc(sizeof(DSVariable));
        var->name = strdup(name);
        var->retainCount = 1;
        DSVariableSetValue(var, INFINITY);
bail:
        return var;
}

/**
 * \brief Function frees allocated memory of a DSVariable.
 *
 * This function should be
 * used for each newDSVariable that is called.  The internal structure is 
 * subject to changes in consequent versions and therefore freeing memory
 * of DSVariables should be strictly through this function.
 * \param var The pointer to the variable to free.
 */
extern void DSVariableFree(DSVariable *var)
{
        if (var == NULL) {
                DSError(M_DS_NULL ": Variable to free is null", A_DS_ERROR);
                goto bail;
        }
        if (var->name != NULL)
                DSSecureFree(var->name);
        else
                DSError(M_DS_WRONG ": Variable name was NULL", A_DS_WARN);
        DSSecureFree(var);
bail:
        return;
}


/**
 * \brief Function to increase variable retain count by one.
 *
 * Variables utilize a similar memory management system used in 
 * Objective-C NSObject subclasses, where objects a retained/released.
 *
 * \param aVariable The variable which will have its retain count increased.
 *
 * \return The same variable which received the retain count increase is returned,
 *         for convinience.
 * 
 * \see DSVariableRelease
 */
extern DSVariable * DSVariableRetain(DSVariable *aVariable)
{
        if (aVariable == NULL) {
                DSError(M_DS_NULL ": Retaining a NULL varaible", A_DS_ERROR);
                goto bail;
        }
        aVariable->retainCount++;
bail:
        return aVariable;
}

/**
 * \brief Function to decrease variable retain count by one.
 *
 * Fast processing tree is made to decrease its retain count by one, when the
 * retain count hits zero, the function DSVariableFree() is invoked, freeing the 
 * memory space. Fast processing tree does not have an equivalent to
 * autorelease, forcing the developer to use greater care when directly managing
 * memory.
 *
 * \param aVariable The variable which will have its retain count reduced.
 *
 * \see DSVariableRetain
 * \see DSVariableFree
 */
extern void DSVariableRelease(DSVariable *aVariable)
{
        if (aVariable == NULL) {
                DSError(M_DS_NULL ": releasing a NULL variable.", A_DS_ERROR);
                goto bail;
        }
        aVariable->retainCount--;
        if (aVariable->retainCount == 0)
                DSVariableFree(aVariable);
bail:
        return;
}





#if defined(__APPLE__) && defined(__MACH__)
#pragma mark - Variable dictionary Functions
#endif

/**
 * \brief Internal function for searching a dictionary for a variable name.
 *
 * This function is for internal use only.  It is a RECURSIVE function which 
 * checks the difference between the name and the dictionary character by 
 * character.  If it encounters a matching Null character in both the dictionar
 * and the name, a pointer to the DSVariable is returned.
 * \param name A string, the one passed to DSVariableWithName, with the name of the DSVariable.
 * \param node The node of the dictionary being compared.
 * \param pos The position of the character in the name string being compared with the node.
 * \return A pointer to the DSVariable.  If the name name does not match, NULL is returned.
 * \see DSVariable
 * \see DSVariableWithName
 * \see struct _varDictionary
 * \see DSVariablePoolAddVariable
 */
static DSVariable *dsCheckNameAtPos(const char *name, struct _varDictionary *node, int pos)
{
        DSVariable *aVariable = NULL;
        if (node == NULL)
                goto bail;
        while (node) {
                if (name[pos] == node->current) {
                        if (node->current == '\0') {
                                aVariable = node->variable;
                                break;
                        }
                        node = node->next;
                        pos++;
                } else if (name[pos] == '\0') {
                        node = node->alt;
                } else if (name[pos] < node->current) {
                        break;
                }
                else if (name[pos] > node->current) {
                        node = node->alt;
                }
        }
bail:
        return aVariable;
}

__deprecated static DSVariable *_checkNameAtPos(const char *name, struct _varDictionary *node, int pos)
{
        return dsCheckNameAtPos(name, node, pos);
}

/**
 * \brief Creates a simple branch containing the remaining nodes for the DSVariable.
 *
 * This internal function is used to create all the nodes not currently existing
 * in the dictionary.  This function is auxiliary to DSVariablePoolAddVariable and should
 * not be used outside of that function.
 *
 * \param var The DSVariable being added to the dictionary.
 * \param atPos The position of the name string of the DSVariable at which to start making the new nodes.
 *
 * \return The pointer to the root of the newly formed branch.
 *
 * \see DSVariable
 * \see DSVariablePoolAddVariable
 */
static struct _varDictionary *dsAddNewBranch(DSVariable *var, int atPos)
{
        struct _varDictionary *root = NULL, *current;
        if (var == NULL) {
                DSError(M_DS_NULL ": Variable being added is NULL", A_DS_WARN);
                goto bail;
        }
        const char *name;
        root = DSSecureCalloc(sizeof(struct _varDictionary), 1);
        current = root;
        name = var->name;
        while(name[atPos] != '\0') {
                current->current = name[atPos++];
                current->next = DSSecureCalloc(sizeof(struct _varDictionary), 1);
                current = current->next;
        }
        current->current = '\0';
        current->variable = DSVariableRetain(var);
bail:
        return root;
}

__deprecated static struct _varDictionary *_addNewBranch(DSVariable *var, int atPos)
{
        return dsAddNewBranch(var, atPos);
}


/**
 * \brief Adds a DSVariable to the dictionary.
 *
 * This is the function to add DSVariables to
 * the dictionary.  Since the dictionary works alphabetically, the root of the 
 * dictionary changes and is defined by this function, and the root, be it the
 * new one or the old one, is returned.  This function is used in creating a new dictionary.
 * \param newVar The pointer to the variable which is to be added to the dictionary.
 * \param root The root of the dictionary. THE ROOT MAY BE REASSIGNED. If null, creates a new dictionary.
 * \return The root of the dictionary, which is likely to change.
 * \see _varDictionary
 * \see DSVariableWithName
 * \see _addNewBranch
 */
static struct _varDictionary * dsVarDictionaryAddVariable(DSVariable *newVar, struct _varDictionary *root)
{
        int pos;
        struct _varDictionary *current, *previous, *temp, *top;
        
        bool changeRoot;
        char errorMessage[100];
        
        if (newVar == NULL) {
                DSError(M_DS_WRONG ": Variable is NULL", A_DS_ERROR);
                goto bail;
        }
        if (root == NULL) {
                root = dsAddNewBranch(newVar, 0);
                goto bail;
        }
        if (dsCheckNameAtPos(newVar->name, root, 0) != NULL) {
                sprintf(errorMessage, "%.30s: Variable pool has variable \"%.10s\"", M_DS_EXISTS, newVar->name);
                DSError(errorMessage, A_DS_WARN);
                goto bail;
        }
        top = root;
        current = root;
        previous = NULL;
        temp = NULL;
        pos = 0;
        changeRoot = true;
        while (current) {
                if (newVar->name[pos] < current->current || current->current == '\0') {
                        temp = dsAddNewBranch(newVar, pos);
                        temp->alt = current;
                        if (changeRoot == true)
                                root = temp;
                        else if (previous == NULL) {
                                top->next = temp;
                        }
                        else
                                previous->alt = temp;
                        break;
                } else if (current->current == newVar->name[pos]) {
                        top = current;
                        changeRoot = false;
                        previous = NULL;
                        current = current->next;
                        pos++;
                } else if (current->alt == NULL) {
                        current->alt = dsAddNewBranch(newVar, pos);
                        break;
                } else {
                        previous = current;
                        current = current->alt;
                        changeRoot = false;
                }
        }
bail:
        return root;
}

__deprecated static struct _varDictionary * VarDictionaryAddVariable(DSVariable *newVar, struct _varDictionary *root)
{
        return dsVarDictionaryAddVariable(newVar, root);
}

/**
 * \brief Adds a DSVariable to the dictionary.
 *
 * This is the function to add DSVariables to
 * the dictionary.  Since the dictionary works alphabetically, the root of the 
 * dictionary changes and is defined by this function, and the root, be it the
 * new one or the old one, is returned.  This function is used in creating a new dictionary.
 * \param newVar The pointer to the variable which is to be added to the dictionary.
 * \param root The root of the dictionary. THE ROOT MAY BE REASSIGNED. If null, creates a new dictionary.
 * \return The root of the dictionary, which is likely to change.
 * \see _varDictionary
 * \see DSVariableWithName
 * \see _addNewBranch
 */
static struct _varDictionary *dsVarDictionaryAddVariableWithName(const char * name, struct _varDictionary *root)
{
        DSVariable *newVar = NULL;
        if (name == NULL) {
                DSError(M_DS_WRONG ": Name is a NULL pointer", A_DS_ERROR);
                goto bail;
        }
        if (strlen(name) == 0) {
                DSError(M_DS_WRONG ": Name string is empty", A_DS_WARN);
                goto bail;
        }
        newVar = DSVariableAlloc(name);
        root = dsVarDictionaryAddVariable(newVar, root);
        DSVariableRelease(newVar);
bail:
        return root;
}

__deprecated static struct _varDictionary *VarDictionaryAddVariableWithName(const char * name, struct _varDictionary *root)
{
        return dsVarDictionaryAddVariableWithName(name, root);
}

/**
 * \brief Retrieves the variable in the dictionary with a matching name.
 *
 * This is the function for retriving a DSVariable from a dictionary.
 * \param name A string containing a NULL terminated string containing the name of the desired DSVariable.
 * \param root The root of the dictionary, determined during the adding of variables.
 * \return A pointer to the variable. If there is no matching variable, NULL is returned.
 * \see _checkNameAtPos
 * \see DSVariablePool
 * \see DSVariablePoolAddVariable
 */
static DSVariable *dsVarDictionaryVariableWithName(const char *name, struct _varDictionary *root)
{
        DSVariable *variable = NULL;
        if (name == NULL) {
                DSError(M_DS_NULL ": Name is a NULL pointer", A_DS_ERROR);
        }
        if (name == NULL || root == NULL) {
                DSError(M_DS_NULL ": Variable pool is NULL: Assuming empty", A_DS_WARN);
                goto bail;
        }
        variable = dsCheckNameAtPos(name, root, 0);
bail:
        return variable;
}

__deprecated static DSVariable *VarDictionaryVariableWithName(const char *name, struct _varDictionary *root)
{
        return dsVarDictionaryVariableWithName(name, root);
}
/**
 * \brief Function to remove all nodes in the dictionary.
 *
 * The function is RECURSIVE
 * and frees all the nodes past the root which is passed.  In theory, it could
 * be used to clear a portion of the dictionary, yet doing so would require
 * direct manipulation of the dictionary which is strongly discouraged.  The 
 * variables themselves are NOT freed, since the dictionary does not allocate
 * them.
 * \param root The root of the dictionary which is to be cleared.
 * \see DSVariablePool
 * \see DSVariableWithName
 * \see DSVariablePoolAddVariable
 */
static void dsVarDictionaryFree(struct _varDictionary *root)
{
        if (root == NULL)
                goto bail;
        dsVarDictionaryFree(root->alt);
        dsVarDictionaryFree(root->next);
        root->alt = NULL;
        root->next = NULL;
        if (root->variable != NULL)
                DSVariableRelease(root->variable);
        DSSecureFree(root);
bail:
        return;
}

__deprecated static void VarDictionaryFree(struct _varDictionary *root)
{
        dsVarDictionaryFree(root);
        return;
}


/**
 * \brief Prints the VarDictionary.
 *
 * A debugging function which prints the dictionary structure to the stderr file
 */
static void dsPrintVarDictionary(struct _varDictionary *root, DSUInteger indent)
{
        DSUInteger i;
        int (*print)(const char *, ...);
        if (root == NULL)
                goto bail;
        print = DSPrintf;
        if (DSPrintf == NULL)
                print = printf;
        for (i = 1; i < indent+1; i++)
                print(" ");
        if (root->current == '\0')
                print("+-[%s] = %lf\n", DSVariableName(root->variable), DSVariableValue(root->variable));
        else
                print("+-%c\n", root->current);
        dsPrintVarDictionary(root->next, indent+2);
        dsPrintVarDictionary(root->alt, indent);
bail:
        return;
}

__deprecated static void printVarDictionary(struct _varDictionary *root, DSUInteger indent)
{
        dsPrintVarDictionary(root, indent);
}

/**
 * \brief Function that prints the members in the dictionary.
 * 
 * This function recursively
 * checks the dictionary for all names, and prints them out.  It requires of
 * a buffer string which is used to store the current characters, since it is
 * a recursive function.
 *
 * \param root The root of the dictionary, and the current node for consequent calls.
 * \param buffer The string in which to store the characters, function does not check size of buffer.
 * \param position Should be 0 when called, increases as it searches the dictionary.
 *
 * \see _varDictionary
 * \see DSVariablePoolAddVariable
 *
 * \todo Create a wrapper function which creates the buffer, and initiates the position at 0.
 */
static void dsPrintMembers(struct _varDictionary *root, char *buffer, int position)
{
        if (root == NULL)
                return;
        buffer[position] = root->current;
        if (root->current == '\0')
                printf("%s\n", buffer);
        dsPrintMembers(root->next, buffer, position+1);
        dsPrintMembers(root->alt, buffer, position);
}

__deprecated static void printMembers(struct _varDictionary *root, char *buffer, int position)
{
        dsPrintMembers(root, buffer, position);
}

#if defined(__APPLE__) && defined(__MACH__)
#pragma mark - Variable Pool Functions
#endif

extern DSVariablePool * DSVariablePoolAlloc(void)
{
        DSVariablePool *pool = NULL;
        pool = DSSecureCalloc(1, sizeof(DSVariablePool));
        pool->lock = DSLockReadWrite;
        return pool;
}

extern DSVariablePool * DSVariablePoolCopy(const DSVariablePool * const pool)
{
        DSUInteger i;
        DSVariablePool * copy = NULL;
        const DSVariable * const * allVariables = NULL;
        if (pool == NULL) {
                DSError(M_DS_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        copy = DSVariablePoolAlloc();
        allVariables = DSVariablePoolAllVariables(pool);
        for (i = 0; i < DSVariablePoolNumberOfVariables(pool); i++)
                DSVariablePoolAddVariable(copy, (DSVariable *)(allVariables[i]));
        copy->lock = pool->lock;
bail:
        return copy;
}

extern void DSVariablePoolFree(DSVariablePool *pool)
{
        if (pool == NULL) {
                DSError(M_DS_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (pool->lock == DSLockReadOnly) {
                DSError(M_DS_VAR_LOCKED, A_DS_ERROR);
                goto bail;
        }
        dsVarDictionaryFree(DSVariablePoolInternalDictionary(pool));
        if (DSVariablePoolVariableArray(pool) != NULL) {
                DSSecureFree(DSVariablePoolVariableArray(pool));
        }
        DSSecureFree(pool);
bail:
        return;
}

extern void DSVariablePoolSetReadOnly(DSVariablePool * pool)
{
        if (pool == NULL) {
                DSError(M_DS_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        pool->lock = DSLockReadOnly;
bail:
        return;
}

extern void DSVariablePoolSetReadWrite(DSVariablePool * pool)
{
        if (pool == NULL) {
                DSError(M_DS_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        pool->lock = DSLockReadWrite;
bail:
        return;
}

extern void DSVariablePoolAddVariableWithName(DSVariablePool *pool, const char * name)
{
        DSVariable *var = NULL;
        bool varIsNew = true;
        if (pool == NULL) {
                DSError(M_DS_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (pool->lock == DSLockReadOnly) {
                DSError(M_DS_VAR_LOCKED, A_DS_ERROR);
                goto bail;
        }
        if (name == NULL) {
                DSError(M_DS_WRONG ": Name is a NULL pointer", A_DS_ERROR);
                goto bail;
        }
        if (strlen(name) == 0) {
                DSError(M_DS_WRONG ": Name string is empty", A_DS_WARN);
                goto bail;
        }
        if (dsCheckNameAtPos(name, DSVariablePoolInternalDictionary(pool), 0) != NULL) {
                varIsNew = false;
        }
        DSVariablePoolInternalDictionary(pool) = dsVarDictionaryAddVariableWithName(name, DSVariablePoolInternalDictionary(pool));
        if (varIsNew == true) {
                var = DSVariablePoolVariableWithName(pool, name);
                if (DSVariablePoolNumberOfVariables(pool) == 0) 
                        DSVariablePoolVariableArray(pool) = DSSecureCalloc(DSVariablePoolNumberOfVariables(pool)+1, sizeof(DSVariable *));
                else
                        DSVariablePoolVariableArray(pool) = DSSecureRealloc(DSVariablePoolVariableArray(pool), (DSVariablePoolNumberOfVariables(pool)+1)*sizeof(DSVariable *));
                DSVariablePoolVariableArray(pool)[DSVariablePoolNumberOfVariables(pool)++] = var;
        }
bail:
        return;
}

extern void DSVariablePoolAddVariable(DSVariablePool * pool, DSVariable *newVar)
{
        DSVariable * var = NULL;
        if (pool == NULL) {
                DSError(M_DS_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (pool->lock == DSLockReadOnly) {
                DSError(M_DS_VAR_LOCKED, A_DS_ERROR);
                goto bail;
        }
        if (newVar == NULL) {
                DSError(M_DS_WRONG ": Variable is NULL", A_DS_ERROR);
                goto bail;
        }
        DSVariablePoolInternalDictionary(pool) = dsVarDictionaryAddVariable(newVar, DSVariablePoolInternalDictionary(pool));
        var = DSVariablePoolVariableWithName(pool, newVar->name);
        if (var == newVar) {
                if (DSVariablePoolNumberOfVariables(pool) == 0) 
                        DSVariablePoolVariableArray(pool) = DSSecureCalloc(DSVariablePoolNumberOfVariables(pool)+1, sizeof(DSVariable *));
                else
                        DSVariablePoolVariableArray(pool) = DSSecureRealloc(DSVariablePoolVariableArray(pool), (DSVariablePoolNumberOfVariables(pool)+1)*sizeof(DSVariable *));
                DSVariablePoolVariableArray(pool)[DSVariablePoolNumberOfVariables(pool)++] = var; 
        }
bail:
        return;
}

extern bool DSVariablePoolHasVariableWithName(const DSVariablePool *pool, const char * const name)
{
        bool hasVariable = false;
        if (pool == NULL) {
                DSError(M_DS_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (name == NULL) {
                DSError(M_DS_WRONG ": Name of variable is NULL", A_DS_ERROR);
                goto bail;
        }
        if (strlen(name) == 0) {
                DSError(M_DS_WRONG ": Name of variable is empty", A_DS_WARN);
                goto bail;                
        }
        if (DSVariablePoolNumberOfVariables(pool) == 0)
                goto bail;
        if (DSVariablePoolVariableWithName(pool, name) != NULL)
                hasVariable = true;
bail:
        return hasVariable;
}

extern DSVariable *DSVariablePoolVariableWithName(const DSVariablePool *pool, const char *name)
{
        DSVariable * variable = NULL;
        if (pool == NULL) {
                DSError(M_DS_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (name == NULL) {
                DSError(M_DS_WRONG ": Name of variable is NULL", A_DS_ERROR);
                goto bail;
        }
        if (strlen(name) == 0) {
                DSError(M_DS_WRONG ": Name of variable is empty", A_DS_WARN);
                goto bail;                
        }
        variable = dsVarDictionaryVariableWithName(name, DSVariablePoolInternalDictionary(pool));
bail:
        return variable;
}

extern void DSVariablePoolSetValueForVariableWithName(DSVariablePool *pool, const char *name, const double value)
{
        DSVariable *variable = NULL;
        if (pool == NULL) {
                DSError(M_DS_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (pool->lock == DSLockReadOnly) {
                DSError(M_DS_VAR_LOCKED, A_DS_ERROR);
                goto bail;
        }
        if (name == NULL) {
                DSError(M_DS_WRONG ": Name of variable is NULL", A_DS_ERROR);
                goto bail;
        }
        if (strlen(name) == 0) {
                DSError(M_DS_WRONG ": Name of variable is empty", A_DS_WARN);
                goto bail;                
        }
        if (DSVariablePoolHasVariableWithName(pool, name) == false) {
                DSError(M_DS_WRONG ": Variable pool does not have varable", A_DS_ERROR);
                goto bail;
        }
        variable = DSVariablePoolVariableWithName(pool, name);
        DSVariableSetValue(variable, value);
bail:
        return;
}


extern const DSVariable * * DSVariablePoolAllVariables(const DSVariablePool *pool)
{
        const DSVariable ** allVariables = NULL;
        if (pool == NULL) {
                DSError(M_DS_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        allVariables = (const DSVariable **)DSVariablePoolVariableArray(pool);
bail:
        return allVariables;
}

extern const char ** DSVariablePoolAllVariableNames(const DSVariablePool *pool)
{
        DSUInteger i;
        char ** variableNames = NULL;
        if (pool == NULL) {
                DSError(M_DS_WRONG ": Variable pool is NULL", A_DS_ERROR);
                goto bail;
        }
        variableNames = DSSecureCalloc(sizeof(char *), 
                                       DSVariablePoolNumberOfVariables(pool));
        for (i = 0; i < DSVariablePoolNumberOfVariables(pool); i++)
                variableNames[i] = DSVariableName(DSVariablePoolAllVariables(pool)[i]);
bail:
        return (const char **)variableNames;
}

extern DSUInteger DSVariablePoolIndexOfVariable(const DSVariablePool *pool, const DSVariable *var)
{
        DSUInteger index = DSVariablePoolNumberOfVariables(pool);
        DSUInteger i;
        if (pool == NULL) {
                DSError(M_DS_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (var == NULL) {
                DSError(M_DS_WRONG ": Variable is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolHasVariableWithName(pool, DSVariableName(var)) == false) {
                DSError(M_DS_WRONG ": Variable does not contain desired variable", A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < DSVariablePoolNumberOfVariables(pool); i++)
                if (var == DSVariablePoolAllVariables(pool)[i])
                        break;
        index = i;
bail:
        return index;
}

extern DSUInteger DSVariablePoolIndexOfVariableWithName(const DSVariablePool *pool, const char *name)
{
        DSUInteger index = DSVariablePoolNumberOfVariables(pool);
        DSUInteger i;
        DSVariable *variable = NULL;
        if (pool == NULL) {
                DSError(M_DS_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (name == NULL) {
                DSError(M_DS_WRONG ": Name of variable is NULL", A_DS_ERROR);
                goto bail;
        }
        if (strlen(name) == 0) {
                DSError(M_DS_WRONG ": Name of variable is empty", A_DS_WARN);
                goto bail;                
        }
        if (DSVariablePoolHasVariableWithName(pool, name) == false) {
                DSError(M_DS_WRONG ": Variable does not contain desired variable", A_DS_ERROR);
                goto bail;
        }
        variable = DSVariablePoolVariableWithName(pool, name);
        for (i = 0; i < DSVariablePoolNumberOfVariables(pool); i++)
                if (variable == DSVariablePoolAllVariables(pool)[i])
                        break;
        index = i;
bail:
        return index;       
}

extern DSVariablePool * DSVariablePoolByParsingString(const char *string)
{
        DSVariablePool *pool = NULL;
        void *parser = NULL;
        struct variable_token *tokens, *current;
        double value = 0.0;
        if (string == NULL) {
                DSError(M_DS_WRONG ": String to parse is NULL", A_DS_ERROR);
                goto bail;
        }
        if (strlen(string) == 0) {
                DSError(M_DS_WRONG ": String to parse is empty", A_DS_WARN);
                goto bail;                
        }
        tokens = DSVariablePoolTokenizeString(string);
        if (tokens == NULL) {
                DSError(M_DS_PARSE ": Token stream is NULL", A_DS_ERROR);
                goto bail;
        }
        pool = DSVariablePoolAlloc();
        parser = DSVariablePoolParserAlloc(DSSecureMalloc);
        current = tokens;
        while (current != NULL) {
                switch (DSVariableTokenType(current)) {
                        case DS_VARIABLE_TOKEN_DOUBLE:
                                DSVariablePoolParser(parser, 
                                                     DS_VARIABLE_TOKEN_DOUBLE, 
                                                     &(current->data.value),
                                                     pool);
                                break;
                        case DS_VARIABLE_TOKEN_ID:
                                DSVariablePoolParser(parser, 
                                                     DS_VARIABLE_TOKEN_ID, 
                                                     (double *)DSVariableTokenString(current),
                                                     pool);
                                break;
                        case DS_VARIABLE_TOKEN_ASSIGN:
                                value = 0.0;
                                DSVariablePoolParser(parser, 
                                                     DS_VARIABLE_TOKEN_ASSIGN,
                                                     &value,
                                                     pool);
                                break;
                        case DS_VARIABLE_TOKEN_SEPERATOR:
                                value = 0.0;
                                DSVariablePoolParser(parser, 
                                                     DS_VARIABLE_TOKEN_SEPERATOR,
                                                     &value,
                                                     pool);
                                break;
                        default:
                                break;
                }
                current = DSVariableTokenNext(current);
        }
        DSVariablePoolParser(parser, 
                             0, 
                             &value,
                             pool);
        DSVariablePoolParserFree(parser, DSSecureFree);
        DSVariableTokenFree(tokens);
bail:
        return pool;
}


extern void DSVariablePoolPrint(const DSVariablePool * const pool)
{
        if (pool == NULL) {
                DSError(M_DS_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        dsPrintVarDictionary(DSVariablePoolInternalDictionary(pool), 0);
bail:
        return;
}




