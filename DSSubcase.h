//
//  DSSubcase.h
//  DesignSpaceToolboxV2
//
//  Created by Jason Lomnitz on 9/20/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "DSTypes.h"
#include "DSCase.h"

#ifndef __DS_SUBCASE__
#define __DS_SUBCASE__

extern DSMatrix * DSSubcaseProblematicEquations(const DSCase * aCase);
extern DSMatrixArray * DSSubcaseProblematicTerms(const DSCase *aCase, const DSMatrix * dependentEquations);
extern DSMatrixArray * DSSubcaseCoefficientsOfInterest(const DSCase * aCase, const DSMatrixArray * problematicTerms);

extern void DSSubcaseDesignSpaceForUnderdeterminedCase(const DSCase * aCase, const DSDesignSpace * original);

#endif
