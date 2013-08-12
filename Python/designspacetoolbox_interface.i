%module designspacetoolbox_test

%{
#define SWIG_FILE_WITH_INIT
#include <DSStd.h>
#include <DSTypes.h>
#include <DSCase.h>
%}

/* Type Map Data */
%typemap(in) char ** {
        /* Check if is a list */
        if (PyList_Check($input)) {
                int size = PyList_Size($input);
                int i = 0;
                $1 = (char **) malloc((size+1)*sizeof(char *));
                for (i = 0; i < size; i++) {
                        PyObject *o = PyList_GetItem($input,i);
                        if (PyString_Check(o))
                        $1[i] = PyString_AsString(PyList_GetItem($input,i));
                        else {
                                PyErr_SetString(PyExc_TypeError,"list must contain strings");
                                free($1);
                                return NULL;
                        }
                }
                $1[i] = 0;
        } else {
                PyErr_SetString(PyExc_TypeError,"not a list");
                return NULL;
        }
}

%typemap(out) char * {
        if ($1 == NULL) {
                return NULL;
        }
        $result = PyString_FromFormat("%s", $1);
        DSSecureFree($1);
}

%typemap(out) const char * {
        if ($1 == NULL) {
                return NULL;
        }
        $result = PyString_FromFormat("%s", $1);
}

%typemap(out) DSUInteger {
        $result = PyInt_FromLong((unsigned long)$1);
}

%typemap(in) DSUInteger {
        $1 = (DSUInteger) PyLong_AsUnsignedLongMask($input);
}

%typemap(out) const DSVariable * {
        DSVariable * variable = NULL;
        variable = $1;
        PyObject * list = NULL, *pString, *pFloat;
        if (variable == NULL) {
                $result = NULL;
                return;
        }
        list = PyList_New(2);
        pString = PyString_FromFormat("%s", DSVariableName(variable));
        pFloat = PyFloat_FromDouble(DSVariableValue(variable));
        PyList_SetItem(list, 0, pString);
        PyList_SetItem(list, 1, pFloat);
        $result = list;
}

%typemap(out) DSVertices *  {
        DSUInteger i, j;
        PyObject *tuple = NULL;
        DSVertices * vertices = $1;
        if (vertices == NULL) {
                $result = NULL;
                return;
        }
        $result = PyList_New(vertices->numberOfVertices);
        for (i = 0; i < vertices->numberOfVertices; i++) {
                tuple = PyTuple_New(vertices->dimensions);
                for (j = 0; j < vertices->dimensions; j++) {
                        PyTuple_SetItem(tuple, j, PyFloat_FromDouble(DSVerticesVertexAtIndex(vertices, i)[j]));
                }
                PyList_SetItem($result, i, tuple);
        }
        DSVerticesFree(vertices);
}

%typemap(out) DSMatrix * {
        DSUInteger i, j;
        PyObject *tuple = NULL;
        DSMatrix *matrix = $1;
        if (matrix == NULL) {
                $result = NULL;
                return;
        }
        $result = PyList_New(DSMatrixRows(matrix));
        for (i = 0; i < DSMatrixRows(matrix); i++) {
                tuple = PyTuple_New(DSMatrixColumns(matrix));
                for (j = 0; j < DSMatrixColumns(matrix); j++) {
                        PyTuple_SetItem(tuple, j, PyFloat_FromDouble(DSMatrixDoubleValue(matrix, i, j)));
                }
                PyList_SetItem($result, i, tuple);
        }
        DSMatrixFree(matrix);
}

%typemap(in) const DSCase ** {
        /* Check if is a list */
        if (PyList_Check($input)) {
                int size = PyList_Size($input);
                int i = 0;
                $1 = (const DSCase **) malloc((size+1)*sizeof(char *));
                for (i = 0; i < size; i++) {
                        PyObject *o = PyList_GetItem($input,i);
                        if (SWIG_ConvertPtr(o, &($1[i]), $descriptor(DSCase *), SWIG_POINTER_EXCEPTION) == -1) {
                                PyErr_SetString(PyExc_TypeError,"list must contain DSCase objects");
                                free($1);
                                return NULL;
                        }
                }
                $1[i] = 0;
        } else {
                PyErr_SetString(PyExc_TypeError,"not a list");
                return NULL;
        }
}
/**
 * DSVariablePool functions available to the internal python module.
 */
