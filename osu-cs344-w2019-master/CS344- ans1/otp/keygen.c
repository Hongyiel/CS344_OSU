/*********************************************
Assignment 4: OTP
Christopher Rico
CS344
OSU ID: 933239746
keygen.c: this program generates the key
that will be used for encryption/decryption
**********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc, char *argv[])
{
  srand(time(NULL));

  int random;
  int keylength;
  int i;

  if(argc < 2)
  {
    fprintf(stderr, "ERROR: incorrect number of arguments.");
    exit(0);
  }

  //the key length is the second argument
  keylength = atoi(argv[1]);

  for(i = 0; i < keylength; i++)
  {
    //generate a random character A-Z plus space
    //capital characters
    random = 65 + (rand() % 27);
    //space
    if(random == 91)
    {
      random = 32;
    }

    //write the random character to stdout
    fprintf(stdout, "%c", random);
  }

  //newline at the end of the file
  fprintf(stdout, "\n");

  return 0;
}
