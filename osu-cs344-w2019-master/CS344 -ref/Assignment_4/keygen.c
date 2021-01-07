/*Name: Meghan Dougherty
 * Date 6/7/2019
 * Description: The key generator algorithm for the OTP program. The algorithm
 * generates a key by reading in the length of a plaintext the user wishes
 * to encrypt. The algorithm produces a file of the same same length where
 * every character is randomly selected from an array of the upper case
 * alphabet and the space character.
 */ 


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]) {
	
	//make sure the user supplied a length. 
	if(argc < 2)
	{
		fprintf(stderr, "too few arguments");
		exit(1);
	}

	//convert the length to an integer.
	int length = atoi(argv[1]);
	char  key[length+1];

	int i=0;

	//array of upper case alpha plus the space character
	char* arr ="ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

	//seed the random number generator.
	srand(time(NULL));

	//fill the array with random capital letters or ' '
	while(i<length)
	{
		//generate a random numer
		int x = random() % 27;
		//use that number as an index to select a character
		//from the array
		key[i] = arr[x];
		++i;
	}
	//add a null terminator to the end of the file.
	key[length] = '\0'; 

	//print the key with an added newline character.
	printf("%s\n", key); 
	return 0;
}
