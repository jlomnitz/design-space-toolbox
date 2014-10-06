/**
 * \file DSIO.c
 * \brief Implementation file with standard input and output functions.
 *
 * \details
 *
 * Copyright (C) 2011 Jason Lomnitz.
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
 */

#include <stdio.h>
#include <string.h>
#include "DSIO.h"
#include "DSMemoryManager.h"
#include "DSVariable.h"
#include "DSMatrix.h"
#include "DSMatrixArray.h"
#include "DSGMASystem.h"
#include "DSSSystem.h"
#include "DSCase.h"


/**
 * \defgroup DS_IO_TAG_TYPES
 * \{
 */
#define DS_IO_TAG_TYPE_Matrix         "\"DSMatrix\""
#define DS_IO_TAG_TYPE_MatrixArray    "\"DSMatrixArray\""
#define DS_IO_TAG_TYPE_VariablePool   "\"DSVariablePool\""
#define DS_IO_TAG_TYPE_Dictionary     "\"DSDictionary\""
#define DS_IO_TAG_TYPE_SSystem        "\"DSSSystem\""
#define DS_IO_TAG_TYPE_Case           "\"DSCase\""
#define DS_IO_TAG_TYPE_DesignSpace    "\"DSDesignSpace\""
/**
 * \}
 */

int (*DSPrintf)(const char *, ...);
void (*DSPostWarning)(const char *message);
void (*DSPostError)(const char *message);
void (*DSPostFatalError)(const char *message);
FILE * DSIOErrorFile;


extern const char * DSDesignSpaceToolboxVersionString(void)
{
        return __DS_DESIGN_SPACE_VERSION__;
}


/**
 * \brief Variable with flags controlling S-System to JSON string conversion.
 *
 * \details This global variable is checked when converting a S-System structure
 * to a JSON string.  This variable will check several flags as specified by 
 * DS_SSYSTEM_JSON_OPTIONS.  The default value of the variable indicates that
 * all the properties will be included in the JSON string.
 *
 * \see DS_SSYSTEM_JSON_OPTIONS
 * \see DSIOSetSSystemJSONOptions()
 */
DSUInteger DSSSystemPrintingOptions;

/**
 * \brief Variable with flags controlling the conversion of a Case to a JSON 
 *        string.
 *
 * \details This global variable is checked when converting a Case structure
 * to a JSON string.  This variable will check several flags as specified by 
 * DS_CASE_JSON_OPTIONS.  The default value of the variable indicates that
 * all the properties will be included in the JSON string.
 *
 * \see DS_CASE_JSON_OPTIONS
 * \see DSIOSetCaseJSONOptions()
 */
DSUInteger DSCasePrintingOptions;

/**
 * \brief Function to assign default error file.
 *
 * \details This function is used to assign the default error file,
 * DSIOErrorFile.  Changing the error file should be done via this function,
 * as it circumvents potential problems associated with dynamic linking.
 *
 * \param aFile A FILE * that will be used to write error messages when the
 *        default error posting mechanism is used.
 *
 * \see DSIOSetPostWarningFunction
 * \see DSIOSetPostErrorFunction
 * \see DSIOSetPostFatalErrorFunction
 * \see DSError
 */
extern void DSIOSetErrorFile(FILE *aFile)
{
        DSIOErrorFile = aFile;
}

/**
 * \brief Function to assign default printf function.
 *
 * \details This function is used to assign the formated print function,
 * DSPrintf.  This function assigns the DSPrintf pointer to the function
 * that should be used to print formatted strings. This function MUST be used
 * to avoid problems relating to dynamic linking; by using this function the
 * global variable DSPrintf is loaded into memory prior to changing its value.
 *
 * \param printFunction A pointer to a function of the form
 *        int function(const char *, ...).  If NULL, default behavior is
 *        restored.
 *
 */
extern void DSIOSetPrintFunction(int (*printFunction)(const char *, ...))
{
        DSPrintf = printFunction;
}

