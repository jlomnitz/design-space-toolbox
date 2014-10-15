/**
 * \file DSTypes.h
 * \brief Header file with definitions for data types.
 *
 * \details 
 * This file specifies the design space standard data types. Contained here are
 * strictly the data type definitions.  Functions applying to these data types
 * are contained elsewhere, and the individual data structures should refer
 * to the respective files.
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

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>
#include <complex.h>
#include <pthread.h>

#ifndef __DS_TYPES__
#define __DS_TYPES__

#ifndef __BEGIN_DECLS
#define __BEGIN_DECLS extern "C" {
#endif

#ifndef __END_DECLS
#define }
#endif

#ifndef __deprecated
#define __deprecated
#endif

#ifndef INFINITY
#define INFINITY HUGE_VAL
#endif

#ifndef __DS_DESIGN_SPACE_VERSION__
#define __DS_DESIGN_SPACE_VERSION__   "0.3.0a0"
#endif

#ifdef __cplusplus
__BEGIN_DECLS
#endif

typedef int DSInteger;

typedef unsigned int DSUInteger;


/**
 * \brief Data type that contains vertices of an N-Dimensional object.
 *
 * \details This data type is used ofr determining the region of validity of a
 * case in design space.  If the vertices represent a polygon, they can be
 * orderd according to their clockwise position, starting by the right-most
 * vertex in a XY plane.
 *
 * \see DSVertices.h
 * \see DSVertices.c
 */
typedef struct {
        double **vertices;
        DSUInteger dimensions;
        DSUInteger numberOfVertices;
} DSVertices;
 
/**
 * \brief Basic variable structure containing name, value and NSString with
 *        special unicode characters for greek letters.
 *
 * Structure that carries variable information.  Internal to BSTVariables class
 * and should not be created and/or freed manually and beyond the context of
 * the BSTVariables class.
 *
 * \see DSVariable.h
 * \see DSVariable.c
 */
typedef struct {
        char *name;             //!< Dynamically allocated name of the variable.
        double value;           //!< Value of the variable.
        DSUInteger retainCount; //!< Retain counter for memory management.
        pthread_mutex_t thread_lock; //!< A mutex to ensure proper retain/release behavior.
} DSVariable;

/**
 * \brief Internal dictionary structure.
 *
 * Internal dictionary for fast variable querying.  The structure of the 
 * dictionary uses an alternative path, where each character is checked in order
 * at each position, if there is a match, the next position is consequently
 * checked.  The dictionary should never be manipulated manually, adding, 
 * retrieving and removing variables should be done through the accesory 
 * functions.
 *
 * \todo Implement a tree balancing algorithm to make variable search more
 *       efficient, such as the red and black tree.
 * \see DSDictionary
 */
typedef struct _varDictionary
{
        char current;                  //!< The current character in the dictionary.
        struct _varDictionary *lower;  //!< The alternative character in the dictionary that is less than the current.
        struct _varDictionary *higher; //!< The alternative character in the dictionary that is greater than current.
        struct _varDictionary *next;   //!< The next character in the dictionary.
        void *value;                   //!< The variable stored. Only when current is '\\0'.
} DSInternalDictionary;

/**
 * \brief Dictionary object with C strings for keys and pointers for values.
 *
 * \details A dictionary structure with strings for keys and pointers for values.
 *          The dictionary uses a tree structure to minimize un-used memory usage
 *          as look-up time is not expected to be a problem. The dictionary is
 *          ordered by maintaining an ordered array of pointers.
 *
 * \see DSDictionary.h
 * \see DSDictionary.c
 */
typedef struct {
        DSInternalDictionary *internal; //!< The pointer to the internal ternary tree root.
        DSUInteger count;               //!< The number of objects in the dictionary.
        char ** names;                  //!< A standard C array with all the names in the dictionary.
        void (*freeFunction)(void *);   //!< Free function (Not yet implemented)
        pthread_mutex_t lock;           //!< A mutex lock to make the dictionary thread safe.
} DSDictionary;

