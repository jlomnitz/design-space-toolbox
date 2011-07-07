/**
 * \file DSErrors.h
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

#include "DSStd.h"

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
        
        FILE * temp = stdout;
        char nullPrint = 0;
        if (DSErrorFile == NULL)
                DSErrorFile = stderr;
                
        if (DSPrintFunction == NULL) {
                nullPrint = 1;
                stdout = DSErrorFile;
                DSPrintFunction = printf;
        }
        switch(A_DS_ACTION) {					
                case A_DS_WARN:
                        DSPrintFunction("DST: File: %s Function: %s Line: %i: Warning: %s.\n",
                                       FILEN,
                                       FUNC,
                                       LINE,
                                       M_DS_Message);
                        break;
                case A_DS_ERROR:
                        DSPrintFunction("DST: File: %s Function: %s Line: %i: Error: %s.\n",
                                      FILEN,
                                      FUNC,
                                      LINE,
                                      M_DS_Message);
                        break;
                case A_DS_KILLNOW:
                        DSPrintFunction("DST: File: %s Function: %s Line: %i: Fatal Error: %s.\n",
                                      FILEN,
                                      FUNC,
                                      LINE,
                                      M_DS_Message);
                        exit(A_DS_KILLNOW);
        }
        if (nullPrint == 1) {
                DSPrintFunction = NULL;
                stdout = temp;
        }
}

