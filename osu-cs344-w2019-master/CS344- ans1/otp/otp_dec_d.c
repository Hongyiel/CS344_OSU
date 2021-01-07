/*********************************************
Assignment 4: OTP
Christopher Rico
CS344
OSU ID: 933239746
otp_dec_d: This is the 'daemon' process that waits
for an incoming connection from otp_dec,
then decodes the ciphertext message with the provided key
**********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stddef.h>
#define BUFSIZE 80000

//function prototypes
void error(const char *msg, int num);
int setUpListenSocket(char* arg1, int numConnects);
void childProc(int connectFD);
void decryptText(char** cipherText, char**key);
char iToC(int i);
int cToI(char c);



int main (int argc, char *argv[])
{
  int listenSocketFD,
      establishedConnectionFD;
  struct sockaddr_in clientAddress;
  socklen_t sizeOfClientInfo;
  pid_t pid;


  //check number of arguments - should only be 2 (process name and port num)
  if(argc != 2)
  {
    fprintf(stderr, "USAGE: port\n");
    exit(1);
  }

  //set up the socket to listen for 5 incoming connections
  listenSocketFD = setUpListenSocket(argv[1], 5);


  //loop to accept incoming connections and fork them off to new processes
  while(1)
  {
    //accept a connection, blocking if one is not available until one connects
    //get the size of the address for the client that will connect
    sizeOfClientInfo = sizeof(clientAddress);
    //accept connection
    establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *) &clientAddress, &sizeOfClientInfo);
    if(establishedConnectionFD < 0)
      error("ERROR on accept", 1);


    //when new connection is accepted, fork into new process
    //and run switch to determine what action to take
    pid = fork();
    switch(pid)
    {
      //if there is an error with forking
      case -1:
        error("Error on fork", 1);
        break;
      //child process, handle all incoming connections this way
      case 0:
        childProc(establishedConnectionFD);
        exit(0);
        break;
    }

    //close the existing socket which is connected to the client
    close(establishedConnectionFD);

  }

  //close the listening socket
  close(listenSocketFD);
  return 0;
}



/*********************************************
  error
  For easy error reporting
**********************************************/
void error(const char *msg, int num)
{
  perror(msg);
  exit(num);
}



/*********************************************
  childProc
  Takes the socket to send to as an argument
  Accepts connection from otp_dnc process and authenticates
  Recv in the ciphertext + key and separates them
  Decrypts ciphertext by calling decryptText
  Sends plaintext back to otp_dnc
**********************************************/
void childProc(int connectFD)
{
  int i,
      ret_stat,
      charsRead,
      msgSize,
      numBytes;
  char* key;
  char* message;
  char* saveptr;

  char buffer[BUFSIZE];

  bzero(buffer, sizeof(buffer));


  //verify that we are connected to otp_enc by checking for auth token
  recv(connectFD, buffer, sizeof(buffer) -1, 0);
  if(strcmp(buffer, "_") == 0)
  {
    //if the auth token is not correct, return that to the connection
    char auth[] = "+";
    send(connectFD, auth, sizeof(auth), 0);
  }
  //otherwise, return the auth token to otp_enc
  else if(strcmp(buffer, "!") == 0)
  {
    char auth[] = "!";
    send(connectFD, auth, sizeof(auth), 0);
  }
  else
  {
    char auth[] = "?";
    send(connectFD, auth, sizeof(auth), 0);
  }

  //clear out the buffer for more use
  bzero(buffer, sizeof(buffer));


  //get the number of bytes to expect to be sent in the message
  ret_stat = recv(connectFD, &numBytes, sizeof(int), 0);
  numBytes = ntohl(numBytes);


  //receive message and key from otp_enc
  //read the client's message from the socket
  charsRead = recv(connectFD, buffer, numBytes, 0);
  if (charsRead < 0)
    fprintf(stderr, "otp_dec_d: Error reading from socket");

  // Read the client's message from the socket
  while (charsRead != numBytes)
  {
     charsRead += recv(connectFD, buffer + charsRead, numBytes, 0);
     if (charsRead < 0)
        fprintf(stderr, "otp_dec_d: Error reading from socket");
  }


  //tokenize the recv data into message and key by using *
  message = strtok_r(buffer, "*", &saveptr);
  key = strtok_r(NULL, "*", &saveptr);


  //decrypt the message using the key
  decryptText(&message, &key);


  //send the decrypted message back to otp_enc
  //send size of message to server first
  msgSize = strlen(message);
  msgSize = htonl(msgSize);
  send(connectFD, &msgSize, sizeof(msgSize), 0);


  //now send the actual decrypted message back
  int charsWritten = 0;
  do
  {
    charsWritten += send(connectFD, message, strlen(message), 0);
  } while(charsWritten != strlen(message));

  if(charsWritten < 0)
    fprintf(stderr, "otp_dec_d: Error writing to socket");
  if(charsWritten < strlen(message))
    fprintf(stderr, "otp_dec_d: WARNING: Not all data written to socket");


  return;
}



