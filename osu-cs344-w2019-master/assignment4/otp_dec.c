/*
*	NAME : HONGYIEL SUH
*	STUDENT NUMBER: 933612189
* File Name : otp_dec.c
*	decrypt file input to send result  on the screen
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <ctype.h>

void error(const char *msg) {
	perror(msg);
	exit(1);
} // Error function give message that contain problem

int main(int argc, char *argv[])
{
	char buffer[1024];
	char plaintext[100000];
	int portNumber, charsSignal, charsRead;
	struct sockaddr_in serverAddress;


	if (argc < 4) { fprintf(stderr, "USAGE: %s text key port\n", argv[0]); exit(1); } // Error function used

	//memory setting for serverAddress
	memset((char*)&serverAddress, '\0', sizeof(serverAddress));

	portNumber = atoi(argv[3]); //getting port number from input


	serverAddress.sin_family = AF_INET; 													// creating Network socket
	serverAddress.sin_port = htons(portNumber); 									// storing port number
	struct hostent* serverHostInfo = gethostbyname("localhost"); // getting host name : localhost

	if (serverHostInfo == NULL) { fprintf(stderr, "ERROR: Client no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
	// Setting the socket
	int socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("ERROR: Client opening socket");
	int good = 1;
	setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &good, sizeof(int)); //setting socket

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){ // comparing connect
		error("ERROR: client connecting");
	}
	// getting two files as long types because of plaintext4
	long msglength = get_file_len(argv[1]);
	long keylength = get_file_len(argv[2]);
	// comparing message's length and key's length
	if(msglength > keylength)
		error("Error: key ‘myshortkey’ is too short");
	//send signal to comparing
	char* msg = "decrypt";
	charsSignal = send(socketFD, msg, strlen(msg), 0);
	memset(buffer, '\0', sizeof(buffer));
	// memory cleaning for recv another buffer

	charsSignal = 0; // init signal
	while(charsSignal == 0)
		charsSignal = recv(socketFD, buffer, sizeof(buffer) - 1, 0);	// recv signal to buffer
	if(strcmp(buffer, "err") == 0)
		error("otp_enc error: input contains bad characters");	// setting buffer cleaning

	memset(buffer, '\0', sizeof(buffer));
	sprintf(buffer, "%d", msglength);													//get file legnth to buffer
	charsSignal = send(socketFD, buffer, sizeof(buffer), 0);	//Sending length to the server

	// cleaning memory setting
	memset(buffer, '\0', sizeof(buffer));
	// open file distruptor
	int fd = open(argv[1], 'r');
	charsSignal = 0;

	/* FILE OPEN */

	int bytesread;
	// sending file buffer to server
	while(charsSignal <= msglength){
		bytesread = read(fd, buffer, sizeof(buffer)-1);							//read data from the file.
		charsSignal += send(socketFD, buffer, strlen(buffer), 0);		//send the data to the server
	}
	memset(buffer, '\0', sizeof(buffer));	//clear buffer

	//Repeat the same process as above, but now with the key.
	fd = open(argv[2], 'r');
	charsSignal = 0;
	while(charsSignal <= msglength){
		bytesread = read(fd, buffer, sizeof(buffer)-1);
		charsSignal += send(socketFD, buffer, strlen(buffer), 0);

	}

	/*
	 WARNING:: File need to open seperately!!
		IF not it will give segmentfalut() error
	*/


	//clear the buffer.
	memset(buffer, '\0', sizeof(buffer));
	charsSignal = 0;
	// looping all data recieved
	while(charsSignal < msglength){
		charsSignal += recv(socketFD, buffer, sizeof(buffer)-1, 0);		//get messages
		strcat(plaintext, buffer); 						// recieved data will added in message
	}
	strcat(plaintext, "\n"); // adding newline in the plaintext
	printf("%s", plaintext); // print plaintext
	close(socketFD); 				// Closing socket
	return 0;
}

// length of file will return
// length of file will return
// open file in read only
int get_file_len(const char* filename){
	int len = 0;
	FILE* file = fopen(filename, "r");
	int character = fgetc(file); 	// get first letter
	// Reading until End Of File
	// if characters are not upper case or ' ' space it will give error
  while (!(character == EOF || character == '\n')){
		if(!isupper(character) && character != ' ')
			error("File contains bad characters!");
		character = fgetc(file); 			// get next character
        	++len;
    	}
	fclose(file);// closing file
	return len;
}
