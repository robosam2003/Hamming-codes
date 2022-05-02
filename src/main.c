/* Assignment 3 - Hamming codes
 * C Version: C23
 * Author: Samuel Scott
 * Date: 23/04/2022
 * Description: This is a program to demonstrate how Hamming codes can be used to encode data and make it resilient to errors, and correct them if they are present.
 *              This is a common problem in data transmission and storage, where data can get corrupted due to electromagnetic interference, scratches on the disks etc.
 *              Hamming codes are an early form of error correction codes, and similar systems are used today.
 *              The program uses interlacing of blocks to ensure resilience to burst errors.
 *
 *              The user is presented with two options. Either they can 1) Have an educational, interactive walk through of how Hamming codes work or
 *              2) they can encode a message / file, simulate a "bit flip" error if they like, and then decode the message back to its original form, having corrected
 *              any errors (when possible).
 *
 *
 * Required files: hammingCodesTitleAscii.txt
 * Files used for as examples (for encoding and decoding): arcade.mp3, arrival.jpg, attractor1.jpg, attractor2.png, dunkirk.png, thrust.mp4, hammingCodesTitleAscii.txt
 *
 *
 * Please note that with large files (e.g. attractor2.png) the program will take a long time to decode, (about 30 seconds).
 */



#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

// Colour codes for text output to console
#include "colours.h" // for conciseness, I've put the colour codes in a header file.

#define MAX_MESSAGE_SIZE 30*1024*1024 // 30 MiB of memory

typedef char byte; // makes the code easier to read

struct smallBlock {
    short bits;
    int blockNo; // Although in _this_ application block numbers are not *strictly* needed (contiguous storage),
                 //  they are needed for tasks like transmission, where blocks may arrive at different times (e.g. packet switching)
                 //  and makes the decoding process more life-like (this program only *simulates* transmission).
};

enum blockDisplayStates { // makes the code easier to read, instead of using numbers
    parityVsDataColoured,
    dataColoured,
    parityColoured,
    totalParityColoured,
    noColour,
    parityGroup1Coloured, // odd or even columns
    parityGroup2Coloured, // right or left half
    parityGroup3Coloured, // odd or even rows
    parityGroup4Coloured  // top or bottom half
};

// Prototypes
byte getBit(short bits, int bitIndex);
void setBit(short *bits, int bitIndex, int val);
void flipBit(short *bits, int bitIndex);
int calcNumOfBlocks(byte data[], int numBytes);
void blockDisplayBin (struct smallBlock block, enum blockDisplayStates blockDisplayState);
void blockDisplayMultipleBin (struct smallBlock blocks[], int numBlocks, enum blockDisplayStates blockDisplayState);
int getParity(struct smallBlock block);
int getTotalParity(struct smallBlock block);
void hammingEncodeFast(byte message[], struct smallBlock blocks[], int numBytes);
void hammingDecodeFast(char hexFilename[], char originalFilename[]);
void writeBlocksToFile(struct smallBlock blocks[], int numBlocks, char filename[]);
void writeBlocksToFileAndInterlace(struct smallBlock blocks[], int numBlocks, char fileName[]);
void readBlocksFromFileAndDelace(struct smallBlock blocks[], int numBlocks, char* filename);
void introduceError(int length, char hexFilename[], int bitIndex);
int openingSequence();
int findFileSize(FILE* fp);
void option1();
void option2();

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

int calcNumOfBlocks(byte data[], int numBytes) {
    unsigned long long numDataBits=0;
    if (!numBytes) {
        numDataBits = (1*8*strlen(data));
    }
    else {
        numDataBits = (1*8*numBytes);
    }
    double temp = ((double)numDataBits)/11;
    int numBlocks = ceil(temp);
    return numBlocks;
}

