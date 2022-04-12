// Assignment 3 - Hamming codes / error correction codes.
// C23
// TODO: Proper header file

// - Input message
// - determine message size
// - split into small blocks (16 bits)
// -
// - display the message in binary to the screen.

#include <stdio.h>
#include <string.h>
#include <math.h>


/// MACROS
// shorthand if statements:
#define setBit(bits, bitIndex, val)  (val) ? (bits |= (val << (bitIndex)) ) : (bits &= (val << (bitIndex)) )
#define getBit(bits, bitIndex)   (bits & (1 << (bitIndex))) ? 1 : 0
#define flipBit(bits, bitIndex)  (bits & (1 << (bitIndex))) ? (bits &= ~(1 << (bitIndex))) : (bits |= (1 << (bitIndex)));



typedef char byte; // 8 bits is a byte


struct smallBlock {
    short bits : 16;
    int blockNo; // Used for ordering the blocks after transmission.
};


/// Prototypes
int calcNumOfBlocks(byte data[]);
void blockDisplayBin (struct smallBlock message);
void hammingEncodeFast(byte message[], struct smallBlock blocks[]);

int calcNumOfBlocks(byte data[]) {
    float numBits = (sizeof(byte)*8*strlen(data));
    int numBlocks = ceil(numBits/11);
    return numBlocks;
};


void blockDisplayBin (struct smallBlock block) {
    printf("\n");
    for (int i=0;i<4;i++) { // rows
        for (int j=0;j<4;j++) { // columns
            byte bit = getBit(block.bits, (i*4)+j);
            printf("%d   ", bit);
        }
        printf("\n");
    }
}


void hammingEncodeFast(byte message[], struct smallBlock blocks[]) {
    unsigned long numBits = (sizeof(byte)*8*strlen(message));
    byte finished = 0;
    byte dataIndexes[11] = {3, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15};
    for (int i=0;i<numBits;i++) {

    }


    for (int i=0; i<calcNumOfBlocks(message); i++) {
        short bits16 = (message[2*i] | (message[2*i + 1] << 8));
        printf("16bits: %d\n", bits16);
        for (int j=0;j<11;j++) {
            setBit(blocks[i].bits,
                   dataIndexes[j],
                   (getBit(bits16, j) << j));
            printf("%d", (getBit(bits16, j) << j));
        }
        printf("\n");

    }

}

int main() {


    byte message[1024] = "Hello";

    //printf("Please enter the string to be encoded using Hamming code\n");
    unsigned long numBits = (sizeof(byte)*8*strlen(message));


    int numBlocks = calcNumOfBlocks(message);
    printf("%u, %d", numBits, numBlocks);
    struct smallBlock blocks[numBlocks];
    blocks[0].bits = 0x00;
    blockDisplayBin(blocks[0]);
    setBit(blocks[0].bits, 11, 1);
    blockDisplayBin(blocks[0]);
    hammingEncodeFast(message, blocks);
    blockDisplayBin(blocks[0]);


}
