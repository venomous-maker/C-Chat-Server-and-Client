#include "encDec.h"
//#define MAX_BUFFER 1024
#define MAX_USERS 10
#define MAX_CHARS 50
/*
Queue implementation using a char array.
Contains a mutex for functions to lock on before modifying the array,
and condition variables for when it's not empty or full.
*/
typedef struct {
    char *buffer[MAX_BUFFER];
    int head, tail;
    int full, empty;
    pthread_mutex_t *mutex;
    pthread_cond_t *notFull, *notEmpty;
} queue;

typedef struct{
    char *name;
    char *password;
    bool logged_in;
}user;
/*
Struct containing important data for the server to work.
Namely the list of client sockets, that list's mutex,
the server's socket for new connections, and the message queue
*/
typedef struct {
    fd_set serverReadFds;
    int socketFd;
    int clientSockets[MAX_BUFFER];
    int numClients;
    pthread_mutex_t *clientListMutex;
    queue *queue;
    user Users[MAX_USERS];
} chatDataVars;


/*
Simple struct to hold the chatDataVars and the new client's socket fd.
Used only in the client handler thread.
*/
typedef struct {
    chatDataVars *data;
    int clientSocketFd;
} clientHandlerVars;

// Define a structure to store tag-value pairs
/*struct TagValuePair {
    char tag[50];
    char value[MAX_BUFFER];
};*/

void startChat(int socketFd);
void buildMessage(char *result, char *name, char *msg);
void bindSocket(struct sockaddr_in *serverAddr, int socketFd, long port);
void removeClient(chatDataVars *data, int clientSocketFd);

void *newClientHandler(void *data);
void *clientHandler(void *chv);
void *messageHandler(void *data);

void queueDestroy(queue *q);
queue* queueInit(void);
void queuePush(queue *q, char* msg);
char* queuePop(queue *q);
// Helper functions
void* charAThread(void* data);
void* charEThread(void* data);
void* charIThread(void* data);
void* charOThread(void* data);
void* charUThread(void* data);
void* writerThread(void* data);

// Function to extract tags and values from a message
//int extractTagsAndValues(const char *, struct TagValuePair *, int);
void readUsers(char*, user*, int *);
user *Users;
int legal_users = 0;
void readUsers(char* __db_filename, user __users[], int *__count) {
    FILE *file = fopen(__db_filename, "r");
    if (file == NULL) {
        perror("Error opening db file");
        exit(1);
    }

    /*while (fscanf(file, "%s %s", __users[*__count].name, __users[*__count].password) == 2) {
        (*__count)++;
        if (*__count >= MAX_USERS) {
            fprintf(stderr,"Max users limit reached\n");
            break;
        }
    }*/
// Read and parse user and password information
    int numUsers = 0;
    char line[MAX_CHARS + MAX_CHARS + 2]; // Maximum line length
    while (fgets(line, sizeof(line), file) != NULL && numUsers < MAX_USERS) {
        char username[MAX_CHARS];
        char password[MAX_CHARS];
        
        line[strcspn(line, "\n")] = '\0';

        // Parse the line into username and password
        if (sscanf(line, "%s %s", username,password) == 2) {
            // Copy the username and password into the structure
            __users[numUsers].name = (char*)calloc(MAX_CHARS, sizeof(char));
            __users[numUsers].password = (char*)calloc(MAX_CHARS, sizeof(char));
            strcpy(__users[numUsers].name, username);
            strcpy(__users[numUsers].password, password);
            numUsers++;
            (*__count)++;
        } else {
            fprintf(stderr,"Invalid format on line: %s", line);
        }
    }
    fclose(file);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in serverAddr;
    long port = 9999;
    int socketFd;
    Users = (user*)calloc(MAX_USERS, sizeof(user));
    char *__db_filename = "encrypted.db";
    readUsers(__db_filename, Users, &legal_users);
    // Printing the user-password pairs for verification
    fprintf(stderr,"Read users from the file: \n");
    for (int i = 0; i < legal_users; i++) {
        printf("Username: %s, Password: %s\n", Users[i].name, Users[i].password);
    }
    if(argc == 2) port = strtol(argv[1], NULL, 0);

    if((socketFd = socket(AF_INET, SOCK_STREAM, 0))== -1)
    {
        perror("Socket creation failed");
        exit(1);
    }

    bindSocket(&serverAddr, socketFd, port);
    if(listen(socketFd, 1) == -1)
    {
        perror("listen failed: ");
        exit(1);
    }

    startChat(socketFd);
    
    close(socketFd);
}

