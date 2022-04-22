// Assignment 3 - Hamming codes / error correction codes.
// C23
// TODO: Proper header file

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

/// Colour codes for text.
/*#define blackText "\033[0;30m"
#define redText "\033[0;31m"
#define greenText "\033[0;32m"
#define yellowText "\033[0;33m"
#define blueText "\033[0;34m"
#define magentaText "\033[0;35m"
#define cyanText "\033[0;36m"
#define whiteText "\033[0;37m"
#define resetText "\033[0m"
#define boldText "\033[1m"*/

#define MAX_MESSAGE_SIZE 25*1024*1024 // 25 MiB of memory

typedef char byte; // makes the code easier to read

struct smallBlock {
    short bits;
    int blockNo; // Although in _this_ application block numbers are not *strictly* needed (contiguous storage),
                 //  they are needed for tasks like transmission, where blocks may arrive at different times (e.g. packet switching)
};

/// Prototypes
// TODO: ADD prototypes at end
int findFileSize(FILE* fp);
void readBlocksFromFileAndDelace(struct smallBlock blocks[], int numBlocks, char* filename);
void writeBlocksToFile(struct smallBlock blocks[], int numBlocks, char filename[]);

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
        blockDisplayBin(blocks[k]);
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

void hammingEncodeFast(byte message[], struct smallBlock blocks[], int numBytes) { // input zero if the input is a string and you don't know the number of bytes.
    double numDataBits;
    if (!numBytes) {
        numDataBits = (double)(sizeof(byte) * 8 * strlen(message));
    }
    else {
        numDataBits = (double)(sizeof(byte) * 8 * numBytes);
    }

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

void hammingDecodeFast(char hexFilename[], char originalFilename[]) {
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
    blocks = (struct smallBlock*) malloc(numBlocks * sizeof(struct smallBlock)); /// have to use malloc() because cannot declare an array of structs that is that large
    (wasOddNumBlocks) ? numBlocks-- : 0;  // we only set it to even so that malloc would work.


    // delace the bits from the file and write them to the blocks
    readBlocksFromFileAndDelace(blocks, numBlocks, hexFilename);
    int numErrors = 0;
    // find errors in the blocks and decode them.
    for (int i=0;i<numBlocks;i++) {
        struct smallBlock currentBlock = blocks[ blocks[i].blockNo ];
        int currentBlockParity = getParity(currentBlock);
        if ((currentBlockParity == 0) && (getTotalParity(currentBlock) == 1)) { // if there is an error at position zero (total parity)
            blockDisplayBin(currentBlock);
            printf("1 error found in block %d, in position 0\n", currentBlock.blockNo);
            printf("Correcting error...\n");
            flipBit(&currentBlock.bits, currentBlockParity);
            printf("Error bit corrected! Parity of block is now %d\n", getParity(currentBlock));
            numErrors++;
        }
        else if (currentBlockParity && (getTotalParity(currentBlock) == 0)) { // there is two errors in the block
            // if the parity of the whole block is even, then two bits got flipped in a single block.
                blockDisplayBin(currentBlock);
                // Note, cases with 3 or more errors per block is not covered by hamming codes
                printf("There is two errors in block %d - cannot correct for them both\n", currentBlock.blockNo);
                numErrors += 2;
        }
        else if (currentBlockParity && (getTotalParity(currentBlock) == 1)) { // there is one error in the block
                blockDisplayBin(currentBlock);
                printf("1 error found in block %d, in position %d\n", currentBlock.blockNo, currentBlockParity);
                printf("Correcting error...\n");
                flipBit(&currentBlock.bits, currentBlockParity);
                printf("Error bit corrected! Parity of block is now %d\n", getParity(currentBlock));
                numErrors++;
        }
    }

    if (numErrors == 0) {
        printf("\nThere are no errrors!!\n");
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
    printf("NUMDATABITS %d\n", numDataBits);
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

void readBlocksFromFileAndDelace(struct smallBlock blocks[], int numBlocks, char* hexFilename) {
    // delace the bits from the file and write them to the blocks
    FILE* fp = fopen(hexFilename, "rb");
    if (fp == NULL) {
        printf("Error opening file!\n");
    }
    for (int j=0;j<numBlocks;j++) {
        blocks[j].blockNo = j;
    }
    printf("numbits: %d\n", numBlocks*8* sizeof(short));
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
    printf("\nThis is an program to demonstrate how Hamming Codes can be used to encode data and make it resilient to errors - a self-correcting message\n");
    printf("This is a common problem in data transmission and storage, and similar systems are used today.\n\n");

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

int findFileSize(FILE* fp) {
    fseek(fp, 0L, SEEK_END); // seek to end of file
    int size = ftell(fp); // find counter position
    fseek(fp, 0L, SEEK_SET); // seek back to beginning of file to put pointer back in place
    return size;
}

void option1() {
    // educational walk through of encoding and decoding data using Hamming Codes
    // TODO: get coloured output working for the educational walk through
}

void option2() {
    // Quickly encode and decode text using Hamming Codes. Interlacing blocks is used for resilience against burst errors
    printf("\nENCODING TEXT\n");
// 2.1 Quickly encode text data and save the encoded data
    printf("Would you like to encode a message you input or a file?: (1,2)\n");
    int messageOrFile = 0;
    while (!(messageOrFile == 1 || messageOrFile == 2)) {
        scanf("%d", &messageOrFile);
        (!(messageOrFile >= 1 && messageOrFile <= 2)) ? printf("Not a valid option!\nPlease select an option in the range: (1,2)\n"): 0;
    }

    static byte message[MAX_MESSAGE_SIZE]; // Must be declared static, otherwise theres a segmentation fault
    char originalFileName[100] = "";
    int numBytes = 0;
    if (messageOrFile==1) {
        // getting user input
        printf("Please enter the message you would like to encode: (Max length is %d characters)\n", MAX_MESSAGE_SIZE);
        getchar(); // get rid of newline character
        gets(message);

        printf("message is %d characters long\n", strlen(message));
        sprintf(originalFileName, "userMessage.txt");
    }
    else {
        printf("Please make sure the file you want to encode is in the same directory as this program\n");
        printf("Please enter the name of the file you would like to encode ( with file extension e.g. .txt, jpeg, .mp3) Max file size is %d\n", MAX_MESSAGE_SIZE);
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
    printf("NUMBER OF BLOCKS: %d\n", numBlocks);

    struct smallBlock* blocks;
    int wasOddNumBlocks = numBlocks % 2;
    if (numBlocks%2){
        numBlocks++;
        // malloc can't initialise odd number of blocks?????????? - very weird behaviour
    }
    blocks = (struct smallBlock*) malloc(numBlocks * sizeof(struct smallBlock)); /// have to use malloc() because cannot declare an array of structs that is that large
    (wasOddNumBlocks) ? numBlocks-- : 0;  // we only set it to even so that malloc would work.

    hammingEncodeFast(message, blocks, numBytes); // encode the message

    printf("ENCODED MESSAGE:\n");
    blockDisplayMultipleBin(blocks, (numBlocks>=10) ? 10 : numBlocks ); // only print 10 blocks max

    char hexFilename[100];
    writeBlocksToFileAndInterlace(blocks, numBlocks, hexFilename); // generates the filename, interlaces the blocks and saves to file
    free(blocks); /// Important to free memory allocated with malloc()

// 2.2 Introduce a single or burst error to the encoded data to simulate bit flips in an actual transmission / storage system.
    printf("\nSimulating error \n");
    int maxBitIndex = numBlocks * 8 * sizeof(short );
    int bitIndex = -1;
    int length = 0;
    while ((bitIndex < 0) || (bitIndex > maxBitIndex)) {
        printf("Please enter the bit index you would like to flip (0-%d):\n", maxBitIndex);
        scanf("%d", &bitIndex);
    }
    while ((length <= 0) || (length > numBlocks)) {
        printf("Please enter the length of the error (1 for single error)  (0-%d):\n", numBlocks);
        scanf("%d", &length);
        if ((length <= 0) || (length > numBlocks)) {
            printf("The length must be more than 0 and less than the number of blocks\n");
            printf("If the length of the burst error is more than the number of blocks (with interlacing), there will be more than two errors per block, which hamming codes can detect, but not correct for.\n");

        }

    }

    introduceError(length, hexFilename, bitIndex); // length, filename, bitIndex
// 2.3 Quickly detect and correct errors in data and decode it
    hammingDecodeFast(hexFilename, originalFileName);
}

int main() {

    int option = openingSequence();
    switch (option) {
        case 1:
            option1();
            break;
        case 2:
            option2();
            break;
    }
    while (1); // TODO implement a repeat option and exit option.
}

