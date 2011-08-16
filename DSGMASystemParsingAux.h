//
//  DSGMASystemParsingAux.h
//  DesignSpaceToolboxV2
//
//  Created by Jason Lomnitz on 8/12/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

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

typedef struct parser_aux{
        char sign;
        union base_info {
                char * name;
                double value;
        } * bases;
        bool succeded;
        double *exponents;
        DSUInteger numberOfBases;
        struct parser_aux *next, *root;
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
extern const char * const DSGMAParserAuxVariableAtIndex(const gma_parseraux_t * const aux, const DSUInteger index);
extern const double DSGMAParseAuxsConstantBaseAtIndex(const gma_parseraux_t * const aux, const DSUInteger index);
extern const bool DSGMAParserAuxParsingFailed(const gma_parseraux_t *const aux);


#endif
