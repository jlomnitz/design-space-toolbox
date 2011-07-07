/**
 * \file DSVariable.c
 * \brief Implementation file with functions for dealing with variables.
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


#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "DSMemoryManager.h"
#include "DSErrors.h"
#include "DSVariable.h"


static DSVariable *_checkNameAtPos(const char *name, DSVariablePool *root, int pos);
static DSVariablePool *_addNewBranch(DSVariable *var, int atPos);

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
extern DSVariable *DSVariableWithName(const char *name, DSVariablePool *root)
{
        if (name == NULL || root == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
bail:
        return _checkNameAtPos(name, root, 0);
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
                DSError(M_DS_NULL, A_DS_WARN);
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
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        aVariable->retainCount--;
        if (aVariable->retainCount == 0)
                DSVariableFree(aVariable);
bail:
        return;
}

/**
 * \brief Internal function for searching a dictionary for a variable name.
 *
 *This function is for internal use only.  It is a RECURSIVE function which 
 *checks the difference between the name and the dictionary character by 
 * character.  If it encounters a matching Null character in both the dictionar
 * and the name, a pointer to the DSVariable is returned.
 * \param name A string, the one passed to DSVariableWithName, with the name of the DSVariable.
 * \param node The node of the dictionary being compared.
 * \param pos The position of the character in the name string being compared with the node.
 * \return A pointer to the DSVariable.  If the name name does not match, NULL is returned.
 * \see DSVariable
 * \see DSVariableWithName
 * \see DSVariablePool
 * \see DSVariablePoolAddVariable
 */
static DSVariable *_checkNameAtPos(const char *name, DSVariablePool *node, int pos)
{
        DSVariable *aVariable = NULL;
        if (node == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
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
extern DSVariablePool *DSVariablePoolAddVariable(DSVariable *newVar, DSVariablePool *root)
{
        int pos;
        DSVariablePool *current, *previous, *temp, *top;
        
        bool changeRoot;
        if (newVar == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
                goto bail;
        }
        if (root == NULL) {
                root = _addNewBranch(newVar, 0);
                goto bail;
        }
        if (_checkNameAtPos(newVar->name, root, 0) != NULL) {
                DSError(M_DS_EXISTS, A_DS_WARN);
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
                        temp = _addNewBranch(newVar, pos);
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
                        current->alt = _addNewBranch(newVar, pos);
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

/**
 * \brief Creates a simple branch containing the remaining nodes for the DSVariable.
 *
 * This internal function is used to create all the nodes not currently existing
 * in the dictionary.  This function is auxiliary to DSVariablePoolAddVariable and should
 * not be used outside of that function.
 * \param var The DSVariable being added to the dictionary.
 * \param atPos The position of the name string of the DSVariable at which to start making the new nodes.
 *
 * \return The pointer to the root of the newly formed branch.
 *
 * \see DSVariable
 * \see DSVariablePoolAddVariable
 */
static DSVariablePool *_addNewBranch(DSVariable *var, int atPos)
{
        DSVariablePool *root = NULL, *current;
        if (var == NULL) {
                DSError(M_DS_NULL, A_DS_WARN);
        }
        const char *name;
        root = DSSecureCalloc(sizeof(DSVariablePool), 1);
        current = root;
        name = var->name;
        while(name[atPos] != '\0') {
                current->current = name[atPos++];
                current->next = DSSecureCalloc(sizeof(DSVariablePool), 1);
                current = current->next;
        }
        current->current = '\0';
        current->variable = DSVariableRetain(var);
        return root;
}

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
extern DSVariable *DSNewVariable(const char *name)
{
        DSVariable *var;
        if (name == NULL)
                return NULL;
        if (strlen(name) == 0)
                return NULL;
        var = DSSecureMalloc(sizeof(DSVariable));
        var->name = strdup(name);
        var->retainCount = 1;
        DSVariableAssignValue(var, INFINITY);
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
        if (var == NULL)
                return;
        if (var->name != NULL)
                free(var->name);
        free(var);
}

#if defined(__APPLE__) && defined(__MACH__)
#pragma mark - Variable Pool Functions
#endif
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
extern void DSVariablePoolFree(DSVariablePool *root)
{
        if (root == NULL) {
                goto bail;
        }
        DSVariablePoolFree(root->alt);
        DSVariablePoolFree(root->next);
        root->alt = NULL;
        root->next = NULL;
        if (root->variable != NULL)
                DSVariableRelease(root->variable);
        free(root);
bail:
        return;
}

/**
 * \brief Prints the VarDictionary.
 *
 * A debugging function which prints the dictionary structure to the stderr file
 */
extern void printVarDictionary(DSVariablePool *root, int indent)
{
        if (root == NULL)
                return;
        printf("\t");
        if (root->current == '\0')
                fprintf(stderr, "\\0");
        else
                fprintf(stderr, "%c", root->current);
        printVarDictionary(root->next, indent);
        fprintf(stderr, "\n");
        printVarDictionary(root->alt, indent+1);
}

/**
 * \brief Function that prints the members in the dictionary.
 * 
 * This function recursively
 * checks the dictionary for all names, and prints them out.  It requires of
 * a buffer string which is used to store the current characters, since it is
 * a recursive function.
 * \param root The root of the dictionary, and the current node for consequent calls.
 * \param buffer The string in which to store the characters, function does not check size of buffer.
 * \param position Should be 0 when called, increases as it searches the dictionary.
 * \see _varDictionary
 * \see DSVariablePoolAddVariable
 * \todo Create a wrapper function which creates the buffer, and initiates the position at 0.
 */
extern void printMembers(DSVariablePool *root, char *buffer, int position)
{
        if (root == NULL)
                return;
        buffer[position] = root->current;
        if (root->current == '\0')
                printf("%s\n", buffer);
        printMembers(root->next, buffer, position+1);
        printMembers(root->alt, buffer, position);
}


