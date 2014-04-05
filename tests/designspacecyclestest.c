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
        char * strings[7] = {"\0"};
        strings[0] = strdup("x1. = v1 + k21*x2 + k61*x6 + k71*x7 - k12*x1 - k16*x1 - k17*x1");
        strings[1] = strdup("x2. = k12*x1 - k21*x2 - k23*x2*x7");
        strings[2] = strdup("x3. = k23*x2*x7 + k563*x5*x6 - k34*x3 - k356*x3");
        strings[3] = strdup("x4. = k34*x3 - k45*x4 - k4*x4");
        strings[4] = strdup("x5. = k356*x3 + k45*x4 + k65*x6 - k563*x5*x6 - k56*x5");
        strings[5] = strdup("x6. = k356*x3 + k16*x1 + k56*x5 + k76*x7 - k563*x5*x6 - k61*x6 - k65*x6 - k67*x6");
        strings[6] = strdup("x7. = k17*x1 + k67*x6 - k23*x2*x7 - k76*x7 - k71*x7 - k7*x7");
        DSDesignSpace * ds;
        DSExpression ** expr = NULL;
        
        ds = DSDesignSpaceByParsingStrings(strings, NULL, 7);
        DSDesignSpaceCalculateCyclicalCases(ds);
        printf("Number of valid cases is: %i\n", DSDesignSpaceNumberOfValidCases(ds));
        printf("DSDesignSpaceNumberOfValidCases passed!\n");
        
        DSDesignSpaceFree(ds);
        printf("DSDesignSpaceFree passed!\n");
        
        return 0;
}