void blockDisplayBin (struct smallBlock block, enum blockDisplayStates blockDisplayState) {
    byte totalParityBitIndex = 0;
    byte parityBitIndexes[5] = {0, 1, 2, 4, 8};
    byte parityGroup1Indexes[8] = {1, 3, 5, 7, 9, 11, 13, 15};
    byte parityGroup2Indexes[8] = {2, 3, 6, 7, 10, 11, 14, 15};
    byte parityGroup3Indexes[8] = {4, 5, 6, 7, 12, 13, 14, 15};
    byte parityGroup4Indexes[8] = {8, 9, 10, 11, 12, 13, 14, 15};

    printf("\nBlock No: %d\n", block.blockNo);
    for (int i=0;i<4;i++) { // rows
        for (int j=0;j<4;j++) { // columns
            switch (blockDisplayState) {
                case noColour:
                    printf(reset);
                    break;
                case parityVsDataColoured:
                    printf(reset);
                    printf(BLUB);
                    for (int a=0;a<5;a++) {
                        if ((i * 4) + j == parityBitIndexes[a]) {
                            printf(YELB);
                        }
                    }
                    (( (i*4) + j ) == totalParityBitIndex) ? printf(REDB) : 0;
                    break;
                case dataColoured:
                    printf(reset);
                    printf(BLUB);
                    for (int a=0;a<5;a++) {
                        if ((i * 4) + j == parityBitIndexes[a]) {
                            printf(BLKB BLK);
                        }
                    }
                    (( (i*4) + j ) == totalParityBitIndex) ? printf(BLK) : 0;
                    break;
                case parityColoured:
                    printf(reset);
                    printf(BLK);
                    for (int a=0;a<5;a++) {
                        if ((i * 4) + j == parityBitIndexes[a]) {
                            printf(YELB);
                        }
                    }
                    (( (i*4) + j ) == totalParityBitIndex) ? printf(REDB) : 0;
                    break;
                case parityGroup1Coloured:
                    printf(reset);
                    for (int a=0;a<8;a++) {
                        if ((i * 4) + j == parityGroup1Indexes[a]) {
                            printf(MAGB);
                        }
                    }
                    (( (i*4) + j ) == parityBitIndexes[1]) ? printf(GRNB) : 0;
                    break;
                case parityGroup2Coloured:
                    printf(reset);
                    for (int a=0;a<8;a++) {
                        if ((i * 4) + j == parityGroup2Indexes[a]) {
                            printf(MAGB);
                        }
                    }
                    (( (i*4) + j ) == parityBitIndexes[2]) ? printf(GRNB) : 0;
                    break;
                case parityGroup3Coloured:
                    printf(reset);
                    for (int a=0;a<8;a++) {
                        if ((i * 4) + j == parityGroup3Indexes[a]) {
                            printf(MAGB);
                        }
                    }
                    (( (i*4) + j ) == parityBitIndexes[3]) ? printf(GRNB) : 0;
                    break;
                case parityGroup4Coloured:
                    printf(reset);
                    for (int a=0;a<8;a++) {
                        if ((i * 4) + j == parityGroup4Indexes[a]) {
                            printf(MAGB);
                        }
                    }
                    (( (i*4) + j ) == parityBitIndexes[4]) ? printf(GRNB) : 0;
                    break;
                case totalParityColoured:
                    printf(reset);
                    (( (i*4) + j ) == totalParityBitIndex) ? printf(GRNB) : 0;
                    break;
                default:
                    break;
            }
            byte bit = getBit(block.bits, (i*4)+j);
            printf("%d  " reset, bit);
        }
        printf(BLK "_\n"reset);
    }
}

void blockDisplayMultipleBin (struct smallBlock blocks[], int numBlocks, enum blockDisplayStates blockDisplayState) {
    for (int k=0; k<numBlocks;k++) {
        blockDisplayBin(blocks[k], blockDisplayState);
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
        (getBit(block.bits, i)) ? totalParity ^= 1 : 0;
    }
    return (totalParity);
}

