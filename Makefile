CC = gcc
CFLAGS = -Wall -Wextra -l c -std=gnu17 -g3

# Executable names
MAIN = Main
SERVER = Server
CLIENT = Client
# Source files
MAIN_SRC = Main.c
CLIENT_SRC = client.c 
SERVER_SRC = server.c
CONST_SRC = encDec.h application.c data_link.c physical.c

# Object files
MAIN_OBJS = $(MAIN_SRC:.c=.o)
CLIENT_OBJS = $(CLIENT_SRC:.c=.o)
SERVER_OBJS = $(SERVER_SRC:.c=.o)
.PHONY: all clean

all: $(MAIN) $(SERVER) $(CLIENT)


$(MAIN): $(MAIN_OBJS) $(CONST_SRC)
	$(CC) $(CFLAGS) -o $@ $^
$(SERVER): $(SERVER_OBJS) $(CONST_SRC)
	$(CC) $(CFLAGS) -o $@ $^
$(CLIENT):$(CLIENT_OBJS) $(CONST_SRC)
	$(CC) $(CFLAGS) -o $@ $^
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(MAIN) $(MAIN_OBJS) $(CLIENT) $(CLIENT_OBJS) $(SERVER) $(SERVER_OBJS)
