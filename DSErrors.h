/**
 * \file DSErrors.h 
 * \brief Header file with functions for error and exception handling.
 *
 * \details
 * This file specifies the design space standard for error handling.
 * Contained here are the necessary macros and functions to succesfully report
 * the errors throughout the design space library.
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
#include <stdlib.h>
#include "DSTypes.h"
#include "DSIO.h"

#ifndef __DS_ERRORS__
#define __DS_ERRORS__

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Error Messages
#endif

#define M_DS_NOFILE    "File not found"                //!< Message for no file found.
#define M_DS_NULL      "NULL pointer"                  //!< Message for NULL pointer.
#define M_DS_NOFORMAT  "Format not known"              //!< Message for unknown format.
#define M_DS_WRONG     "Inconsistent data"             //!< Message for inconsistent data being used.
#define M_DS_EXISTS    "Data already exists"           //!< Message for data aleady existing.
#define M_DS_NOTHREAD  "Thread not created"            //!< Message for no thread created.
#define M_DS_MALLOC    "Memory alloc failed"           //!< Message for failure to allocate data.
#define M_DS_NOT_IMPL  "Functionality not implemented" //!< Message for a feature not yet implemented.
#define M_DS_PARSE     "Could not parse data"          //!< Message for an error during parsing
/**
 *\defgroup M_DS_Messages Messages for DS Errors.
 *
 * Defined here are the generic messages used to report the
 * appropriate errors.  These are used with the different actions in
 * the macro DS_ERROR.  Other messages can be reported by literally
 * writting them in instead of these messages in the DSError macro. Also, these
 * messages can be modified by appending a literal string in the DSError macro.
 *
 *\see A_DS_Actions
 *\see DSError
 */
/*\{*/
/**
 * \def M_DS_NOFILE
 * \def M_DS_NULL
 * \def M_DS_NOFORMAT
 * \def M_DS_WRNG
 * \def M_DS_EXISTS
 * \def M_DS_NOTHRAD
 * \def M_DS_MALLOC
 * \def M_DS_NOT_IMPL
 */
/*\}*/

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Error Actions
#endif


#define A_DS_NOERROR     0                          //!< Value for no error.
#define A_DS_WARN       -1                          //!< Value for a warning
#define A_DS_ERROR      -2                          //!< Value for an error.
#define A_DS_FATAL      -3                          //!< Value for a fatal error, kills program.
#define A_DS_KILLNOW    A_DS_FATAL                  //!< DEPRECATED: 

/**
 *\defgroup A_DS_Actions Actions for DS Errors.
 *
 * Defined here are the appropriate reactions to a specific error, an
 * error can have different actions depending on the sensitivity of
 * the region involved.
 *\see M_DS_Messages
 *\see DS_ERROR
 */
/*@{*/
/**
 * \def A_DS_NOERROR
 * \def A_DS_WARN
 * \def A_DS_ERROR
 * \def A_DS_FATAL
 */
/*@}*/


#ifdef __cplusplus
__BEGIN_DECLS
#endif



#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Error Handling
#endif

/**
 * \brief Error reporting macro.
 * \details
 *
 * Definition of the error reporting macro used within the DesignSpace C
 * toolbox, this is a define which takes a string, which may be a standard
 * message, and an action and reports it via the standard warning and error
 * posting functions in the standard IO functions. A default behavior of the
 * DSError macro posts warning and errors to stderr, while a fatal error
 * posts the error to stderr and aborts the program.
 *
 * \see DSPostWarning
 * \see DSPostError
 * \see DSPostFatalError
 *
 * \see M_DS_Messages
 * \see A_DS_Actions
 */
#define DSError(M_DS_Message, A_DS_Action) DSErrorFunction(M_DS_Message, A_DS_Action, __FILE__, __LINE__, __func__)

__deprecated extern void DSErrorSetPrintFunction(void (*function)(const char * restrict));
__deprecated extern void DSErrorSetErrorFile(FILE *aFile);
extern void DSErrorFunction(const char * M_DS_Message, char A_DS_ACTION, const char *FILEN, int LINE, const char * FUNC);



#ifdef __cplusplus
__END_DECLS    
#endif

#endif