void hammingEncodeFast(byte message[], struct smallBlock blocks[], int numBytes) { // input zero if the input is a string and you don't know the number of bytes.
    double numDataBits;
    if (!numBytes) {
        numDataBits = (double)(sizeof(byte) * 8 * strlen(message));
    }
    else {
        numDataBits = (double)(sizeof(byte) * 8 * numBytes);
    }

    // Input Data into correct positions in blocks
    byte dataPositions[11] = {3, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15};
    int counter = 0;
    for (int i = 0; i < ceil(numDataBits / 11) * 11;) {
        blocks[counter].blockNo = counter;
        blocks[counter].bits = 0x0000;
        for (int j = 0; j < 11; j++) {
            int bit = ( getBit(message[i / 8], 7-(i % 8)) )       ?   1 : 0; // This is in MSB first order
            setBit(&(blocks[ blocks[counter].blockNo ].bits), dataPositions[j], bit);
            i++;
        }
        counter++;
    }

    // Calculate Parity bits
    for (int i = 0; i < ceil(numDataBits / 11); i++) {
        int parity = getParity(blocks[i]);
        (getBit(parity, 0)) ? flipBit(&blocks[i].bits, 1) : 0; // set parity bit of first parity group (odd columns)
        (getBit(parity, 1)) ? flipBit(&blocks[i].bits, 2) : 0; // set parity bit of second group (right half)
        (getBit(parity, 2)) ? flipBit(&blocks[i].bits, 4) : 0; // set parity bit of third group (odd rows)
        (getBit(parity, 3)) ? flipBit(&blocks[i].bits, 8) : 0; // set parity bit of fourth group (bottom half)
        // Parity should now be zero.
        // Calculate total parity - This is used for detecting if there are multiple errors.
        int totalParity = getTotalParity(blocks[i]);
        (getBit(totalParity, 0)) ? flipBit(&blocks[i].bits, 0) : 0; // set total parity bit (at index 0)
    }

}

void hammingDecodeFast(char hexFilename[], char originalFilename[]) {
    printf("\n Decoding file %s   ...   Please wait.\n\n", hexFilename);
    FILE* fp = fopen(hexFilename, "r+");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return;
    }
    int numBlocks = findFileSize(fp)/sizeof(short); // only the bits were written to the file.
    struct smallBlock* blocks;
    int wasOddNumBlocks = numBlocks % 2;
    if (numBlocks%2){
        numBlocks++;
        // malloc can't initialise odd number of blocks?????????? - very weird behaviour
    }
    blocks = (struct smallBlock*) malloc(numBlocks * sizeof(struct smallBlock)); // have to use malloc() because cannot declare an array of structs that is that large
    (wasOddNumBlocks) ? numBlocks-- : 0;  // we only set it to even so that malloc would work.


    // delace the bits from the file and write them to the blocks
    readBlocksFromFileAndDelace(blocks, numBlocks, hexFilename);
    int num1Errors = 0;
    int num2Errors = 0;
    // find errors in the blocks and decode them.
    for (int i=0;i<numBlocks;i++) {
        struct smallBlock currentBlock = blocks[ blocks[i].blockNo ];
        int currentBlockParity = getParity(currentBlock);
        if ((currentBlockParity == 0) && (getTotalParity(currentBlock) == 1)) { // if there is an error at position zero (total parity)
            flipBit(&currentBlock.bits, currentBlockParity);
            num1Errors++;
        }
        else if (currentBlockParity && (getTotalParity(currentBlock) == 0)) { // there is two errors in the block
                // if the parity of the whole block is even, AND the current block parity is non-zero then two bits got flipped in a single block.
                // Note, cases with 3 or more errors per block is not covered by hamming codes
                num2Errors += 1;
        }
        else if (currentBlockParity && (getTotalParity(currentBlock) == 1)) { // there is one error in the block
                flipBit(&currentBlock.bits, currentBlockParity);
                num1Errors++;
        }
    }

    if (num1Errors == 0) {
        printf("\nThere are no errrors!!\n");
    }
    else {
        printf("Found a total of %d errors in the file.\nA total of %d errors were corrected\n", (num1Errors+2*num2Errors), num1Errors);
        if (num2Errors > 0 ) {
            printf("\nSome errors could not be corrected, as there were too many, hence the output file is corrupted.");
        }
    }

    // write the corrected blocks back to the file
    writeBlocksToFile(blocks, numBlocks, originalFilename);

    free(blocks);
    fclose(fp);
}

