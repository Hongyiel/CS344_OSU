#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	if (argc < 4) { fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); exit(0); } // Check usage & args

	struct stat mstat, kstat;

	// Check for existence of files using stat()
	if (stat(argv[1], &mstat) == -1){
		perror("Invalid path or filename");
		exit(1);
	}

	// Open the message file for read-only, error out if necessary
	int messageFD = open(argv[1], O_RDONLY);
	if (messageFD == -1){
		perror("Unable to open message file");
		exit(1);
	}

	// Open the key file for read-only, error out if necessary
	int keyFD = open(argv[2], O_RDONLY);
	if (keyFD == -1){
		perror("Unable to open key file");
		exit(1);
	}

	// This code block adapted from https://stackoverflow.com/a/21074125
	size_t pos = lseek(messageFD, 0, SEEK_CUR);
	size_t mSize = lseek(messageFD, 0, SEEK_END);	// Find the position of EOF
	lseek(messageFD, pos, SEEK_SET);// Return to the beginning of the file

	// Allocate memory for our buffer according to what we found
	char* message = calloc(mSize, sizeof(char));
	read(messageFD, message, mSize);

	// Rinse and repeat for key
	// This code block adapted from https://stackoverflow.com/a/21074125
	pos = lseek(keyFD, 0, SEEK_CUR);
	size_t kSize = lseek(keyFD, 0, SEEK_END);	// Find the position of EOF
	lseek(keyFD, pos, SEEK_SET);// Return to the beginning of the file


	// Allocate memory for our buffer according to what we found
	char* key = calloc(kSize, sizeof(char));
	read(keyFD, key, kSize);

	// Throw an error and exit if the key is too small
	if (kSize < mSize){
		fprintf(stderr, "Key is too small. Exiting."); exit(1);
	}


	// Trim off newlines if they're received
	message = strtok(message, "\n");
	mSize = mSize - 1;
	key = strtok(key, "\n");
	kSize = kSize - 1;

	// Input validation -- ONLY capital letters or spaces
	int i;
	char c;
	for (i = 0; message[i] != '\0' ; i++){
		c = message[i];
		if (c != ' '){
			if (c < 'A' || c > 'Z'){
				fprintf(stderr, "Found invalid char %c, exiting!\n", c);
				exit(0);
			}
		}
	}

	for (i = 0; key[i] != '\0' ; i++){
		c = key[i];
		if (c != ' '){
			if (c < 'A' || c > 'Z'){
				fprintf(stderr, "Found invalid char %c, exiting!\n", c);
				exit(0);
			}
		}
	}


	// Set up our socket parameters
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3 ]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // We'll only ever use localhost here
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr_list[0], serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// Receive the server type, quit if it's not the right one
	char serverType[16];
	charsWritten = recv(socketFD, serverType, 16, 0); // Receive type of server
	if (charsWritten < 0)
		perror("Error reading from socket");

	if (strcmp(serverType, "Encrypt") == 0){
		fprintf(stderr, "Wrong server type; exiting!"); exit(1);
	}

	// Send message length to server
	uint32_t msglen = htonl(mSize);
	charsWritten = send(socketFD, &msglen, sizeof(uint32_t), 0);
	if (charsWritten < 0) 
		error("CLIENT: ERROR sending message length to socket");

	// Send message to server
	charsWritten = send(socketFD, message, mSize, 0); // Write message to the server
	if (charsWritten < 0) 
		error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(message)) // Deal with this
		fprintf(stderr, "CLIENT: WARNING: Not all data written to socket!\n");

	// Send key length to server
	uint32_t keylen = htonl(kSize);
	charsWritten = send(socketFD, &keylen, sizeof(uint32_t), 0);
	if (charsWritten < 0) 
		error("CLIENT: ERROR sending message length to socket");

	// Send key to server
	charsWritten = send(socketFD, key, kSize, 0); // Write key to the server
	if (charsWritten < 0)
		error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(key)) 
		printf("CLIENT: WARNING: Not all data written to socket!\n");

	// Get return message from server
	memset(message, '\0', strlen(message)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, message, mSize, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	
	// Send decrypted message to stdout
	printf("%s\n", message);

	free(message);
	free(key);
	close(messageFD);
	close(keyFD);
	close(socketFD); // Close the socket
	return 0;
}
