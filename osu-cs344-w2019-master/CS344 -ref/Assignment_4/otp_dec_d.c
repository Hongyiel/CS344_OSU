/*Name: Meghan Dougherty
 * Date: 6/5/2019
 * Description: The server side code for the decryption aspect of the OTP 
 * program. When the program is run, it starts listening for connections
 * and can sustain 5 concurrent connections. If a connection is made, the 
 * program accepts a cipher file and a key and uses these items to produce
 * a decoded plain text which is sent back to the client.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "helper.h"


int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsWritten, charsSent, status;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;
	pid_t pid;

	if (argc < 2) { fprintf(stderr, "USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	//Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");

	//Infinite server loop	
	while(1)
	{
		//Flip the socket on for five connections
		listen(listenSocketFD, 5); 	
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");

		//create children
		pid = fork();
		switch (pid){

			//Case for a fork error
			case -1:
			{
				error("Hull Breach! Couldn't fork!\n");
			}
			//Child instructions
			case 0:
			{
				char buffer[1024];
				char* encryptedMessage[100000];
				char message[100000];
				char key[100000];
				

				//clear the buffer
				memset(buffer, '\0', sizeof(buffer));

				charsWritten = 0;

				//read the client's message
				while(charsWritten == 0)
					charsWritten = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
				//print error if there was a problem communicating
				if (charsWritten < 0) error("ERROR reading from socket");
				
				//make sure we are connected to the correct server
				if(strcmp(buffer, "otp_dec") != 0)
				{
					charsWritten = send(establishedConnectionFD, "no", 2, 0);
					exit(2);
				}
				else
				{
					//clear buffer
					memset(buffer, '\0', sizeof(buffer));

					//send "yes" response
					charsWritten = send(establishedConnectionFD, "yes", 3, 0);
					charsWritten = 0;

					//recieve the file length
					while(charsWritten == 0)
						charsWritten = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
					
					//convert the length to an integer		
					int size = atoi(buffer);
					
					charsWritten = 0;
					charsSent = 0;
					
					//recieve the cipher text from the client. 
					//I initially copied otp_enc_d.c directly to otp_dec_d.c but for some reason
					//it was causing some funky results with plaintext4. This method works. 
					while(charsWritten < size)
					{
						//recieve the message
						charsSent += recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
						charsWritten += charsSent;
						charsSent = 0;

						//add the data to the message string
						strcat(message, buffer);
				
					}
					//clear the buffer
					memset(buffer, '\0', sizeof(buffer));
					charsWritten = 0;
					charsSent = 0;

					//follow the same steps as above, but for the key
					//this time
					while(charsWritten < size){
										
						charsSent = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
						charsWritten += charsSent;
						charsSent = 0;
						strcat(key, buffer);
										
					}
					
				
					//clear the buffer
					memset(buffer, '\0', sizeof(buffer));

					//decrypt
					decode(message, key);
					
					//send the decoded plaintext back to the client
					charsWritten = 0;
					while(charsWritten < size)
						charsWritten += send(establishedConnectionFD, message, sizeof(message), 0);
					
					exit(0);

				}
			}
			//parent instructions
			default:
			{
				pid_t actualpid = waitpid(pid, &status, WNOHANG);
			}
		}
		//close the client socket
		close(establishedConnectionFD);
	}
	//close the listen socket
	close(listenSocketFD); 
	return 0;
}

