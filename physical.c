#include "encDec.h"
#include <stdint.h>

void charToBinary(char input, uint8_t *output) {
    // Convert the character 'input' to a binary bit pattern
    for (int i = 0; i < 8; i++) {
        output[i] = (input >> (7 - i)) & 1;
    }
}

void binaryToChar(const uint8_t *input, char *output, int size) {
    // Convert a binary pattern 'input' of 'size' bits to a character
    char result = 0;
    for (int i = 0; i < size; i++) {
        result = (result << 1) | input[i];
    }
    *output = result;
}

int addParityBit(uint8_t *data, int size) {
    // Calculate and add a parity bit (odd parity) to the 'data' array
    int parity = 0;
    for (int i = 0; i < size; i++) {
        parity ^= data[i];
    }
    data[size] = parity; // Add the parity bit
    return size + 1; // Return the new size with the parity bit
}

int removeParityBit(uint8_t *data, int size) {
    // Check and remove the parity bit (odd parity) from the 'data' array
    int parity = 0;
    for (int i = 0; i < size - 1; i++) {
        parity ^= data[i];
    }
    if (parity == data[size - 1]) {
        // Parity check passed, remove the parity bit
        return size - 1;
    } else {
        // Parity check failed, return an error code
        return -1;
    }
}

