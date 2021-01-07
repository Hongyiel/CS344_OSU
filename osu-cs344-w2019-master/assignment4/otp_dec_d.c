/*
*	NAME : HONGYIEL SUH
*	STUDENT NUMBER: 933612189
* File Name : otp_dec_c.c
* deamon file that process running on the background during decrypt file input
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void decript_Otp(char* cipher, char* key); // declare decrypt message
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

	int charsSignal, status; // these are signal for comparing
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

// decrypt
void decript_Otp(char* cipher, char* key){ //decrypt
	char n;
	//for the length of the message
	char alphabet[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	// These arrays will hold the numerical equivalents of our message and key
	int msgidx[strlen(cipher)];
	int keyidx[strlen(key)];

	// Trim off newlines if they're received
	if (cipher[strlen(cipher)] == '\n'){ cipher[strlen(cipher)] = '\0';}
	if (key[strlen(key)] == '\n'){ key[strlen(key)] = '\0';}
	// setting indx
	int i;
	int j;
	int k;

	for (i = 0; i < strlen(cipher); i++){
		for (j = 0; j < strlen(alphabet); j++)
			if (cipher[i] == alphabet[j])
				msgidx[i] = j;
	}
	// message length approved storing message
	j = 0;

	for (k = 0; k < strlen(key); k++){
		for (j=0; j < strlen(alphabet); j++)
			if (key[k] == alphabet[j])
				keyidx[k] = j;
	}
	// key will approved

	i = 0;
	int offset;
	for (i = 0; i < strlen(cipher); i++){
		offset = i % strlen(key);
		int counterE = (msgidx[i] - keyidx[offset]); // getting integer message
		if (counterE < 0)
			counterE = 27 + counterE;
		cipher[i] = alphabet[counterE];	// Storing the encrypted letters
	}
	// Reference :: http://www.trytoprogram.com/c-examples/c-program-to-encrypt-and-decrypt-string/


}
