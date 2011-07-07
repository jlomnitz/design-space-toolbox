//
//  DSIO.c
//  DesignSpaceToolboxV2
//
//  Created by Jason Lomnitz on 7/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "DSIO.h"

/*
int (*DSPrintf)(const char *, ...);
void (*DSPostWarning)(const char *message);
void (*DSPostError)(const char *message);
void (*DSPostFatalError)(const char *message);
FILE *DSIOErrorFile;
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