/**
 * \file DSGMASystemParsingAux.h
 * \brief Header file with functions for dealing with the parsing of GMA
 * Systems.
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
 * \date 2011
 */


#include "DSTypes.h"

#ifndef __DS_GMA_PARSING_AUX__
#define __DS_GMA_PARSING_AUX__

#define AUX_EXPONENT_CONSTANT_BASE     NAN
#define AUX_SIGN_UNDEFINED             '?'
#define AUX_SIGN_POSITIVE              '+'
#define AUX_SIGN_NEGATIVE              '-'

#define AUX_PARSER_FAILED               false
#define AUX_PARSER_SUCCESS              true

#define DSGMAParserAuxNumberOfBases(x)             (x->numberOfBases)
#define DSGMAParserAuxBaseAtIndexIsVariable(x, y)  (!isnan(DSGMAParserAuxExponentAtIndex(x, y)))
#define DSGMAParserAuxSetParserFailed(x)           ((x)->succeded = false) 

/**
 * \brief Data type used to parse strings to GMA System.
 *
 * \details This data structure forms an organized list of terms, each
 * with base exponent pairs that are then used to create the system matrices.
 * This data structure is key for the parsing of GMA systems.  Each node in 
 * the gma_parseraux_t list represent a term in an expression in the order it
 * was found, and each node points to the next term.  Each expression, or
 * equation, has it's own list of terms.  If a base is a constant, then it should
 * not have an exponent, and hence it's exponent is assigned a NAN value and this
 * is used to indicate that the base is a constant.
 */
typedef struct parser_aux{
        char sign;                //!< The sign of the term represented by the current node.
        union base_info {
                char * name;      //!< The string representing the name of the variable.
                double value;     //!< The variable representing the value of a constant.
        } * bases;                //!< Dynamically allocated array of bases, can be either variables or constants.
        bool succeded;            //!< A flag indicating if the parsing of the expression was succesful.
        double *exponents;        //!< A dynamically allocated array of exponents, must be constants.
        DSUInteger numberOfBases; //!< The number of base-exponents pairs in the term.
        struct parser_aux *next;  //!< A pointer to the next node, representing the next term in the equation.
} gma_parseraux_t;

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Allocation, deallocation and initialization
#endif

extern gma_parseraux_t * DSGMAParserAuxAlloc(void);
extern void DSGMAParserAuxFree(gma_parseraux_t *root);
extern void DSGMAParserAuxNewTerm(gma_parseraux_t *current);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Setter functions
#endif

extern gma_parseraux_t * DSGMAParserAuxNextNode(const gma_parseraux_t * const aux);
extern void DSGMAParserAuxSetSign(gma_parseraux_t *aux, const char sign);
extern void DSGMAParserAuxAddVariableExponentPair(gma_parseraux_t *aux, const char *const name, const double exponent);
extern void DSGMAParserAuxAddConstantBase(gma_parseraux_t *aux, const double base);

#if defined (__APPLE__) && defined (__MACH__)
#pragma mark - Getter functions
#endif

extern const char DSGMAParserAuxSign(const gma_parseraux_t * const aux);
extern const double DSGMAParserAuxExponentAtIndex(const gma_parseraux_t *const aux, const DSUInteger index);
extern const char * DSGMAParserAuxVariableAtIndex(const gma_parseraux_t * const aux, const DSUInteger index);
extern const double DSGMAParseAuxsConstantBaseAtIndex(const gma_parseraux_t * const aux, const DSUInteger index);
extern const bool DSGMAParserAuxParsingFailed(const gma_parseraux_t *const aux);


#endif
