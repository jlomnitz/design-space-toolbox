//
//  designspacetest.c
//
//
//  Created by Jason Lomnitz on 5/13/13.
//
//
#include <stdio.h>
#include <string.h>
#include <designspace/DSStd.h>

int main(int argc, const char ** argv) {
        int i;
        char * strings[2] = {'\0'};
        strings[0] = strdup("x1. = a + b*x1*x2 - c*x1");
        strings[1] = strdup("x2. = c*x1 - x2");
        DSDesignSpace * ds;
        DSExpression ** expr = NULL;
        DSExpression * anExpression;
        
        ds = DSDesignSpaceByParsingStrings(strings, NULL, 2);
        expr = DSDesignSpaceEquations(ds);
        
        for (i = 0; i < DSDesignSpaceNumberOfEquations(ds); i++) {
                DSExpressionPrint(expr[i]);
                DSExpressionFree(expr[i]);
        }
        
        DSSecureFree(expr);
        
        printf("Number of valid cases is: %i\n", DSDesignSpaceNumberOfValidCases(ds));
        printf("DSDesignSpaceNumberOfValidCases passed!\n");
        
        DSDesignSpaceFree(ds);
        printf("DSDesignSpaceFree passed!\n");
        
        anExpression = DSExpressionByParsingString("real(sqrt(-1)+(&i))");
        double complex complex_test = DSExpressionEvaluateWithVariablePool(anExpression, NULL);
        DSExpressionPrint(anExpression);
        printf("complex test = %f + %f*i\n", creal(complex_test), cimag(complex_test));
        printf("Complex arithmatic passed passed!\n");
        
        return 0;
}