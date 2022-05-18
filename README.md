# Hamming codes

This is a program to demonstrate the capabilities of [Hamming codes](https://en.wikipedia.org/wiki/Hamming_code "Wikipedia: Hamming Code") - An early form of error correction code developed by Richard W. Hamming.

The program encodes data to be stored or transmitted in order to make it resilient to errors, (which may arise due to noisy transmission lines, EMI, scratches on a disk etc).

It achieves this by splitting the data into blocks, and then adding several parity bits in just the right place in the data, which enables it to pinpoint the exact location of single bit errors in the block.

<img src = https://github.com/robosam2003/Hamming_codes/blob/master/resources/smallBlockColoured.jpg>



The program also interlaces the bits of the blocks to make the data resilient to burst errors (common for scratches on disks etc.).
The interlaced blocks are saved to an intermediate file with a unique filename.
In the case where the length of the burst error is shorter than the size of the block of data (16 bits in our case), the program is able to correct *every single error* (below), sometimes up to hundreds of thousands of bits.

Otherwise, it will detect two bit errors, but it does not have enough data to correct them. 

<img src = https://github.com/robosam2003/Hamming_codes/blob/master/resources/errorCorrectionSuccess.jpg>
If all errors 

For a more detailed walkthrough of how Hamming codes work, run the educational walkthrough (option 1) or check out the videos by 3Blue1Brown:
- [How to send a self-correcting message (Hamming codes)](https://www.youtube.com/watch?v=X8jsijhllIA&t=681s)
- [Hamming codes part 2, the elegance of it all](https://www.youtube.com/watch?v=b3NxrZOu_CE&t=362s)

<img src = https://github.com/robosam2003/Hamming_codes/blob/master/resources/titleDisplay.jpg>
