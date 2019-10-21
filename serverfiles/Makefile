CC=gcc
CFLAG=-Wall

all: client server

OTHER_OBS = DieWithError.o clientHandler.o

client: client.o $(OTHER_OBS)

server: server.o $(OTHER_OBS)

DieWithError.o: DieWithError.c
	$(CC) -c DieWithError.c

clientHandler.o: clientHandler.c
	$(CC) -c clientHandler.c

client.o: client.c
	$(CC) -c client.c

server.o: server.c
	$(CC) -c server.c

clean:
	rm -f client server core *.o *~
