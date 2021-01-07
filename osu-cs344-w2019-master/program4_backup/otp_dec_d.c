#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>

static const char* GET_CHAR = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
static int number_child; // process count
// static const char* process;
void decrypt_Otp(char* Connection_Tx, const char* message, const char* key);
int Positive_int(const char* str);

void catch_signal(int signo){
  number_child--;
}

int main(int argc, char const *argv[]) {

  // line command is not available
  if(argc != 2){
    // give error if input is not 2
    fprintf(stderr, "ERROR: WRONG INPUT : %s is not right type \n", argv[0] );
    return 1;
  }
  // listening port num is not available
  int port_num = Positive_int(argv[1]); // get number of address

  if(!port_num){
    // give error if input is wrong
      fprintf(stderr,"ERROR INPUT :: %s is not possitive input :: %s\n", argv[0]);
      return 1;
  }

  struct sigaction get_signal_action = {{0}};    // action start
  get_signal_action.sa_handler = catch_signal;   // handler when put
  sigfillset(&get_signal_action.sa_mask);        // blocking all signal types
  get_signal_action.sa_flags = SA_RESTART;       // system calls
  sigaction(SIGCHLD, &get_signal_action, NULL);  // regist
  //reference from http://man7.org/linux/man-pages/man2/sigaction.2.html

  // number_child = 0; // init argv[0]




  struct sockaddr_in serverAddress;
  //setting sockaddr_in arrays




  //setting server address
  memset((char*)&serverAddress, '\0', sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;            // socket that network capable
  serverAddress.sin_port = htons(port_num);      // number of port
  serverAddress.sin_addr.s_addr = INADDR_ANY;    // accept address if any.
  //setting socket address and if succeed finished.
  int socket_listener = socket(AF_INET, SOCK_STREAM, 0);
  if(socket_listener < 0){
    fprintf(stderr, "ERROR: socket_listener cannot open %s\n", argv[0] );
    return 1;
    //compare error message
  }

  // if socket is enable, it will start to listen
  if(bind(socket_listener,(struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0 ){
    fprintf(stderr, "ERROR: IN bind():: socket_listener cannot reachable ::: PortNUM - %s\n",argv[0]);
    close(socket_listener);
    return 1;
  }
  // get the socket on (5 connection will accept)
  listen(socket_listener, 5);

  //start
  int get_ConnectionFD;
  int get_recv;

  while(1){
    struct sockaddr_in serverAddress, clientAddress;
    //coonection was accepted and blocking if other connection is not available
    socklen_t len_Of_Client = sizeof(clientAddress);
    get_ConnectionFD = accept(socket_listener,(struct sockaddr*)&clientAddress,&len_Of_Client);
    // Accept connection
    if(get_ConnectionFD < 0){
      fprintf(stderr, "ERROR: get_ConnectionFD is ERROR on Accept ::code:: %s\n", argv[0] );
      continue;
    }

    if(get_ConnectionFD){

      unsigned int message_len;
      unsigned int key_len;
      char* otp_client_type = "Decrypt";
      char* message;
      unsigned char otp_encryptor = 0;

      pid_t getPid = fork();
      // get ERROR MSG if there are error on fork()
      if(getPid < 0){
        fprintf(stderr, "ERROR: Fork() has been ERROR :: port num = %s\n", argv[0]);
        exit(1);
      }else if(getPid == 0){
        // number_child++; // get process number

        // it will give information of client server
        get_recv = send(get_ConnectionFD, otp_client_type, sizeof(otp_client_type), 0);
        if(get_recv < 0){
          fprintf(stderr, "ERROR: recv error :: %s\n", argv[0]);
          break;
        }

        // getting length of client server address
        get_recv = recv(get_ConnectionFD, &message_len, sizeof(message_len), 0);
        if(get_recv < 0){
          fprintf(stderr, "ERROR: recv error :: %s\n", argv[0] );
          break;
        }

        // reference :: https://linux.die.net/man/3/ntohl
        message_len = ntohl(message_len);

        // start to get message and setting memory for message
        message = malloc(message_len);
        memset(message,'\0',message_len);

        ssize_t total_chars_read = 0; // count total character read
        // avoid network interruptions
        while(total_chars_read < message_len){
          get_recv = recv(get_ConnectionFD, message + total_chars_read, message_len, 0);
          if(get_recv < 0){
            fprintf(stderr,"ERROR: Empty reading !!");
            break;
          }
          total_chars_read += get_recv;
        }

        get_recv = recv(get_ConnectionFD, &key_len, sizeof(key_len), 0);
        // key_len = ntohl(key_len);
        // setting memory about key
        char* key;

        key = malloc(key_len);
        memset(key, '\0', key_len);

        total_chars_read = 0;
        while(total_chars_read < key_len){
          get_recv = recv(get_ConnectionFD, key, key_len, 0);
          if(get_recv < 0 ){
              fprintf(stderr, "ERROR: Empty recv !! :: port = %s\n");
              break;
            }
          total_chars_read += get_recv;
        }

        //key = key
        //ciphertext = get_ConnectionFD
        //plaintext_len = message_len
        // allocated +1 because of '\0' in last char
        char Connection_Tx[message_len + 1];
        memset(Connection_Tx, '\0', sizeof(Connection_Tx));
        decrypt_Otp(Connection_Tx, message, key);
        // get encrypt OTP on temporary arrays
        // sending succeed message to client from system back
        get_recv = send(get_ConnectionFD, message, strlen(message), 0);
        if( get_recv < 0 ){
          fprintf(stderr, "ERROR: sending socket err\n");
          break;
        }

        // closing existing sockets and listening sockets
        close(get_ConnectionFD);
        close(socket_listener);
        // Free all memory
        free(message);
        free(key);

        exit(0);
      }else
      continue;


    }//end of if statement
    else
    continue;

  }
  return 0;
}
void decrypt_Otp(char* Connection_Tx, const char* message, const char* key){

  size_t message_len = strlen(message);
  size_t key_len = strlen(key);

  int i = 0;
  for(i = 0; i < message_len; i++){
    // comparing each char character
    int Plain_index = strchr(GET_CHAR, message[i]) - GET_CHAR;
    Plain_index -= strchr(GET_CHAR, key[i]) - GET_CHAR;
    if(Plain_index < 0){
      Plain_index += 27; // characters range ABCDEFGHIJKLMNOPQRSTUVWXYZ include ' ';
    }else{
      Plain_index %= 27; // this is char range that GET_CHAR's range
    }
    Connection_Tx[i] = GET_CHAR[Plain_index];

  }

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