void writeBlocksToFile(struct smallBlock blocks[], int numBlocks, char filename[]) {
    char newFilename[100];
    int unixTime = time(NULL); // Unix time in seconds
    sprintf(newFilename, "%d_%s", unixTime, filename);
    FILE* fp = fopen(newFilename, "wb");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return;
    }
    int numDataBits = numBlocks * 11;
    byte dataPositions[11] = {3, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15};
    for (int i = 0; i < numDataBits;) {
        byte dataByte = 0;
        for (int j = 0; j < 8; j++) {
            int bit = getBit(blocks[i / 11].bits, dataPositions[i % 11]);
            setBit(&dataByte, 7-j, bit); // MSB OR LSB???
            i++;
        }
        fwrite(&dataByte, sizeof(byte), 1, fp);
    }

    printf("Blocks written to file %s\n", newFilename);
    fclose(fp);
};

void writeBlocksToFileAndInterlace(struct smallBlock blocks[], int numBlocks, char fileName[]) {
    int unixTime = time(NULL); // Unix time in seconds
    // By using the unix time as a file name, each file name will be unique each time the program is run
    sprintf(fileName, "HammingFile%d.hex", unixTime);
    FILE* fp = fopen(fileName, "wb");
    if (fp == NULL) {
        printf("Error opening file!\n");
    }
    short interlacedBits = 0;

    printf("Number of bits: %d\n", numBlocks*8*sizeof(short));

    for (int i=0; i<numBlocks*8*sizeof(short); i++) {
        byte currentBit = getBit( blocks[i%numBlocks].bits,
                                  (i/numBlocks));
        setBit( &interlacedBits,
                15-(i%16),
                currentBit ); // interlacing the bits // MSB first
        if ((i+1)%16 == 0 ) { // has to be (i+1)%16 so that it writes at the correct time, i.e, i is one less than a multiple of 16
            byte highByte = (byte)(interlacedBits >> 8);
            byte lowByte = (byte)(interlacedBits & 0xFF);
            //fwrite(&interlacedBits, sizeof(short), 1, fp);
            putc(lowByte, fp); // NOTE: this will fail on big endian machines
            putc(highByte, fp);
            interlacedBits = 0;
        }
    }
    fclose(fp);
    printf("Encoded message blocks written to %s\n", fileName);
}

void readBlocksFromFileAndDelace(struct smallBlock blocks[], int numBlocks, char hexFilename[]) {
    // delace the bits from the file and write them to the blocks
    FILE* fp = fopen(hexFilename, "rb");
    if (fp == NULL) {
        printf("Error opening file!\n");
    }
    for (int j=0;j<numBlocks;j++) {
        blocks[j].blockNo = j;
    }
    short interlacedBits;
    for (int i=0; i<numBlocks*8*sizeof(short); i++) {
        if ((i%16) == 0) {
            rewind(fp);
            (fseek(fp, 2*(i/16), SEEK_SET)) ? printf("error seeking file") : 0; // seek to the correct position in the file
            int a = fread(&interlacedBits, sizeof(short), 1, fp); // read the bits from the file
        }

        byte currentBit = getBit(interlacedBits, 15-(i%16));
        int b = (i / numBlocks);
        setBit(&blocks[ i%numBlocks ].bits,
               (i / numBlocks), // MSB first
               currentBit); // delacing the bits // MSB first
    }
    fclose(fp);
}

void introduceError(int length, char hexFilename[], int bitIndex) {
    printf("Filename: %s\n", hexFilename);
    printf("\n Introducing error  ...  Please wait.\n\n");
    FILE* fp = fopen(hexFilename, "rb+"); // note: rb+ mode allows us to overwrite certain parts of the file.
    if (fp == NULL) { // check if the file exists
        printf("Error opening file!\n");
    }
    int counter = 0;
    rewind(fp);
    for (int i=0;i<length;i++) {
        rewind(fp);
        (fseek(fp, 2*((bitIndex+i) / 16), SEEK_SET)) ? printf("Error seeking to index!\n") : 0;
        short sixteenBits = 0x0000;
        int numShortsRead = fread(&sixteenBits, sizeof(short), 1, fp);
        if (feof(fp)) {
            printf("Error reading file: unexpected end of file\n");
            return;
        }
        else if (ferror(fp)) {
            printf("Error reading test.bin");
            return;
        }
        (fseek(fp, 2*((bitIndex+i) / 16), SEEK_SET)) ? printf("Error seeking to index!\n"): 0; // seek back to position

        flipBit(&sixteenBits, ((bitIndex+i) % 16)); // flip the bit
        int numShortsWritten = fwrite(&sixteenBits, sizeof(sixteenBits), 1, fp);
        (fseek(fp, 2*((bitIndex+i) / 16), SEEK_SET)) ? printf("Error seeking to index!\n") : 0; // seek back to position
    }
    printf("Error introduced at bit index %d of length %d \n", bitIndex, length);
    fclose(fp);

}