//Spawns the new client handler thread and message consumer thread
void startChat(int socketFd)
{
    chatDataVars data;
    data.numClients = 0;
    data.socketFd = socketFd;
    data.queue = queueInit();
    data.clientListMutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    // Initialize values
    for (int i = 0; i < MAX_BUFFER; i++) {
        data.clientSockets[i] = 0;
    }
    for (int i = 0; i < MAX_USERS; i++) {
        data.Users[i].name = (char*)calloc(MAX_CHARS, sizeof(char));
        data.Users[i].password = (char*)calloc(MAX_CHARS, sizeof(char));
        data.Users[i].logged_in = false;
    }
    FD_ZERO(&data.serverReadFds);
    //data.serverReadFds;
    pthread_mutex_init(data.clientListMutex, NULL);

    //Start thread to handle new client connections
    pthread_t connectionThread;
    if((pthread_create(&connectionThread, NULL, (void *)&newClientHandler, (void *)&data)) == 0)
    {
        fprintf(stderr, "Connection handler started\n");
    }

    FD_ZERO(&(data.serverReadFds));
    FD_SET(socketFd, &(data.serverReadFds));
    
    // Initialize the thread data for helper threads
    
    // Spawn threads for each helper function
    pthread_t charAThreadId, charEThreadId, charIThreadId, charOThreadId, charUThreadId, writerThreadId;

    if(pthread_create(&charAThreadId, NULL, (void *)&charAThread, (void*)&data) == 0){
        fprintf(stderr, "CharA thread started\n");
    };
    if(pthread_create(&charEThreadId, NULL, (void *)&charEThread, (void*)&data) == 0){
        fprintf(stderr, "CharE thread started\n");
    };
    if(pthread_create(&charIThreadId, NULL, (void *)&charIThread, (void*)&data)==0){
        fprintf(stderr, "CharI thread started\n");
    };
    if(pthread_create(&charOThreadId, NULL, (void *)&charOThread, (void*)&data) == 0){
        fprintf(stderr, "CharO thread started\n");
    };
    if(pthread_create(&charUThreadId, NULL, (void *)&charUThread, (void*)&data) == 0){
        fprintf(stderr, "CharU thread started\n");
    };
    if(pthread_create(&writerThreadId, NULL, (void *)&writerThread, (void*)&data) == 0){
        fprintf(stderr, "Writer thread started\n");
    };
    //Start thread to handle messages received
    pthread_t messagesThread;
    if((pthread_create(&messagesThread, NULL, (void *)&messageHandler, (void *)&data)) == 0)
    {
        fprintf(stderr, "Message handler started\n");
    }

    pthread_join(connectionThread, NULL);
    // Join the helper threads
    pthread_join(charAThreadId, NULL);
    pthread_join(charEThreadId, NULL);
    pthread_join(charIThreadId, NULL);
    pthread_join(charOThreadId, NULL);
    pthread_join(charUThreadId, NULL);
    pthread_join(writerThreadId, NULL);
    
    pthread_join(messagesThread, NULL);

    queueDestroy(data.queue);
    pthread_mutex_destroy(data.clientListMutex);
    free(data.clientListMutex);
}

