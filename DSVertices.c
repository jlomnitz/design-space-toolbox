/**
 * \file DSVertices.c
 * \brief Implementation file with functions for dealing with n-dimensional
 *        vertices.
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

#include <stdio.h>
#include "DSVertices.h"
#include "DSErrors.h"
#include "DSMemoryManager.h"

extern DSVertices * DSVerticesAlloc(const DSUInteger dimensions)
{
        DSVertices *vertices = NULL;
        vertices = DSSecureCalloc(sizeof(DSVertices), 1);
        vertices->dimensions = dimensions;
        return vertices;
}

extern void DSVerticesFree(DSVertices *vertices)
{
        DSUInteger i;
        if (vertices == NULL) {
                DSError(M_DS_VERTICES_NULL, A_DS_ERROR);
                goto bail;
        }
        if (vertices->numberOfVertices != 0) {
                for (i = 0; i < vertices->numberOfVertices; i++) {
                        DSSecureFree(vertices->vertices[i]);
                }
                DSSecureFree(vertices->vertices);
        }
        DSSecureFree(vertices);
bail:
        return;
}

extern const bool DSVerticesAddVertex(DSVertices *vertices, const double * coordinates)
{
        DSUInteger i, j;
        bool isSuccess = false;
        if (vertices == NULL) {
                DSError(M_DS_VERTICES_NULL, A_DS_ERROR);
                goto bail;
        }
        if (coordinates == NULL) {
                DSError(M_DS_NULL ": Array of coordinates is NULL", A_DS_ERROR);
                goto bail;
        }
        for (i = 0; i < vertices->numberOfVertices; i++) {
                for (j = 0; j < vertices->dimensions; j++) {
                        if (fabs(vertices->vertices[i][j] - coordinates[j]) >= 1E-14)
                                break;
                        
                }
                if (j == vertices->dimensions)
                        break;
        }
        if (i == vertices->numberOfVertices) {
                isSuccess = true;
                if (vertices->numberOfVertices == 0) {
                        vertices->vertices = DSSecureMalloc(sizeof(double*)*1);
                } else {
                        vertices->vertices = DSSecureRealloc(vertices->vertices, sizeof(double*)*(vertices->numberOfVertices+1));
                }
                vertices->vertices[vertices->numberOfVertices] = DSSecureMalloc(sizeof(double)*vertices->dimensions);
                for (i = 0; i < vertices->dimensions; i++) {
                        vertices->vertices[vertices->numberOfVertices][i] = coordinates[i];
                }
                vertices->numberOfVertices++;
        }
bail:
        return isSuccess;
}

extern const bool DSVerticesAreEqual(const DSVertices *vert1, const DSVertices *vert2)
{
        bool areEqual = false;
        DSUInteger i, j;
        if (vert1 == NULL) {
                DSError(M_DS_VERTICES_NULL, A_DS_ERROR);
                goto bail;
        }
        if (vert2 == NULL) {
                DSError(M_DS_VERTICES_NULL, A_DS_ERROR);
                goto bail;                
        }
        if (vert1->dimensions != vert2->dimensions)
                goto bail;
        if (vert1->numberOfVertices != vert2->numberOfVertices)
                goto bail;
        areEqual = true;
        for (i = 0; i < vert1->numberOfVertices; i++) {
                for (j = 0; j < vert1->dimensions; j++) {
                        if (fabs(vert1->vertices[i][j] - vert1->vertices[i][j]) >= 1E-14)
                                break;
                }
                if (j != vert1->dimensions)
                        break;
        }
        if (i != vert1->numberOfVertices)
                areEqual = false;
bail:
        return areEqual;
}

extern const double * DSVerticesVertexAtIndex(const DSVertices *vertices, const DSUInteger index)
{
        double * vertex = NULL;
        if (vertices == NULL) {
                DSError(M_DS_VERTICES_NULL, A_DS_ERROR);
                goto bail;
        }
        if (index >= vertices->numberOfVertices) {
                DSError(M_DS_WRONG ": Index out of range", A_DS_ERROR);
                goto bail;
        }
        vertex = vertices->vertices[index];
bail:
        return vertex;
}

static double dsVertices2DSlope(const DSVertices * vertices, const DSUInteger pointA, const DSUInteger pointB)
{
        double m = NAN;
        double subX, subY;
        if (vertices == NULL) {
                DSError(M_DS_VERTICES_NULL, A_DS_ERROR);
                goto bail;
        }
        if (vertices->dimensions != 2) {
                DSError(M_DS_WRONG ": Vertices must be two dimensional", A_DS_ERROR);
                goto bail;
        }
        if (pointA >= vertices->numberOfVertices || pointB >= vertices->numberOfVertices) {
                DSError(M_DS_WRONG ": Vertex is out of bounds", A_DS_ERROR);
                goto bail;
        }

        subX = (vertices->vertices[pointA][0]-vertices->vertices[pointB][0]);
        subY = (vertices->vertices[pointA][1]-vertices->vertices[pointB][1]);
        if (subX == 0)
                m = INFINITY;
        else
                m = subY/subX;
bail:
        return m;
}

extern void DSVerticesOrder2DVertices(DSVertices *vertices)
{
        DSVertices * newVertices = NULL;
        DSUInteger i, current;
        DSUInteger start, indexMinX = 0, indexMinY = 0;
        DSUInteger indexMaxX = 0, indexMaxY = 0;
        double ** allVertices;
        if (vertices == NULL) {
                DSError(M_DS_VERTICES_NULL, A_DS_ERROR);
                goto bail;
        }
        if (vertices->dimensions != 2) {
                DSError(M_DS_WRONG ": Vertices must be 2 Dimensions", A_DS_ERROR);
                goto bail;
        }
        if (vertices->numberOfVertices == 0) {
                goto bail;
        }
        allVertices = vertices->vertices;
        for (i = 1; i < vertices->numberOfVertices; i++) {
                indexMinX = (allVertices[i][0] <= allVertices[indexMinX][0]) ? i : indexMinX;
                indexMaxX = (allVertices[i][0] > allVertices[indexMaxX][0]) ? i : indexMaxX;
                indexMinY = (allVertices[i][1] <= allVertices[indexMinY][1]) ? i : indexMinY;
                indexMaxY = (allVertices[i][1] > allVertices[indexMaxY][1]) ? i : indexMaxY;
        }
        newVertices = DSVerticesAlloc(2);
        DSVerticesAddVertex(newVertices, allVertices[indexMaxX]);
        current = indexMaxX;
        start = indexMaxX;
        while (current != indexMinY) {
                if (newVertices->numberOfVertices == vertices->numberOfVertices)
                        break;
                indexMaxX = indexMinY;
                for (i = 0; i < vertices->numberOfVertices; i++) {
                        if (i == current)
                                continue;
                        if (allVertices[i][1] >= allVertices[current][1])
                                continue;
                        indexMaxX = (fabs(dsVertices2DSlope(vertices, indexMaxX, current)) < fabs(dsVertices2DSlope(vertices, i, current))) ? i : indexMaxX;
                }
                current = indexMaxX;
                if (DSVerticesAddVertex(newVertices, allVertices[current]) == 0)
                        break;
        }
        while (current != indexMinX) {
                if (newVertices->numberOfVertices == vertices->numberOfVertices)
                        break;
                indexMinY = indexMinX;
                for (i = 0; i < vertices->numberOfVertices; i++) {
                        if (i == current)
                                continue;
                        if (allVertices[i][0] >= allVertices[current][0])
                                continue;
                        indexMinY = (fabs(dsVertices2DSlope(vertices, indexMinY, current)) > fabs(dsVertices2DSlope(vertices, i, current))) ? i : indexMinY;

                }
                current = indexMinY;
                if (DSVerticesAddVertex(newVertices, allVertices[current]) == 0)
                        break;
        }
        while (current != indexMaxY) {
                if (newVertices->numberOfVertices == vertices->numberOfVertices)
                        break;
                indexMinX = indexMaxY;
                for (i = 0; i < vertices->numberOfVertices; i++) {
                        if (i == current)
                                continue;
                        if (allVertices[i][1] <= allVertices[current][1])
                                continue;
                        indexMinX = (fabs(dsVertices2DSlope(vertices, indexMinX, current)) < fabs(dsVertices2DSlope(vertices, i, current))) ? i : indexMinX;

                }
                current = indexMinX;
                if (DSVerticesAddVertex(newVertices, allVertices[current]) == 0)
                        break;
        }
        while (current != start) {
                if (newVertices->numberOfVertices == vertices->numberOfVertices)
                        break;
                indexMaxY = start;
                for (i = 0; i < vertices->numberOfVertices; i++) {
                        if (i == current)
                                continue;
                        if (allVertices[i][0] <= allVertices[current][0])
                                continue;
                        indexMaxY = (fabs(dsVertices2DSlope(vertices, indexMaxY, current)) > fabs(dsVertices2DSlope(vertices, i, current))) ? i : indexMaxY;

                }
                current = indexMaxY;
                if (DSVerticesAddVertex(newVertices, allVertices[current]) == 0)
                        break;
        }
        if (newVertices->numberOfVertices != vertices->numberOfVertices) {
                DSError("Unable to order vertices", A_DS_ERROR);
        } else {
                vertices->vertices = newVertices->vertices;
                newVertices->vertices = allVertices;
        }
        DSVerticesFree(newVertices);
bail:
        return;
}

extern void DSVerticesPrint(const DSVertices *vertices)
{
        int (*print)(const char *, ...);
        DSUInteger i, j;
        if (vertices == NULL) {
                DSError(M_DS_VERTICES_NULL, A_DS_ERROR);
                goto bail;
        }
        if (DSPrintf == NULL)
                print = printf;
        else
                print = DSPrintf;
        for (i = 0; i < vertices->numberOfVertices; i++) {
                for (j = 0; j < vertices->dimensions; j++) {
                        print("%lf\t", vertices->vertices[i][j]);
                }
                print("\n");
        }
bail:
        return;
}