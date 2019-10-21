// This file is the main server program. It is run like so:
// ./server <listeningport>
// After starting the server, it waits for a connection at its listening port.
// Once a connection is established, the connection socket waits for messages.
// For every message, it assumes it recieves a filename. If it does not exist
// an error message is returned. If it does, it begins reading the file into
// the buffer and then sends the buffer to the client.
// After this, the process repeats until the client ends the connection.

#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXPENDING 5 

void DieWithError(char *errorMessage);
void HandleTCPClient(int clntSocket);

int main(int argc, char* argv[])
{
  int servSock; /* Socket descriptor for server */
  int clntSock; /* Socket descriptor for client */
  struct sockaddr_in echoServAddr; /* Local address */
  struct sockaddr_in echoClntAddr; /* Client address */
  unsigned short echoServPort; /* Server port */
  unsigned int clntLen; /* Length of client address data structure */

  /* Check for the correct number of arguments */
  if(argc != 2) {
    fprintf(stderr, "Usage: %s <listeningport>\n", argv[0]);
    return 1;
  }
  printf("Starting server...\n");

  /* start listening socket */
  echoServPort = atoi(argv[1]);
  if((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    DieWithError("Could not create socket, try another port?");
  }

  /* Construct local address structure */
  memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
  echoServAddr.sin_family = AF_INET; /* Internet address family */
  echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
  echoServAddr.sin_port = htons(echoServPort); /* Local port */

  /* Bind to the local address */
  if (bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0) {
    DieWithError ("bind() failed");
  }
  else {
    printf("bind() succeed!\n");
  }

  /* Mark the socket so it will listen for incoming connections */
  if (listen(servSock, MAXPENDING) < 0) {
    DieWithError("listen() failed") ;
  }
  else {
    printf("listen succeed!\n");
  }

  for (;;) /* Run forever */
    {
      /* Set the size of the in-out parameter */
      clntLen = sizeof(echoClntAddr);
      /* Wait for a client to connect */
      if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
	DieWithError("accept() failed");
      /* clntSock is connected to a client! */
      printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
      HandleTCPClient (clntSock) ;
    }
  
  printf("Closing server...\n");
  close(servSock);
  printf("Server Closed.\n");
  return 0;
}
