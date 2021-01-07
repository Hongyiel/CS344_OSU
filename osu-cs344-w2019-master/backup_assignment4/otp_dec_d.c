#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void decript_Otp(char* cipher, char* key); // decrypt message
int charInt (char c);
char intChar(int i);


void error(const char *msg) {
	// int errno = 1;
	perror(msg);
	exit(1);
} // Error function give message that contain problem

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsSent, status;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;

	// checking USAGE Error
	if (argc < 2) {
		fprintf(stderr, "USAGE: %s port\n", argv[0]);
		exit(1);
	}
	// Memory setting in NULL point for getting serverAddress
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear memory

	portNumber = atoi(argv[1]); // getting portNumber

	serverAddress.sin_family = AF_INET; // Network create socket
	serverAddress.sin_port = htons(portNumber); // port number storing
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Address contains
	//setting up socket and halding error

	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Creating the socket
	if (listenSocketFD < 0) error("ERROR opening socket");
	//binding if enable socket
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");

	int charsSignal;
	// process start
	while(1)
	{
		//turn over socket in 5 connections
		listen(listenSocketFD, 5);
		sizeOfClientInfo = sizeof(clientAddress); // Getting size of Client that connected
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR: on accept");

// process created
		pid_t pid = fork();
		if (pid == -1) {
				// prcess ERROR handler
					error("Hull Breach.\n");
					// exit will activated

		}
		else if(pid == 0){
			char buffer[1024]; // buffer will 1024
			char* encryptedMessage[100000]; // MAXIMUM buffer
			// setting message and key
			char message[100000];
			char key[100000];
			// buffer should cleaning up
			memset(buffer, '\0', sizeof(buffer));
			charsSignal = 0;

			// getting Client message
			while(charsSignal == 0)
				charsSignal = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
					// read socket is wrong then // this will active otp_dec_d.c
					if (charsSignal < 0) error("ERROR reading from socket");
					// if buffer is not correct : server is not connected
					if(strcmp(buffer, "decrypt") != 0){
						charsSignal = send(establishedConnectionFD, "err", 3, 0);
						exit(2);
					}else{
						// setting clean buffer
						memset(buffer, '\0', sizeof(buffer));
						// seding Signal that correct function
						charsSignal = send(establishedConnectionFD, "good", 4, 0);
						charsSignal = 0;

						//getting file length
						while(charsSignal == 0)
							charsSignal = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
						//getting buffer length

						int size = atoi(buffer);
						// init
						charsSignal = 0;
						charsSent = 0;
						//recieving the cipertext
						while(size > charsSignal){
							//reading message
							charsSent += recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
							charsSignal += charsSent;
							//getting init charsSent
							charsSent = 0;

							// message get from buffer
							strcat(message, buffer);

						}
						// buffer clean
						memset(buffer, '\0', sizeof(buffer));
						charsSignal = 0;
						charsSent = 0;
						// Signal comparing to getting read buffer
						while(size > charsSignal){

							charsSent = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
							charsSignal += charsSent;
							charsSent = 0;
							//getting key from buffer
							strcat(key, buffer);
						}
						//Cleaning buffer & getting encript_Otp
						memset(buffer, '\0', sizeof(buffer));
						decript_Otp(message, key);
						// sending plaintext and getting beck to the client
						charsSignal = 0;
						while( size > charsSignal )
							charsSignal += send(establishedConnectionFD, message, sizeof(message), 0);
						exit(0);
					}
		}else{
					pid_t pid_wait = waitpid(pid, &status, WNOHANG);
					// parents process wait for child
		}//end of if
		//closing client socket
		close(establishedConnectionFD);
	}
	//closing listening socket
	close(listenSocketFD);
	return 0;
}

/* intChar: takes one integer as an argument. Returns a charcater based on the character
 * array defined in keygen.
 */
// char intChar(int i){
//
// 	//if the number is 26, return a space
// 	if (i == 26){
// 		return ' ';
// 	}
// 	//otherwise, return the integer plus ascii of A
// 	else
// 	{
// 		return (i + 'A');
// 	}
// }
//
// /*decript_Otp: takes two char arrays, a mesage and a key. uses the key to decript_Otp the message
//  * using the One Time Pad technique. Returns stores the decrypted message in the
//  * passed message array.
//  */
//  int charInt (char c){
//
//  	//if c is a space
//  	if (c == ' '){
//  		return 26;
//  	}
//  	//therwise subtract the ascii of A from the character
//  	else {
//  		return (c - 'A');
//  	}
//  	return 0;
//  }

void decript_Otp(char* cipher, char* key){ //decrypt
	int i=0;
	char n;
	//for the length of the message
	while (cipher[i] != '\n')
	{
		//convert the message character and it's matching key character
		//subtract the key integer from the meessage integer
		n = charInt(cipher[i]) - charInt(key[i]);

		//if the result is negative, add 27
	  	if (n<0)
			n += 27;
	  	//convert the result to a character and store in place of the original
		//character.
	  	cipher[i] = intChar(n);
		i++;
	  }
	  //add a null terminator to the end.
	  cipher[i] = '\0';
	  return;
}
