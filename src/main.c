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

typedef char byte; // 8 bits is a byte

byte getBit(short bits, int bitIndex) {
    return (bits & (1 << (bitIndex))) ? 1 : 0;
}

void setBit(short *bits, int bitIndex, int val) {
    (val) ? (*bits |= (1 << (bitIndex))) : (*bits &= ~(1 << (bitIndex)));
}

void flipBit(short *bits, int bitIndex) {
    (*bits & (1 << (bitIndex))) ? (*bits &= ~(1 << (bitIndex))) : (*bits |= (1 << (bitIndex)));
}

struct smallBlock {
    short bits;
    int blockNo; // Used for ordering the blocks after transmission.
};


/// Prototypes
int calcNumOfBlocks(byte data[]);
void blockDisplayBin (struct smallBlock message);
void hammingEncodeFast(byte message[], struct smallBlock blocks[]);

int calcNumOfBlocks(byte data[]) {
    double numDataBits = (sizeof(byte)*8*strlen(data));
    int numBlocks = ceil(numDataBits/11);
    return numBlocks;
}

void blockDisplayBin (struct smallBlock block) {
    printf("\nBlock No: %d\n", block.blockNo);
    for (int i=0;i<4;i++) { // rows
        for (int j=0;j<4;j++) { // columns
            byte bit = getBit(block.bits, (i*4)+j);
            printf("%d   ", bit);
        }
        printf("\n");
    }
}

int getParity(struct smallBlock block) {
    int parity = 0;
    for (int i=0;i<16;i++) {
        if (getBit(block.bits, i)) {
            parity ^= i;
        }
    }
    return parity;
}


void hammingEncodeFast(byte message[], struct smallBlock blocks[]) {
    double numDataBits = (sizeof(byte) * 8 * strlen(message));

    /// Input Data into correct positions in blocks
    byte dataPositions[11] = {3, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15};
    int counter = 0;
    for (int i = 0; i < ceil(numDataBits / 11) * 11;) {
        blocks[counter].blockNo = counter;
        blocks[counter].bits = 0x0000;
        for (int j = 0; j < 11; j++) {
            int bit = (getBit(message[i / 8], 7 - (i % 8))) ? 1 : 0;
            setBit(&(blocks[counter].bits), dataPositions[j], bit);
            i++;
        }
        counter++;
    }

    /// Calculate Parity bits
    for (int i = 0; i < ceil(numDataBits / 11); i++) {
        int parity = getParity(blocks[i]);
        (getBit(parity, 0)) ? flipBit(&blocks[i].bits, 1) : 0; // set parity bit of first parity group (odd columns)
        (getBit(parity, 1)) ? flipBit(&blocks[i].bits, 2) : 0; // set parity bit of second group (right half)
        (getBit(parity, 2)) ? flipBit(&blocks[i].bits, 4) : 0; // set parity bit of third group (odd rows)
        (getBit(parity, 3)) ? flipBit(&blocks[i].bits, 8) : 0; // set parity bit of fourth group (bottom half)
        // Parity should now be zero.
    }

    printf("%d\n", getParity(blocks[0]));
}

int main() {


    byte message[1024] = "samyak";

    //printf("Please enter the string to be encoded using Hamming code\n");
    unsigned long numBits = (sizeof(byte)*8*strlen(message));
    int numBlocks = calcNumOfBlocks(message);
    struct smallBlock blocks[numBlocks];

    hammingEncodeFast(message, blocks);

    blockDisplayBin(blocks[0]);
    blockDisplayBin(blocks[1]);
    blockDisplayBin(blocks[2]);
    blockDisplayBin(blocks[3]);
    //flipBit(&(blocks[0].bits), 0);
    printf("%d, %d, %d, %d\n", getParity(blocks[0]),
           getParity(blocks[1]),
           getParity(blocks[2]),
           getParity(blocks[3]));



}
