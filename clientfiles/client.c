/* This file is the main client program.
 * It is run like so:
 * ./client <destaddr> <listeningport> <filename>
 * After starting and successfully establishing a connection, the client
 * asks for file names to download, one at a time.
 * If a file does not exists, an error is returned. If it does, the file
 * contents are returned.
 * After this, the client verifies that the file is correct.
 * Then, the client can download a new file, or close the connection and exit.
 */

#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RCVBUFSIZE 1024 /* Size of receive buffer */

void DieWithError(char *errorMessage);

int main(int argc, char* argv[])
{
  int sock; /*Socket descriptor*/
  struct sockaddr_in servAddr;
  unsigned short servPort;
  char *servIP;
  char *fileString; /* filename */
  char buffer[RCVBUFSIZE];
  unsigned int fileStringLen;
  int bytesRcvd, totalBytesRcvd;
  FILE *file;
  int eof = 0; /* end of file boolean, same as server */
  int checksum = -1; /* validation checksums */
  int serverChecksum = -1;

  /* Check for the correct number of arguments */
  if(argc != 4) {
    fprintf(stderr, "Usage: %s <destaddr> <listeningport> <filename>\n", argv[0]);
    return 1;
  }

  printf("Starting client...\n");
  servIP = argv[1];
  servPort = atoi(argv[2]);
  fileString = argv[3];

  /* create the file for the bytes to be downloaded */
  file = fopen(fileString,"w");
  if(file == NULL){
    DieWithError("Can't create file. Does it already exist, or is it in use?");
  }

  /*Create a socket using TCP*/
  if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    DieWithError("socket() failed");
  }

  /*Construct the server address structure*/
  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = inet_addr(servIP);
  servAddr.sin_port = htons(servPort);

  /*Establish connection to echo server*/
  if(connect(sock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0) {
    DieWithError("connect( ) failed");
  }

  printf("connect succeed\n");
  fileStringLen = strlen(fileString);

  /* request the file */
  if (send(sock, fileString, fileStringLen, 0) != fileStringLen) {
    DieWithError("send() sent a different number of bytes than expected. Memory error?");
  }

  printf("sent request successfully\n");
  /*Receive the same string back from the server*/
  totalBytesRcvd=0;

  /* runs until eof (end-of-file boolean) is set */
  while(eof == 0)
    {
      /* error handling */
      if((bytesRcvd = recv(sock, buffer, RCVBUFSIZE, 0)) <= 0) {
	DieWithError("Something was sent recv returned an error code. Is the server alive?");
      }
      if(strncmp(buffer,"404: File not found.\n\0",bytesRcvd) == 0){
	DieWithError("404: File not found.\n");
      }
      else{
	/* Do the same checksum as the server, sum of ascii bytes */
	for(int i = 0; i < bytesRcvd; i++){
	  checksum = (checksum + (int)buffer[i]) % RCVBUFSIZE;
	}

	if((recv(sock, &eof, sizeof(eof), 0)) <= 0) {
	  DieWithError("recv() failed or connection closed prematurely");
	}

	/* write bytes to file */
	fprintf(file, "%s", buffer);
      }
    }

  /* after eof is set, loop breaks, go to checksum validation */
  if(checksum > -1){
    recv(sock, &serverChecksum, sizeof(serverChecksum), 0);
    printf("\nServer's checksum: %d",serverChecksum);
    printf("\nClient's checksum: %d",checksum);
    if(checksum == serverChecksum){
      printf("Validation successful.\n");
    }
    else{
      DieWithError("Checksums did not match.");
    }
  }

  printf("Closing client...\n");
  close(sock);
  printf("Client Closed.\n");
  return 0;
}