extern DSUInteger DSVariablePoolNumberOfVariables(const DSVariablePool *pool);
extern DSVariablePool * DSVariablePoolAlloc(void);
extern DSVariablePool * DSVariablePoolCopy(const DSVariablePool * const pool);
extern void DSVariablePoolFree(DSVariablePool *pool);
extern void DSVariablePoolAddVariableWithName(DSVariablePool *pool, const char * name);
extern void DSVariablePoolSetValueForVariableWithName(const DSVariablePool *pool, const char *name, const double value);
extern bool DSVariablePoolHasVariableWithName(const DSVariablePool *pool, const char * const name);
extern double DSVariablePoolValueForVariableWithName(const DSVariablePool *pool, const char *const name);
extern DSUInteger DSVariablePoolIndexOfVariableWithName(const DSVariablePool *pool, const char *name);
extern void DSVariablePoolPrint(const DSVariablePool * const pool);
extern DSMatrix * DSVariablePoolValuesAsVector(const DSVariablePool *pool, const bool rowVector);

extern const DSVariable * DSVariablePoolVariableAtIndex(const DSVariablePool *pool, const DSUInteger index);
extern void DSVariablePoolSetReadWrite(DSVariablePool *pool);
extern void DSVariablePoolSetReadWriteAdd(DSVariablePool *pool);


/**
 * DSDesignSpace functions available to internal python module.
 */
extern DSDesignSpace * DSDesignSpaceByParsingStrings(char * const * const strings, const DSVariablePool * const Xd_a, const DSUInteger numberOfEquations);
void DSDesignSpaceFree(DSDesignSpace * ds);

extern const DSDictionary * DSDesignSpaceSubcaseDictionary(const DSDesignSpace *ds);

extern void DSDesignSpacePrint(const DSDesignSpace * ds);
extern void DSDesignSpaceCalculateValidityOfCases(DSDesignSpace *ds);

//extern const DSVariablePool * DSDesignSpaceXd(const DSDesignSpace *ds);
extern const DSVariablePool * DSDesignSpaceXi(const DSDesignSpace *ds);

extern const DSUInteger DSDesignSpaceNumberOfEquations(const DSDesignSpace *ds);
extern DSExpression ** DSDesignSpaceEquations(const DSDesignSpace *ds);
extern const DSUInteger * DSDesignSpaceSignature(const DSDesignSpace *ds);

extern const DSUInteger DSDesignSpaceNumberOfValidCases(const DSDesignSpace *ds);
extern const DSUInteger DSDesignSpaceNumberOfCases(const DSDesignSpace *ds);

extern DSCase * DSDesignSpaceCaseWithCaseNumber(const DSDesignSpace * ds, const DSUInteger caseNumber);
extern DSCase * DSDesignSpaceCaseWithCaseSignature(const DSDesignSpace * ds, const DSUInteger * signature);

extern const bool DSDesignSpaceCaseWithCaseNumberIsValid(const DSDesignSpace *ds, const DSUInteger caseNumber);
extern const bool DSDesignSpaceCaseWithCaseSignatureIsValid(const DSDesignSpace *ds, const DSUInteger * signature);

//extern const DSStack * DSDesignSpaceSubcasesForCaseNumber(DSDesignSpace *ds, const DSUInteger caseNumber);
//extern const DSGMASystem * DSDesignSpaceGMASystem(const DSDesignSpace * ds);

extern DSCase ** DSDesignSpaceCalculateAllValidCases(DSDesignSpace *ds);
extern DSDictionary * DSDesignSpaceCalculateAllValidCasesForSlice(DSDesignSpace *ds, const DSVariablePool *lower, const DSVariablePool *upper);

extern void DSDesignSpaceCalculateUnderdeterminedCases(DSDesignSpace *ds);
/**
 * DSSSystem functions available to internal python module
 */

extern DSSSystem * DSSSystemByRemovingAlgebraicConstraints(const DSSSystem * originalSSystem);


