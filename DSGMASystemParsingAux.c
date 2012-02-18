/**
 * \file DSGMASystemParsingAux.h
 * \brief Implementation file with functions for dealing with the parsing of GMA
 * Systems.
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
#include <string.h>
#include <unistd.h>
#include "DSTypes.h"
#include "DSErrors.h"
#include "DSMemoryManager.h"
#include "DSGMASystemParsingAux.h"

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, deallocation and initialization
#endif

extern gma_parseraux_t * DSGMAParserAuxAlloc(void)
{
        gma_parseraux_t *newAux = NULL;
        newAux = DSSecureCalloc(sizeof(gma_parseraux_t), 1);
        newAux->sign = AUX_SIGN_UNDEFINED;
        newAux->succeded = true;
        //        newAux->root = newAux;
        newAux->next = NULL;
        return newAux;
}

extern void DSGMAParserAuxFree(gma_parseraux_t *root)
{
        DSUInteger i, num;
        gma_parseraux_t *next = NULL;
        if (root == NULL) {
                DSError(M_DS_NULL ": Parser auxiliary is NULL", A_DS_ERROR);
                goto bail;
        }
        while (root) {
                next = DSGMAParserAuxNextNode(root);
                num = DSGMAParserAuxNumberOfBases(root);
                if (num != 0) {
                        for (i = 0; i < num; i++) {
                                if (DSGMAParserAuxBaseAtIndexIsVariable(root,i) &&
                                    DSGMAParserAuxVariableAtIndex(root, i) != NULL)
                                        DSSecureFree(root->bases[i].name);
                        }
                        DSSecureFree(root->exponents);
                        DSSecureFree(root->bases);
                }
                DSSecureFree(root);
                root = next;
        }
bail:
        return;
}

extern void DSGMAParserAuxNewTerm(gma_parseraux_t *current)
{
        if (current == NULL) {
                DSError(M_DS_NULL ": Parser auxiliary is NULL", A_DS_ERROR);
                goto bail;
        }
        current->next = DSGMAParserAuxAlloc();
bail:
        return;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Setter functions
#endif

extern gma_parseraux_t * DSGMAParserAuxNextNode(const gma_parseraux_t * const aux)
{
        gma_parseraux_t *next = NULL;
        if (aux == NULL) {
                DSError(M_DS_NULL ": Parser auxiliary is NULL", A_DS_ERROR);
                goto bail;
        }
        next = aux->next;
bail:
        return next;
}

extern void DSGMAParserAuxSetSign(gma_parseraux_t *aux, const char sign)
{
        if (aux == NULL) {
                DSError(M_DS_NULL ": Parser auxiliary is NULL", A_DS_ERROR);
                goto bail; 
        }
        switch (sign) {
                case AUX_SIGN_NEGATIVE:
                case AUX_SIGN_POSITIVE:
                        aux->sign = sign;
                        break;
                default:
                        aux->sign = AUX_SIGN_UNDEFINED;
                        DSError(M_DS_WRONG ": Sign type is undefined", A_DS_ERROR);
                        break;
        }
bail:
        return;
}

extern void DSGMAParserAuxAddVariableExponentPair(gma_parseraux_t *aux, const char *const name, const double exponent)
{
        if (aux == NULL) {
                DSError(M_DS_NULL ": Parser auxiliary is NULL", A_DS_ERROR);
                goto bail; 
        }
        if (name == NULL) {
                DSError(M_DS_WRONG ": Name of variable is NULL", A_DS_ERROR);
                goto bail;
        }
        if (strlen(name) == 0) {
                DSError(M_DS_WRONG ": Name of variable is empty", A_DS_ERROR);
                goto bail;
        }
        if (aux->bases == NULL && DSGMAParserAuxNumberOfBases(aux) != 0) {
                DSError(M_DS_NULL ": Base array is NULL", A_DS_FATAL);
                goto bail;
        }
        if (aux->bases == NULL && aux->exponents == NULL) {
                aux->bases = DSSecureMalloc(sizeof(union base_info)*1);
                aux->exponents = DSSecureMalloc(sizeof(double)*1);
        } else {
                aux->bases = DSSecureRealloc(aux->bases, 
                                             sizeof(union base_info)*(DSGMAParserAuxNumberOfBases(aux)+1));
                aux->exponents = DSSecureRealloc(aux->exponents, 
                                                 sizeof(double)*(DSGMAParserAuxNumberOfBases(aux)+1));
        }
        aux->bases[DSGMAParserAuxNumberOfBases(aux)].name = strdup(name);
        aux->exponents[DSGMAParserAuxNumberOfBases(aux)] = exponent;
        DSGMAParserAuxNumberOfBases(aux)++;
bail:
        return;
}

extern void DSGMAParserAuxAddConstantBase(gma_parseraux_t *aux, const double base)
{
        if (aux == NULL) {
                DSError(M_DS_NULL ": Parser auxiliary is NULL", A_DS_ERROR);
                goto bail; 
        }
        if (aux->bases == NULL && DSGMAParserAuxNumberOfBases(aux) != 0) {
                DSError(M_DS_NULL ": Base array is NULL", A_DS_FATAL);
                goto bail;
        }
        if (aux->bases == NULL && aux->exponents == NULL) {
                aux->bases = DSSecureMalloc(sizeof(union base_info)*1);
                aux->exponents = DSSecureMalloc(sizeof(double)*1);
        } else {
                aux->bases = DSSecureRealloc(aux->bases, 
                                             sizeof(union base_info)*(DSGMAParserAuxNumberOfBases(aux)+1));
                aux->exponents = DSSecureRealloc(aux->exponents, 
                                                 sizeof(double)*(DSGMAParserAuxNumberOfBases(aux)+1));
        }
        aux->bases[DSGMAParserAuxNumberOfBases(aux)].value = base;
        aux->exponents[DSGMAParserAuxNumberOfBases(aux)] = AUX_EXPONENT_CONSTANT_BASE;
        DSGMAParserAuxNumberOfBases(aux)++;
bail:
        return;
}

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Getter functions
#endif

extern const char DSGMAParserAuxSign(const gma_parseraux_t * const aux)
{
        char sign = AUX_SIGN_UNDEFINED;
        if (aux == NULL) {
                DSError(M_DS_NULL ": Parser auxiliary is NULL", A_DS_ERROR);
                goto bail; 
        }
        sign = aux->sign;
bail:
        return sign;
}

extern const double DSGMAParserAuxExponentAtIndex(const gma_parseraux_t *const aux, const DSUInteger index)
{
        double exponent = INFINITY;
        if (aux == NULL) {
                DSError(M_DS_NULL ": Parser auxiliary is NULL", A_DS_ERROR);
                goto bail; 
        }
        if (index >= DSGMAParserAuxNumberOfBases(aux)) {
                DSError(M_DS_WRONG ": Exponent index out of bounds", A_DS_ERROR);
                goto bail;
        }
        if (aux->exponents == NULL) {
                DSError(M_DS_NULL ": Exponent array is NULL", A_DS_FATAL);
                goto bail;
        }
        exponent = aux->exponents[index];
bail:
        return exponent;
}

extern const char * const DSGMAParserAuxVariableAtIndex(const gma_parseraux_t * const aux, const DSUInteger index)
{
        char * name = NULL;
        if (aux == NULL) {
                DSError(M_DS_NULL ": Parser auxiliary is NULL", A_DS_ERROR);
                goto bail; 
        }
        if (index >= DSGMAParserAuxNumberOfBases(aux)) {
                DSError(M_DS_WRONG ": Base index out of bounds", A_DS_ERROR);
                goto bail;
        }
        if (aux->bases == NULL) {
                DSError(M_DS_NULL ": Base array is NULL", A_DS_FATAL);
                goto bail;
        }
        name = aux->bases[index].name;
bail:
        return name;       
}

extern const double DSGMAParseAuxsConstantBaseAtIndex(const gma_parseraux_t * const aux, const DSUInteger index)
{
        double base = INFINITY;
        if (aux == NULL) {
                DSError(M_DS_NULL ": Parser auxiliary is NULL", A_DS_ERROR);
                goto bail; 
        }
        if (index >= DSGMAParserAuxNumberOfBases(aux)) {
                DSError(M_DS_WRONG ": Base index out of bounds", A_DS_ERROR);
                goto bail;
        }
        if (aux->bases == NULL) {
                DSError(M_DS_NULL ": Base array is NULL", A_DS_FATAL);
                goto bail;
        }
        base = aux->bases[index].value;
bail:
        return base;
}

extern const bool DSGMAParserAuxParsingFailed(const gma_parseraux_t *const aux)
{
        bool didFail = true;
        const gma_parseraux_t *temp = NULL;
        if (aux == NULL) {
                DSError(M_DS_NULL ": GMA Parser aux is NULL", A_DS_WARN);
                goto bail;
        }
        temp = aux;
        while (temp) {
                if (temp->succeded == false)
                        break;
                temp = temp->next;
        }
        if (temp == NULL)
                didFail = false;
bail:
        return didFail;
}









