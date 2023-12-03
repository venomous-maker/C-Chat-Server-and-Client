
// data_link.c
#include "encDec.h"

int frameData(const char *input, int inputSize, uint8_t *output) {
    if (inputSize <= 64) {
        // Create a frame with two SYN characters, LENGTH, and data
        output[0] = SYN;
        output[1] = SYN;
        output[2] = inputSize; // LENGTH
        for (int i = 0; i < inputSize; i++) {
            output[i + 3] = input[i];
        }
        return inputSize + 3; // Return the size of the frame
    } else {
        // Input data too large for a single frame
        return -1;
    }
}



int deframeData(const uint8_t *input, int inputSize, char *output) {
    if (inputSize >= 3) {
        // Check for SYN characters at the beginning
        if (input[0] == SYN && input[1] == SYN) {
            int dataLength = input[2]; // LENGTH
            if (inputSize == dataLength + 3) {
                // Extract the data from the frame
                for (int i = 0; i < dataLength; i++) {
                    output[i] = input[i + 3];
                }
                return dataLength; // Return the size of the extracted data
            }
        }
    }
    // Frame is invalid or incomplete
    return -1;
}

void printBinary(const uint8_t *data, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 7; j >= 0; j--) {
            printf("%d", (data[i] >> j) & 1);
        }
        printf(" ");
    }
    printf("\n");
}

char* stringToBinary(char* sTring) {
    if(sTring == NULL) return 0; /* no input string */
    size_t len = strlen(sTring);
    char *binary = malloc(len*8 + 1); // each char is one byte (8 bits) and + 1 at the end for null terminator
    binary[0] = '\0';
    for(size_t i = 0; i < len; ++i) {
        char ch = sTring[i];
        for(int j = 7; j >= 0; --j){
            if(ch & (1 << j)) {
                strcat(binary,"1");
            } else {
                strcat(binary,"0");
            }
        }
    }
    return binary;
}