int openingSequence() {
    system("cls"); // clear the screen
    // print the hammingCodesTitleAscii.txt to screen

    FILE* fp = fopen("hammingCodesTitleAscii.txt", "r");
    if (fp == NULL) {
        printf(BWHT "Error opening Title file (hammingCodesTitleAscii.txt) - File is not in current workspace or doesnt exist.\n");
    }
    char c;
    printf(BHWHT);
    while ((c = fgetc(fp)) != EOF) {
        printf("%c", c); // bold white text
    }
    printf(reset);
    fclose(fp);

    // print the opening instructions
    printf(BHWHT "\nThis is an program to demonstrate how Hamming Codes can be used to encode data and make it resilient to errors - a self-correcting message\n");
    printf("This is a common problem in data transmission and storage, and similar systems are used today.\n\n" reset);

    printf("The capabilities of this program are:\n");
    printf(CYN"\t1. Walk through the process of encoding and decoding data using Hamming codes, for educational purposes (recommended for first try).\n");
    printf("\t2. Quickly encode and decode data using Hamming Codes. Interlacing blocks is used for resilience against burst errors:\n");
    printf("\t\t2.1. Quickly encode data and save the encoded data\n");
    printf("\t\t2.2. Introduce a single or burst error to the encoded data to simulate bit flips in an actual transmission / storage system.\n");
    printf("\t\t2.3. Quickly detect and correct errors in data and decode it\n" reset);


    printf("Please select an option to start: (1 or 2)\n");
    int option = 0;
    while (!(option== 1 || option == 2)) {
        fflush(stdin);
        scanf("%d", &option);
        (!(option == 1 || option == 2) )? printf("Not a valid option!\nPlease select an option to start: (1,2)\n"): 0;
    }
    return option;
}

int findFileSize(FILE* fp) {
    fseek(fp, 0L, SEEK_END); // seek to end of file
    int size = ftell(fp); // find counter position
    fseek(fp, 0L, SEEK_SET); // seek back to beginning of file to put pointer back in place
    return size;
}