//Initializes queue
queue* queueInit(void)
{
    queue *q = (queue *)malloc(sizeof(queue));
    if(q == NULL)
    {
        perror("Couldn't allocate anymore memory!");
        exit(EXIT_FAILURE);
    }

    q->empty = 1;
    q->full = q->head = q->tail = 0;
    q->mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    if(q->mutex == NULL)
    {
        perror("Couldn't allocate anymore memory!");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_init(q->mutex, NULL);

    q->notFull = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    if(q->notFull == NULL)
    {
        perror("Couldn't allocate anymore memory!");
        exit(EXIT_FAILURE);   
    }
    pthread_cond_init(q->notFull, NULL);

    q->notEmpty = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    if(q->notEmpty == NULL)
    {
        perror("Couldn't allocate anymore memory!");
        exit(EXIT_FAILURE);
    }
    pthread_cond_init(q->notEmpty, NULL);

    return q;
}

//Frees a queue
void queueDestroy(queue *q)
{
    pthread_mutex_destroy(q->mutex);
    pthread_cond_destroy(q->notFull);
    pthread_cond_destroy(q->notEmpty);
    free(q->mutex);
    free(q->notFull);
    free(q->notEmpty);
    free(q);
}

//Push to end of queue
void queuePush(queue *q, char* msg)
{
    q->buffer[q->tail] = msg;
    q->tail++;
    if(q->tail == MAX_BUFFER)
        q->tail = 0;
    if(q->tail == q->head)
        q->full = 1;
    q->empty = 0;
}

//Pop front of queue
char* queuePop(queue *q)
{
    char* msg = q->buffer[q->head];
    q->head++;
    if(q->head == MAX_BUFFER)
        q->head = 0;
    if(q->head == q->tail)
        q->empty = 1;
    q->full = 0;

    return msg;
}

//Sets up and binds the socket
void bindSocket(struct sockaddr_in *serverAddr, int socketFd, long port)
{
    memset(serverAddr, 0, sizeof(*serverAddr));
    serverAddr->sin_family = AF_INET;
    serverAddr->sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr->sin_port = htons(port);

    if(bind(socketFd, (struct sockaddr *)serverAddr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("Socket bind failed: ");
        exit(1);
    }
}

//Removes the socket from the list of active client sockets and closes it
void removeClient(chatDataVars *data, int clientSocketFd)
{
    pthread_mutex_lock(data->clientListMutex);
    for(int i = 0; i < MAX_BUFFER; i++)
    {
        if(data->clientSockets[i] == clientSocketFd)
        {
            data->clientSockets[i] = 0;
            close(clientSocketFd);
            data->numClients--;
            i = MAX_BUFFER;
        }
    }
    pthread_mutex_unlock(data->clientListMutex);
}

//Thread to handle new connections. Adds client's fd to list of client fds and spawns a new clientHandler thread for it
void *newClientHandler(void *data)
{
    chatDataVars *chatData = (chatDataVars *) data;
    while(1)
    {
        int clientSocketFd = accept(chatData->socketFd, NULL, NULL);
        if(clientSocketFd > 0)
        {
            fprintf(stderr, "Server accepted new client. Socket: %d\n", clientSocketFd);

            //Obtain lock on clients list and add new client in
            pthread_mutex_lock(chatData->clientListMutex);
            if(chatData->numClients < MAX_BUFFER)
            {
                //Add new client to list
                for(int i = 0; i < MAX_BUFFER; i++)
                {
                    if(!FD_ISSET(chatData->clientSockets[i], &(chatData->serverReadFds)))
                    {
                        chatData->clientSockets[i] = clientSocketFd;
                        i = MAX_BUFFER;
                    }
                }

                FD_SET(clientSocketFd, &(chatData->serverReadFds));

                //Spawn new thread to handle client's messages
                clientHandlerVars chv;
                chv.clientSocketFd = clientSocketFd;
                chv.data = chatData;

                pthread_t clientThread;
                if((pthread_create(&clientThread, NULL, (void *)&clientHandler, (void *)&chv)) == 0)
                {
                    chatData->numClients++;
                    fprintf(stderr, "Client has joined chat. Socket: %d\n", clientSocketFd);
                }
                else
                    close(clientSocketFd);
            }
            pthread_mutex_unlock(chatData->clientListMutex);
        }
    }
}

//The "producer" -- Listens for messages from client to add to message queue
void *clientHandler(void *chv)
{
    clientHandlerVars *vars = (clientHandlerVars *)chv;
    chatDataVars *data = (chatDataVars *)vars->data;

    queue *q = data->queue;
    int clientSocketFd = vars->clientSocketFd;

    char msgBuffer[MAX_BUFFER];
    while(1)
    {
        int numBytesRead = read(clientSocketFd, msgBuffer, MAX_BUFFER - 1);
        msgBuffer[numBytesRead] = '\0';
        
        //If the client sent /exit\n, remove them from the client list and close their socket
        if(strcmp(msgBuffer, "/exit\n") == 0)
        {
            fprintf(stderr, "Client on socket %d has disconnected.\n", clientSocketFd);
            removeClient(data, clientSocketFd);
            return NULL;
        }
        else
        {
            //Wait for queue to not be full before pushing message
            while(q->full)
            {
                pthread_cond_wait(q->notFull, q->mutex);
            }

            //Obtain lock, push message to queue, unlock, set condition variable
            pthread_mutex_lock(q->mutex);
            fprintf(stderr, "Pushing message to queue: %s\n", msgBuffer);
            queuePush(q, msgBuffer);
            pthread_mutex_unlock(q->mutex);
            pthread_cond_signal(q->notEmpty);
        }
    }
}

//The "consumer" -- waits for the queue to have messages then takes them out and broadcasts to clients
void *messageHandler(void *data)
{
    chatDataVars *chatData = (chatDataVars *)data;
    queue *q = chatData->queue;
    int *clientSockets = chatData->clientSockets;
    
    while(1)
    {
        bool is_l = false, is_f = false;
        //Obtain lock and pop message from queue when not empty
        pthread_mutex_lock(q->mutex);
        while(q->empty)
        {
            pthread_cond_wait(q->notEmpty, q->mutex);
        }
        char* msg = queuePop(q);
        struct TagValuePair tagValuePairs[6];
        int count = extractTagsAndValues(msg, tagValuePairs, 6);
        char *FROM = (char*)calloc(MAX_CHARS, sizeof(char)), *TO = (char*)calloc(MAX_CHARS, sizeof(char)); // Initialize pointers to NULL

        for(int i = 0; i < count; i++){
            if(strcmp(tagValuePairs[i].tag, "REQUEST") == 0){
                if(strcmp(tagValuePairs[i].value, "LOGIN") == 0){
                    is_l = true;
                    for(int j = 0; j < count; j++)
                        if(strcmp(tagValuePairs[j].tag, "FROM") == 0){
                            FROM = tagValuePairs[j].value;
                            strcpy(chatData->Users[chatData->numClients-1].name, FROM);
                            for(int k = 0; k < MAX_USERS; k++){
                                if(strcmp(tagValuePairs[j].value, Users[k].name)==0){
                                    is_f = true;
                                    strcpy(chatData->Users[chatData->numClients-1].name, Users[k].name);
                                    chatData->Users[chatData->numClients-1].logged_in = true;
                                    break;
                                }
                            }
                                break;
                        }
                }
            }
            printf("%s : %s\n", tagValuePairs[i].tag, tagValuePairs[i].value);
        }
        pthread_mutex_unlock(q->mutex);
        pthread_cond_signal(q->notFull);
        
        if(is_l){
            if(is_f){
                sprintf(msg, "<MSG><FROM>System</FROM><TO>%s</TO><INFO>%s</INFO></MSG>",chatData->Users[chatData->numClients-1].name, "Login Successful!");
                strcat(msg,"<CLIENT_LIST>");
                for(int i = 0; i < chatData->numClients; i++){
                    strcat(msg, chatData->Users[i].name);
                    strcat(msg," ");
                }
                strcat(msg,"</CLIENT_LIST>");
            } else{
                
                sprintf(msg, "<MSG><FROM>System</FROM><TO>%s</TO><INFO>%s</INFO></MSG>",FROM, "Login Failed!");
            }
            //Broadcast message to all connected clients
            fprintf(stderr, "Broadcasting message: %s\n", msg);
            
            for(int i = 0; i < chatData->numClients; i++)
            {
                int socket = clientSockets[i];
                //printf("%s\n", chatData->Users[i].name);
                if (strcmp(FROM, chatData->Users[i].name) == 0){
                    if(socket != 0 && write(socket, msg, MAX_BUFFER - 1) == -1)
                        perror("Socket write failed: ");
                }else{
                    continue;
                }
            };
        }else{
            for(int i = 0; i < count; i++){
                if(strcmp(tagValuePairs[i].tag, "TO") == 0){
                    TO = tagValuePairs[i].value;
                    break;
                }
            }
            //Broadcast message to all connected clients
            fprintf(stderr, "Broadcasting message: %s\n", msg);
            strcat(msg,"<CLIENT_LIST>");
                for(int i = 0; i < chatData->numClients; i++){
                    strcat(msg, chatData->Users[i].name);
                    strcat(msg," ");
                }
                strcat(msg,"</CLIENT_LIST>");
            for(int i = 0; i < chatData->numClients; i++)
            {
                int socket = clientSockets[i];
                if (strcmp(TO, chatData->Users[i].name) == 0){
                    if(socket != 0 && write(socket, msg, MAX_BUFFER - 1) == -1)
                        perror("Socket write failed: ");
                }else{
                    continue;
                }
            }
        }
        
    }
}

void *charAThread(void* data) {
    chatDataVars* threadata = (chatDataVars*) data;
    queue *q = threadata->queue;
    while (1) {
        //Obtain lock and pop message from queue when not empty
        pthread_mutex_lock(q->mutex);
        while(q->empty)
        {
            pthread_cond_wait(q->notEmpty, q->mutex);
        }
        // Receive data from the input queue
        char* receivedData = queuePop(threadata->queue);

        // Process the data (replace lowercase 'a' with 'A')
        for (int i = 0; receivedData[i] != '\0'; ++i) {
            if (receivedData[i] == 'a') {
                receivedData[i] = 'A';
            }
        }
        
        pthread_mutex_unlock(q->mutex);
        pthread_cond_signal(q->notFull);

        // Share the processed data with the chare thread through its input queue
        pthread_mutex_lock(q->mutex);
        while (q->full) {
            pthread_cond_wait(q->notFull, q->mutex);
        }
        // Push the processed data to the output queue of the charE thread
        queuePush(q, receivedData);
        pthread_mutex_unlock(q->mutex);
        pthread_cond_signal(q->notEmpty);
    }
    return NULL;
}

void *charEThread(void* data) {
    chatDataVars* threadata = (chatDataVars*) data;
    queue *q = threadata->queue;
    while (1) {
        //Obtain lock and pop message from queue when not empty
        pthread_mutex_lock(q->mutex);
        while(q->empty)
        {
            pthread_cond_wait(q->notEmpty, q->mutex);
        }
        // Receive data from the input queue
        char* receivedData = queuePop(threadata->queue);

        // Process the data (replace lowercase 'a' with 'A')
        for (int i = 0; receivedData[i] != '\0'; ++i) {
            if (receivedData[i] == 'e') {
                receivedData[i] = 'E';
            }
        }
        
        pthread_mutex_unlock(q->mutex);
        pthread_cond_signal(q->notFull);

        // Share the processed data with the chare thread through its input queue
        pthread_mutex_lock(q->mutex);
        while (q->full) {
            pthread_cond_wait(q->notFull, q->mutex);
        }
        // Push the processed data to the output queue of the charE thread
        queuePush(q, receivedData);
        pthread_mutex_unlock(q->mutex);
        pthread_cond_signal(q->notEmpty);
    }
    return NULL;
}

void *charIThread(void* data) {
    chatDataVars* threadata = (chatDataVars*) data;
    queue *q = threadata->queue;
    while (1) {
        //Obtain lock and pop message from queue when not empty
        pthread_mutex_lock(q->mutex);
        while(q->empty)
        {
            pthread_cond_wait(q->notEmpty, q->mutex);
        }
        // Receive data from the input queue
        char* receivedData = queuePop(threadata->queue);

        // Process the data (replace lowercase 'a' with 'A')
        for (int i = 0; receivedData[i] != '\0'; ++i) {
            if (receivedData[i] == 'i') {
                receivedData[i] = 'I';
            }
        }
        
        pthread_mutex_unlock(q->mutex);
        pthread_cond_signal(q->notFull);

        // Share the processed data with the chare thread through its input queue
        pthread_mutex_lock(q->mutex);
        while (q->full) {
            pthread_cond_wait(q->notFull, q->mutex);
        }
        // Push the processed data to the output queue of the charE thread
        queuePush(q, receivedData);
        pthread_mutex_unlock(q->mutex);
        pthread_cond_signal(q->notEmpty);
    }
    return NULL;
}

void *charOThread(void* data) {
    chatDataVars* threadata = (chatDataVars*) data;
    queue *q = threadata->queue;
    while (1) {
        //Obtain lock and pop message from queue when not empty
        pthread_mutex_lock(q->mutex);
        while(q->empty)
        {
            pthread_cond_wait(q->notEmpty, q->mutex);
        }
        // Receive data from the input queue
        char* receivedData = queuePop(threadata->queue);

        // Process the data (replace lowercase 'a' with 'A')
        for (int i = 0; receivedData[i] != '\0'; ++i) {
            if (receivedData[i] == 'o') {
                receivedData[i] = 'O';
            }
        }
        
        pthread_mutex_unlock(q->mutex);
        pthread_cond_signal(q->notFull);

        // Share the processed data with the chare thread through its input queue
        pthread_mutex_lock(q->mutex);
        while (q->full) {
            pthread_cond_wait(q->notFull, q->mutex);
        }
        // Push the processed data to the output queue of the charE thread
        queuePush(q, receivedData);
        pthread_mutex_unlock(q->mutex);
        pthread_cond_signal(q->notEmpty);
    }
    return NULL;
}

void *charUThread(void* data) {
    chatDataVars* threadata = (chatDataVars*) data;
    queue *q = threadata->queue;
    while (1) {
        //Obtain lock and pop message from queue when not empty
        pthread_mutex_lock(q->mutex);
        while(q->empty)
        {
            pthread_cond_wait(q->notEmpty, q->mutex);
        }
        // Receive data from the input queue
        char* receivedData = queuePop(threadata->queue);

        // Process the data (replace lowercase 'a' with 'A')
        for (int i = 0; receivedData[i] != '\0'; ++i) {
            if (receivedData[i] == 'u') {
                receivedData[i] = 'U';
            }
        }
        
        pthread_mutex_unlock(q->mutex);
        pthread_cond_signal(q->notFull);

        // Share the processed data with the chare thread through its input queue
        pthread_mutex_lock(q->mutex);
        while (q->full) {
            pthread_cond_wait(q->notFull, q->mutex);
        }
        // Push the processed data to the output queue of the charE thread
        queuePush(q, receivedData);
        pthread_mutex_unlock(q->mutex);
        pthread_cond_signal(q->notEmpty);
    }
    return NULL;
}

void* writerThread(void* data) {
    return NULL;
}