typedef struct {
        void ** base;                   //!< The pointer to the array of DSUIntegers storing the case numbers.
        void ** current;                //!< A pointer to the top of the stack.
        DSUInteger count;               //!< The number of elements in the stack.
        DSUInteger size;                //!< The current size of the base array.
        pthread_mutex_t pushpop;        //!< The mutex used when pushing and popping data from the stack.
} DSStack;

/**
 * \brief Data type used to lock different properties of the DSVariablePool.
 *
 * This data type enumerates the properties of the variable pool access rights.
 * Its values indicate the different operations that can be taken with a variable
 * pool, such as read/write/add, read/write and read.
 *
 * \see DSVariable.h
 * \see DSVariable.c
 */
typedef enum {
        DSLockReadWriteAdd, //!< The value of the Variable pool lock indicating read/write/add
        DSLockReadWrite,    //!< The value of the Variable pool lock indicating read/write
        DSLockReadOnly,     //!< The value of the Variable pool lock indicating read/
        DSLockLocked        //!< The value of the Variable pool lock indicating no access
} DSVariablePoolLock;

/**
 * \brief User-level variable pool.
 *
 * \details This data type keeps an internal dictionary structure of type
 * struct _varDictionary to keep track of all the variables associated with a
 * variable pool.  This data type also records the number of variables in the
 * dictionary and the order with which they were added.
 *
 * \see struct _varDictionary
 *
 * \see DSVariable.h
 * \see DSVariable.c
 */
typedef struct
{
        DSDictionary * dictionary;      //!< The dictionary with the variables arranged.
        DSUInteger numberOfVariables;   //!< Number of variables in the pool.
        DSVariable **variables;         //!< A C array with the variables stored.
        DSVariablePoolLock lock;        //!< Indicates if the variable pool is read-only.
        pthread_mutex_t thread_lock;    //!< A mutex that locks access to the variable pool.
} DSVariablePool;

/**
 * \brief Data type representing mathematical expressions.
 *
 * \details This data type is the internal representation of matematical 
 * expressions.  This data type is an Abstracts Syntax Tree with only
 * three operators: '+', '*' and '^'.  All other operators ('-' and '/') are
 * represented by a combination of the former operators.  The DSExpression 
 * automatically groups constant values, and reserves the first branch of the
 * multiplication and addition operator for constant values.  These operators
 * can have any number of branches.  The '^' operator can have two, and only two,
 * branches.
 *
 * \note Functions are handled as variables with a single argument
 *
 * \see DSExpression.h
 * \see DSExpression.c
 */
typedef struct dsexpression {
        union {
                char op_code;
                double constant;
                char *variable;          //!< A string with the name of the variable
        } node;                          //!< Union of data types potentially contained in the node.
        int type;                        //!< Integer specifying the type of node.
        int numberOfBranches;            //!< Number of branches of children, relevant to operators and functions.
        struct dsexpression **branches;  //!< Array of expression nodes with children nodes.
} DSExpression;

/**
 * \brief Data type representing a symbolic matrix.
 *
 * \details This data type is the front end of the matric manipulation portion
 * of the design space toolbox involving symbolic data..  Currently, the DST 
 * library has a very limited manipulation of symbolic libraries, and is used
 * exclusive to parse gma equations and design spaces.  When performing any
 * analysis of design space, the symbolic matrices are converted to numerical
 * expressions.
 *
 * \see DSMatrix.h
 * \see DSMatrix.c
 */
typedef struct {
        DSExpression *** mat;
        DSUInteger rows;
        DSUInteger columns;
} DSSymbolicMatrix;

/**
 * \brief Data type representing a matrix.
 *
 * \details This data type is the front end of the matric manipulation portion
 * of the design space toolbox.  Currently, the DST library uses the gsl library; 
 * however, it is designed to be used with different back-ends.  In particular, 
 * the CLAPACK package should be considered, as it will offer better performance.
 * Thus, the matrix API should be independent of implementation, and hence
 * a new matrix library could be used if chosen.
 *
 * \see DSMatrix.h
 * \see DSMatrix.c
 */