extern double DSSSystemSteadyStateFunction(const DSSSystem *ssys, const DSVariablePool *Xi0, const char * function);
extern DSMatrix * DSSSystemSteadyStateValues(const DSSSystem *ssys, const DSVariablePool *Xi0);
extern DSMatrix * DSSSystemSteadyStateFlux(const DSSSystem *ssys, const DSVariablePool *Xi0);
extern DSUInteger DSSSystemRouthIndex(const DSSSystem *ssys, const DSVariablePool *Xi0);
extern DSUInteger DSSSystemCharacteristicEquationCoefficientIndex(const DSSSystem *ssys, const DSVariablePool *Xi0);
extern DSMatrix * DSSSystemRouthArray(const DSSSystem *ssys, const DSVariablePool *Xi0);

extern void DSSSystemPrint(DSSSystem * ssys);
extern void DSSSystemPrintEquations(DSSSystem * ssys);
extern void DSSSystemPrintSolution(DSSSystem * ssys);
extern void DSSSystemPrintLogarithmicSolution(DSSSystem *ssys);

extern void DSSSystemFree(DSSSystem * ssys);

extern const DSVariablePool * DSSSystemXd(const DSSSystem * const ssys);
extern const DSVariablePool * DSSSystemXi(const DSSSystem * const ssys);

/**
 * DSCase functions available to internal python module.
 */

extern void DSCaseFree(DSCase *aCase);
extern void DSCasePrint(const DSCase *aCase);
extern DSExpression ** DSCaseEquations(const DSCase *aCase);

extern DSExpression ** DSCaseSolution(const DSCase *aCase);
extern DSExpression ** DSCaseLogarithmicSolution(const DSCase *aCase);

extern DSExpression ** DSCaseConditions(const DSCase *aCase);
extern DSExpression ** DSCaseLogarithmicConditions(const DSCase *aCase);

extern DSExpression ** DSCaseBoundaries(const DSCase *aCase);
extern DSExpression ** DSCaseLogarithmicBoundaries(const DSCase *aCase);
extern DSUInteger DSCaseNumberOfEquations(const DSCase *aCase);
extern DSUInteger DSCaseNumberOfConditions(const DSCase *aCase);

extern DSVertices * DSCaseVerticesFor2DSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable);
extern DSVertices * DSCaseVerticesFor1DSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable);
extern DSVertices * DSCaseBoundingRangeForVariable(const DSCase *aCase, const char * variable);
extern DSVertices * DSCaseBoundingRangeForVariableWithConstraints(const DSCase *aCase, const char * variable, DSVariablePool * lowerBounds, DSVariablePool * upperBounds);

extern DSUInteger DSCaseNumber(const DSCase * aCase);
extern const DSUInteger * DSCaseSignature(const DSCase * aCase);
extern char * DSCaseSignatureToString(const DSCase *aCase);
extern const DSSSystem * DSCaseSSystem(const DSCase *aCase);

extern const bool DSCaseIntersectionIsValid(const DSUInteger numberOfCases, const DSCase **cases);
extern const bool DSCaseIntersectionIsValidAtSlice(const DSUInteger numberOfCases, const DSCase **cases,  const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds);
extern const bool DSCaseIsValidAtPoint(const DSCase *aCase, const DSVariablePool * variablesToFix);

extern DSVertices * DSCaseIntersectionVerticesForSlice(const DSUInteger numberOfCases, const DSCase **cases, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const DSUInteger numberOfVariables, const char ** variables);

extern const bool DSCaseIntersectionExceptSliceIsValid(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames);
extern const bool DSCaseIntersectionExceptSliceIsValidAtSlice(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames, const DSVariablePool * lowerBounds, const DSVariablePool * upperBounds);
extern DSVariablePool * DSCaseIntersectionExceptSliceValidParameterSet(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames);
extern DSVariablePool * DSCaseIntersectionExceptSliceValidParameterSetAtSlice(const DSUInteger numberOfCases, const DSCase **cases, const DSUInteger numberOfExceptions, const char ** exceptionVarNames, const DSVariablePool * lowerBounds, const DSVariablePool * upperBounds);

extern double DSCaseLogarithmicGain(const DSCase *aCase, const char *XdName, const char *XiName);

extern DSVariablePool * DSCaseValidParameterSet(const DSCase *aCase);
extern DSVariablePool * DSCaseValidParameterSetAtSlice(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds);

