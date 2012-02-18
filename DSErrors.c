/**
 * \file DSErrors.c
 * \brief Implementation file with functions for error and exception handling.
 *
 * \details
 * This file specifies the design space standard for error handling.
 * Contained here are the necessary macros and functions to report
 * the errors throughout the design space library.  The DSErrorFunction allows
 * different behaviors; the default behavior, errors are printed to the
 * DSIOErrorFile, which is set to stderr by default.  This behavior can be
 * changed by setting changing DSPostWarning, DSPostError and DSPostFatalError
 * function pointers.
 *
 * \see DSIOErrorFile
 * \see DSPostWarning
 * \see DSPostError
 * \see DSPostFatalError
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
 *
 * \todo Implement locks when making the error strings.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <execinfo.h>

#include "DSErrors.h"
#include "DSMemoryManager.h"

/**
 * \brief Maximum number of traces on the call stack.
 *
 * \details This number represents the maximum number of traces on the call
 * stack that the DSError function adds to the error string.  The trace
 * represents all the functions called up to the error.
 */
#define STACK_TRACE_NUM 10

/**
 * \brief The maximum size of the error message string.
 *
 * \details This represents the maximum number of characters that an error
 * string can contain.  The error string is a statically allocated string.
 */
#define MSIZE           1500

/**
 * \brief Implicit error handling function.  Called by DSError which
 * automatically adds file and line arguments.
 *
 * This function is called implicity when using the DSError macro.  The DSError 
 * adds the FILE, LINE and FUNC arguments, to report the error/warning at the
 * appropriate file, line and function.
 *
 * \param M_DS_Message A string containing the error message.
 * \param A_DS_ACTION A character representing an error code as described in A_DS_Actions.
 * \param FILEN A string with the name of the file where the error was reported.
 * \param LINE An integer with the line number in the file where the error was reported.
 * \param FUNC A string with the name of the function where the error was reported.
 *
 * \see DSError
 * \see A_DS_Actions
 */
extern void DSErrorFunction(const char * M_DS_Message, char A_DS_ACTION, const char *FILEN, int LINE, const char * FUNC)
{
        void *stackArray[STACK_TRACE_NUM];
        int size;
        char ** strings;
        char errorString[MSIZE];
        int i;
        if (A_DS_ACTION == A_DS_NOERROR)
                goto bail;
        if (DSIOErrorFile == NULL)
                DSIOSetErrorFile(stderr);
        size = backtrace (stackArray, STACK_TRACE_NUM);
        strings = backtrace_symbols (stackArray, size);
        sprintf(errorString, "Design Space Toolbox: %.100s.\n# %i : %.30s: %.200s.\nCall stack:\n",
                M_DS_Message,
                LINE,
                FUNC,
                FILEN);
        i = 0;
        for (i = 1; i < size; i++) {
                if (strlen(errorString) + strlen(strings[i]) >= MSIZE)
                    break;
                strncat(errorString, strings[i], MSIZE);
                strncat(errorString, "\n", MSIZE);
                
        }
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
        DSSecureFree(strings);
bail:
        return;
}