typedef struct {
        void *mat;          //!< The pointer to the internal representation of the matrix.
        DSUInteger rows;    //!< A DSUInteger specifying the number of rows in the matrix.
        DSUInteger columns; //!< A DSUInteger specifying the number of columns in the matrix.
} DSMatrix;


/**
 * \brief Data type representing a matrix with complex values.
 *
 * \details This data type is the front end of the matric manipulation portion
 * of the design space toolbox.  Currently, the DST library uses the gsl library; 
 * however, it is designed to be used with different back-ends.  In particular, 
 * the CLAPACK package should be considered, as it will offer better performance.
 * Thus, the matrix API should be independent of implementation, and hence
 * a new matrix library could be used if chosen.
 *
 * \see DSComplexMatrix.h
 * \see DSComplexMatrix.c
 */
typedef DSMatrix DSComplexMatrix;

/**
 * \brief Data type representing an array of matrices.
 *
 * \details This data type is a utility data type that keeps track of arrays of
 * matrices.  This structure is used to represent three-dimensional 
 * matrices, as used internally by GMA's systems.
 *
 * \see DSMatrixArray.h
 * \see DSMatrixArray.c
 */
typedef struct {
        DSUInteger numberOfMatrices; //!< A DSUInteger specifying the number of matrices in the array.
        DSMatrix **matrices;         //!< A pointer the the C-style array of matrices.
} DSMatrixArray;


/**
 * \brief Data type representing a GMA-System.
 *
 * \details
 * This data structure is a standard representation of an GMA using 
 * matrix notation.  Here, the positive and negative terms are explicitly
 * represented according to the Gs and Hs.  Also, matrices are split up 
 * relating to either dependent and independent parameters.  The GMA system
 * uses an array of matrices to represent all the terms in all of the equations.
 *
 */
typedef struct {
        DSMatrix *alpha;
        DSMatrix *beta;
        DSMatrixArray *Gd;
        DSMatrixArray *Gi;
        DSMatrixArray *Hd;
        DSMatrixArray *Hi;
        DSVariablePool *Xd;
        DSVariablePool *Xd_a;     //!< A pointer to the DSVariablePool with the algebraic dependent variables.
        DSVariablePool *Xd_t;
        DSVariablePool *Xi;
        DSUInteger *signature;
} DSGMASystem;

/**
 * \brief Data type representing an S-System.
 *
 * \details This data structure is a standard representation of an S-System
 * using matrix notation.  Here, the positive and negative terms are explicitly
 * represented according to the Gs and Hs.  Also, matrices are split up 
 * relating to either dependent and independent parameters.
 *
 */
typedef struct {
        DSMatrix *alpha;
        DSMatrix *beta;
        DSMatrix *Gd;
        DSMatrix *Gi;
        DSMatrix *Hd;
        DSMatrix *Hi;
        DSMatrix *M;
        DSVariablePool *Xd;
        DSVariablePool *Xd_t;
        DSVariablePool *Xd_a;      //!< A pointer to the DSVariablePool with the algebraic dependent variables.
        DSVariablePool *Xi;
        bool isSingular;
        bool shouldFreeXd;
        bool shouldFreeXi;
        DSDictionary * fluxDictionary;
} DSSSystem;

/**
 * \brief Data type used to represent a case.
 *
 * \details This data type has all the necessary information for a case in
 * design space.  It has a pointer to the dependent and independent variables of
 * the system, a pointer to the corresponding S-System, the Condition matrices
 * and boundary matrices.  It also has information about the case number and
 * case signature.
 *
 * \note The case number is arbitrary, and can be generated by two algorithms to
 * be either big endian or small endian.  For compatibility with the current
 * design space toolbox, big endian is the default.
 *
 * \note The case is not responsible for freeing the Xd and Xi data structures.
 * If the case is generated from a design space, then the design space is
 * responsible for freeing the Xi and Xd variable pools; otherwise the internal
 * S-System is responsible for freeing this data.
 */
