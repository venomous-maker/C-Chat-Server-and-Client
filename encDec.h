// encDec.h
#ifndef ENCDEC_H
#define ENCDEC_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <errno.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>
// Function prototypes for


#ifndef PHYSICAL_H
#define PHYSICAL_H

void charToBinary(char input, uint8_t *output);
void binaryToChar(const uint8_t *input, char *output, int size);
int addParityBit(uint8_t *data, int size);
int removeParityBit(uint8_t *data, int size);

#endif

// Function prototypes for the data link layer
#ifndef DATALINK_H
#define DATALINK_H
#define SYN 22
int frameData(const char *input, int inputSize, uint8_t *output);
int deframeData(const uint8_t *input, int inputSize, char *output);
void printBinary(const uint8_t *data, int size);
char* stringToBinary(char*);
#endif

#ifndef APPLICATION_H
#define APPLICATION_H

int readFile(const char *filename, char *buffer, int bufferSize,  char*);
int writeFile(const char *filename, const char *data, int dataSize, char*);
void convertToLowerToUpper(char *data, int dataSize);
int ReadBytes(char* inputString);
char * slicerfunc(const char* string, long int start, long int stop);
char* concatenate_string(char* s, char* s1);
void separateNameAndMessage(const char *input, char *name, char *message, size_t maxSize);
#define MAX_BUFFER 1024
// Define a structure to store tag-value pairs
struct TagValuePair {
    char tag[MAX_BUFFER];
    char value[MAX_BUFFER];
};
// Function to extract tags and values from a message
int extractTagsAndValues(const char *, struct TagValuePair *, int);

#endif

#endif
