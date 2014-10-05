//
//  serializationtest.c
//  DesignSpaceToolboxV2
//
//  Created by Jason Lomnitz on 10/4/14.
//
//

#include <stdio.h>
#include <string.h>
#include <designspace/DSStd.h>
#include <designspace/DSDataSerialization.pb-c.h>


int main(int argc, const char ** argv) {
        DSDesignSpace * ds = DSDesignSpaceByParsingStringList("x1. = a1 + a2*x2 - b1*x1", NULL, "x2. = a3*x1 + a4 - b2*x2", NULL);
        DSCase * case1 = DSDesignSpaceCaseWithCaseNumber(ds, 1);
        const DSSSystem * test = DSCaseSSystem(case1);
        DSSSystem * decoded = NULL;
        DSSSystemMessage * message = DSSSystemEncode(test);
        void * buffer;
        size_t length;
        length = dsssystem_message__get_packed_size(message);
        buffer = DSSecureMalloc(length);
        printf("size: %li\n", length);
        dsssystem_message__pack(message, buffer);
        decoded = DSSSystemDecode(length, buffer);
        printf("=== Encoded ===\n");
        DSSSystemPrintEquations(test);
        DSSSystemPrintSolution(test);
        printf("=== Decoded ===\n");
        DSSSystemPrintEquations(decoded);
        DSSSystemPrintSolution(decoded);
        DSSecureFree(buffer);
        return 0;
}
