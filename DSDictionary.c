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
#include <pthread.h>

#include "DSMemoryManager.h"
#include "DSErrors.h"
#include "DSDictionary.h"

/**
 *\defgroup DS_DICTIONARY_ACCESSORY Macros to manipulate dictionary nodes.
 *
 * \details The following macros are in place for portability and consistency.
 * As the structure of the DSDictionary is subject to changes, using these
 * macros will make the dependent code less subject to errors.
 */
/*\{*/

/**
 * \brief Macro to set the value of a DSDictionary node.
 *
 * \details This macro provides a consistent way for changing the value of a
 * dictionary node, despite the internal structure of the data type.  
 * This macro is expanded to a simple assignment.
 */
#define dsInternalDictionarySetValue(x, y)  ((x != NULL) ? x->value = y : DSError(M_DS_WRONG ": Dictionary is NULL", A_DS_ERROR))

/**
 * \brief Macro to get the value of a dictionary node.
 *
 * \details This macro provides a consistent way for retrieving the value of a
 * DSDictionary node, despite changes in the internal structure of the data type.
 */
#define dsInternalDictionaryValue(x)        ((x != NULL) ? x->value : NULL)

/*\}*/

#if defined(__APPLE__) && defined(__MACH__)
#pragma mark - Internal dictionary functions
#endif

/**
 * \brief Function for searching a dictionary for a variable name.
 *
 * This function is for internal use only.  It is a RECURSIVE function which 
 * checks the difference between the name and the dictionary character by 
 * character.  If it encounters a matching Null character in both the dictionar
 * and the name, a pointer to the DSVariable is returned.
 *
 * \param name A string, the one passed to DSVariableWithName, with the name of the DSVariable.
 * \param node The node of the dictionary being compared.
 * \param pos The position of the character in the name string being compared with the node.
 *
 * \return A pointer to the DSVariable.  If the name name does not match, NULL is returned.
 *
 * \see DSVariable
 * \see DSVariableWithName
 * \see struct _varDictionary
 * \see DSVariablePoolAddVariable
 */
