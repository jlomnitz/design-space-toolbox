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
        DSDesignSpace * ds = DSDesignSpaceByParsingStringList("x1. = a1 + a2*x2 - b1*x1", NULL, "x2. = a3*x1 + a4 + b1*x1 - a2*x2 - b2*x2", NULL);
        DSDesignSpaceCalculateCyclicalCases(ds);
        DSDesignSpace * decodedDs;
        DSUInteger i;
        void * buffer, * newBuffer;
        size_t length;
        printf("=== Encoded ===\n");
        DSDesignSpaceMessage * message = DSDesignSpaceEncode(ds);
        length = dsdesign_space_message__get_packed_size(message);
        buffer = DSSecureMalloc(length);
        printf("size: %li\n", length);
        DSDesignSpacePrint(ds);
        DSDictionaryPrint(ds->cyclicalCases);
        dsdesign_space_message__pack(message, buffer);
        DSIOWriteBinaryData("/var/tmp/tmodhjshdjds.kaka", length, buffer);
        DSSecureFree(buffer);
        buffer = DSIOReadBinaryData("/var/tmp/tmodhjshdjds.kaka", &length);
        decodedDs = DSDesignSpaceDecode(length, buffer);
        printf("=== Decoded ===\n");
        printf("size: %li\n", length);
        DSDesignSpacePrint(decodedDs);
        DSDictionaryPrint(decodedDs->cyclicalCases);
        DSSecureFree(buffer);
        printf("\n");
        remove("/var/tmp/tmodhjshdjds.kaka");
        DSDesignSpaceFree(ds);
        ds = decodedDs;
        for (i = 0; i < DSDesignSpaceNumberOfCases(ds); i++) {
                DSCase * case1 = DSDesignSpaceCaseWithCaseNumber(ds, i+1);
                DSCase * decoded = NULL;
                printf("=== Encoded ===\n");
                DSCaseMessage * message = DSCaseEncode(case1);
                length = dscase_message__get_packed_size(message);
                buffer = DSSecureMalloc(length);
                printf("size: %li\n", length);
                DSCasePrint(case1);
                dscase_message__pack(message, buffer);
                DSIOWriteBinaryData("/var/tmp/tmodhjshdjds.kaka", length, buffer);
                newBuffer = DSIOReadBinaryData("/var/tmp/tmodhjshdjds.kaka", &length);
                printf("Reading %li bytes\n", length);
                decoded = DSCaseDecode(length, newBuffer);
                printf("=== Decoded ===\n");
                printf("size: %li\n", length);
                DSCasePrint(decoded);
                DSSecureFree(buffer);
                DSSecureFree(newBuffer);
                printf("\n");
                remove("/var/tmp/tmodhjshdjds.kaka");
        }
        return 0;
}
