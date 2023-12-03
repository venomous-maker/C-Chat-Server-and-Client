//Eugene Li - Multithreaded chat client
#include "encDec.h"

void chatloop(char *name, int socketFd);
void chatLoop(char*,int);
void buildMessage(char* result, char* name, char* msg, char* To);
void setupAndConnect(struct sockaddr_in *serverAddr, struct hostent *host, int socketFd, long port);
//int extractTagsAndValues(const char *message, struct TagValuePair *result, int maxPairs);
void setNonBlock(int fd);
void interruptHandler(int sig);

static int socketFd;
char *To = "";

int main(int argc, char *argv[])
{
    char *name;
    struct sockaddr_in serverAddr;
    struct hostent *host;
    long port;
    if(argc > 4)
    {
        To = argv[4];
    }
    else if(argc != 4)
    {
        fprintf(stderr, "./client [username] [host] [port] [recipent(optional)]\n");
        exit(1);
    }
    name = argv[1];
    if((host = gethostbyname(argv[2])) == NULL)
    {
        fprintf(stderr, "Couldn't get host name\n");
        exit(1);
    }
    port = strtol(argv[3], NULL, 0);
    if((socketFd = socket(AF_INET, SOCK_STREAM, 0))== -1)
    {
        fprintf(stderr, "Couldn't create socket\n");
        exit(1);
    }

    setupAndConnect(&serverAddr, host, socketFd, port);
    setNonBlock(socketFd);
    setNonBlock(0);

    //Set a handler for the interrupt signal
    signal(SIGINT, interruptHandler);

    chatLoop(name, socketFd);
}
bool Start = false;
//Main loop to take in chat input and display output
void chatloop(char *name, int socketFd)
{
    fd_set clientFds;
    char chatMsg[MAX_BUFFER];
    char chatBuffer[MAX_BUFFER], msgBuffer[MAX_BUFFER];
    
    while(1)
    {
        //Reset the fd set each time since select() modifies it
        FD_ZERO(&clientFds);
        FD_SET(socketFd, &clientFds);
        FD_SET(0, &clientFds);
        struct TagValuePair tagValuePairs[6];
        if(select(FD_SETSIZE, &clientFds, NULL, NULL, NULL) != -1) //wait for an available fd
        {
            for(int fd = 0; fd < FD_SETSIZE; fd++)
            {
                if(FD_ISSET(fd, &clientFds))
                {
                    if(fd == socketFd) //receive data from server
                    {
                        int numBytesRead = read(socketFd, msgBuffer, MAX_BUFFER - 1);
                        msgBuffer[numBytesRead] = '\0';
                        
                        int count = extractTagsAndValues(msgBuffer, tagValuePairs, 9);
                        //printf("%s: %d\n", msgBuffer, count);
                        for(int i = 0; i < count; i++){
                            if(strcmp(tagValuePairs[i].tag, "FROM") == 0 || strcmp(tagValuePairs[i].tag, "BODY") == 0 || strcmp(tagValuePairs[i].tag, "CLIENT_LIST") == 0 
                                || strcmp(tagValuePairs[i].tag, "INFO") == 0 || strcmp(tagValuePairs[i].tag, "TO") == 0){
                                printf("%s: %s...", tagValuePairs[i].tag, tagValuePairs[i].value);
                                char b[MAX_BUFFER*2+4];
                                memset(b, 0, MAX_BUFFER);
                                sprintf(b, "%s: %s...", tagValuePairs[i].tag, tagValuePairs[i].value);
                                int bytesWritten;
                                if((bytesWritten = writeFile("receive.txt", b, strlen(b), "a+"))< 0) {
                                    fprintf(stderr, "Error writing to file\n");
                                } else {
                                    printf("Successfully wrote %d bytes to file\n", bytesWritten);
                                };
                            }
                            if(strcmp(tagValuePairs[i].value, "Login Successful!") == 0 ){
                                Start = true;
                            }
                            //printf("%s: %s\n", tagValuePairs[i].tag, tagValuePairs[i].value);
                            
                        }
                        
                        memset(&msgBuffer, 0, sizeof(msgBuffer));
                    }
                    else if(fd == 0) //read from keyboard (stdin) and send to server
                    {
                        {
                            chatBuffer[0]=' ';
                            chatBuffer[1]='\0';
                            // Use sprintf to construct your message
                            if(!Start){
                                To = "System";
                            }
                            if(Start){
                                fgets(chatBuffer, MAX_BUFFER - 1, stdin);
                            }
                            if(strcmp(chatBuffer, "/exit\n") == 0)
                                interruptHandler(-1); //Reuse the interruptHandler function to disconnect the client
                            else
                            {
                                buildMessage(chatMsg, name, chatBuffer, To);
                                if(write(socketFd, chatMsg, MAX_BUFFER - 1) == -1)
                                    perror("write failed: ");
                                //printf("%s", chatMsg);
                                memset(&chatBuffer, 0, sizeof(chatBuffer));
                            }
                        }
                    }
                }
            }
        }
    }
}
void readFromServer(int socketFd) {
    struct TagValuePair tagValuePairs[6];
    char msgBuffer[MAX_BUFFER];
    
    int numBytesRead = read(socketFd, msgBuffer, MAX_BUFFER - 1);
    msgBuffer[numBytesRead] = '\0';

    int count = extractTagsAndValues(msgBuffer, tagValuePairs, 9);
    
    for (int i = 0; i < count; i++) {
        if (strcmp(tagValuePairs[i].tag, "FROM") == 0 || strcmp(tagValuePairs[i].tag, "BODY") == 0 || 
            strcmp(tagValuePairs[i].tag, "CLIENT_LIST") == 0 || strcmp(tagValuePairs[i].tag, "INFO") == 0 || 
            strcmp(tagValuePairs[i].tag, "TO") == 0) {
            printf("%s: %s\n", tagValuePairs[i].tag, tagValuePairs[i].value);
            char b[MAX_BUFFER*2+4];
            memset(b, 0, MAX_BUFFER);
            sprintf(b, "%s: %s\n", tagValuePairs[i].tag, tagValuePairs[i].value);
            int bytesWritten;
            if((bytesWritten = writeFile("receive.txt", b, strlen(b), "a+"))< 0) {
                fprintf(stderr, "Error writing to file\n");
            } else {
                //printf("Successfully wrote %d bytes to file\n", bytesWritten);
            }
        }
        
        if (strcmp(tagValuePairs[i].value, "Login Successful!") == 0 ) {
            Start = true;
        }
    }
    memset(&msgBuffer, 0, sizeof(msgBuffer));
}

