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
        DSMatrix * decoded;
        DSMatrix * matrix = DSMatrixRandomNumbers(5, 5);
        DSMatrixMessage message = DSMatrixEncode(matrix);
        void * buffer;
        DSUInteger length;
        DSMatrixPrint(matrix);

        DSMatrixFree(matrix);
        length = dsmatrix_message__get_packed_size(&message);
        buffer = DSSecureMalloc(length);
        printf("size: %i\n", length);
        dsmatrix_message__pack(&message, buffer);
        decoded = DSMatrixDecode(length, buffer);
        DSMatrixPrint(decoded);
        DSMatrixFree(decoded);
        DSSecureFree(buffer);
        return 0;
}