/**
 * \brief Function to assign default warning posting function.
 *
 * \details This function is used to assign the function that handles the
 * warnings generated from the design space toolbox. Internally, it assigns
 * the global variable DSPostWarning which points to a function.
 *
 * \param warningFunction A pointer to a function of the form
 *        void function(const char *).  If NULL, default behavior is
 *        restored.
 *
 */
extern void DSIOSetPostWarningFunction(void (*warningFunction)(const char *message))
{
        DSPostWarning = warningFunction;
}

/**
 * \brief Function to assign default error posting function.
 *
 * \details This function is used to assign the function that handles the
 * errors generated from the design space toolbox. Internally, it assigns
 * the global variable DSPostError which points to a function.
 *
 * \param errorFunction A pointer to a function of the form
 *        void function(const char *).  If NULL, default behavior is
 *        restored.
 */
extern void DSIOSetPostErrorFunction(void (*errorFunction)(const char *message))
{
        DSPostError = errorFunction;
}

/**
 * \brief Function to assign default fatal error posting function.
 *
 * \details This function is used to assign the function that handles the fatal
 * errors generated from the design space toolbox. Internally, it assigns
 * the global variable DSPostFatalError which points to a function.
 *
 * \param errorFunction A pointer to a function of the form
 *        void function(const char *).  If NULL, default behavior is
 *        restored.
 */
extern void DSIOSetPostFatalErrorFunction(void (*fatalErrorFunction)(const char *message))
{
        DSPostFatalError = fatalErrorFunction;
}


#if defined(__APPLE__) && defined(__MACH__)
#pragma mark - Export objects to JSON format
#endif

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Functions controlling output
#endif

/**
 * \brief Function that sets the conversion options for a DSCase to JSON
 *        format.
 *
 * This function is used to overwrite the default export behavior of the DSCase
 * object.  The default behavior converts all of the data fields of the DSCase
 * into a JSON format, these options can be changed so the JSON conversion only
 * includes some fields, such as excluding the conditions, excluding the
 * S-System, etc.
 *
 * \param options A DSUInteger with the option flags, as specified by the 
 *                DSCase options.
 *
 * \see DS_CASE_JSON_OPTIONS
 */
extern void DSIOSetCaseJSONOptions(const DSUInteger options)
{
        if (options >= 32) {
                DSError(M_DS_WRONG ": Unrecognized options", A_DS_ERROR);
                goto bail;
        }
        DSCasePrintingOptions = options;
bail:
        return;
}

/**
 * \brief Function that sets the conversion options for a DSSSystem to JSON
 *        format.
 *
 * This function is used to overwrite the default export behavior of the DSSSystem
 * object.  The default behavior converts all of the data fields of the S-System
 * into a JSON format, these options can be changed so the JSON conversion only
 * includes some fields, such as excluding the solution.
 *
 * \param options A DSUInteger with the option flags, as specified by the 
 *                DSSSystem options.
 *
 * \see DS_SSYSTEM_JSON_OPTIONS
 */
