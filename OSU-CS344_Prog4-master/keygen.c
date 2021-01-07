

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int main (int argc, char* argv[]) {

	srand(time(NULL));

	// Set up our alphabet, include a space
	char alphabet[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

	int keysize = atoi(argv[1]);	// Read first command line argument as keysize
	char key[keysize];				// Can't initialize variable array, so be careful of garbage

	int i;
	for (i = 0; i < keysize; i++){
		int select = rand() % 27;
		key[i] = alphabet[select];
	}

	key[i] = '\0'; // Terminate with a null-byte so we don't output garbage

	printf("%s\n", key);
	return 0;

}