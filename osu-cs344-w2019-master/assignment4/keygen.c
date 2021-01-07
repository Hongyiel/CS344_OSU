/*
*	NAME : HONGYIEL SUH
*	STUDENT NUMBER: 933612189
* File Name : keygen.c
* This file generates letters with numbers that user input
* ex keygen 10 --> will give 10 random letters on the screen
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <ctype.h>

//keygen file should give number of alphabets which user's input (ex. keygen 10 --> which mean is that user return 10 letters)

static const char* GET_CHAR = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
// int get_char(const char* chr);

int main(int argc, char* argv[]) {

  if(argc != 2){
    fprintf(stderr,"ERROR INPUT :: %s\n",argv[0]);
    return 1;  // error message --> strerr
  }
  //give error message if input is not 2 words;
  int keylength = atoi(argv[1]);
  //compare keylength is not number
  // if keylength is not number (digit style)
  if(!keylength){
      fprintf(stderr,"ERROR INPUT :: argv[1] is not possitive input :: %s\n", argv[0]);
      return 1;
  }
  srand(time(NULL)); // random
  int i;
  for(i = 0; i<keylength ;i++){
    printf("%c", GET_CHAR[rand() % 27]);
    //get characters
  }
  printf("\n");
  return 0;
}
