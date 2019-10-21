#include <stdio.h> /* for printf() and fprintf() */
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <unistd.h> /* for close() */

#define RCVBUFSIZE 1024 /* Size of receive buffer */

FILE *file;

void DieWithError(char *errorMessage);

void HandleTCPClient(int clntSocket)
{
  char buffer[RCVBUFSIZE];     /* Buffer for echo string */
  int sendMsgSize = 0; /* modified because the server holds the file */
  int sentSize = 0; /* verification value to compare with sendMsgSize*/
  int checksum = 0;
  int clientChecksum = 0;
  int size; /* filesize in bytes */
  int eof; /* boolean (0 or 1) for whether the eof has been reached on file transfer */
  
  printf("Enter clientHandler, clntSocket: %d\n", clntSocket);


  /* Receive message from client */
  if((sendMsgSize = recv(clntSocket, buffer, RCVBUFSIZE, 0)) < 0) {
    DieWithError("recv() failed") ;
  }

  printf("sendMsgSize: %d\n", sendMsgSize);

  /* fix string and check that file is present */
  buffer[sendMsgSize] = '\0';
  printf("Requested file: %s\n", buffer);
  if(access(buffer, F_OK ) != -1) {
    printf("File Exists\n");
    
    file = fopen(buffer, "r"); /* buffer file and get size */
    fseek(file, 0L, SEEK_END);
    size = ftell(file);
    printf ("Sending: %d bytes in total\n", size);
    rewind(file); 
    
    eof = 0;
    while(size > 0 && eof == 0){

      sendMsgSize = fread(buffer, sizeof(char), RCVBUFSIZE - 1, file);

      /* checksum calculator, just sums all the byte ascii values */
      for(int i = 0; i < sendMsgSize; i++) {
	checksum = (checksum + (int)buffer[i]) % RCVBUFSIZE;
      }

      if(ferror( file ) != 0) {
	DieWithError("Error reading file");
      } else {
	buffer[sendMsgSize++] = '\0';
      }

      /* Send segment */
      sentSize = send(clntSocket, buffer, sendMsgSize, 0);

      /* remaining bytes */
      size -= (sendMsgSize - 1);

      /* validates that the number of bytes buffered to send were successfully sent */
      if(sentSize != sendMsgSize) {
	DieWithError("Something went wrong with sending. Memory error?");
      }

      if(size <= 0) {
	eof = 1;
	printf("Done sending.\n");
	checksum--;
	printf("Server Checksum: %d\n",checksum);
	/* let the client know that the eof is reached and then send a checksum for validation */
	send(clntSocket,&eof,sizeof(int),0);
	sleep(1);
	printf("Server Checksum: %d\n",checksum);
	send(clntSocket,&checksum,sizeof(int),0);
      }
    }
    fclose(file);
  }
  else { /* File does not exist or we don't have access. Just tell the user it doesn't exist */
    printf("404: File not found.\n");
    sprintf(buffer, "404: File not found.\n\0");
    sendMsgSize = strlen(buffer);
    send(clntSocket, buffer, sendMsgSize,0);
  }

  close(clntSocket); /* Close client socket */
}
