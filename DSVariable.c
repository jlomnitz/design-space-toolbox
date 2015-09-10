/**
 * \file DSVariable.c
 * \brief Implementation file with functions for dealing with variables.
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


#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "DSMemoryManager.h"
#include "DSErrors.h"
#include "DSVariable.h"
#include "DSVariableTokenizer.h"
#include "DSMatrix.h"

#define dsVarDictionarySetValue(x, y)  (((x) != NULL) ? x->value = (y) : DSError(M_DS_WRONG ": Dictionary is NULL", A_DS_ERROR))

#define dsVarDictionaryValue(x)        (((x) != NULL) ? (x)->value : NULL)

#define dsVariablePoolNumberOfVariables(x) ((x)->numberOfVariables)
#define dsVariablePoolThreadLock(x) ((x)->thread_lock)

#define dsVariableThreadLock(x)     ((x)->thread_lock)

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
        DSVariable *var = NULL;
        if (name == NULL) {
                DSError(M_DS_NULL ": Name is a NULL pointer", A_DS_ERROR);
                goto bail;
        }
        if (strlen(name) == 0) {
                DSError(M_DS_WRONG ": Name is empty", A_DS_WARN);
                goto bail;
        }
        var = DSSecureMalloc(sizeof(DSVariable));
        DSVariableName(var) = strdup(name);
        var->retainCount = 1;
        DSVariableSetValue(var, INFINITY);
        pthread_mutex_init(&dsVariableThreadLock(var), NULL);
bail:
        return var;
}

/**
 * \brief Function frees allocated memory of a DSVariable.
 *
 * This function should not be used explicitly, as the DSVariable object has an 
 * internal memory counter. This function is ultimately called when the variable
 * memory counter reaches zero. Freeing a DSVariable object should be done
 * through the DSVariableRelease function, and never should a DSVariable be 
 * directly freed, as its internal structure may be subject to future changes.
 *
 * \param var The pointer to the variable to free.
 *
 * \see DSVariableRetain()
 * \see DSVariableRelease()
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
                DSError(M_DS_WRONG ": Variable name is NULL", A_DS_WARN);
        pthread_mutex_destroy(&dsVariableThreadLock(var));
        DSSecureFree(var);
bail:
        return;
}

extern void DSVariablePrint(const DSVariable *var)
{
        int (*print)(const char *, ...) = DSPrintf;
        if (var == NULL) {
                DSError(M_DS_VAR_NULL ": Variable is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSPrintf == NULL) {
                print = printf;
        }
        print("%s\t%lf", DSVariableName(var), DSVariableReturnValue(var));
bail:
        return;
}

/**
 * \brief Function to increase variable retain count by one.
 *
 * Variables utilize a similar memory management system used in 
 * Objective-C NSObject subclasses. A DSVariable recently allocated begins
 * with a retain count of one.
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
        pthread_mutex_lock(&dsVariableThreadLock(aVariable));
        aVariable->retainCount++;
        pthread_mutex_unlock(&dsVariableThreadLock(aVariable));
bail:
        return aVariable;
}

/**
 * \brief Function to decrease variable retain count by one.
 *
 * DSVariable object is made to decrease its retain count by one, when the
 * retain count hits zero, the function DSVariableFree() is invoked, freeing the 
 * memory of the DSVariable object. DSVariable objects do not have an
 * equivalent to autorelease, forcing the developer to invoke a DSRelease for each
 * DSRetain explicitly called.
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
        pthread_mutex_lock(&dsVariableThreadLock(aVariable));
        aVariable->retainCount--;
        pthread_mutex_unlock(&dsVariableThreadLock(aVariable));
        if (aVariable->retainCount == 0)
                DSVariableFree(aVariable);
bail:
        return;
}

#if defined(__APPLE__) && defined(__MACH__)
#pragma mark - Variable Pool Functions
#endif

/**
 * \brief Creates a new DSVariablePool with an empty var dictionary.
 * 
 * The variable pool is initialized with read/write privilages.  The variable
 * pool stores a indexed version of the variables added, as well as the order in
 * which the variables were added.  The order of the variables is kept to ensure
 * a consistent variable index with system matrices of S-Systems and GMAs.
 *
 *
 * \return The pointer to the allocated DSVariablePool.
 *
 * \see DSVariablePoolFree
 */