void option1() {
    // educational walk through of encoding and decoding data using Hamming Codes
    printf("\n\n"BHBLU"Educational walk through of encoding and decoding data using Hamming Codes" reset ": \n");
    printf(BHWHT "Hamming Codes"  reset " are a way of encoding data to make it resilient to errors.\n");
    printf("Data can get" BWHT " corrupted" reset " during transmission or storage, (Through electromagnetic interference, scratches on the disk etc) which is obviously a problem, if you like your data intact.\n");
    printf("This is where Hamming Codes come in. Hamming encoding can find single bit errors by adding parity bits in specific places in memory\n\n");
    printf("Reminder: Parity is the state of being even or odd, and in the case of data, the parity is EVEN (0) if there is an even number of 1's in a given set of data,\nand ODD (1) if there is an odd number of 1's in a given set of data.\n\n");
    printf("For the purpose of this educational walk though, please provide a word or phrase to encode: (<20 characters)\n");


    char phrase[50] = "";
    getchar(); // get rid of newline character
    gets(phrase);
    while((strlen(phrase) == 0) || (strlen(phrase) >= 20)) {
        fflush(stdin);
        printf("That phrase is not less that 20 characters! Please try again:\n");
        gets(phrase);
    }




    printf(BHBLU"\n\nThe process of encoding data using Hamming Codes is as follows:\n");
    printf(BHGRN"\t1. Choose a block size that is\n"
           "\t\t a) a power of 2 and\n"
           "\t\t b) small enough such that the likelihood of multiple errors in a single block is low but as large as possible to increase memory efficiency.\n" reset);
    printf("\t   For simplicity, we will use a block size of 16 (11 data bits and 5 parity bits)\n\n");

    printf(BHGRN"\t2. Split the data into chunks of (block size - log_2(block size) -1)   (11 in this case)\n" reset);
    printf("\t   For the phrase you provided, "BHYEL "(\"%s\")" reset", the binary data is coloured in BLUE (below, MSB first)\n\n", phrase);
    printf(BHGRN"\t3. Arrange these chunks into blocks of size (block size) (16 in our case) arranging them in indexes that are not powers of 2 and not zero (as these are reserved for parity bits)\n"reset);
    int numBlocks = calcNumOfBlocks(phrase, 0);
    struct smallBlock* blocks;
    int wasOddNumBlocks = numBlocks % 2;
    if (numBlocks%2) {
        numBlocks++;
        // malloc can't initialise odd number of blocks?????????? - very weird behaviour
    }
    blocks = (struct smallBlock*) malloc(numBlocks * sizeof(struct smallBlock)); // have to use malloc() because cannot declare an array of structs that is that variable size
    (wasOddNumBlocks) ? numBlocks-- : 0;  // we only set it to even so that malloc would work, resetting it.

    hammingEncodeFast(phrase, blocks, 0); // encode the message
    blockDisplayMultipleBin(blocks, numBlocks, dataColoured);
    printf("For reference later: The indexes of the bits in the block in binary:\n");
    printf("0000  0001  0010  0011\n0100  0101  0110  0111\n1000  1001  1010  1011\n1100  1101  1110  1111\n\n");

    printf("Press enter to continue\n");
    getchar();

    printf(BHGRN"\t4. Find the parity bits. There are log_2(block size) (4 in our case) \"parity groups\" of numbers plus an overall parity bit:\n"reset);
    printf("\t   Let's take a look at the first block. \n");
    printf("The first parity group is: (Note how all indexes have binary 1 in the LSB (0001, 0011, 0101, 0111, 1001, 1011, 1101, 1111) )\n");
    blockDisplayBin(blocks[0], parityGroup1Coloured);
    printf("The job of the parity bit is to make the parity of the group EVEN, so in this case, it's set to %d (IN GREEN)\n\n", getBit(blocks[0].bits, 1));
    printf("The second parity group is: (Note how all the indexes have binary 1 in the second digit (0010, 0011, 0110, 0111, 1010, 1011, 1110, 1111) )\n");
    blockDisplayBin(blocks[0], parityGroup2Coloured);
    printf("The job of the parity bit is to make the parity of the group EVEN, so in this case, it's set to %d (IN GREEN)\n\n", getBit(blocks[0].bits, 2));
    printf("The third parity group is: (Note how all the indexes have binary 1 in the third digit (0100, 0101, 0110, 0111, 1100, 1101, 1110, 1111) )\n");
    blockDisplayBin(blocks[0], parityGroup3Coloured);
    printf("The job of the parity bit is to make the parity of the group EVEN, so in this case, it's set to %d (IN GREEN)\n\n", getBit(blocks[0].bits, 4));
    printf("The fourth parity group: (Note how all the indexes have binary 1 in the fourth digit (1000, 1001, 1010, 1011, 1100, 1101, 1110, 1111) )\n");
    blockDisplayBin(blocks[0], parityGroup4Coloured);
    printf("The job of the parity bit is to make the parity of the group EVEN, so in this case, it's set to %d (IN GREEN)\n\n", getBit(blocks[0].bits, 8));
    printf("The overall parity bit is: ");
    blockDisplayBin(blocks[0], totalParityColoured);
    printf("The job of this parity bit is to make the parity of the ENTIRE block EVEN, so in this case, it's set to %d (IN GREEN)\n\n", getBit(blocks[0].bits, 0));

    printf("Press enter to continue\n");
    getchar();

    printf("Because the bits are arranged in this manner, we can use a single operation to find the parity bits. The XOR (^) function.\n");
    printf("If the indexes of all of the bits that are 1 in a certain block are XORed together, the result will give us the parity bits, in this case: %d%d%d%d\n",
           getBit(blocks[0].bits, 1), getBit(blocks[0].bits, 2), getBit(blocks[0].bits, 4), getBit(blocks[0].bits, 8));
    printf("The \"Total parity\" bit is the XOR of all the bits in the block that are 1.\n\n");

    printf("Press enter to continue\n");
    getchar();


    printf(BHGRN"\t5. Interlacing the blocks\n"reset);
    printf("\t   Now that we have the blocks encoded, we can interlace them to make them resilient to burst errors (many errors in a row.\n");
    printf("\t   Interlacing is the process of ordering all the blocks such that their data is spread across the whole file instead of being bunched up together:\n");
    printf("\t   The bits look like this after interlacing, with each colour representing the nth bit from a different block:\n");

    char* interlaceCols[6] = {REDB, GRNB, YELB, MAGB, CYNB};
    printf("\t\t");
    for (int i=0;i<5;i++) {
        printf(interlaceCols[i%numBlocks]);
        printf("%d", getBit(blocks[i%numBlocks].bits, i%numBlocks));
    }
    printf(reset "..." BLK "_\n" reset);

    printf("The Blocks are now encoded using hamming encoding and ready for transmission / storage\n\n");
    printf("Press enter to continue\n");
    getchar();



    // decode the message
    printf(BHBLU"\n\nThe process of decoding data using Hamming Codes is as follows:\n"reset);
    printf(BHGRN"\t1. De-lacing the blocks. This is simply rearranging the bits such that they are in the same order they were in before interlacing (the reverse of step 5. above)\n"reset);
    printf(BHGRN"\t2. Decoding the blocks. The parity bits are now used to check the data. We can obtain the parity of the whole block again to find out if there is an error.\n"reset);
    printf("\t   If there is only one error, then the total parity will be odd and the parity of the whole block will point to the position of the error.\n");
    printf("\t   If there are two errors, then the total parity will be even and the parity of the whole block will be NON-ZERO, but we will not be able to locate the error\n");
    printf("\t   If there are three errors or more, Hamming encoding fails.\n\n");
    printf("\t   For example: if the 5th bit is flipped...\n");
    blockDisplayBin(blocks[0], parityVsDataColoured);
    printf("\t   flipping bit...\n");
    short data = blocks[0].bits;
    flipBit(&data, 5);
    blocks[0].bits = data;

    blockDisplayBin(blocks[0], parityVsDataColoured);
    printf("\t   After flipping the 5th bit of the first block, the parity of the block is: %d%d%d%d\n",
           getBit(getParity(blocks[0]), 3), getBit(getParity(blocks[0]), 2), getBit(getParity(blocks[0]), 1), getBit(getParity(blocks[0]), 0));

    printf("\t   And the total parity is %d\n", getTotalParity(blocks[0]));
    printf("\t   The parity tells is the exact location of the error because we can see it IS in the FIRST parity group, NOT in the Second parity group, IS in the third parity group, and is NOT in the fourth parity group.\n"
           "The only bit that satisfies all 4 of those criteria is the 5th bit:\n");
    printf("\t   Correcting this error:\n");
    flipBit(&data, 5);
    blocks[0].bits = data;
    blockDisplayBin(blocks[0], parityVsDataColoured);
    printf("\t   If we flip the 5th bit again, the parity of the block is: %d\n",
           getParity(blocks[0]));
    printf("\t   And the total parity is %d\n", getTotalParity(blocks[0]));



    free(blocks);
}

