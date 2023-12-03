
#include "encDec.h"
// Constants for pipe file descriptors
#define PIPE_PRODUCER_TO_CONSUMER 0
#define PIPE_CONSUMER_TO_PRODUCER 1

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_filename>\n", argv[0]);
        return 1;
    }

    // Define file names
    char *inputFileName = argv[1];
    char *binFileName = "filename.binf";
    char *outFileName = "filename.outf";
    char *chckFileName = "filename.chck";
    char *doneFileName = "filename.done";

    // Create pipes
    int pipes[2][2];
    if (pipe(pipes[0]) == -1 || pipe(pipes[1]) == -1) {
        perror("Pipe creation failed");
        return 1;
    }

    // Fork for producer and consumer
    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        return 1;
    }

    if (pid == 0) {
        // Child process (producer)

        // Close unused pipe ends
        close(pipes[PIPE_PRODUCER_TO_CONSUMER][1]);
        close(pipes[PIPE_CONSUMER_TO_PRODUCER][0]);

        // Read and encode data from the input file
        FILE *inputFile = fopen(inputFileName, "r");
        FILE *binFile = fopen(binFileName, "w");
        FILE *doneFile = fopen(doneFileName, "w");
        if (!inputFile || !binFile) {
            perror("File open failed");
            return 1;
        }
        
        // Read data, encode, and write to the pipe
        char inputBuffer[65];
        uint8_t encodedBuffer[80];
        int bytesRead;
        while ((bytesRead = fread(inputBuffer, 1, 64, inputFile)) > 0) {
            inputBuffer[bytesRead] = '\0';

            // Encode, add parity, and frame the data
            int encodedSize = frameData(inputBuffer, bytesRead, encodedBuffer);
            
            printf("Input File: %s\n\n\n", inputBuffer);
            printBinary(encodedBuffer, encodedSize);
            fwrite(stringToBinary(inputBuffer), 1, strlen(stringToBinary(inputBuffer)), binFile);
            
            // Write the encoded data to the pipe
            if (write(pipes[PIPE_CONSUMER_TO_PRODUCER][1], encodedBuffer, encodedSize) == -1) {
                perror("Write to pipe failed");
                return 1;
            }
            
            // Read processed data from the pipe
            int bytesRead = read(pipes[PIPE_PRODUCER_TO_CONSUMER][0], encodedBuffer, sizeof(encodedBuffer));
            if (bytesRead == -1) {
                perror("Read from pipe failed");
                return 1;
            }

            // Decode, deframe, remove parity, and write to bin file
            char outputBuffer[65];
            int decodedSize = deframeData(encodedBuffer, bytesRead, outputBuffer);
            
            fwrite(outputBuffer, 1, decodedSize, doneFile);
        }

        // Close file descriptors and files
        fclose(inputFile);
        fclose(binFile);
        close(pipes[PIPE_CONSUMER_TO_PRODUCER][1]);
        close(pipes[PIPE_PRODUCER_TO_CONSUMER][0]);

        // Exit the producer process
        exit(0);
    } else {
        // Parent process (consumer)

        // Close unused pipe ends
        close(pipes[PIPE_CONSUMER_TO_PRODUCER][1]);
        close(pipes[PIPE_PRODUCER_TO_CONSUMER][0]);

        // Receive encoded data from the producer through one pipe
        uint8_t receivedBuffer[80];
        int bytesRead = read(pipes[PIPE_CONSUMER_TO_PRODUCER][0], receivedBuffer, sizeof(receivedBuffer));
        if (bytesRead == -1) {
            perror("Read from pipe failed");
            return 1;
        }

        // Decode, process (convert lowercase to uppercase), and encode the data
        char processedBuffer[65];
        int decodedSize = deframeData(receivedBuffer, bytesRead, processedBuffer);
        convertToLowerToUpper(processedBuffer, decodedSize);
        printf("Output File: %s\n\n\n", processedBuffer);
        writeFile(outFileName, processedBuffer, decodedSize, "w");
        printBinary(receivedBuffer, decodedSize);
        int encodedSize = frameData(processedBuffer, decodedSize, receivedBuffer);

        // Send the encoded data back to the producer through the second pipe
        if (write(pipes[PIPE_PRODUCER_TO_CONSUMER][1], receivedBuffer, encodedSize) == -1) {
            perror("Write to pipe failed");
            return 1;
        }

        // Store the processed data in chck file
        FILE *chckFile = fopen(chckFileName, "w");
        if (!chckFile) {
            perror("File open failed");
            return 1;
        }
        //fwrite(processedBuffer, 1, decodedSize, chckFile);
        fwrite(stringToBinary(processedBuffer), 1, strlen(stringToBinary(processedBuffer)), chckFile);
        fclose(chckFile);

        // Wait for the producer to finish and collect its exit status
        int status;
        wait(&status);

        // Exit the consumer process
        exit(0);
    }

    return 0;
}


