/**
 * \file DSIO.h
 * \brief Header file with standard input and output functions.
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
 *
 * \todo Define standard input and output file formats.
 * \todo Define criteria for warnings, errors and fatal errors.
 *
 */

#include <stdio.h>
#include "DSTypes.h"

#ifndef __DS_IO__
#define __DS_IO__

#ifdef __cplusplus
__BEGIN_DECLS
#endif

/**
 *\defgroup DS_CASE_JSON_OPTIONS Options for JSON conversion of DSCase object.
 *
 * Defined here are different options determining the information stored in a
 * JSON string for a DSCase object.  These options are passed to the
 * DSIOSetCaseJSONOptions function.  These options designate the value
 * for a global flag variable
 */

/*\{*/
#define DS_CASE_JSON_NO_SSYSTEM           1  //!< Flag value indicating that the S-System information should not be included in the JSON string.
#define DS_CASE_JSON_NO_CASE_SIGNATURE    2  //!< Flag value indicating that the case signature should not be included in the JSON string.
#define DS_CASE_JSON_NO_CONDITIONS        4  //!< Flag value indicating that the conditions for validity should not be included in the JSON string.
/*\}*/

/**
 *\defgroup DS_SSYSTEM_JSON_OPTIONS Options for JSON conversion of DSSSystem object.
 *
 * Defined here are different options determining the information stored in a
 * JSON string for a DSSSystem object.  These options are passed to the
 * DSIOSetSSystemJSONOptions function.  These options designate the value
 * for a global flag variable.
 */

/*\{*/
#define DS_SSYSTEM_JSON_NO_SOLUTION       1  //!< Flag value indicating that the S-System solution should not be included in the JSON string.
#define DS_SSYSTEM_JSON_NO_SINGULAR       2  //!< Flag value indicating that the JSON string will not indicate if the S-System is singular.
/*\}*/

/**
 * \brief Pointer to a function determining how messages are printed.
 *
 * This pointer to a function tells the error handling system which function to
 * call with the error messages.  If this pointer is NULL, the design space 
 * toolbox should have a default printing format, using printf to stdout.
 * This pointer is intended to be used to override default behavior to be
 * override. An example could be by using the mexPrintf function in matlab.
 *
 * \see DSIOSetPrintFunction
 */
extern int (*DSPrintf)(const char *, ...);

/**
 * \brief Pointer to a function determining how warning are handled.
 *
 * This pointer to a function is used by DSErrorFunction to post warnings.  This
 * pointer should be used to allow better integration of warnings in programs
 * that make use of the DesignSpaceToolbox.  The function takes one argument, a
 * constant C string with the warning message.  To change the function used,
 * the function DSIOSetPostWarningFunction should be used. This is to avoid
 * errors caused by dynamic linking.  These errors involve changing the value
 * of a global variable that has not yet been loaded by the linker.
 *
 * \see DSIOSetPostWarningFunction
 */
extern void (*DSPostWarning)(const char *message);

/**
 * \brief Pointer to a function determining how errors are handled.
 *
 * This pointer to a function is used by DSErrorFunction to post erros.  This
 * pointer should be used to allow better integration of errors in programs
 * that make use of the DesignSpaceToolbox.  The function takes one argument, a
 * constant C string with the error message.  To change the function used,
 * the function DSIOSetPostErrorFunction should be used. This is to avoid
 * errors caused by dynamic linking.  These errors involve changing the value
 * of a global variable that has not yet been loaded by the linker.
 *
 *
 * \see DSIOSetPostErrorFunction
 */
extern void (*DSPostError)(const char *message);

/**
 * \brief Pointer to a function determining how fatal errors are handled.
 *
 * This pointer to a function is used by DSErrorFunction to post fatal erros.
 * This pointer should be used to allow better integration of errors in programs
 * that make use of the DesignSpaceToolbox.  The function takes one argument, a
 * constant C string with the error message.  To change the function used,
 * the function DSIOSetPostFatalErrorFunction should be used. This is to avoid
 * errors caused by dynamic linking.  These errors involve changing the value
 * of a global variable that has not yet been loaded by the linker.
 *
 *
 * \see DSIOSetPostErrorFunction
 */
extern void (*DSPostFatalError)(const char *message);

/**
 * \brief FILE pointer used for default error/warning printing.
 *
 * This pointer to a FILE tells the error handling system which FILE to
 * print the error messages to.  If this pointer is NULL, then the system sets
 * it to the stderr file.  This variable is only used internally with the 
 * default behavior of DSErrorFunction.  To change the error file, the function 
 * DSIOSetErrorFile should be used in order to avoid errors caused by dynamic
 * linking. These errors involve changing the value of a global variable that 
 * has not yet been loaded by the linker.
 *
 * \see DSIOSetErrorFile
 * \see DSErrorFunction
 */
extern FILE * DSIOErrorFile;


extern void DSIOSetErrorFile(FILE *aFile);

extern void DSIOSetPrintFunction(int (*printFunction)(const char *, ...));
extern void DSIOSetPostWarningFunction(void (*warningFunction)(const char *message));
extern void DSIOSetPostErrorFunction(void (*errorFunction)(const char *message));
extern void DSIOSetPostFatalErrorFunction(void (*fatalErrorFunction)(const char *message));

#if defined(__APPLE__) && defined(__MACH__)
#pragma mark - Export objects to JSON format
#endif

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Functions controlling output
#endif

extern void DSIOSetCaseJSONOptions(const DSUInteger options);
extern void DSIOSetSSystemJSONOptions(const DSUInteger options);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Write JSON data
#endif

extern char * DSVariablePoolStringInJSONFormat(const DSVariablePool *pool);
extern char * DSMatrixStringInJSONFormat(const DSMatrix * matrix);
extern char * DSMatrixArrayStringInJSONFormat(const DSMatrixArray *array);
extern char * DSSSystemStringInJSONFormat(const DSSSystem *ssys);
extern char * DSCaseStringInJSONFormat(const DSCase * aCase);


#if defined (__APPLE__) && defined (__MACH__)
#pragma mark Read JSON data
#endif

//extern DSVariablePool * DSVariablePoolByParsingStringInJSONFormat(const char * string);
//extern DSMatrix * DSMatrixByParsingStringInJSONFormat(const char * string);
//extern DSMatrixArray * DSMatrixArrayByParsingStringInJSONFormat(const char * string);
//extern DSSSystem * DSSSystemByParsingStringInJSONFormat(const char * string);
//extern DSCase * DSCaseByParsingStringInJSONFormat(const char * string);

extern void * DSIOReadBinaryData(const char * fileName, size_t * length);
extern void DSIOWriteBinaryData(const char * fileName, size_t length, void * binaryData);

#ifdef __cplusplus
__END_DECLS
#endif

#endif