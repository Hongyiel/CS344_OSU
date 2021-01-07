#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>


void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

// Decrypt takes an encrypted message and a key, and diffs them modularly
int decrypt(char* message, char* key){

	// Set up a mapping array to avoid dealing with Ascii conversion
	char alphabet[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

	// These arrays will hold the numerical equivalents of our message and key
	int mIndeces[strlen(message)];
	int kIndeces[strlen(key)];

	// Trim off newlines if they're received
	if (message[strlen(message)] == '\n'){
		message[strlen(message)] = '\0';
	}
	if (key[strlen(key)] == '\n'){
		key[strlen(key)] = '\0';
	}

	int m;	// Map values of message[] to indeces of alphabet[]
	for (m = 0; m < strlen(message); m++){
		int a;
		for (int a = 0; a < strlen(alphabet); a++)
			if (message[m] == alphabet[a])
				mIndeces[m] = a;
	}

	int k;	// Map values of key[] to indeces of alphabet[]
	for (k = 0; k < strlen(key); k++){
		int a = 0;
		for (int a=0; a < strlen(alphabet); a++)
			if (key[k] == alphabet[a])
				kIndeces[k] = a;
	}

	int i;
	int offset;
	for (i = 0; i < strlen(message); i++){
		offset = i % strlen(key);
		int enc = (mIndeces[i] - kIndeces[offset]);// Subtract the two letters and mod the result
		if (enc < 0)
			enc = 27 + enc;
		message[i] = alphabet[enc];	// Store the encrypted letter back in message[]
	}
}

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		perror("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections
	sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect

	while (1) {
		// Accept a connection, blocking if one is not available until one connects
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0)
			perror("ERROR on accept");

		if (establishedConnectionFD){
			// Can't declare inside a switch statement
			uint32_t msglen;
			uint32_t keylen;
			char* key;
			char* message;
			int count;
			char serverType[16] = "Decrypt";			

			pid_t newPid = fork();

			if (newPid < 0){
				perror("Hull Breach!");
				exit(1);
			}

			else if (newPid == 0){
				
				charsRead = send(establishedConnectionFD, serverType, 16, 0);
				if (charsRead < 0) 
					error("ERROR writing to socket");

				// Get the length of the message from the client
				charsRead = recv(establishedConnectionFD, &msglen, sizeof(uint32_t), 0);
				if (charsRead < 0)
					error("ERROR reading from socket");
				msglen = ntohl(msglen);	// Unnecessary here; fixing network order between machines

				// Allocate memory and fill with null bytes
				message = malloc(msglen);
				memset(message, '\0', msglen);

				// Loop to avoid network interruptions causing a truncated message
				count = 0;
				while (count < msglen){
					charsRead = recv(establishedConnectionFD, message + count, msglen, 0);
					if (charsRead < 0)
						perror("ERROR reading from socket");
					count += charsRead;	
				}
				// printf("SERVER: Received message %s\n", message);

				// Get the length of the key from the client
				charsRead = recv(establishedConnectionFD, &keylen, sizeof(uint32_t), 0);
				keylen = ntohl(keylen);	// Unnecessary here; fixing network order between machines

				// Allocate memory and fill with null bytes
				key = malloc(keylen);
				memset(key, '\0', keylen);

				// Loop to avoid network interruptions causing a truncated message
				count = 0;
				while (count < keylen){
					charsRead = recv(establishedConnectionFD, key, keylen, 0);
					if (charsRead < 0)
						perror("ERROR reading from socket");
					count += charsRead;
				}
				// printf("SERVER: Received key %s\n", key);

				// Do the actual encryption
				decrypt(message, key);
				// printf("SERVER: I have decrypted the message and the result is \"%s\"\n", message);

				// Send a Success message back to the client
				charsRead = send(establishedConnectionFD, message, strlen(message), 0); // Send success back
				if (charsRead < 0) error("ERROR writing to socket");
				close(establishedConnectionFD); // Close the existing socket which is connected to the client
				close(listenSocketFD); // Close the listening socket
				
				// Free allocated memory and exit
				free(message);
				free(key);
				exit(0);

				exit(0);
			}
			else
				continue;
		}

		else
			continue;
	}

	return 0;

}
