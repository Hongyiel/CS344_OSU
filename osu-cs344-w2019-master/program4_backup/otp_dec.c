#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <ctype.h>
#include <unistd.h>

#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

void Client_send_recv(int get_ConnectionFD,int get_recv, size_t len_of, char* input);
size_t Errorhandler_arr(const char* file_name, char* buffer, size_t len);
int Positive_int(const char* str);

// error msg
int main(int argc, char* argv[]) {

  if(argc < 4){
    fprintf(stderr, "ERROR: USAGE input argv is error :: %s >> is not correct input\n", argv[0]);
    return 1;
  }
  //get ensure port numbers
  int port_num = Positive_int(argv[3]);
  if(!port_num){
    fprintf(stderr, "ERROR: Connecting ERROR ::: %s\n", argv[0] );
    return 1;
  }
  // open for read alttext file
  // making readonly file
  char message[100000];
  char key[100000];
  // Error handling ::
  // len is 0 than exit
  // wrong input than exit;
  // close file_key and file_message

  size_t msg_len = Errorhandler_arr(argv[1],message,100000);
  size_t key_len = Errorhandler_arr(argv[2],key,100000);

  // if key is shorter than plaintext will terminate
  if(key_len < msg_len){
    fprintf(stderr, "ERROR: %s Key \"%s\" is too short\n",argv[0],argv[2]);
    exit(1);
  }

  //setting socket parameter
  struct sockaddr_in serverAddress;
  struct hostent* serverHost_info = gethostbyname("localhost");

  if(!serverHost_info){
    fprintf(stderr, "ERROR: Cannot get host %s ///  localhost : gethostbyname error\n", argv[0]);
    return 1;
  }
  // setting server address
  memset((char*)&serverAddress, '\0', sizeof(serverAddress));
  serverAddress.sin_family = AF_INET; // network socket which capable
  serverAddress.sin_port = htons(port_num); // storing port number
  //copy address
  memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHost_info->h_addr_list[0], serverHost_info->h_length);
  // destination , source , how much will copy

  //setting the socket
  int get_ConnectionFD = socket(AF_INET, SOCK_STREAM, 0);
  if(get_ConnectionFD < 0 ){
    fprintf(stderr, "ERROR: %s: get_ConnectionFD has been error :: cannot open \n", argv[0] );
    return 1;
  }
  // Connecting server

  if(connect(get_ConnectionFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    fprintf(stderr, "ERROR: connection error : connot connect %s\n", argv[0] );
    return 1;
  }

  // Start Recieve server type compaing...
  char* otp_client_type; // server type
  int get_recv;

  get_recv = recv(get_ConnectionFD, otp_client_type, sizeof(otp_client_type), 0);

  if(get_recv < 0){
    perror("ERROR: reading from socket");
  }

  if(strcmp(otp_client_type, "Encrypt") == 0){
    fprintf(stderr, "ERROR: server type was wrong \n");
    exit(1);
  }
  // comparing sending recving Client
  Client_send_recv(get_ConnectionFD, get_recv, msg_len, message );
  Client_send_recv(get_ConnectionFD, get_recv, key_len, key);


  // reading string last char
  int read_str;
  memset(message, '\0', strlen(message));
  read_str = recv(get_ConnectionFD, message, msg_len, 0);
  if(read_str < 0){
    fprintf(stderr, "ERROR: client reading from socket is error \n");
    return 1;
  }
  // get result
  printf("%s\n", message);
  // free all memory before end of the program
  // free(message);
  // free(key);
  close(get_ConnectionFD);

  return 0;
}
void Client_send_recv(int get_ConnectionFD,int get_recv, size_t len_of, char* input){

  unsigned int len_of_km = htonl(len_of);
  get_recv = send(get_ConnectionFD, &len_of_km, sizeof(len_of_km), 0);

  if(get_recv < 0){
    fprintf(stderr, "ERROR: sending clint message length has been error\n");
  }

  get_recv = send(get_ConnectionFD, input, len_of, 0 );
  if(get_recv < 0){
    fprintf(stderr, "ERROR: get_recv is under 0\n" );
  }
  if(get_recv < strlen(input)){
    fprintf(stderr, "ERROR: sockets data is not wrriten all from client \n" );
  }


}
// ERROR handling after get file
size_t Errorhandler_arr(const char* file_name, char* buffer, size_t size){

  FILE* file_open = fopen(file_name, "r"); // this is for text open
  // FILE* file_key = fopen(argv[2], "r"); // this is for key open
  // opening both will be get error because of confilct files

  if(!file_open){
    fprintf(stderr, "ERROR: file message connot open \n");
    exit(1);
  }
  //message setting each file input

  memset(buffer,'\0',100000);
  fgets(buffer, 100000 - 1, file_open);
  buffer[strcspn(buffer, "\n")] = '\0';
  // Reference https://www.tutorialspoint.com/c_standard_library/c_function_strcspn.htm

  size_t len = strlen(buffer);
  // if length is 0 it will be error
  if(len == 0){
    fprintf(stderr, "ERROR: len is 0 \n");
    exit(1);
  }

  // if bad characters in the files it will be error
  int i;
  for(i=0; i<len; i++){
    if(!isupper(buffer[i]) && buffer[i] != ' '){
      fprintf(stderr, "ERROR: file \"%s\" has bad characters \n", file_name  );
      exit(1);
    }
  }
  // end error handler
  fclose(file_open);
  return len;

}

// Comparing input arr is possitive --
// string will convert positive type
int Positive_int(const char* str){
  int i;
  int str_len = strlen(str);
  for(i=0;i<str_len;i++){
    if(!isdigit(str[i]))
    return 0;
  }
  // compaing...
  return atoi(str);
  // retrun str port number in int type
}