/*********************************************
  setUpListenSocket
  Takes port number as char*, as well as
  The number of connections to listen for
  Sets up low-level networking, turns on listen socket
  Returns listen socket file descriptor
**********************************************/
int setUpListenSocket(char* arg1, int numConnects)
{
  struct sockaddr_in serverAddress;
  int portNum,
      listenSocketFD,
      yes = 1;


  //create the socket and save its file descriptor for later use
  listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
  if(listenSocketFD < 0)
    error("ERROR opening socket", 1);

  setsockopt(listenSocketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));


  //set up the address struct for this process (the server)
  //clear out the address struct
  memset((char *)&serverAddress, '\0', sizeof(serverAddress));
  //get the prot number, convert to an integer from a string
  portNum = atoi(arg1);
  //create a network-capable socket
  serverAddress.sin_family = AF_INET;
  //store the port portNumber
  serverAddress.sin_port = htons(portNum);
  //any address is allowed for connection to this process
  serverAddress.sin_addr.s_addr = INADDR_ANY;


  //connect socket to port
  if(bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    error("ERROR on binding", 1);

  //turn on socket - it can now receive up to 5 connections
  listen(listenSocketFD, numConnects);

  return listenSocketFD;
}



/*********************************************
  encryptText
  Takes pointer to pointers of plaintext and key arrays
  Decrypts using a One-Time-Pad method
**********************************************/
void decryptText(char** cipherText, char**key)
{
  int i,
      textNum,
      keyNum,
      decryptNum,
      length;

  //get the length of the plain text string
  length = (strlen(*cipherText));

  //loop through the string
  for (i = 0; i < length; i++)
  {
    //get the character value for each key and string char
    textNum = cToI(*(*cipherText + i));
    keyNum = cToI(*(*key + i));

    //add them and take the modulo 27
    decryptNum = ((textNum - keyNum) % 27);

    if(decryptNum < 0)
      decryptNum += 27;

    //then use this hash to assign encoded chars to the plaintext string
    *(*cipherText + i) = iToC(decryptNum);

  }
  //can't forget the null term
  *(*cipherText + i) = '\0';

  return;
}



/*********************************************
  iToC
  Converts single integer 0-27 to corresponding char A-Z plus space
**********************************************/
char iToC(int i)
{
  //all the acceptable characters
  static const char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

  //if i is invalid, say so to stderr
  if(i > 27 || i < 0)
    fprintf(stderr, "Encryption: invalid character detected.");

  return chars[i];
}


/*********************************************
  cToI
  Converts char A-Z plus space to corresponding int 1-27
**********************************************/
int cToI(char c)
{
  int i;
  static const char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

  //loop through and if we find the correct character, return its position
  for (i = 0; i < 27; i++)
  {
    if(c == chars[i])
      return i;
  }

  //else there was an invalid characteer. say so to stderr
  fprintf(stderr, "Encryption: invalid character detected.");
}
