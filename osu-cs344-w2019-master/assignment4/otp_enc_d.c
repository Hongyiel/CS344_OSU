/*
*	NAME : HONGYIEL SUH
*	STUDENT NUMBER: 933612189
* File Name : otp_enc_c.c
* the is deamen process that encrypt system input from file that generated
* process will run during file acticating.
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
void encript_Otp(char*  cipher, char* key); //encrypt a given message

void error(const char *msg) {
	perror(msg);
	exit(1);
} // Error function give message that contain problem
int main(int argc, char *argv[])
{
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;

	// checking USAGE Error
	if (argc < 2) {
		fprintf(stderr, "USAGE: %s port\n", argv[0]);
		exit(1);
	}

	// get network port
	int listenSocketFD, establishedConnectionFD, charsSent;
	// Memory setting in NULL point for getting serverAddress
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear memory

	int portNumber = atoi(argv[1]); // getting portNumber

	serverAddress.sin_family = AF_INET; // Network create socket
	serverAddress.sin_port = htons(portNumber); // port number storing
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Address contains
	//setting up socket and halding error

	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Creating the socket
	if (listenSocketFD < 0) error("ERROR opening socket");
	//binding if enable socket
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");

		int charsSignal, status; // this will status signal on waitpid
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
// encrypt
void encript_Otp(char*  cipher, char* key){ //encrypt a given message

		// Set up a mapping array to avoid dealing with Ascii conversion
		char alphabet[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

		// These arrays will hold the numerical equivalents of our message and key
		int msgidx[strlen(cipher)];
		int keyidx[strlen(key)];
		// setting idx
		int i;
		int j;
		int k;

		for (i = 0; i < strlen(cipher); i++){
			for ( j = 0; j < strlen(alphabet); j++)
				if (cipher[i] == alphabet[j])
					msgidx[i] = j;
		}
		// message length approved storing message

		for (k = 0; k < strlen(key); k++){
			j = 0;
			for (j=0; j<strlen(alphabet); j++)
				if (key[k] == alphabet[j])
					keyidx[k] = j;
		}
		// key will approved
		i = 0;
		int offset;
		for (i = 0; i < strlen(cipher); i++){
			offset = i % strlen(key);
			int counterE = (msgidx[i] + keyidx[offset]) % 27; // comparing
			cipher[i] = alphabet[counterE];	// Storing message encrypt
		}
		// Reference :: http://www.trytoprogram.com/c-examples/c-program-to-encrypt-and-decrypt-string/

}