typedef struct {
        const DSVariablePool *Xd;         //!< A pointer to the DSVariablePool with the dependent variables.
        const DSVariablePool *Xd_a;       //!< A pointer to the DSVariablePool with the algebraic dependent variables.
        const DSVariablePool *Xi;         //!< A pointer to the DSVariablePool with the independent variables.
        DSSSystem *ssys;                  //!< The DSSSystem of the case.
        DSMatrix *Cd;                     //!< The condition matrix corresponding to the dependent variables.
        DSMatrix *Ci;                     //!< The condition matrix corresponding to the independent variables.
        DSMatrix *U;                      //!< The boundary matrix corresponding to the independent variables.
        DSMatrix *delta;                  //!< The condition matrix corresponding to the constants.
        DSMatrix *zeta;                   //!< The boundary matrix corresponding to the constants.
        DSUInteger caseNumber;            //!< The case number used to identify the case. [will be deprecated]
        DSUInteger *signature;            //!< The case signature indicating the dominant terms used to generate the case.
        char * caseIdentifier;      //!< A case identifier used to identify cases and subcases [will replace case number].
} DSCase;


typedef DSCase DSPseudoCase;

typedef struct {
//        DSExpression *** fluxEquations;
//        DSUInteger * numberOfFluxes;
//        DSUInteger ** fluxIndex;
//        DSUInteger * cycleVariables;
//        DSUInteger numberCycles;
//        DSDictionary * cycleFluxes;
        char ** extendedEquations;
} DSCycleExtensionData;


/**
 * \brief Data type used to represent a design space
 *
 * \details The design space data structure is a convenience structure that
 * automates the construction and analysis of cases, and manages the memory
 * associated with these cases.  This behavior can be avoided by working
 * directly with the gma system of the designspace.
 *
 * \see DSDesignSpace.h
 * \see DSDesignSpace.c
 */
typedef struct {
        DSGMASystem *gma;                //!< The gma system of the design space.
        const DSVariablePool *Xd;        //!< A pointer to the DSVariablePool with the dependent variables.
        const DSVariablePool *Xd_a; //!< A pointer to the DSVariablePool with the algebraic dependent variables.
        const DSVariablePool *Xi;        //!< A pointer to the DSVariablePool with the dependent variables.
        DSDictionary * validCases;       //!< DSVariablePool with case number that are valid.
        DSUInteger numberOfCases;        //!< DSUInteger indicating the maximum number of cases in the design space.
        DSMatrix * Cd, *Ci, *delta;      //!< Condition matrices.
        DSDictionary *cyclicalCases;     //!< DSDictionary containing design space objects with subcases.
        DSMatrix * Rn;                   //!< Matrix Used to calculate the coefficients of the characteristic equations using the method of underdetermined coefficients.
        bool seriesCalculations;
        DSDictionary * cycleFluxes;
        DSCycleExtensionData * extensionData;
        char * casePrefix;
//        DSUInteger numberOfCycles;
//        DSUInteger ** fluxSources;
} DSDesignSpace;

/**
 * \brief Data type used to represent a subcase.
 *
 * \details The subcase data structure is a structure that represents an 
 * underdetermined system that arises by creating cycles while picking dominant
 * terms, this creates an artificial singularity that would not exist in the 
 * original system. The data structure contains a design space object that has
 * all the individual subcases.
 *
 * \note This data structure will change in the forseable future, as it does not
 * associate the newly generated equations with the appropriate dependent
 * variable, which leads to the flux through a pool to be calculated incorrectly.
 *
 * \see DSCyclicalCase.h
 * \see DSCyclicalCase.c
 */
typedef struct{
//        DSDesignSpace * internal;
        DSDesignSpace ** internalDesignspaces;
        DSDesignSpace * internalDesignspace;
        DSUInteger numberOfInternal;
        DSCase * originalCase;
        DSUInteger caseNumber;
} DSCyclicalCase;


#ifdef __cplusplus
__END_DECLS
#endif

#endif



