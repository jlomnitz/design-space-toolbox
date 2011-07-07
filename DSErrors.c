/**
 * \file DSErrors.c
 * \brief Implementation file with functions for error and exception handling.
 *
 * \details
 * This file specifies the design space standard for error handling.
 * Contained here are the necessary macros and functions to succesfully report
 * the errors throughout the design space library.
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "DSErrors.h"

#define MSIZE  400
/**
 * \brief Pointer to a function determining how error messages are handled.
 *
 * This pointer to a function tells the error handling system which function to
 * call with the error messages.  If this pointer is NULL, then the system uses
 * printf, except that it prints to DSErrorFile instead of stdout.  This pointer
 * is intended to be used to override default behavior.  If used with MATLAB,
 * the pointer should be to mexPrintf.  If used in a Cocoa app, a function that
 * uses the notification system may be used.
 *
 * \see DSErrorFile
 * \see DSErrorFunction
 */
void (*DSPrintFunction)(const char *restrict);

/**
 * \brief FILE pointer used for default DSPrintFunction.
 *
 * This pointer to a FILE tells the error handling system which FILE to
 * print the error messages to.  If this pointer is NULL, then the system uses
 * the stderr file.  This variable is only used internally with the default 
 * behavior of DSErrorFunction, however, it is intended to be used with 
 * custom functions.
 *
 * \see DSPrintFunction
 * \see DSErrorFunction
 */

extern void DSErrorSetPrintFunction(void (*function)(const char * restrict))
{
        DSIOSetPostErrorFunction(function);
        DSIOSetPostWarningFunction(function);
        DSIOSetPostFatalErrorFunction(function);
        return;
}

extern void DSErrorSetErrorFile(FILE *aFile)
{
        DSIOSetErrorFile(aFile);
}
/**
 * \brief Implicit error handling function.  Called by DSError which
 * automatically adds file and line arguments.
 *
 * This function is called implicity when using the DSError macro.  The DSError 
 * adds the FILE and LINE argument, to report the error/warning at the
 * appropriate file and line.
 *
 * \see DSError
 * \see A_DS_Actions
 */
extern void DSErrorFunction(const char * M_DS_Message, char A_DS_ACTION, const char *FILEN, int LINE, const char * FUNC)
{
        
        char errorString[MSIZE];
        if (DSIOErrorFile == NULL)
                DSIOSetErrorFile(stderr);
        sprintf(errorString, "Design Space Toolbox: %.50s.\n# %i : %.20s.\nIn: %.200s.\n",
                M_DS_Message,
                LINE,
                FUNC,
                FILEN);
        switch (A_DS_ACTION) {					
                case A_DS_WARN:
                        if (DSPostWarning == NULL)
                                fprintf(DSIOErrorFile, "Warning: %s\n", errorString);
                        else
                                DSPostWarning(errorString);
                        break;
                case A_DS_ERROR:
                        if (DSPostError == NULL)
                                fprintf(DSIOErrorFile, "Error: %s\n", errorString);
                        else
                                DSPostError(errorString);
                        break;
                case A_DS_FATAL:
                        if (DSPostFatalError == NULL) {
                                fprintf(DSIOErrorFile, "Error: %s\n", errorString);
                                exit(EXIT_FAILURE);
                        } else {
                                DSPostFatalError(errorString);
                        }
                        break;
        }

}

