//
//  DSExpressionTokenizer.c
//  DesignSpaceToolboxV2
//
//  Created by Jason Lomnitz on 8/9/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include "DSExpressionTokenizer.h"

extern struct expression_token * DSExpressionTokenAlloc()
{
        struct expression_token *aToken = NULL;
        aToken = DSSecureCalloc(1, sizeof(struct expression_token));
        DSExpressionTokenSetType(aToken, DS_EXPRESSION_TOKEN_START);
        return aToken;
}

extern void DSExpressionTokenFree(struct expression_token *root)
{
        struct expression_token *next = NULL;
        if (root == NULL) {
                DSError(M_DS_NULL ": token to free is NULL", A_DS_ERROR);
                goto bail;
        }
        while (root) {
                next = DSExpressionTokenNext(root);
                if (DSExpressionTokenType(root) == DS_EXPRESSION_TOKEN_ID)
                        DSSecureFree(DSExpressionTokenString(root));
                DSSecureFree(root);
                root = next;
        }
bail:
        return;
}

extern void DSExpressionTokenSetString(struct expression_token *root, char *string)
{
        if (root == NULL) {
                DSError(M_DS_NULL ": Variable token is NULL", A_DS_ERROR);
                goto bail;
        }
        if (string == NULL) {
                DSError(M_DS_WRONG ": String is NULL", A_DS_ERROR);
                goto bail;
        }
        root->data.name = string;
bail:
        return;
}

extern void DSExpressionTokenSetDouble(struct expression_token *root, double value)
{
        if (root == NULL) {
                DSError(M_DS_NULL ": Variable token is NULL", A_DS_ERROR);
                goto bail;
        }
        root->data.value = value;
bail:
        return;
}

extern char * DSExpressionTokenString(struct expression_token *root)
{
        char *string = NULL;
        if (root == NULL) {
                DSError(M_DS_NULL ": Variable token is NULL", A_DS_ERROR);
                goto bail;
        }
        string = DSExpressionTokenData(root).name;
bail:
        return string;
}

extern double DSExpressionTokenDouble(struct expression_token *root)
{
        double value = NAN;
        if (root == NULL) {
                DSError(M_DS_NULL ": Variable token is NULL", A_DS_ERROR);
                goto bail;
        }
        value = DSExpressionTokenData(root).value;
bail:
        return value;
}
