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
	int errno = 1;
	perror(msg);
	exit(1);
} // Error function give message that contain problem

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead, bytesread;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[1024];
	char ciphertext[100000];

	if (argc < 3) { fprintf(stderr, "USAGE: %s hostname port\n", argv[0]); exit(1); } // Check usage & args

	//Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	int yes = 1;
	setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); //make socket reusable

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){ // Connect socket to address
		error("CLIENT: ERROR connecting");
	}

	//get the two file lengths as longs.
	//used long instead of in because plaintext 4 could max out an int

	long filelength = fileLength(argv[1]);
	long keylength = fileLength(argv[2]);

	//check if the file is greater than the key.
	if(filelength > keylength)
		error("Key is too short!");

	//send handshake to server to make sure we are connected to enc (as opposed to dec).
	char* msg = "encrypt";
	charsWritten = send(socketFD, msg, strlen(msg), 0);

	//clear buffer
	memset(buffer, '\0', sizeof(buffer));

	charsWritten = 0;
	//get return handshake from server, leaving room for a null terminator
	while(charsWritten == 0)
		charsWritten = recv(socketFD, buffer, sizeof(buffer) - 1, 0);

	//make sure we're connected to a good port.
	if(strcmp(buffer, "err") == 0)
		error("Bad client");

	//clear buffer again.
	memset(buffer, '\0', sizeof(buffer));

	//insert the file length into the buffer
	sprintf(buffer, "%d", filelength);

	//Send the file length to the server
	charsWritten = send(socketFD, buffer, sizeof(buffer), 0);

	//clear buffer.
	memset(buffer, '\0', sizeof(buffer));

	//open the plaintext file as a file descriptor.
	int fd = open(argv[1], 'r');
	charsWritten = 0;

	//send the plaintext to the server, using a loop to ensure that it is completely sent.
	while(charsWritten <= filelength)
	{
		//read data from the file.
		bytesread = read(fd, buffer, sizeof(buffer)-1);
		//send the data to the server
		charsWritten += send(socketFD, buffer, strlen(buffer), 0);
	}
	//clear buffer
	memset(buffer, '\0', sizeof(buffer));

	//Repeat the same process as above, but now with the key.
	fd = open(argv[2], 'r');
	charsWritten = 0;
	while(charsWritten <= filelength)
	{
		bytesread = read(fd, buffer, sizeof(buffer)-1);
		charsWritten += send(socketFD, buffer, strlen(buffer), 0);

	}

	//clear the buffer.
	memset(buffer, '\0', sizeof(buffer));


	charsWritten = 0;

	//loop to ensure that all data is recieved
	while(charsWritten < filelength)
	{
		//get the message
		charsWritten += recv(socketFD, buffer, sizeof(buffer)-1, 0);
		//add the recieved data to the end of the ciphertext array
		strcat(ciphertext, buffer);
	}
	//add a newline to the end of the cipher
	strcat(ciphertext, "\n");

	//print the cipher to STDOUT
	printf("%s", ciphertext);

	close(socketFD); // Close the socket
	return 0;
}


/*getNumChars: takes a file name as an argument. Opens the file then counts the characters within the file.
 * Concurrently validates the file, unsuring that there are no characters beside Uppercase alpha and " ".
 * Returns the character count.*/
int fileLength(const char* filename){
	int character;
	int count = 0;
	//open the file for reading
	FILE* file = fopen(filename, "r");

	//get the first character of the file
	character = fgetc(file);

	//keep reading until we reach the end of the file, either a null terminator or an endline.
    	while (!(character == EOF || character == '\n'))
	{
		//if the character isn't valid, call the error function.
		if(!isupper(character) && character != ' ')
			error("File contains bad characters!");

		//get the next character
		character = fgetc(file);

		//increase the count.
        	++count;
    	}

	//close the file.
	fclose(file);
	//return the final count.
	return count;
}