extern void *DSDictionaryValueForName(const DSDictionary *dictionary, const char *name)
{
        DSVariable *value = NULL;
        DSUInteger position = 0;
        if (name == NULL) {
                DSError(M_DS_NULL ": Name is a NULL pointer", A_DS_ERROR);
                goto bail;
        }
        if (dictionary == NULL) {
                DSError(M_DS_NULL ": Dictionary is empty", A_DS_WARN);
                goto bail;
        }
        while (dictionary) {
                if (name[position] == dictionary->current) {
                        if (dictionary->current == '\0') {
                                value = dsInternalDictionaryValue(dictionary);
                                break;
                        }
                        dictionary = dictionary->next;
                        position++;
                } else if (name[position] == '\0') {
                        dictionary = dictionary->alt;
                } else if (name[position] < dictionary->current) {
                        break;
                }
                else if (name[position] > dictionary->current) {
                        dictionary = dictionary->alt;
                }
        }
bail:
        return value;
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
static DSDictionary *dsDictionaryBranchAlloc(void *var, const char *name, int atPos)
{
        struct _varDictionary *root = NULL, *current;
        if (var == NULL) {
                DSError(M_DS_NULL ": Variable being added is NULL", A_DS_WARN);
                goto bail;
        }
        root = DSSecureCalloc(sizeof(struct _varDictionary), 1);
        current = root;
        while(name[atPos] != '\0') {
                current->current = name[atPos++];
                current->next = DSSecureCalloc(sizeof(struct _varDictionary), 1);
                current = current->next;
        }
        current->current = '\0';
        dsInternalDictionarySetValue(current, var);
bail:
        return root;
}


/**
 * \brief Adds an object to the dictionary.
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
extern DSDictionary * DSDictionaryAddValueWithName(DSDictionary *root, const char * name, void *value)
{
        int pos;
        struct _varDictionary *current, *previous, *temp, *top;
        
        bool changeRoot;
        char errorMessage[100];
        
        if (value == NULL) {
                DSError(M_DS_WRONG ": value to add is NULL", A_DS_ERROR);
                goto bail;
        }
        if (root == NULL) {
                root = dsDictionaryBranchAlloc(value, name, 0);
                goto bail;
        }
        if (DSDictionaryValueForName(root, name) != NULL) {
                sprintf(errorMessage, "%.30s: Dictionary has entry with name \"%.10s\"", M_DS_EXISTS, name);
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
                if (name[pos] < current->current || current->current == '\0') {
                        temp = dsDictionaryBranchAlloc(value, name, pos);
                        temp->alt = current;
                        if (changeRoot == true)
                                root = temp;
                        else if (previous == NULL) {
                                top->next = temp;
                        }
                        else
                                previous->alt = temp;
                        break;
                } else if (current->current == name[pos]) {
                        top = current;
                        changeRoot = false;
                        previous = NULL;
                        current = current->next;
                        pos++;
                } else if (current->alt == NULL) {
                        current->alt = dsDictionaryBranchAlloc(value, name, pos);
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

///**
// * \brief Adds a DSVariable to the dictionary.
// *
// * This is the function to add DSVariables to
// * the dictionary.  Since the dictionary works alphabetically, the root of the 
// * dictionary changes and is defined by this function, and the root, be it the
// * new one or the old one, is returned.  This function is used in creating a new dictionary.
// * \param newVar The pointer to the variable which is to be added to the dictionary.
// * \param root The root of the dictionary. THE ROOT MAY BE REASSIGNED. If null, creates a new dictionary.
// * \return The root of the dictionary, which is likely to change.
// * \see _varDictionary
// * \see DSVariableWithName
// * \see _addNewBranch
// */
//static struct _varDictionary *dsVarDictionaryAddVariableWithName(const char * name, struct _varDictionary *root)
//{
//        DSVariable *newVar = NULL;
//        if (name == NULL) {
//                DSError(M_DS_WRONG ": Name is a NULL pointer", A_DS_ERROR);
//                goto bail;
//        }
//        if (strlen(name) == 0) {
//                DSError(M_DS_WRONG ": Name string is empty", A_DS_WARN);
//                goto bail;
//        }
//        newVar = DSVariableAlloc(name);
//        root = dsVarDictionaryAddVariable(newVar, root);
//        DSVariableRelease(newVar);
//bail:
//        return root;
//}

/**
 * \brief Function to remove all nodes in the dictionary.
 *
 * The function RECURSIVELY frees all the nodes past the root which is passed.
 * This function could be used to clear a portion of the dictionary, yet doing
 * so would require direct manipulation of the dictionary which is strongly 
 * discouraged.  The values themselves are NOT freed. To free the values of a
 * dictionary, please see DSDictionaryFreeWithFunction().
 *
 * \param dictionary The root of the dictionary which is to be freed.
 *
 * \see DSDictionaryFreeWithFunction()
 */
extern void DSDictionaryFree(DSDictionary * dictionary)
{
        if (dictionary == NULL) {
                DSError(M_DS_DICTIONARY_NULL, A_DS_ERROR);
                goto bail;
        }
        DSDictionaryFreeWithFunction(dictionary, NULL);
bail:
        return;
}
/**
 * \brief Function to remove all nodes in the dictionary.
 *
 * The function RECURSIVELY frees all the nodes past the root which is passed.
 * This function could be used to clear a portion of the dictionary, yet doing
 * so would require direct manipulation of the dictionary which is strongly 
 * discouraged.  The values themselves are freed by passing a function that is
 * called at each node with the value of the dictionary, unless the free 
 * function passed is NULL, at which point the data is not freed.
 *
 * \param dictionary The root of the dictionary which is to be freed.
 *
 * \see DSDictionaryFree()
 */
extern void DSDictionaryFreeWithFunction(DSDictionary * dictionary, void * freeFunction)
{
        void (*Function)(void *);
        if (dictionary == NULL) {
                DSError(M_DS_DICTIONARY_NULL, A_DS_ERROR);
                goto bail;
        }
        DSDictionaryFreeWithFunction(dictionary->alt, freeFunction);
        DSDictionaryFreeWithFunction(dictionary->next, freeFunction);
        dictionary->alt = NULL;
        dictionary->next = NULL;
        if (freeFunction != NULL) {
                Function = freeFunction;
                Function(dsInternalDictionaryValue(dictionary));
        }
        DSSecureFree(dictionary);
bail:
        return;
}

/**
 * \brief Prints the VarDictionary.
 *
 * A debugging function which prints the dictionary structure to the stderr file
 */
static void dsDictionaryPrintInternalWithFunction(const DSDictionary *dictionary, const void *printFunction, const DSUInteger position)
{
        DSUInteger i;
        int (*print)(const char *,...) = DSPrintf;
        int (*printObject)(void *);
        if (dictionary == NULL) {
                DSError(M_DS_DICTIONARY_NULL, A_DS_ERROR);
                goto bail;
        }
        printObject = printFunction;
        if (print == NULL)
                print = printf;
        for (i = 1; i < position+1; i++)
                print(" ");
        if (dictionary->current == '\0') {
                print("+-");
                printObject(dsInternalDictionaryValue(dictionary));
                print("\n");
        } else {
                print("+-%c\n", dictionary->current);
        }
        dsDictionaryPrintInternalWithFunction(dictionary->next, printFunction, position+2);
        dsDictionaryPrintInternalWithFunction(dictionary->alt, printFunction, position);
bail:
        return;
}

/**
 * \brief Prints the VarDictionary.
 *
 * A debugging function which prints the dictionary structure to the stderr file
 */
static void dsDictionaryPrintInternal(const DSDictionary *dictionary, const DSUInteger position)
{
        DSUInteger i;
        int (*print)(const char *, ...) = DSPrintf;
        if (dictionary == NULL) {
                DSError(M_DS_DICTIONARY_NULL, A_DS_ERROR);
                goto bail;
        }
        if (print == NULL)
                print = printf;
        for (i = 1; i < position+1; i++)
                print(" ");
        if (dictionary->current == '\0')
                print("+-[%p]\n", dsInternalDictionaryValue(dictionary));
        else
                print("+-%c\n", dictionary->current);
        dsDictionaryPrintInternal(dictionary->next, position+2);
        dsDictionaryPrintInternal(dictionary->alt, position);
bail:
        return;
}


extern void DSDictionaryPrint(const DSDictionary *dictionary)
{
        if (dictionary == NULL) {
                DSError(M_DS_DICTIONARY_NULL, A_DS_ERROR);
                goto bail;
        } 
        dsDictionaryPrintInternal(dictionary, 0);
bail:
        return;
}

extern void DSDictionaryPrintWithFunction(const DSDictionary *dictionary, const void * printFunction)
{
        if (dictionary == NULL) {
                DSError(M_DS_DICTIONARY_NULL, A_DS_ERROR);
                goto bail;
        }
        if (printFunction == NULL) {
                DSDictionaryPrint(dictionary);
                goto bail;
        }
        dsDictionaryPrintInternalWithFunction(dictionary, printFunction, 0);
bail:
        return;        
}

/**
 * \brief Function that prints the members in the dictionary.
 * 
 * This function recursively checks the dictionary for all names, and prints
 * them out.  It requires of a buffer string which is used to store the current
 * characters, since it is a recursive function.
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


#if defined(__APPLE__) && defined(__MACH__)
#pragma mark - Allocation and freeing
#endif
extern DSDictionary * DSDictionaryInitialize()
{
        DSDictionary * dictionary = NULL;
        return dictionary;
}