extern const DSGMASystem * DSDesignSpaceGMASystem(const DSDesignSpace * ds);

/**
 * DSGMASystem functions available to internal python module
 */

extern DSGMASystem * DSGMASystemByParsingStrings(char * const * const strings, const DSVariablePool * const Xd_a, const DSUInteger numberOfEquations);
extern void DSGMASystemFree(DSGMASystem * gma);

extern const DSUInteger DSGMASystemNumberOfEquations(const DSGMASystem *gma);
extern DSExpression ** DSGMASystemEquations(const DSGMASystem *gma);

extern const DSVariablePool *DSGMASystemXd(const DSGMASystem *gma);
extern const DSVariablePool *DSGMASystemXi(const DSGMASystem *gma);

/**
* Subcases
*/
//extern DSSubcase * DSSubcaseForCaseInDesignSpace(const DSDesignSpace * ds, const DSCase * aCase);
//extern void DSSubcaseFree(DSSubcase * aSubcase);
//
//extern const bool DSSubcaseIsValid(const DSSubcase *aSubcase);
//extern const bool DSSubcaseIsValidAtSlice(const DSSubcase *aSubcase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds);
//
//extern DSDictionary * DSSubcaseVerticesFor2DSlice(const DSSubcase *aSubcase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds, const char * xVariable, const char *yVariable);
//
//extern const DSDesignSpace * DSSubcaseInternalDesignSpace(const DSSubcase * subcase);
/**
 * Dictionary Functions
 */

extern DSDictionary * DSDictionaryAlloc(void);
extern void DSDictionaryFree(DSDictionary * aDictionary);
extern void DSDictionaryFreeWithFunction(DSDictionary * aDictionary, void * freeFunction);

extern DSUInteger DSDictionaryCount(const DSDictionary *aDictionary);
extern void * DSDictionaryValueForName(const DSDictionary *dictionary, const char *name);

extern const char ** DSDictionaryNames(const DSDictionary *aDictionary);

extern void DSDictionaryAddValueWithName(DSDictionary *dictionary, const char * name, void *value);


/**
 * Vertices 
 */
//extern void DSVerticesFree(DSVertices *vertices);

//extern const bool DSVerticesAddVertex(DSVertices *vertices, const double * coordinates);

//extern const bool DSVerticesAreEqual(const DSVertices *vert1, const DSVertices *vert2);

//extern const double * DSVerticesVertexAtIndex(const DSVertices *vertices, const DSUInteger index);

#if defined(__APPLE__) && defined(__MACH__)
#pragma mark - Utility functions
#endif


extern void DSSecureFree(void * ptr);
extern DSDictionary * DSDictionaryFromArray(void * array, DSUInteger size);

extern DSExpression * DSExpressionByParsingString(const char *string);
extern double DSExpressionEvaluateWithVariablePool(const DSExpression *expression, const DSVariablePool *pool);
extern char * DSExpressionAsString(const DSExpression *expression);
extern void DSExpressionFree(DSExpression *expression);

//extern DSCase * DSSWIGVoidAsCase(void * ptr);

//extern void * DSSWIGDesignSpaceParseWrapper(const DSVariablePool * const Xd, char ** const strings, const DSUInteger numberOfEquations);
%inline %{

//extern DSSubcase * DSSWIGVoidAsSubcase(void *ptr)
//{
//        return ptr;
//}

extern DSCase * DSSWIGVoidAsCase(void * ptr)
{
        return ptr;
}
        
extern DSExpression * DSSWIGVoidAsExpression(void * ptr)
{
        return ptr;
}
        
extern DSDesignSpace * DSSWIGDesignSpaceParseWrapper(char ** const strings, const DSUInteger numberOfEquations, char ** Xd_list, const DSUInteger numberOfXd)
{
        DSUInteger i;
        DSVariablePool * Xd = DSVariablePoolAlloc();
        for (i = 0; i < numberOfXd; i++) {
                DSVariablePoolAddVariableWithName(Xd, Xd_list[i]);
        }
        DSDesignSpace * ds = DSDesignSpaceByParsingStrings(strings, Xd, numberOfEquations);
        DSVariablePoolFree(Xd);
        return ds;
}
%}


#define VERSION "0.01.1"