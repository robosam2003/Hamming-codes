// Assignment 3 - Hamming codes / error correction codes.
// C23
// TODO: Proper header file

#include <stdio.h>
#include <string.h>
#include <math.h>

/// Colour codes for text.
#define blackText "\033[0;30m"
#define redText "\033[0;31m"
#define greenText "\033[0;32m"
#define yellowText "\033[0;33m"
#define blueText "\033[0;34m"
#define magentaText "\033[0;35m"
#define cyanText "\033[0;36m"
#define whiteText "\033[0;37m"
#define resetText "\033[0m"
#define boldText "\033[1m"

#define MAX_MESSAGE_LENGTH 256

typedef char byte; // 8 bits is a byte

byte getBit(short bits, int bitIndex) {
    return (bits & (1 << (bitIndex))) ? 1 : 0;
}

void setBit(short *bits, int bitIndex, int val) {
    // This way, bits is ORed a one that is shifted left by bitIndex, which sets the bit to 1
    // likewise bit is ANDed with a byte like 0b11101111 for example, if the bit index was 4. This sets ONLY the 5th bit to 0;
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

int calcNumOfBlocks(byte data[MAX_MESSAGE_LENGTH]) {
    unsigned long long numDataBits = (1*8*strlen(data));
    double temp = numDataBits/11;
    int numBlocks = ceil(temp);
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

void blockDisplayMultipleBin (struct smallBlock blocks[], int numBlocks) {
    for (int k=0; k<numBlocks;k++) {
        int powersOf2 = {1,2,4,8};
        printf("\nBlock No: %d\n", blocks[k].blockNo);
        for (int i = 0; i < 4; i++) { // rows
            for (int j = 0; j < 4; j++) { // columns
//                (((i * 4) + j) == 1 | ((i * 4) + j) == 2 | ((i * 4) + j) == 4 | ((i * 4) + j) == 8) ? printf(yellowText) : printf(resetText);
//                ((i+j) == 0) ? printf(redText) : 0; // TODO: Uncomment in codeblocks
                byte bit = getBit(blocks[k].bits, (i * 4) + j);
                printf("%d   ", bit);
            }
            printf("\n");
        }
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

int getTotalParity(struct smallBlock block) {
    int totalParity = 0;
    for (int i=0;i<16;i++) {
        (getBit(block.bits, i)) ? totalParity ^=1 : 0;
    }
    return (totalParity);
}

void hammingEncodeFast(byte message[], struct smallBlock blocks[]) {
    double numDataBits = (double)(sizeof(byte) * 8 * strlen(message));
    /// Input Data into correct positions in blocks
    byte dataPositions[11] = {3, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15};
    int counter = 0;
    for (int i = 0; i < ceil(numDataBits / 11) * 11;) {
        blocks[counter].blockNo = counter;
        blocks[counter].bits = 0x0000;
        for (int j = 0; j < 11; j++) {
            int bit = ( getBit(message[i / 8], 7-(i % 8)) )       ?   1 : 0; /// This is in MSB first order
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
        /// Calculate total parity - This is used for detecting if there are multiple errors.
        int totalParity = getTotalParity(blocks[i]);
        (getBit(totalParity, 0)) ? flipBit(&blocks[i].bits, 0) : 0; // set total parity bit (at index 0)
    }

}

int openingSequence() {
    // print the hammingCodesTitleAscii.txt to screen

    FILE* fp = fopen("hammingCodesTitleAscii.txt", "r");



    if (fp == NULL) {
        printf("Error opening file - File is not in current workspace\n");
    }
    char c;
    while ((c = fgetc(fp)) != EOF) {
        printf("%c", c);
    }
    fclose(fp);

    /// print the opening instructions
    printf(boldText);
    //printf(redText);
    printf("\nThis is an program to demonstrate how Hamming Codes can be used to encode data and make it resilient to errors - a self-correcting message\n");
    printf("This is a common problem in data transmission and storage, and similar systems are used today.\n\n");
    printf(resetText);

    printf("The capabilities of this program are:\n");
    printf("\t1.Walk through the process of encoding and decoding data using Hamming codes, for educational purposes (recommended for first try)\n");
    printf("\t2 Quickly encode and decode text using Hamming Codes. Interlacing blocks is used for resilience against burst errors\n");
    printf("\t\t2.1. Quickly encode text data and save the encoded data\n");
    printf("\t\t2.2. Introduce a single or burst error to the encoded data to simulate bit flips in an actual transmission / storage system.\n");
    printf("\t\t2.3. Quickly detect and correct errors in data and decode it\n");


    printf("Please select an option to start: (1,2)\n");
    int option = 0;
    while (!(option== 1 || option == 2)) {
        scanf("%d", &option);
        (!(option == 1 || option == 2) )? printf("Not a valid option!\nPlease select an option to start: (1,2)\n"): 0;
    }
    return option;
}

void option1() {
    // educational walk through of encoding and decoding data using Hamming Codes
}
void option2() {
    printf(boldText);
    printf("\nENCODING TEXT\n");
    printf(resetText);
    printf("Would you like to encode a message you input or a file?: (1,2)\n");
    int messageOrFile = 0;
    while (!(messageOrFile == 1 || messageOrFile == 2)) {
        scanf("%d", &messageOrFile);
        (!(messageOrFile >= 1 && messageOrFile <= 2)) ? printf("Not a valid option!\nPlease select an option in the range: (1,2)\n"): 0;
    }
    if (messageOrFile == 1) {
        // getting user input
        printf("Please enter the message you would like to encode: (Max length is %d characters)\n", MAX_MESSAGE_LENGTH);
        byte message[MAX_MESSAGE_LENGTH];


        getchar(); // get rid of newline character
        gets(message); // TODO: Check if the message is too lo1ng

        printf("message is %d characters long\n", strlen(message));
        // encoding the message
        int numBlocks = calcNumOfBlocks(message);
        printf("NUMBER OF BLOCKS: %d\n", numBlocks);
        struct smallBlock blocks[numBlocks];
        hammingEncodeFast(message, blocks);

        if (numBlocks<=10) {
            // displaying the encoded message in binary#
            printf("ENCODED MESSAGE:\n");
            blockDisplayMultipleBin(blocks, numBlocks);
        }
    }
    else if (messageOrFile == 2){
        // TODO: Implement file reading and encoding
    }
}



int main() {
    int option = openingSequence();
    switch (option) {
        case 1:
            option1();
            break;
        case 2:

            break;
        case 3:

            break;
        case 4:

            break;
        default:
            printf("Error - invalid option\n");
            break;
    }
    while (1); // TODO implement a repeat option and exit option.
}

