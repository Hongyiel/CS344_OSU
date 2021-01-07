/*********************************************
Assignment 4: OTP
Christopher Rico
CS344
OSU ID: 933239746
otp_enc: This is the client process that accepts
a plaintext file and key, and connects on the
provided portto otp_enc_d.
Then sends the file+key to otp_dec_d, accepts the ciphertext
response, and writes ciphertext to stdout.
**********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#define BUFSIZE 80000


//function prototypes
void sendToServer(char* filename, char* filename2, int socketFD);
void error(const char *msg, int ex);
int setUpConnection(char* portNum);



int main(int argc, char *argv[])
{
  int socketFD,
      portNumber,
      charsWritten,
      status = 1,
      ret_stat,
      numBytes,
      charsRead;
  char buffer[BUFSIZE];

  bzero(buffer, sizeof(buffer));

  //check usage and args
  if(argc != 4)
  {
    fprintf(stderr, "USAGE: input_file key_file port\n");
    exit(0);
  }

  //set up the connection with the server
  socketFD = setUpConnection(argv[3]);


  //Get the entered text and the size of the text
  int plainText = open(argv[1], O_RDONLY);
  int plainTextSize = lseek(plainText, 0 , SEEK_END);
  //Get entered key and its size
  int key = open(argv[2], O_RDONLY);
  int keySize = lseek(key, 0, SEEK_END);
  //check that the key size is greater than the ciphertext size
  if(plainTextSize > keySize)
  {
    fprintf(stderr, "Error: key is shorter than plain text. Need longer key.");
    exit(1);
  }

  //Check user-entered text for invalid characters by looping throuh each one;
  //error if a character isn't valid
  int check = open(argv[1], 'r');
  while(read(check, buffer, 1) != 0)
  {
      if(isspace(buffer[0]) || isalpha(buffer[0]) || buffer[0] == '\n')
      {}
      else
      {
          fprintf(stderr, "Invalid characters in %s\n", argv[1]);
          exit(1);
      }
  }
  //clear the buffer afterwards
  bzero(buffer, sizeof(buffer));


  //send the text and they key to the server for encryption
  sendToServer(argv[1], argv[2], socketFD);


  //get the number of bytes to expect to be sent back from the server
  ret_stat = recv(socketFD, &numBytes, sizeof(int), 0);
  numBytes = ntohl(numBytes);


  //recieve message from otp_enc_d
  //read the server's message from the socket
  charsRead = recv(socketFD, buffer, numBytes, 0);
  if (charsRead < 0)
    fprintf(stderr, "otp_enc: Error reading encrypted message from socket");

  // Read the server's message from the socket
  while (charsRead != numBytes)
  {
     charsRead += recv(socketFD, buffer + charsRead, numBytes, 0);
     if (charsRead < 0)
        fprintf(stderr, "otp_enc: Error reading encrypted message from socket");
  }


  //print encrypted text to stdout
  fprintf(stdout, "%s", buffer);
  fprintf(stdout, "\n");

  close(socketFD);
  return 0;
}



/*********************************************
 error
 For easy error reporting
**********************************************/
void error(const char *msg, int ex)
{
    perror(msg);
    exit(ex);
}



/*********************************************
 sendToServer
 Takes the names of the plaintext and key files
 As well as the socket to send to
 Opens both files, concatenates their contents into a single string
 Separated by an *
 Sends this string to the encryption daemon
**********************************************/
void sendToServer(char* filename, char* filename2, int socketFD)
{
  char buffer[BUFSIZE];
  char buffer2[BUFSIZE];
  int  msgSize;
  size_t newLen;


  bzero(buffer, sizeof(buffer));
  bzero(buffer2, sizeof(buffer2));


  //open plaintext file and copy to buffer
  FILE* f = fopen(filename, "r");
  if(f != NULL)
  {
    //read the file into the buffer
    newLen = fread(buffer, sizeof(char), BUFSIZE, f);

    if(ferror(f) != 0)
      fprintf(stderr, "otp_enc: Error reading %s", filename);

    //copy a * in for toxenization later
    buffer[--newLen] = '*';
    newLen++;
  }
  fclose(f);

  //open key file and copy into buffer
  f = fopen(filename2, "r");
  if(f != NULL)
  {
    //read the file into the buffer
    newLen += fread(buffer2, sizeof(char), BUFSIZE-1, f);

    if(ferror(f) != 0)
      fprintf(stderr, "otp_enc: Error reading %s", filename2);

    //copy a \0 in for toxenization later
    buffer[--newLen] = '\0';
  }
  fclose(f);

  //concatenate the two together to one string
  strncat(buffer, buffer2, strlen(buffer2));


  //send size of message to server first
  msgSize = strlen(buffer);
  msgSize = htonl(msgSize);
  send(socketFD, &msgSize, sizeof(msgSize), 0);


  //now send the actual plaintext + * + key
  int charsWritten = 0;
  do
  {
    charsWritten += send(socketFD, buffer, strlen(buffer), 0);
  } while(charsWritten != strlen(buffer));

  if(charsWritten < 0)
    fprintf(stderr, "otp_enc: Error writing to socket");
  if(charsWritten < strlen(buffer))
    fprintf(stderr, "otp_enc: WARNING: Not all data written to socket");

  return;
}



/*********************************************
 setUpConnection
 Takes a port number in char form as an argument
 Sets up the low-level connection stuff
 Returns the socket file descriptor
**********************************************/
int setUpConnection(char* arg3)
{
  char buffer[BUFSIZE];
  char authenticate[] = "_";
  struct hostent* serverHostInfo;
  struct sockaddr_in serverAddress;
  int socketFD,
      portNum,
      yes = 1;

  //get the port number into int form
  portNum = atoi(arg3);

  //set up server address struct
  //clear out the address struct
  memset((char*)&serverAddress, '\0', sizeof(serverAddress));
  //create a network-capable socket
  serverAddress.sin_family = AF_INET;
  //store the port number
  serverAddress.sin_port = htons(portNum);
  //convert the machine name into a special form of address
  serverHostInfo = gethostbyname("localhost");

  if(serverHostInfo == NULL)
  {
    fprintf(stderr, "otp_enc: ERROR, no such host\n");
    exit(0);
  }

  memcpy((char*) &serverAddress.sin_addr.s_addr, (char*) serverHostInfo->h_addr, serverHostInfo->h_length);

  //set up socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

  //connect socket to address
  if(connect(socketFD, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0)
    error("otp_enc: ERROR connecting", 0);

  //send auth token to server
  send(socketFD, authenticate, sizeof(authenticate), 0);
  //read auth token from server
  recv(socketFD, buffer, sizeof(buffer), 0);
  //if auth token is invalid, reject connection and exit
  if(strcmp(buffer, "+") == 0)
  {
      fprintf(stderr, "otp_enc: connection attempt with otp_dec_d on port %d was rejected.", portNum);
      exit(2);
  }
  else if(strcmp(buffer, "_") != 0)
  {
    fprintf(stderr, "otp_enc: connection attempt on port %d was rejected.", portNum);
    exit(2);
  }
  return socketFD;
}