void option2() {
    // Quickly encode and decode text using Hamming Codes. Interlacing blocks is used for resilience against burst errors
    printf(BWHT"\nEncoding and decoding data using Hamming Codes\n"reset);
    // 2.1 Quickly encode text data and save the encoded data
    printf("Would you like to encode a string message(1) or a file(2)?: (1,2)\n");
    int messageOrFile = 0;
    while (!(messageOrFile == 1 || messageOrFile == 2)) {
        fflush(stdin);
        scanf("%d", &messageOrFile);
        (!(messageOrFile >= 1 && messageOrFile <= 2)) ? printf("Not a valid option!\nPlease select an option in the range: (1,2)\n"): 0;
    }

    static byte message[MAX_MESSAGE_SIZE]; // Must be declared static, otherwise theres a segmentation fault
    char originalFileName[100] = "";
    int numBytes = 0;
    if (messageOrFile==1) {
        // getting user input
        printf("Please enter the message you would like to encode: (Max length is %d characters)\n", MAX_MESSAGE_SIZE);
        getchar(); // gets rid of newline character
        gets(message);

        printf("message is %d characters long\n", strlen(message));
        sprintf(originalFileName, "userMessage.txt");
    }
    else {
        printf("Please make sure the file you want to encode is in the same directory as this program\n");
        printf(BWHT"Please enter the name of the file you would like to encode ( with file extension e.g. .txt, .jpg, .mp3, .mp4) Max file size is %d bytes\n" reset, MAX_MESSAGE_SIZE);
        printf("Example files included: arcade.mp3, arrival.jpg, attractor1.jpg, attractor2.png, dunkirk.png, thrust.mp4, hammingCodesTitleAscii.txt\n");
        scanf("%s", originalFileName);
        FILE* fp = fopen(originalFileName, "rb");
        if (fp == NULL) {
            printf("Error opening file! - Does not exist or is not in the same directory as this program.\n");
            return; // back to main()
        }
        int fileSize = findFileSize(fp);
        printf("File size is %d bytes\n", fileSize);
        if (fileSize > MAX_MESSAGE_SIZE) {
            printf("File is too large!\n");
            return; // back to main()
        }
        numBytes = fileSize;
        fread(message, sizeof(byte), numBytes, fp);
        fclose(fp);
    }

    // encoding the message
    int numBlocks = calcNumOfBlocks(message, numBytes);
    printf("Number of 16 bit blocks: %d\n", numBlocks);

    struct smallBlock* blocks;
    int wasOddNumBlocks = numBlocks % 2;
    if (numBlocks%2){
        numBlocks++;
        // malloc can't initialise odd number of blocks?????????? - very weird behaviour
    }
    blocks = (struct smallBlock*) malloc(numBlocks * sizeof(struct smallBlock)); // have to use malloc() because cannot declare an array of structs that is of variable size
    (wasOddNumBlocks) ? numBlocks-- : 0;  // we only set it to even so that malloc would work, resetting it.

    hammingEncodeFast(message, blocks, numBytes); // encode the message


    char hexFilename[100];
    writeBlocksToFileAndInterlace(blocks, numBlocks, hexFilename); // generates the filename, interlaces the blocks and saves to file
    free(blocks); // Important to free memory allocated with malloc()

// 2.2 Introduce a single or burst error to the encoded data to simulate bit flips in an actual transmission / storage system.
    printf(MAG"\nSimulating error \n"reset);
    int maxBitIndex = numBlocks * 8 * sizeof(short) - (numBlocks);
    int bitIndex = -1;
    int length = 0;
    while ((bitIndex < 0) || (bitIndex > maxBitIndex)) {
        printf("Please enter the bit index from which you would like to flip bits (introduce error) (0-%d):\n", maxBitIndex);
        scanf("%d", &bitIndex);
    }
    while ((length <= 0) || (length > (maxBitIndex - bitIndex))) {
        printf("Please enter the length of the error (1 for single error) (0-%d). Note that if the length of the error is more than the number of blocks (%d), the errors will be found but cannot be corrected for:\n",  maxBitIndex-bitIndex, numBlocks);
        scanf("%d", &length);

    }
    introduceError(length, hexFilename, bitIndex); // length, filename, bitIndex
// 2.3 Quickly detect and correct errors in data and decode it
    hammingDecodeFast(hexFilename, originalFileName);
}

int main() {
    byte rerun = 1;

    while (rerun) {
        int option = openingSequence();

        switch (option) {
            case 1:
                option1();
                break;
            case 2:
                option2();
                break;
            default:
                printf("Invalid option\n"); // should never happen
                break;
        }
        printf("\nWould you like to re-run the program? (y/n)\n");
        char answer;
        scanf(" %c", &answer);
        if ((answer != 'y') && (answer != 'Y')) {
            rerun = 0; // exit the loop
        }
    }
    return 0; // exit the program
}