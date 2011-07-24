/**
 * \file DSIO.c
 * \brief Implementation file with standard input and output functions.
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
 *
 */

#include "DSIO.h"

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


extern void DSIOSetPrintFunction(int (*printFunction)(const char *, ...))
{
        DSPrintf = printFunction;
}


extern void DSIOSetPostWarningFunction(void (*warningFunction)(const char *message))
{
        DSPostWarning = warningFunction;
}


extern void DSIOSetPostErrorFunction(void (*errorFunction)(const char *message))
{
        DSPostError = errorFunction;
}


extern void DSIOSetPostFatalErrorFunction(void (*fatalErrorFunction)(const char *message))
{
        DSPostFatalError = fatalErrorFunction;
}