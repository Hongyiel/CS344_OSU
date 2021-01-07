#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>

//keygen file should give number of alphabets which user's input (ex. keygen 10 --> which mean is that user return 10 letters)

static const char* GET_CHAR = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
// int get_char(const char* chr);

int main(int argc, char* argv[]) {

  if(argc != 2){
    fprintf(stderr,"ERROR INPUT :: %s\n",argv[0]);
    return 1;  // error message --> strerr
  }
  //give error message if input is not 2 words;
  // int get_num = out_char(argv[1]); // give number of letters to out_char
  // int keylength = get_char(argv[1]);
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
// compare err
// return argv[1] to digit number
//ERROR Handling
// int get_char(const char* chr){
//   assert(chr);
//   int i, len;
//   for (i = 0, len = strlen(chr); i < len; i++){
//     if (!isdigit(chr[i])){
//       return 0;
//     }
//   }
//   return atoi(chr);
// }
