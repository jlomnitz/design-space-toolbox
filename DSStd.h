/**
 * \file DSStd.h
 * \brief Header file for the design space toolbox.
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
 * \todo Add all previous functionality.
 * \todo Add vertex enumeration functionality.
 *
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <pthread.h> */

#include "DSTypes.h"
#include "DSIO.h"
#include "DSErrors.h"
#include "DSMemoryManager.h"
#include "DSVariable.h"
#include "DSMatrix.h"
#include "DSMatrixArray.h"
#include "DSExpression.h"
#include "DSGMASystem.h"
#include "DSSSystem.h"
#include "DSCase.h"
#include "DSDesignSpace.h"
#include "DSVertices.h"
#include "DSDictionary.h"
#include "DSStack.h"

#ifndef __DS_STD_INCLUDE__
#define __DS_STD_INCLUDE__

#ifdef __cplusplus
__BEGIN_DECLS
#endif

#define free(x) DSSecureFree(x)
#define malloc(x) DSSecureMalloc(x)
#define calloc(x, y) DSSecureCalloc(x, y)
#define realloc(x, y) DSSecureRealloc(x, y)

#ifdef __cplusplus
__END_DECLS
#endif

#endif