void writeToServer(int socketFd, char* name, char* To) {
    char chatMsg[MAX_BUFFER];
    char chatBuffer[MAX_BUFFER];

    chatBuffer[0]=' ';
    chatBuffer[1]='\0';

    // Use sprintf to construct your message
    /*if (!Start) {
        To = "System";
    }*/
    
    if (1) {
        fgets(chatBuffer, MAX_BUFFER - 1, stdin);
    }

    if (strcmp(chatBuffer, "/exit\n") == 0) {
        interruptHandler(-1); // Reuse the interruptHandler function to disconnect the client
    } else {
        buildMessage(chatMsg, name, chatBuffer, To);
        if (write(socketFd, chatMsg, MAX_BUFFER - 1) == -1)
            perror("write failed: ");
        memset(&chatBuffer, 0, sizeof(chatBuffer));
    }
}

void chatLoop(char *name, int socketFd) {
    fd_set clientFds;
    while (1) {
        FD_ZERO(&clientFds);
        FD_SET(socketFd, &clientFds);
        FD_SET(0, &clientFds);

        if (select(FD_SETSIZE, &clientFds, NULL, NULL, NULL) != -1) {
            for (int fd = 0; fd < FD_SETSIZE; fd++) {
                if (FD_ISSET(fd, &clientFds)) {
                    if (fd == socketFd) {
                        readFromServer(socketFd);
                    } else if (fd == 0) {
                        writeToServer(socketFd, name, To);
                    }
                }
            }
        }
    }
}

//Concatenates the name with the message and puts it into result
void buildMessage(char* result, char* name, char* msg, char* To)
{
    memset(result, 0, MAX_BUFFER);
    // Use sprintf to construct your message
    if(!Start){
        msg = "";
        sprintf(result, "<MSG><REQUEST>LOGIN</REQUEST><FROM>%s</FROM><TO>%s</TO><BODY>%s</BODY></MSG>", name, To, msg);
    }else{
        char * input = msg;
        separateNameAndMessage(input, To, msg, MAX_BUFFER);
        char* buff = (char*)calloc(MAX_BUFFER, sizeof(char));
        //attempt to read input as file
        if(readFile(msg, buff, MAX_BUFFER, "r") != -1){
            //strcpy(msg, buff);
            sprintf(result, "<MSG><FROM>%s</FROM><TO>%s</TO><BODY>%s</BODY></MSG>", name, To, buff);
        }
        else
            sprintf(result, "<MSG><FROM>%s</FROM><TO>%s</TO><BODY>%s</BODY></MSG>", name, To, msg);
    }
}

//Sets up the socket and connects
void setupAndConnect(struct sockaddr_in *serverAddr, struct hostent *host, int socketFd, long port)
{
    memset(serverAddr, 0, sizeof(serverAddr));
    serverAddr->sin_family = AF_INET;
    serverAddr->sin_addr = *((struct in_addr *)host->h_addr_list[0]);
    serverAddr->sin_port = htons(port);
    if(connect(socketFd, (struct sockaddr *) serverAddr, sizeof(struct sockaddr)) < 0)
    {
        perror("Couldn't connect to server");
        exit(1);
    }
}

//Sets the fd to nonblocking
void setNonBlock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    if(flags < 0)
        perror("fcntl failed");

    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}

//Notify the server when the client exits by sending "/exit"
void interruptHandler(int sig_unused)
{
    if(write(socketFd, "/exit\n", MAX_BUFFER - 1) == -1)
        perror("write failed: ");

    close(socketFd);
    exit(1);
}

/*// Function to extract tags and values from a message
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
        if (tagLength < 0 || tagLength >= 1000) {
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

        if (valueLength < 0 || valueLength >= 1000) {
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
