/*Name: Meghan Dougherty
 * Date: 6/5/2019
 *Description: Various helper functions for the OTP program, both enc and dec.
 *Includes: error, getNumChars, charInt, intChar, encode, and decode.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
void error(const char *msg) { errno = 1; perror(msg);exit(1); } // Error function used for reporting issues

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

/*encode: takes two char arrays, a mesage and a key. uses the key to encode the message
 * using the One Time Pad technique. Returns stores the encrypted message in the 
 * passed message array.
 */ 
void encode(char  plaintext[], char key[]){ //encrypt a given message
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

/*decode: takes two char arrays, a mesage and a key. uses the key to decode the message
 * using the One Time Pad technique. Returns stores the decrypted message in the 
 * passed message array.
 */ 
void decode(char cipher[], char key[])
{
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

