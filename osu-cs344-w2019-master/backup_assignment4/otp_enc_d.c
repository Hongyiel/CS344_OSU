#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
void encript_Otp(char*  plaintext, char* key); //encrypt message
int charInt (char c);
char intChar(int i);

void error(const char *msg) {
	// int errno = 1;
	perror(msg);
	exit(1);
} // Error function give message that contain problem
int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsSignal, charsSent, status;
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

		// process start
	while(1)
	{
		//turn over socket in 5 connections
		listen(listenSocketFD, 5);
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");

		//create children
		pid_t pid = fork();
		if(pid == -1){
			error("Hull Breanch.\n");
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

					//make sure we are connected to the correct server
					if(strcmp(buffer, "encrypt") != 0){
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
						// init memory setting
						charsSignal = 0;
						memset(buffer, '\0', sizeof(buffer));
						// get ciphertext
						while(size > charsSignal){
							//reading message
							charsSignal += recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
							// message get from buffer
							strcat(message, buffer);
						}
						// buffer clean
						memset(buffer, '\0', sizeof(buffer));
						charsSignal = 0;
						// Signal comparing to getting read buffer
						while(size > charsSignal){
							charsSignal += recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
							//getting key from buffer
							strcat(key, buffer);
						}
						//Cleaning buffer & getting encript_Otp
						memset(buffer, '\0', sizeof(buffer));
						encript_Otp(message, key);
						// sending plaintext and getting beck to the client
						charsSignal = 0;
						while( size > charsSignal )
							charsSignal += send(establishedConnectionFD, message, sizeof(message), 0);
						exit(0);

					}
		} else {
					pid_t pid_wait = waitpid(pid, &status, WNOHANG);
					// parents process wait for child
		}//end of if else
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
char intChar(int i){

	//if the number is 26, return a space
	if (i == 26){
		return ' ';
	}
	//otherwise, return the integer plus ascii of A
	else
	{
		return (i + 'A');
	}
}

 /*charInt: takes one char as an argument. Returns the numerical value of that argument,
  * in relation to the character array defined in Keygen.
  */
 int charInt (char c){

 	//if c is a space
 	if (c == ' '){
 		return 26;
 	}
 	//therwise subtract the ascii of A from the character
 	else {
 		return (c - 'A');
 	}
 	return 0;
 }
 /*encript_Otp: takes two char arrays, a mesage and a key. uses the key to encript_Otp the message
  * using the One Time Pad technique. Returns stores the encrypted message in the
  * passed message array.
  */
void encript_Otp(char*  plaintext, char* key){ //encrypt a given message
	int i=0;
	char n;

	//for the length of the message
	while (plaintext[i] != '\n')
	{
			//get the character
	  		char c = plaintext[i];

			//convert the character and it's key match to integers.
			//add them and mod 27 to get the encrypted character.
	  		n = (charInt(plaintext[i]) + charInt(key[i])) % 27;

			//store the encrypted charcter in place of the original
			//character
	  		plaintext[i] = intChar(n);
			i++;
	}
	//overwrite the original newline with an endline
	plaintext[i] = '\0';
	return;
}