extern DSVariablePool * DSVariablePoolAlloc(void)
{
        DSVariablePool *pool = NULL;
        pool = DSSecureCalloc(1, sizeof(DSVariablePool));
        DSVariablePoolInternalDictionary(pool) = DSDictionaryAlloc();
        DSVariablePoolSetReadWriteAdd(pool);
        pthread_mutex_init(&dsVariablePoolThreadLock(pool), NULL);
        return pool;
}

/**
 * \brief Creates a new DSVariablePool with a copy of the reference variable 
 *        pool.
 *
 * The variable pool that is created is initialized with the same read/write/add
 * priviliges as the reference variable pool.  The contents of the variable pool
 * are an exact copy of the reference variable pool. Despite the contents being
 * the same, the variables in each pool are independent, thus new variables are
 * created in the copy.
 *
 * \param reference A DSVariablePool data type that serves as the reference 
 *                  variable pool, which is to be copied.
 *
 * \return The copy of the reference DSVariablePool object (must be freed by 
 *         user).
 *
 * \see DSVariablePoolFree()
 */
extern DSVariablePool * DSVariablePoolCopy(const DSVariablePool * const reference)
{
        DSUInteger i;
        DSVariablePool * copy = NULL;
        const DSVariable * const * allVariables = NULL;
        if (reference == NULL) {
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        copy = DSVariablePoolAlloc();
        allVariables = DSVariablePoolAllVariables(reference);
        for (i = 0; i < DSVariablePoolNumberOfVariables(reference); i++) {
                DSVariablePoolAddVariableWithName(copy, DSVariableName((DSVariable *)(allVariables[i])));
                DSVariablePoolSetValueForVariableWithName(copy, DSVariableName((DSVariable *)(allVariables[i])), DSVariableValue((DSVariable *)(allVariables[i])));
        }
bail:
        return copy;
}

/**
 * \brief Creates a new DSVariablePool with a copy of the reference variable 
 *        pool.
 *
 * The variable pool that is created is initialized with the same read/write/add
 * priviliges as the reference variable pool.  The contents of the variable pool
 * are an exact copy of the reference variable pool. Despite the contents being
 * the same, the variables in each pool are independent, thus new variables are
 * created in the copy.
 *
 * \param reference A DSVariablePool data type that serves as the reference 
 *                  variable pool, which is to be copied.
 *
 * \return The copy of the reference DSVariablePool object (must be freed by 
 *         user).
 *
 * \see DSVariablePoolFree()
 */
extern void DSVariablePoolFree(DSVariablePool *pool)
{
        if (pool == NULL) {
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolIsReadWriteAdd(pool) == false) {
                DSError(M_DS_VAR_LOCKED, A_DS_ERROR);
                goto bail;
        }
        pthread_mutex_lock(&dsVariablePoolThreadLock(pool));
        DSDictionaryFreeWithFunction(DSVariablePoolInternalDictionary(pool), DSVariableRelease);
        if (DSVariablePoolVariableArray(pool) != NULL) {
                DSSecureFree(DSVariablePoolVariableArray(pool));
        }
        pthread_mutex_unlock(&dsVariablePoolThreadLock(pool));
        pthread_mutex_destroy(&dsVariablePoolThreadLock(pool));
        DSSecureFree(pool);
bail:
        return;
}

/**
 * \brief Changes the existing priviliges of a DSVariablePool object to read
 *        only.
 *
 * This function acts on an existing DSVariablePool object, and changes the
 * existing priviliges to read-only. This provilige setting prohibits adding
 * new variables to the variable pool, or changing the value of a variable
 * explictly. The value of a variable can be changed directly, but not through
 * the variable pool interface.
 *
 * \param pool A DSVariablePool object that will have its priviliges changed.
 *
 * \see DSVariablePoolSetReadWrite()
 * \see DSVariablePoolSetReadWriteAdd()
 * \see DSVariablePoolLock
 */
extern void DSVariablePoolSetReadOnly(DSVariablePool * pool)
{
        if (pool == NULL) {
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        pool->lock = DSLockReadOnly;
bail:
        return;
}

/**
 * \brief Changes the existing priviliges of a DSVariablePool object to read and
 *        write.
 *
 * This function acts on an existing DSVariablePool object, and changes its
 * priviliges to read and write. This provilige setting prohibits adding
 * new variables to the variable pool. The value of a variable can be changed 
 * through the variable pool interface.
 *
 * \param pool A DSVariablePool object that will have its priviliges changed.
 *
 * \see DSVariablePoolSetReadOnly()
 * \see DSVariablePoolSetReadWriteAdd()
 * \see DSVariablePoolLock
 */
extern void DSVariablePoolSetReadWrite(DSVariablePool * pool)
{
        if (pool == NULL) {
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        pool->lock = DSLockReadWrite;
bail:
        return;
}

/**
 * \brief Changes the existing priviliges of a DSVariablePool object to read,
 *        write and add.
 *
 * This function acts on an existing DSVariablePool object, and changes its
 * priviliges to read, write and add. This provilige setting allows adding
 * new variables to the variable pool and changing the values of the variables.
 *
 * \param pool A DSVariablePool object that will have its priviliges changed.
 *
 * \see DSVariablePoolSetReadOnly()
 * \see DSVariablePoolSetReadWrite()
 * \see DSVariablePoolLock
 */
extern void DSVariablePoolSetReadWriteAdd(DSVariablePool * pool)
{
        if (pool == NULL) {
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        pool->lock = DSLockReadWriteAdd;
bail:
        return;
}

/**
 * \brief Function to retrieve the number of variables in a DSVariablePool.
 *
 * \param pool A DSVariablePool object that to query its number of variables.
 */
extern DSUInteger DSVariablePoolNumberOfVariables(const DSVariablePool *pool)
{
        DSUInteger numberOfVariables = 0;
        if (pool == NULL) {
                DSError(M_DS_VAR_NULL, A_DS_ERROR);
                goto bail;
        }
        numberOfVariables = pool->numberOfVariables;
bail:
        return numberOfVariables;
}

/**
 * \brief Queries the existing priviliges of a DSVariablePool object, checking
 *        it is read only.
 *
 * This function acts on an existing DSVariablePool object, and checks if its
 * priviliges are read only.
 *
 * \param pool A DSVariablePool object to be queried for its priviliges.
 *
 * \see DSVariablePoolIsReadWrite()
 * \see DSVariablePoolIsReadWriteAdd()
 * \see DSVariablePoolLock
 */
extern bool DSVariablePoolIsReadOnly(const DSVariablePool *pool)
{
        bool readOnly = false;
        if (pool == NULL) {
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        readOnly = (pool->lock == DSLockReadOnly);
bail:
        return readOnly;
}

/**
 * \brief Queries the existing priviliges of a DSVariablePool object, checking
 *        it is read and write.
 *
 * This function acts on an existing DSVariablePool object, and checks if its
 * priviliges are read and write.
 *
 * \param pool A DSVariablePool object to be queried for its priviliges.
 *
 * \see DSVariablePoolIsReadOnly()
 * \see DSVariablePoolIsReadWriteAdd()
 * \see DSVariablePoolLock
 */
extern bool DSVariablePoolIsReadWrite(const DSVariablePool *pool)
{
        bool readWrite = false;
        if (pool == NULL) {
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        readWrite = (pool->lock == DSLockReadWrite);
bail:
        return readWrite;
}

/**
 * \brief Queries the existing priviliges of a DSVariablePool object, checking
 *        it is read, write and add.
 *
 * This function acts on an existing DSVariablePool object, and checks if its
 * priviliges are read, write and add.
 *
 * \param pool A DSVariablePool object to be queried for its priviliges.
 *
 * \see DSVariablePoolIsReadOnly()
 * \see DSVariablePoolIsReadWrite()
 * \see DSVariablePoolLock
 */
extern bool DSVariablePoolIsReadWriteAdd(const DSVariablePool *pool)
{
        bool readWriteAdd = false;
        if (pool == NULL) {
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        readWriteAdd = (pool->lock == DSLockReadWriteAdd);
bail:
        return readWriteAdd;
}

/**
 * \brief Creates and adds a new variable to the variable pool.
 *
 * This function acts on an existing DSVariablePool object, creating a new
 * variable with a specified name and adding it to the internal dictionary
 * structure. If a variable already exists with the same name, this function
 * does not create a new variable, and throws a warning.
 *
 * \param pool The DSVariablePool object to which a new variable will be added.
 * \param name A null terminated string with the name of the variable to add.
 *
 */
extern void DSVariablePoolAddVariableWithName(DSVariablePool *pool, const char * name)
{
        DSVariable *var = NULL;
//        bool varIsNew = true;
        if (pool == NULL) {
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolIsReadWriteAdd(pool) == false) {
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
        if (DSDictionaryValueForName(DSVariablePoolInternalDictionary(pool), name) != NULL) {
                DSError(M_DS_WRONG ": Variable pool has variable with same name", A_DS_WARN);
                goto bail;
        }
        var = DSVariableAlloc(name);
        DSDictionaryAddValueWithName(DSVariablePoolInternalDictionary(pool),
                                                                              name, var);
        if (DSVariablePoolNumberOfVariables(pool) == 0)
                DSVariablePoolVariableArray(pool) = DSSecureCalloc(DSVariablePoolNumberOfVariables(pool)+1, sizeof(DSVariable *));
        else
                DSVariablePoolVariableArray(pool) = DSSecureRealloc(DSVariablePoolVariableArray(pool), (dsVariablePoolNumberOfVariables(pool)+1)*sizeof(DSVariable *));
        DSVariablePoolVariableArray(pool)[dsVariablePoolNumberOfVariables(pool)++] = var;
bail:
        return;
}

/**
 * \brief Adds an existing variable to the variable pool.
 *
 * This function acts on an existing DSVariablePool object, adding an existing
 * variable with a specified name to the internal dictionary structure. The 
 * variable added is not created, but this function calls DSVariableRetain, thus
 * increasing the memory retain count of the variable by one. 
 * If a variable already exists with the same name, this function
 * does not add the variable to the pool, and throws a warning.
 *
 * \param pool The DSVariablePool object to which a new variable will be added.
 * \param name A null terminated string with the name of the variable to add.
 *
 * \see DSVariablePoolAddVariableWithName()
 * \see DSVariableRetain()
 */
extern void DSVariablePoolAddVariable(DSVariablePool * pool, DSVariable *newVar)
{
        DSVariable * var = NULL;
        if (pool == NULL) {
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolIsReadWriteAdd(pool) == false) {
                DSError(M_DS_VAR_LOCKED, A_DS_ERROR);
                goto bail;
        }
        if (newVar == NULL) {
                DSError(M_DS_WRONG ": Variable is NULL", A_DS_ERROR);
                goto bail;
        }
        DSDictionaryAddValueWithName(DSVariablePoolInternalDictionary(pool),
                                                                              DSVariableName(newVar), newVar);
        var = DSVariablePoolVariableWithName(pool, DSVariableName(newVar));
        if (var == newVar) {
                if (DSVariablePoolNumberOfVariables(pool) == 0)
                        DSVariablePoolVariableArray(pool) = DSSecureCalloc(DSVariablePoolNumberOfVariables(pool)+1, sizeof(DSVariable *));
                else
                        DSVariablePoolVariableArray(pool) = DSSecureRealloc(DSVariablePoolVariableArray(pool), (DSVariablePoolNumberOfVariables(pool)+1)*sizeof(DSVariable *));
                DSVariablePoolVariableArray(pool)[dsVariablePoolNumberOfVariables(pool)++] = var; 
        }
bail:
        return;
}


extern double DSVariablePoolValueForVariableWithName(const DSVariablePool *pool, const char *name)
{
        DSVariable * var = NULL;
        double value = -INFINITY;
        if (pool == NULL) {
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolIsReadWriteAdd(pool) == false) {
                DSError(M_DS_VAR_LOCKED, A_DS_ERROR);
                goto bail;
        }
        var = DSVariablePoolVariableWithName(pool, name);
        if (var == NULL) {
                DSError(M_DS_WRONG ": Variable Pool does not have variable with given name", A_DS_WARN);
                goto bail;
        }
        value = DSVariableValue(var);
bail:
        return value;
}

/**
 * \brief Checks if a DSVariablePool has a variable with a specified name.
 */
extern bool DSVariablePoolHasVariableWithName(const DSVariablePool *pool, const char * name)
{
        bool hasVariable = false;
        if (pool == NULL) {
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
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
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
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
        variable = DSDictionaryValueForName(DSVariablePoolInternalDictionary(pool), name);
bail:
        return variable;
}

extern const DSVariable * DSVariablePoolVariableAtIndex(const DSVariablePool *pool, const DSUInteger index)
{
        const DSVariable * variable = NULL;
        if (pool == NULL) {
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (index >= DSVariablePoolNumberOfVariables(pool)) {
                DSError(M_DS_WRONG ": Index of variable out of bounds", A_DS_ERROR);
                goto bail;
        }
        variable = DSVariablePoolAllVariables(pool)[index];
bail:
        return variable;
}

extern void DSVariablePoolSetValueForVariableWithName(const DSVariablePool *pool, const char *name, const double value)
{
        DSVariable *variable = NULL;
        if (pool == NULL) {
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolIsReadOnly(pool) == true) {
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
                DSError(M_DS_WRONG ": Variable pool does not have variable", A_DS_ERROR);
                goto bail;
        }
        variable = DSVariablePoolVariableWithName(pool, name);
        DSVariableSetValue(variable, value);
bail:
        return;
}

extern void DSVariablePoolCopyVariablesFromVariablePool(DSVariablePool *to_add, const DSVariablePool *source)
{
        DSUInteger i;
        char * name;
        if (to_add == NULL) {
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (source == NULL) {
                goto bail;
        }
        if (DSVariablePoolIsReadOnly(to_add) == true) {
                DSError(M_DS_VAR_LOCKED, A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < DSVariablePoolNumberOfVariables(source); i++) {
                name = DSVariableName(DSVariablePoolVariableAtIndex(source, i));
                if (DSVariablePoolHasVariableWithName(to_add, name) == true)
                        continue;
                DSVariablePoolAddVariableWithName(to_add, name);
        }
bail:
        return;
}


extern const DSVariable * * DSVariablePoolAllVariables(const DSVariablePool *pool)
{
        const DSVariable ** allVariables = NULL;
        if (pool == NULL) {
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
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
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        if (var == NULL) {
                DSError(M_DS_WRONG ": Variable is NULL", A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolHasVariableWithName(pool, DSVariableName(var)) == false) {
                DSError(M_DS_WRONG ": Variable pool does not have variable", A_DS_ERROR);
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
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
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
                DSError(M_DS_WRONG ": Variable pool does not have variable", A_DS_WARN);
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
                DSError(M_DS_VAR_NULL ": Variable Pool is NULL", A_DS_ERROR);
                goto bail;
        }
        DSDictionaryPrintWithFunction(DSVariablePoolInternalDictionary(pool), DSVariablePrint);
bail:
        return;
}

extern DSMatrix * DSVariablePoolValuesAsVector(const DSVariablePool *pool, const bool rowVector)
{
        DSMatrix *matrix = NULL;
        DSUInteger i;
        if (pool == NULL) {
                DSError(M_DS_VAR_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(pool) == 0) {
                DSError(M_DS_WRONG ": Variable pool is empty", A_DS_ERROR);
                goto bail;
        }
        if (rowVector == true)
                matrix = DSMatrixAlloc(1, DSVariablePoolNumberOfVariables(pool));
        else
                matrix = DSMatrixAlloc(DSVariablePoolNumberOfVariables(pool), 1);
        for (i=0; i< DSVariablePoolNumberOfVariables(pool); i++) {
                DSMatrixSetDoubleValue(matrix, (rowVector == false)*i,
                                       (rowVector == true)*i,
                                       DSVariableValue(DSVariablePoolVariableArray(pool)[i]));
        }
bail:
        return matrix;
}

extern DSUInteger * DSVariablePoolIndicesOfSubPool(const DSVariablePool * superPool, const DSVariablePool * subPool)
{
        DSUInteger i, count;
        const char * name;
        DSUInteger * indices = NULL;
        if (superPool == NULL) {
                DSError(M_DS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (subPool == NULL) {
                DSError(M_DS_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSVariablePoolNumberOfVariables(superPool) == 0 ||
            DSVariablePoolNumberOfVariables(subPool) == 0) {
                DSError(M_DS_WRONG ": Variable pool is empty", A_DS_ERROR);
                goto bail;
        }
        count = DSVariablePoolNumberOfVariables(subPool);
        indices = DSSecureMalloc(sizeof(DSUInteger)*count);
        for (i = 0; i < count; i++) {
                name = DSVariableName(DSVariablePoolVariableAtIndex(subPool, i));
                if (DSVariablePoolHasVariableWithName(superPool, name) == false) {
                        indices[i] = count;
                } else {
                        indices[i] = DSVariablePoolIndexOfVariableWithName(superPool, name);
                }
        }
bail:
        return indices;
}


