/**
 * \file DSNVertexEnumeration.c
 * \brief
 *
 * \details
 *
 * Copyright (C) 2011-2014 Jason Lomnitz.\n\n
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
 * \date 2014
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "DSStd.h"
#include "lrslib.h"
#include "DSCase.h"
/**
 \brief Defining the largest denominator for Multiple Precision. This is 
 * important for vertex enumeration, which is very sensitive to precision error.
 */
#define MP_DENOMINATOR_PRECISION 100

/**
 * \brief Structure to store rational values. Used through out the functions to
 * represent rational functions.
 */
typedef struct {
        long int numerator;
        long int denominator;
        double error;
} DSRational;

/*
 * Find rational approximation to given real number
 * Original code: David Eppstein / UC Irvine / 8 Aug 1993
 * With corrections from Arno Formella, May 2008
 * Modified by: Jason Lomnitz / UC Davis / 23 Feb 2011
 *
 *
 * Based on the theory of continued fractions
 * if x = a1 + 1/(a2 + 1/(a3 + 1/(a4 + ...)))
 * then best approximation is found by truncating this series
 * (with some adjustments in the last term).
 *
 * Note the fraction can be recovered as the first column of the matrix
 *  ( a1 1 ) ( a2 1 ) ( a3 1 ) ...
 *  ( 1  0 ) ( 1  0 ) ( 1  0 )
 * Instead of keeping the sequence of continued fraction terms,
 * we just keep the last partial product of these matrices.
 *
 * As there two possibilities, we only keep the rational number with lowest
 * absolute error.  This might be changed in favor of the rational number with
 * smallest denominator, as long as the error margin is low, as a large
 * denominator may impactthe speed of multiple precision arithmetic used in lrs.
 */
DSRational dsDoubleToRational(double aDouble, DSUInteger maxden) {
        DSRational rat1, rat2;
        long int m[2][2];
        double x, startx;
        long int ai;
        
        startx = x = aDouble;//atof(av[1]);
        
        /* initialize matrix */
        m[0][0] = m[1][1] = 1;
        m[0][1] = m[1][0] = 0;
        
        /* loop finding terms until denom gets too big */
        while (labs(m[1][0] *  ( ai = (long)x ) + m[1][1]) <= maxden) {
                long int t;
                t = m[0][0] * ai + m[0][1];
                m[0][1] = m[0][0];
                m[0][0] = t;
                t = m[1][0] * ai + m[1][1];
                m[1][1] = m[1][0];
                m[1][0] = t;
                if(x==(double)ai) break;     // AF: division by zero
                x = 1/(x - (double) ai);
                if(x>(double)0x7FFFFFFF) break;  // AF: representation failure
        }
        
        /* now remaining x is between 0 and 1/ai */
        /* approx as either 0 or 1/m where m is max that will fit in maxden */
        /* first try zero */
        rat1.numerator = m[0][0];
        rat1.denominator = m[1][0];
        rat1.error = startx - ((double) m[0][0]/(double)m[1][0]);
        
        /* now try other possibility */
        ai = (maxden - m[1][1]) / m[1][0];
        m[0][0] = m[0][0] * ai + m[0][1];
        m[1][0] = m[1][0] * ai + m[1][1];
        rat2.numerator = m[0][0];
        rat2.denominator = m[1][0];
        rat2.error = startx - ((double) m[0][0]/(double)m[1][0]);
        return ((fabs(rat1.error) < fabs(rat2.error)) ? rat1 : rat2);
}

/* Building the constraints for lrs using the A and B matrix, in the format of
 Ax*b = 0 */
void buildConstraints(lrs_dic *P, lrs_dat *Q, const DSMatrix *A, const DSMatrix *b, const DSVariablePool * lower, const DSVariablePool * upper)
{
        DSUInteger i, j;
        long int m, n;
        long int *numerators, *denominators;
//        double *A;
//        double *b;
        int Am, An, bm, bn;
        DSRational rational;
        if (P == NULL || Q == NULL || A == NULL || b == NULL)
                goto bail;
        n = Q->n;
        m = Q->m-2*(n-1);
        /* We get matrices from Matlab mxArray, each pointer is (double*) */
        
        Am = DSMatrixRows(A);
        An = DSMatrixColumns(A);
        bm = DSMatrixRows(A);
        bn = DSMatrixColumns(A);
        
        /* Allocate integer vectors for MP arithmetic in lrs */
        numerators = DSSecureCalloc(sizeof(long int), n);
        denominators = DSSecureCalloc(sizeof(long int), n);
        for (i = 0; i < m; i++) {
                /* convert to rational approx. */
                rational = dsDoubleToRational(DSMatrixDoubleValue(b, i, 0), MP_DENOMINATOR_PRECISION);
                numerators[0]=rational.numerator;
                denominators[0] = rational.denominator;
                for (j = 1; j < n; j++) {
                        rational = dsDoubleToRational(DSMatrixDoubleValue(A, i, j-1),
                                                      MP_DENOMINATOR_PRECISION);
                        numerators[j]=rational.numerator;
                        denominators[j] = rational.denominator;
                }
                /* Insert information to lrs structures, both A and b */
                lrs_set_row(P, Q, i+1, numerators, denominators, GE);
        }
        for (i = 0; i < 2*(n-1); i++) {
                if (i % 2 == 0)
                        numerators[0] = -log10(DSVariableValue(DSVariablePoolVariableAtIndex(lower, i/2)));
                else
                        numerators[0] = log10(DSVariableValue(DSVariablePoolVariableAtIndex(upper, i/2)));
                denominators[0] = 1;
                for (j = 1; j < n; j++) {
                        numerators[j] = 0;
                        denominators[j] = 1;
                }
                numerators[1+i/2] = ((i % 2 == 0) ? 1 : -1);
                for (j = 1; j < n; j++) {
                }
                /* Inserting additional constraints, to bound the polytopes */
                lrs_set_row(P, Q, i+m+1, numerators, denominators, GE);
        }
        DSSecureFree(numerators);
        DSSecureFree(denominators);
bail:
        return;
}


