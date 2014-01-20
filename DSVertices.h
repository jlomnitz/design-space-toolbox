/**
 * \file DSVertices.h
 * \brief Header file with functions for dealing with n-dimensional verices.
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
 */

#ifndef __DS_VERTICES__
#define __DS_VERTICES__

#include "DSTypes.h"

#define M_DS_VERTICES_NULL M_DS_NULL ": Vertices object is NULL"

extern DSVertices * DSVerticesAlloc(const DSUInteger dimensions);
extern void DSVerticesFree(DSVertices *vertices);

extern const bool DSVerticesAddVertex(DSVertices *vertices, const double * coordinates);

extern const bool DSVerticesAreEqual(const DSVertices *vert1, const DSVertices *vert2);

extern const double * DSVerticesVertexAtIndex(const DSVertices *vertices, const DSUInteger index);

extern void DSVerticesOrder2DVertices(DSVertices *vertices);

extern void DSVerticesPrint(const DSVertices *vertices);

extern DSMatrix * DSVerticesConnectivityMatrix(const DSVertices *vertices, const DSCase * aCase, const DSVariablePool * lower, const DSVariablePool * upper);

#endif
