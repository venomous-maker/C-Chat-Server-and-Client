// application.c
#include "encDec.h"

// Read data from a file into a buffer
int readFile(const char *filename, char *buffer, int bufferSize,char* mode) {
    FILE *file = fopen(filename, mode);
    if (file == NULL) {
        return -1;  // Handle the error, file not found or cannot be opened
    }

    // Read the data from the file into the buffer
    int bytesRead = fread(buffer, 1, bufferSize, file);

    fclose(file);
    return bytesRead;  // Return the number of bytes read
}

// Write data to a file
int writeFile(const char *filename, const char *data, int dataSize, char * mode) {
    FILE *file = fopen(filename, mode);
    if (file == NULL) {
        printf("Error\n");
        return -1;  // Handle the error, cannot create or open the file
    }

    // Write the data to the file
    int bytesWritten = fwrite(data, 1, dataSize, file);

    fclose(file);
    return bytesWritten;  // Return the number of bytes written
}

// Convert a string from lowercase to uppercase
void convertToLowerToUpper(char *data, int dataSize) {
    for (int i = 0; i < dataSize; i++) {
        if (islower(data[i])) {
            data[i] = toupper(data[i]);
        }
    }
}

int ReadBytes(char* inputString) {
    int bytesRead = 0;
    char* ptr = (char*)calloc(strlen(inputString),  sizeof(char));
    strcpy(ptr,inputString);
    // Iterate through the string and count the bytes read
    for (; *ptr != '\0'; ptr++) {
        bytesRead++;
    }

    return bytesRead;
}
char * slicerfunc(const char* string, long int start, long int stop){
    if(string == NULL){
        return NULL;
    }
    
    if (stop - start <= 0 || start > stop || start < 0 || stop < 0 || start > strlen(string) || stop > strlen(string)){
        return NULL;
    }
    
    char* value = (char*) calloc(stop-start, sizeof(char));
    int i = 0;
    while(start <= stop){
        value[i] = string[start];
        start++;
        i++;
    }
    
    return value;
}

char* concatenate_string(char* s, char* s1)
{
    int i;
 
    int j = strlen(s);
 
    for (i = 0; s1[i] != '\0'; i++) {
        s[i + j] = s1[i];
    }
 
    s[i + j] = '\0';
 
    return s;
}


void separateNameAndMessage(const char *input, char *name, char *message, size_t maxSize) {
    // Find the position of the first colon in the input string
    const char *colonPos = strchr(input, ':');

    if (colonPos != NULL) {
        // Calculate the length of the name part
        size_t nameLength = colonPos - input;

        // Copy the name part into the 'name' buffer
        strncpy(name, input, nameLength);
        name[nameLength] = '\0';
        //printf("%s\n", name);
        // Copy the message part into the 'message' buffer
        strncpy(message, colonPos + 1, maxSize);
        message[maxSize - 1] = '\0'; // Ensure the message is null-terminated

        // Trim leading and trailing spaces from the message
        size_t i = 0, j = 0;
        for ( ; isspace(message[i]); ++i);
        if (i > 0) {
            for ( ; message[i] != '\0'; ++i, ++j) {
                message[j] = message[i];
            }
            message[j] = '\0';
        }
        for (i = j - 1; i > 0 && isspace(message[i]); --i);
        message[i + 1] = '\0';
    } else {
        // If no colon is found, treat the entire input as the name and the message as an empty string
        strncpy(name, input, maxSize);
        name[maxSize - 1] = '\0';
        message[0] = '\0'; // Empty message
    }
}

/*
// Function to extract tags and values from a message
int extractTagsAndValues(const char *message, struct TagValuePair *result, int maxPairs) {
    const char *start = message;
    int count = 0;

    while (count < maxPairs) {
        // Find the opening and closing tags
        const char *openTag = strchr(start, '<');
        const char *closeTag = strchr(start, '>');

        if (openTag == NULL || closeTag == NULL) {
            break; // No more tags found
        }

        // Copy the tag into the result structure
        int tagLength = closeTag - openTag - 1;
        if (tagLength < 0 || tagLength >= MAX_BUFFER) {
            return -1; // Tag length is too long
        }
        strncpy(result[count].tag, openTag + 1, tagLength);
        result[count].tag[tagLength] = '\0';

        // Find the closing tag of the value
        const char *openEndTag = strchr(closeTag, '<');
        int valueLength;
        if (openEndTag != NULL) {
            // If there is an opening tag after the closing tag, calculate value length accordingly
            valueLength = openEndTag - closeTag - 1;
        } else {
            // If there is no opening tag, consider the rest of the string as the value
            valueLength = strlen(closeTag + 1);
        }

        if (valueLength < 0 || valueLength >= MAX_BUFFER) {
            return -1; // Value length is too long
        }
        strncpy(result[count].value, closeTag + 1, valueLength);
        result[count].value[valueLength] = '\0';

        // Move the start pointer to the character after the closing tag
        if (openEndTag != NULL) {
            start = openEndTag;
        } else {
            break; // No more values to extract
        }

        count++;
    }

    return count; // Return the number of tag-value pairs found
}*/

int extractTagsAndValues(const char *message, struct TagValuePair *result, int maxPairs) {
    const char *start = message;
    int count = 0;

    while (count < maxPairs) {
        // Find the opening and closing tags
        const char *openTag = strchr(start, '<');
        const char *closeTag = strchr(start, '>');

        if (openTag == NULL || closeTag == NULL) {
            break; // No more tags found
        }

        // Copy the tag and value into the result structure
        int tagLength = closeTag - openTag - 1;
        if (tagLength < 0 || tagLength >= MAX_BUFFER) {
            return -1; // Tag length is too long
        }
        //strncpy(result[count].tag, openTag + 1, tagLength);
        snprintf(result[count].tag, sizeof(result[count].tag), "%.*s", tagLength, openTag + 1);
        result[count].tag[tagLength] = '\0';

        const char *openEndTag = strchr(closeTag, '<');
        int valueLength = openEndTag - closeTag - 1;
        if (valueLength < 0 || valueLength >= MAX_BUFFER) {
            return -1; // Value length is too long
        }
        //strncpy(result[count].value, closeTag + 1, valueLength);
        snprintf(result[count].value, sizeof(result[count].value), "%.*s", valueLength, closeTag + 1);
        result[count].value[valueLength] = '\0';

        // Move the start pointer to the character after the closing tag
        start = openEndTag;

        count++;
    }

    return count; // Return the number of tag-value pairs found
}
