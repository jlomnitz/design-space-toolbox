//
//  DSVertices.h
//  DesignSpaceToolboxV2
//
//  Created by Jason Lomnitz on 8/30/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

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


#endif
