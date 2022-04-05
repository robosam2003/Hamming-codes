// Assignment 3 - Hamming codes / error correction codes.
// C23
// TODO: Proper header file

// - Input message
// - determine message size
// - determine appropriate hamming code size
// - add hamming code parity bits
// - display the message in binary to the screen.

#include <stdio.h>
#include <string.h>
#include <math.h>

typedef char byte; // 8 bits is a byte
struct messageBlock {
    byte data[1024];
    int totalSize; // in bits
    int dataSize; // in bits
};

/// Prototypes
void messageDisplayBin (struct messageBlock message);




void messageDisplayBin (struct messageBlock message){
    int dimension = floor((message.totalSize)); // should be a power of two
    for (int i=0; i<dimension; i++) {
        printf()
    }

}

int main() {
    struct messageBlock message;
    printf("Please enter the string to be encoded using Hamming code\n");

    gets(message.data);
    printf("%s\n", message.data);

    printf("%d\n",( 8*sizeof(byte)*strlen(message.data)));
    printf("%d\n", log2())





}