extern void DSIOSetSSystemJSONOptions(const DSUInteger options)
{
        if (options >= 4) {
                DSError(M_DS_WRONG ": Unrecognized options", A_DS_ERROR);
                goto bail;
        }
        DSSSystemPrintingOptions = options;
bail:
        return;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark JSON data creators
#endif

/**
 * \brief Function to convert a DSVariablePool into a JSON formatted string.
 *
 * \details This function is used to convert a DSVariablePool into a JSON
 * object. The variables of the variable pool are stored as pairs of a string
 * and value.
 *
 * \param pool A DSVariablePool that will be used to create the JSON object.
 *
 * \return A C string with the JSON formatted data. If NULL, the conversion
 *         failed.
 */
extern char * DSVariablePoolStringInJSONFormat(const DSVariablePool *pool)
{
        char * string = NULL, *current, value[10000];
        DSUInteger i, length = 0;
        DSUInteger numDigits;
        if (pool == NULL) {
                DSError(M_DS_VAR_NULL, A_DS_ERROR);
                goto bail;
        }
        length = 100;
        string = DSSecureMalloc(sizeof(char)*length);
        sprintf(string, "{%s : {", DS_IO_TAG_TYPE_VariablePool);
        for (i = 0; i < DSVariablePoolNumberOfVariables(pool); i++) {
                numDigits = 14;
                current = DSVariableName(DSVariablePoolVariableArray(pool)[i]);
                if ((double)((DSInteger)DSVariableValue(DSVariablePoolVariableArray(pool)[i])) == DSVariableValue(DSVariablePoolVariableArray(pool)[i]))
                        numDigits = 0;
                sprintf(value, "%.*lg", numDigits, DSVariableValue(DSVariablePoolVariableArray(pool)[i]));
                if (strlen(string)+strlen(current)+strlen(value)+5 >= length) {
                        length += strlen(value)+100;
                        string = DSSecureRealloc(string, sizeof(char)*length);
                }
                strncat(string, "\"", length-strlen(string));
                strncat(string, current, length-strlen(string));
                strncat(string, "\":", length-strlen(string));
                if (DSVariableValue(DSVariablePoolVariableArray(pool)[i]) == INFINITY ||
                    DSVariableValue(DSVariablePoolVariableArray(pool)[i]) == -INFINITY) {
                        strncat(string, "\"", length-strlen(string));
                }
                strncat(string, value, length-strlen(string));
                if (DSVariableValue(DSVariablePoolVariableArray(pool)[i]) == INFINITY ||
                    DSVariableValue(DSVariablePoolVariableArray(pool)[i]) == -INFINITY) {
                        strncat(string, "\"", length-strlen(string));
                }
                if (i != DSVariablePoolNumberOfVariables(pool)-1)
                        strncat(string, ",", length-strlen(string));
        }
        strncat(string, "}}", length-strlen(string));
        string = DSSecureRealloc(string, sizeof(char)*(strlen(string)+1));
bail:
        return string;
}

/**
 * \brief Function to convert a DSMatrix into a JSON formatted string.
 *
 * \details This function is used to convert a DSMatrix into a JSON
 * object. The matrix is stored as an array of arrays.  The array of arrays
 * represents the rows of the matrix, whereas the arrays of value are the
 * values at the columns for a particular row.
 *
 * \param matrix A DSMatrix that will be used to create the JSON object.
 *
 * \return A C string with the JSON formatted data. If NULL, the conversion
 *         failed.
 */
extern char * DSMatrixStringInJSONFormat(const DSMatrix * matrix)
{
        char * string = NULL;
        char value[100];
        DSUInteger i, j, length = 0;
        DSUInteger numDigits;
        if (matrix == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        length = 1000;
        string = DSSecureMalloc(sizeof(char)*length);
        sprintf(string, "{%s:[", DS_IO_TAG_TYPE_Matrix);
        for (i = 0; i < DSMatrixRows(matrix); i++) {
                strncat(string, "[", length-strlen(string));
                for (j = 0; j < DSMatrixColumns(matrix); j++) {
                        numDigits = 14;
                        if ((double)((DSInteger)DSMatrixDoubleValue(matrix, i, j)) == DSMatrixDoubleValue(matrix, i, j))
                                numDigits = 0;
                        if (j == DSMatrixColumns(matrix)-1)
                                sprintf(value, "%.*lg", numDigits, DSMatrixDoubleValue(matrix, i, j));
                        else
                                sprintf(value, "%.*lg,", numDigits, DSMatrixDoubleValue(matrix, i, j));
                        if (strlen(string)+strlen(value) >= length) {
                                length += strlen(value)+1000;
                                string = DSSecureRealloc(string, sizeof(char)*length);
                        }
                        strncat(string, value, length-strlen(string));
                }
                if (i == DSMatrixRows(matrix)-1) 
                        strncat(string, "]", length-strlen(string));
                else
                        strncat(string, "],", length-strlen(string));
        }
        strncat(string, "]}", length-strlen(string));
        string = DSSecureRealloc(string, sizeof(char)*(strlen(string)+1));
bail:
        return string;
}

/**
 * \brief Function to convert a DSMatrixArray into a JSON formatted string.
 *
 * \details This function is used to convert a DSMatrix into a JSON
 * object. The matrix array is stored as an array of objects, where each object
 * is a DSMatrix.  The order of the DSMatrix object in the array represent the
 * order of matrices in the matrix array.
 *
 * \param array A DSMatrixArray that will be used to create the JSON object.
 *
 * \return A C string with the JSON formatted data. If NULL, the conversion
 *         failed.
 */
extern char * DSMatrixArrayStringInJSONFormat(const DSMatrixArray *array)
{
        char * string = NULL;
        char * temp;
        DSUInteger i, length = 0;
        if (array == NULL) {
                DSError(M_DS_MAT_NULL, A_DS_ERROR);
                goto bail;
        }
        length = 1000;
        string = DSSecureMalloc(sizeof(char)*length);
        sprintf(string, "{%s:[\n", DS_IO_TAG_TYPE_MatrixArray);
        for (i = 0; i < DSMatrixArrayNumberOfMatrices(array); i++) {
                temp = DSMatrixStringInJSONFormat(DSMatrixArrayMatrix(array, i));
                if (strlen(string)+strlen(temp) >= length) {
                        length += strlen(temp)+1000;
                        string = DSSecureRealloc(string, sizeof(char)*length);
                }
                strncat(string, temp, length-strlen(string));
                DSSecureFree(temp);
                if (i == DSMatrixArrayNumberOfMatrices(array)-1) 
                        strncat(string, "\n", length-strlen(string));
                else
                        strncat(string, ",\n", length-strlen(string));
        }
        strncat(string, "]}", length-strlen(string));
        string = DSSecureRealloc(string, sizeof(char)*(strlen(string)+1));
bail:
        return string;
}

/**
 * \brief Function to convert a DSSSystem into a JSON formatted string.
 *
 * \details This function is used to convert a DSSSystem into a JSON
 * object. The S-System as a set of objects, where each object represents each
 * of the fields of the DSSSystem.  The default behavior exports all of the
 * fields, this behavior can be overwritten by changing the S-System conversion
 * options.
 *
 * \param ssys A DSSSystem that will be used to create the JSON object.
 *
 * \return A C string with the JSON formatted data. If NULL, the conversion
 *         failed.
 *
 * \see DSIOSetSSystemJSONOptions()
 */
extern char * DSSSystemStringInJSONFormat(const DSSSystem *ssys)
{
        char * string = NULL, *edge;
        char * temp, key[100]; 
        const DSUInteger numberOfFields = 10;
        DSUInteger i, length = 0, actualLength;
        if (ssys == NULL) {
                DSError(M_DS_SSYS_NULL, A_DS_ERROR);
                goto bail;
        }
        length = 1000;
        string = DSSecureMalloc(sizeof(char)*length);
        sprintf(string, "{%s:{", DS_IO_TAG_TYPE_SSystem);
        actualLength = (DSUInteger)strlen(string);
        for (i = 0; i < numberOfFields; i++) {
                temp = NULL;
                switch (i) {
                        case 0:
                                sprintf(key, "\"alpha\":");
                                temp = DSMatrixStringInJSONFormat(DSSSystemAlpha(ssys));
                                break;
                        case 1:
                                sprintf(key, ",\"beta\":");
                                temp = DSMatrixStringInJSONFormat(DSSSystemBeta(ssys));
                                break;
                        case 2:
                                sprintf(key, ",\"Gd\":");
                                temp = DSMatrixStringInJSONFormat(DSSSystemGd(ssys));
                                break;
                        case 3:
                                sprintf(key, ",\"Gi\":");
                                if (DSSSystemGi(ssys) != NULL)
                                        temp = DSMatrixStringInJSONFormat(DSSSystemGi(ssys));
                                else 
                                        temp = strdup("null");
                                break;
                        case 4:
                                sprintf(key, ",\"Hd\":");
                                temp = DSMatrixStringInJSONFormat(DSSSystemHd(ssys));
                                break;
                        case 5:
                                sprintf(key, ",\"Hi\":");
                                if (DSSSystemHi(ssys) != NULL)
                                        temp = DSMatrixStringInJSONFormat(DSSSystemHi(ssys));
                                else 
                                        temp = strdup("null");
                                break;
                        case 6:
                                if (DSSSystemPrintingOptions & DS_SSYSTEM_JSON_NO_SOLUTION)
                                        break;
                                sprintf(key, ",\"M\":");
                                if (DSSSystemM(ssys) != NULL)
                                        temp = DSMatrixStringInJSONFormat(DSSSystemM(ssys));
                                else 
                                        temp = strdup("null");
                                break;
                        case 7:
                                sprintf(key, ",\"Xd\":");
                                temp = DSVariablePoolStringInJSONFormat(DSSSystemXd(ssys));
                                break;
                        case 8:
                                sprintf(key, ",\"Xi\":");
                                temp = DSVariablePoolStringInJSONFormat(DSSSystemXi(ssys));
                                break;
                        case 9:
                                if (DSSSystemPrintingOptions & DS_SSYSTEM_JSON_NO_SINGULAR)
                                        break;

                                sprintf(key, ",\"isSingular\":");
                                if (DSSSystemIsSingular(ssys) == true)
                                        temp = strdup("true");
                                else
                                        temp = strdup("false");
                                break;
                                
                        default:
                                break;
                }
                if (temp == NULL) 
                        continue;
                if (actualLength+strlen(key)+strlen(temp) >= length) {
                        length += strlen(key)+strlen(temp)+1000;
                        string = DSSecureRealloc(string, sizeof(char)*length);
                }
                edge = string+actualLength-1;
                strncat(edge, key, length-actualLength);
                actualLength += strlen(key);
                edge = string+actualLength-1;
                strncat(edge, temp, length-actualLength);
                actualLength += strlen(temp);
                DSSecureFree(temp);
        }
        edge = string+actualLength-1;
        strncat(edge, "}}", length-(actualLength++));
        string = DSSecureRealloc(string, sizeof(char)*(actualLength+1));
bail:
        return string;
}

/**
 * \brief Function to convert a DSCase into a JSON formatted string.
 *
 * \details This function is used to convert a DSCase into a JSON
 * object. The DSCase is represented with a set of objects, where each object is
 * a field of the DSCase object.  The default behavior exports all of the
 * fields, this behavior can be overwritten by changing the DSCase conversion
 * options.
 *
 * \param aCase A DSCase that will be used to create the JSON object.
 *
 * \return A C string with the JSON formatted data. If NULL, the conversion
 *         failed.
 *
 * \see DSIOSetCaseJSONOptions()
 */
extern char * DSCaseStringInJSONFormat(const DSCase * aCase)
{
        char * string = NULL, *edge;
        char *temp = NULL, key[100];
        DSUInteger i, length = 0, actualLength = 0;
        const DSUInteger numberOfFields = 8;
        if (aCase == NULL) {
                DSError(M_DS_CASE_NULL, A_DS_ERROR);
                goto bail;
        }
        length = 1000;
        string = DSSecureMalloc(sizeof(char)*length);
        sprintf(string, "{%s:{", DS_IO_TAG_TYPE_Case);
        actualLength = (DSUInteger)strlen(string);
        for (i = 0; i < numberOfFields; i++) {
                temp = NULL;
                switch (i) {
                        case 0:
                                sprintf(key, "%i", aCase->caseNumber);
                                temp = strdup(key);
                                sprintf(key, "\"caseNumber\":");
                                break;
                        case 1:
                                if (DSCasePrintingOptions & DS_CASE_JSON_NO_CASE_SIGNATURE)
                                        break;
                                sprintf(key, ",\"signature\" : \"");
                                temp = DSCaseSignatureToString(aCase);
                                strncat(temp, "\"", 1);
                                break;
                        case 2:
                                if (DSCasePrintingOptions & DS_CASE_JSON_NO_CONDITIONS)
                                        break;
                                sprintf(key, ",\"delta\":");
                                temp = DSMatrixStringInJSONFormat(aCase->delta);
                                break;
                        case 3:
                                sprintf(key, ",\"zeta\":");
                                if (aCase->zeta != NULL)
                                        temp = DSMatrixStringInJSONFormat(aCase->zeta);
                                else
                                        temp = strdup("null");
                                break;
                        case 4:
                                if (DSCasePrintingOptions & DS_CASE_JSON_NO_CONDITIONS)
                                        break;
                                sprintf(key, ",\"Cd\":");
                                temp = DSMatrixStringInJSONFormat(aCase->Cd);
                                break;
                        case 5:
                                if (DSCasePrintingOptions & DS_CASE_JSON_NO_CONDITIONS)
                                        break;
                                sprintf(key, ",\"Ci\":");
                                if (aCase->Ci != NULL)
                                        temp = DSMatrixStringInJSONFormat(aCase->Ci);
                                else 
                                        temp = strdup("null");
                                break;
                        case 6:
                                sprintf(key, ",\"U\":");
                                if (aCase->U != NULL)
                                        temp = DSMatrixStringInJSONFormat(aCase->U);
                                else 
                                        temp = strdup("null");
                                break;
                        case 7:
                                if (DSCasePrintingOptions & DS_CASE_JSON_NO_SSYSTEM)
                                        break;
                                sprintf(key, ",\"S-System\":");
                                temp = DSSSystemStringInJSONFormat(aCase->ssys);//DSCaseSSystem(aCase));
                                break;
                        default:
                                break;
                }
                if (temp == NULL) 
                        continue;
                if (actualLength+strlen(key)+strlen(temp) >= length) {
                        length += strlen(key)+strlen(temp)+1000;
                        string = DSSecureRealloc(string, sizeof(char)*length);
                }
                edge = string+actualLength-1;
                strncat(edge, key, length-actualLength);
                actualLength += strlen(key);
                edge = string+actualLength-1;
                strncat(edge, temp, length-actualLength);
                actualLength += strlen(temp);
                DSSecureFree(temp);
        }
        edge = string+actualLength-1;
        strncat(edge, "}}", length-actualLength);
        actualLength++;
        string = DSSecureRealloc(string, sizeof(char)*(actualLength+1));
bail:
        return string;
}

extern void * DSIOReadBinaryData(const char * fileName, size_t * length)
{
        FILE * file = NULL;
        void * buffer = NULL;
        size_t size;
        if (fileName == NULL || length == NULL) {
                DSError(M_DS_NULL, A_DS_ERROR);
                goto bail;
        }
        file = fopen(fileName, "rb");
        if (file == NULL) {
                DSError(M_DS_NULL ": file to read does not exist", A_DS_ERROR);
                goto bail;
        }
        size = 1000;
        *length = 0;
        buffer = DSSecureCalloc(sizeof(char), size);
        *length = fread(buffer, sizeof(char), 1000, file);
        while (1) {
                buffer = DSSecureRealloc(buffer, *length+1000);
                size = fread(buffer, sizeof(char), 1000, file);
                *length += size;
                if (size < 1000)
                        break;
        }
        fclose(file);
bail:
        return buffer;
}

extern void DSIOWriteBinaryData(const char * fileName, size_t length, void * binaryData)
{
        FILE * file = NULL;
        if (fileName == NULL || binaryData == NULL) {
                DSError(M_DS_NULL, A_DS_ERROR);
                goto bail;
        }
        file = fopen(fileName, "wb");
        if (file == NULL) {
                DSError(M_DS_NULL ": file to read does not exist", A_DS_ERROR);
                goto bail;
        }
        fwrite(binaryData, 1, length, file);
        fclose(file);
bail:
        return;
}