/* Implementation of reverse search algorithm. */
float *reverseSearch(lrs_dic *P, lrs_dat *Q, long int *numRows, long int **cobasis)
{
        lrs_mp_vector output;
        lrs_mp_matrix Lin;
        long int i, d, rows = 0;
        DSRational number;
        long int col, prune = FALSE;
        float *vertices = NULL;
        long int *temp;
        if (P == NULL || Q == NULL)
                goto bail;
        /* We get the first basis, else there is no feas. region */
        if (!lrs_getfirstbasis(&P, Q, &Lin, TRUE))
                goto bail;
        output = lrs_alloc_mp_vector(Q->n);
        do {
                /* *Prune* tree to backtrack if we have exhausted all possibilities */
                prune = lrs_checkbound(P, Q);
                if(!prune) {
                        for (col = 0; col <= P->d; col++) {
                                if (lrs_getsolution(P, Q, output, col)) {
                                        if (!zero(output[0])) {
                                                /* Need to allocate memory for each vertices and cabasis */
                                                if (vertices == NULL) {
                                                        vertices = DSSecureMalloc(sizeof(float)*(Q->n-1));
                                                        temp = DSSecureMalloc(sizeof(long int)*(Q->n-1));
                                                } else {
                                                        vertices = DSSecureRealloc(vertices, sizeof(float)*(Q->n-1)*(rows+1));
                                                        temp = DSSecureRealloc(temp, sizeof(long int)*(Q->n-1)*(rows+1));
                                                }
                                                /* Extracting coordinates for the vertex */
                                                for (i = 1; i < Q->n; i++) {
                                                        number.numerator = output[i][length(output[i]) - 1];
                                                        number.denominator = output[0][length(output[0]) -1];
                                                        /* Signs are stored independently */
                                                        if (sign (output[i]) * sign (output[0]) == NEG)
                                                                number.numerator *= -1;
                                                        /* Return to a float representation, storing by row */
                                                        vertices[rows*(Q->n-1)+i-1] = ((float)number.numerator)/((float)number.denominator);
                                                        
                                                }
                                                d = P->d;
                                                /* Extracting cobasis */
                                                for (i = 0; i < d; i++)
                                                        temp[rows*(Q->n-1)+i] = Q->inequality[P->C[i] - Q->lastdv];
                                                rows++;
                                        }
                                }
                        }
                }
                /* Get the next basis, the information is stored in dictionary */
        } while (!Q->lponly && lrs_getnextbasis(&P, Q, prune));
        /* Cleaning up */
        lrs_clear_mp_vector(output, Q->n);
        *numRows = rows;
        if (cobasis != NULL)
                *cobasis = temp;
        else
                DSSecureFree(temp);
bail:
        return vertices;
}


/* Mex gateway function */
DSMatrixArray * DSCaseNDVertexEnumeration(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds)
{
        DSUInteger i, j;
        DSMatrixArray * vertexEnumeration = NULL;
        DSMatrix *A;
        DSMatrix *b;
        DSMatrix *vertices, *cobasisMatrix;
        long int *cobasis = NULL, rows = 0;
        float *temp;
        lrs_dic *P;
        lrs_dat *Q;
        int Am, An, bm, bn;
        A = DSCaseU(aCase);
        b = DSCaseZeta(aCase);
        Am = DSMatrixRows(A);
        An = DSMatrixColumns(A);
        bm = DSMatrixRows(b);
        bn = DSMatrixColumns(b);
        
        if (Am != bm) {
                DSError(M_DS_WRONG ": Inconsistent number of rows", A_DS_ERROR);
                goto exit;
        }
        if (bn != 1) {
                DSError(M_DS_WRONG ": Not in standard form", A_DS_ERROR);
                goto exit;
        }
        
        lrs_init("DST Test");
        Q = lrs_alloc_dat("LRS globals");
        Q->n = An+1;
        Q->m = Am+2*An;
        P = lrs_alloc_dic(Q);
        buildConstraints(P, Q, A, b, lowerBounds, upperBounds);
        temp = reverseSearch(P, Q, &rows, &cobasis);
        vertices  = DSMatrixAlloc(rows, Q->n-1);
        for (i = 0; i < rows; i++) {
                for (j = 0; j < Q->n-1; j++) {
                        DSMatrixSetDoubleValue(vertices, i, j, temp[i*(Q->n-1)+j]);
                }
        }
        cobasisMatrix  = DSMatrixAlloc(rows, Q->n-1);
        for (i = 0; i < rows; i++) {
                for (j = 0; j < Q->n-1; j++) {
                        /* Matlab stores by column, we stored by row. Need to invert */
                        DSMatrixSetDoubleValue(cobasisMatrix, i, j, 1.0*cobasis[i*(Q->n-1)+j]);
                }
        }
        vertexEnumeration =DSMatrixArrayAlloc();
        DSMatrixArrayAddMatrix(vertexEnumeration, vertices);
        DSMatrixArrayAddMatrix(vertexEnumeration, cobasisMatrix);
        /* Cleaning up */
        lrs_free_dic(P, Q);
        lrs_free_dat(Q);
        lrs_close("DST Test");
        if (temp != NULL)
                DSSecureFree(temp);
        if (cobasis != NULL)
                DSSecureFree(cobasis);
exit:
        return vertexEnumeration;
}