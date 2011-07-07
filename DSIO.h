/**
 * \file DSIO.h
 * \brief Header file with standard input and output functions.
 *
 * \details
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
 *
 * \todo Add 'plug-in' for printf function.
 * \todo Define standard input and output file formats.
 *
 */

#include <stdio.h>
#include "DSTypes.h"

#ifndef __DS_IO__
#define __DS_IO__

#ifdef __cplusplus
__BEGIN_DECLS
#endif


int (*DSPrintf)(const char *, ...);
void (*DSPostWarning)(const char *message);
void (*DSPostError)(const char *message);
void (*DSPostFatalError)(const char *message);

FILE * DSIOErrorFile;

extern void DSIOSetErrorFile(FILE *aFile);
extern void DSIOSetPrintFunction(int (*printFunction)(const char *, ...));
extern void DSIOSetPostWarningFunction(void (*warningFunction)(const char *message));
extern void DSIOSetPostErrorFunction(void (*errorFunction)(const char *message));
extern void DSIOSetPostFatalErrorFunction(void (*fatalErrorFunction)(const char *message));

#ifdef __cplusplus
__END_DECLS
#endif

#endif