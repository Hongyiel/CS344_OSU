// otp_enc_d.c
//
// Phi Luu
//
// Oregon State University
// CS_344_001_W2019 Operating Systems 1
// Program 4: OTP
//
// This program runs in the background as a daemon. Upon execution,  otp_enc_d
// outputs an error if it cannot be run due to a network error, such as the
// ports being unavailable. Its function is to perform the actual encoding, as
// described in  https://en.wikipedia.org/wiki/One-time_pad . This program
// listens on a particular port/socket, assigned when it is first run.
//
// When a connection is made,  otp_enc_d  calls  accept()  to generate the
// socket used for actual communication, and then uses a separate process to
// handle the rest of the transaction, which will occur on the newly accepted
// socket.
//
// Syntax:
//
//   ./otp_enc_d listening_port
//   ./otp_enc_d listening_port &
//
// where
//   listening_port  port program should listen on
//
// This program is recommended to run in background, using the second syntax.
//
// Outputs:
//   All output text is output to  stdout , if any.
//   All error text is output to  stderr , if any.

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <assert.h>
#include <signal.h>

#define MAX_BUFFER_SIZE (size_t)100000
#define MAX_LISTENS (int)5
#define MAX_CHILDREN (int)5
#define ENCRYPTOR (unsigned char)0
#define DECRYPTOR (unsigned char)1
#define JUNK_VAL (pid_t)-7
#define CHAR_RANGE (int)27  // 26 capital alphabet letters + 1 space

static const char* CHAR_POOL = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

static char* prog;  // executable's name, for convenience
static int num_children;  // number of child processes

void EncryptOtp(char* ciphertext, const char* plaintext, const char* key);
void ReadFromClient(int socket_fd, void* buffer, size_t len, int flags);
void WriteToClient(int socket_fd, void* buffer, size_t len, int flags);
int ToPositiveInt(const char* str);

// Catches SIGCHLD (i.e. CHiLD process finished) and update the number of active
// child process.
void CatchSIGCHLD(int signo) { num_children--; }

int main(int argc, char** argv) {
    // register signal handlers
    struct sigaction SIGCHLD_action = {{0}};    // deal with SIGCHLD
    SIGCHLD_action.sa_handler = CatchSIGCHLD;   // handler when caught
    sigfillset(&SIGCHLD_action.sa_mask);        // block all signal types
    SIGCHLD_action.sa_flags = SA_RESTART;       // restart system calls
    sigaction(SIGCHLD, &SIGCHLD_action, NULL);  // register the struct

    // init global vars
    prog = argv[0];
    num_children = 0;

    // ensure correct usage of command line arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s listening_port\n", argv[0]);
        return 1;
    }

    // ensure  listening_port  argument is a positive integer
    int port = ToPositiveInt(argv[1]);
    if (!port) {
        fprintf(stderr, "%s:  listening_port  must be a positive integer\n", prog);
        return 1;
    }

    // set up the address struct for this process (i.e. the server)
    struct sockaddr_in server_address;
    memset((char*)&server_address, '\0', sizeof(server_address));
    server_address.sin_family = AF_INET;          // network-capable socket
    server_address.sin_port = htons(port);        // store port number
    server_address.sin_addr.s_addr = INADDR_ANY;  // accept any address

    // set up the socket and ensure successful setup
    // general-purpose socket, use TCP, normal behavior
    int listen_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket_fd < 0) {
        fprintf(stderr, "%s: socket() error: Could not open socket\n", prog);
        return 1;
    }

    // enable socket to begin listening
    if (bind(listen_socket_fd,
             (struct sockaddr*)&server_address,
             sizeof(server_address)) < 0) {
        fprintf(stderr, "%s: bind() error: Could not bind to port %d\n",
                prog, port);
        close(listen_socket_fd);
        return 1;
    }

    // flip the socket on and start listening
    listen(listen_socket_fd, MAX_LISTENS);

    // listen for client's connection forever
    while (1) {
        // accept a connection, blocking if one is not available until one connects
        struct sockaddr_in client_address;
        socklen_t client_info_size = sizeof(client_address);
        int connection_fd = accept(listen_socket_fd,
                                   (struct sockaddr*)&client_address,
                                   &client_info_size);
        if (connection_fd < 0) {
            fprintf(stderr, "%s: accept() error\n", prog);
            continue;
        }

        // receive the type of client's OTP (encryptor or decryptor)
        unsigned char client_otp_type = 0;
        ReadFromClient(connection_fd, &client_otp_type, sizeof(client_otp_type), 0);

        // check client's OTP type and determine their permission to connect
        unsigned char client_can_connect = (client_otp_type == ENCRYPTOR);
        // send client permission/rejection to connect
        WriteToClient(connection_fd, &client_can_connect, sizeof(client_can_connect), 0);

        // only start sending/receiving data to/from client if they can connect
        if (!client_can_connect) {
            close(connection_fd);
            continue;
        }

        // if maximum connections have been reached, don't create any child
        if (num_children == MAX_CHILDREN) {
            close(connection_fd);
            continue;
        }

        // otherwise, fork off a child
        pid_t spawnpid = JUNK_VAL;
        spawnpid = fork();
        if (spawnpid == -1) {
            fprintf(stderr, "%s: fork() error\n", prog);
            close(connection_fd);
            continue;
        } else if (spawnpid == 0) {  // child process does the heavy work
            num_children++;

            // receive plaintext from client
            // first read from client how much data is going to be sent
            size_t plaintext_len = 0;
            ReadFromClient(connection_fd, &plaintext_len, sizeof(plaintext_len), 0);
            // then read plaintext from client
            char plaintext[plaintext_len + 1];  // +1 for \0
            memset(plaintext, '\0', sizeof(plaintext));
            ReadFromClient(connection_fd, plaintext, plaintext_len, 0);

            // receive key from client
            // first read from client how much data is going to be sent
            size_t key_len = 0;
            ReadFromClient(connection_fd, &key_len, sizeof(key_len), 0);
            // then read key from client
            char key[key_len + 1];  // + 1 for \0
            memset(key, '\0', sizeof(key));
            ReadFromClient(connection_fd, key, key_len, 0);

            // initialize ciphertext string
            char ciphertext[plaintext_len + 1];  // +1 for \0
            memset(ciphertext, '\0', sizeof(ciphertext));
            EncryptOtp(ciphertext, plaintext, key);

            // first tell client how much data is going to be sent
            size_t ciphertext_len = strlen(ciphertext);
            WriteToClient(connection_fd, &ciphertext_len, sizeof(ciphertext_len), 0);
            // then send ciphertext to client
            WriteToClient(connection_fd, ciphertext, ciphertext_len, 0);
        }

        // close the existing socket which is connected to client
        close(connection_fd);
    }

    // close the listening socket
    close(listen_socket_fd);

    return 0;
}

// Creates a one-time pad by encrypting a given plaintext using a provided key
// using modular addition.
//
// Arguments:
//   ciphertext  output of this function, i.e. the encrypted text
//   plaintext   plaintext to be encrypted
//   key         key used in the encryption process
//
// ciphertext  will be modified and will contain the encrypted text.
void EncryptOtp(char* ciphertext, const char* plaintext, const char* key) {
    assert(ciphertext && plaintext && key);

    // get lengths of plaintext and key and ensure
    // key is at least as long as plaintext
    size_t plaintext_len = strlen(plaintext);
    size_t key_len = strlen(key);
    assert(key_len >= plaintext_len);

    // for each character in plaintext, convert it to ciphertext
    for (size_t i = 0; i < plaintext_len; i++) {
        // use modular addition to get the index of ciphertext
        // strchr() returns a  char*  to the first occurrence of the character,
        // subtract  CHAR_POOL  from that will give the index
        // since  CHAR_POOL  is an array of character
        int cipher_idx = strchr(CHAR_POOL, plaintext[i]) - CHAR_POOL;
        cipher_idx += strchr(CHAR_POOL, key[i]) - CHAR_POOL;
        cipher_idx %= CHAR_RANGE;

        ciphertext[i] = CHAR_POOL[cipher_idx];
    }
}

// Reads data from client to a buffer with the provided socket and flags.
//
// Arguments:
//   socket_fd  file descriptor of the socket
//   buffer     buffer to write the read data from the client into
//   len        length of the buffer
//   flags      flags used in the  recv()  function, use 0 for normal behavior
//
// If the data cannot be read from the client, the function will print error
// messages accordingly and then terminate the program with code 1.
void ReadFromClient(int socket_fd, void* buffer, size_t len, int flags) {
    assert(socket_fd >= 0 && buffer && len > 0 && flags >= 0);

    void* tmp_buffer = buffer;
    ssize_t total_chars_read = 0;

    // keep receiving until all of the data has been read
    while (total_chars_read < len) {
        ssize_t chars_read = recv(socket_fd, tmp_buffer,
                                  len - total_chars_read, flags);
        if (chars_read < 0) {
            fprintf(stderr, "%s: recv() error: Could not read from socket\n", prog);
            break;
        }

        total_chars_read += chars_read;

        if (total_chars_read < len)
            // move pointer to after the last read character
            tmp_buffer += chars_read;
    }
}

// Writes data from a buffer to client with the provided socket and flags.
//
// Arguments:
//   socket_fd  file descriptor of the socket
//   buffer     buffer containing the data to be written to client
//   len        length of the buffer
//   flags      flags used in the  send()  function, use 0 for normal behavior
//
// If the data cannot be written to the client, the function will print error
// messages accordingly and then terminate the program with code 1.
void WriteToClient(int socket_fd, void* buffer, size_t len, int flags) {
    assert(socket_fd >= 0 && buffer && len > 0 && flags >= 0);

    void* tmp_buffer = buffer;
    ssize_t total_chars_written = 0;

    // keep sending until all of the data has been sent
    while (total_chars_written < len) {
        ssize_t chars_written = send(socket_fd, tmp_buffer,
                                     len - total_chars_written, flags);
        if (chars_written < 0) {
            fprintf(stderr, "%s: send() error: Could not write to socket\n", prog);
            break;
        }

        total_chars_written += chars_written;

        if (total_chars_written < len)
            // move pointer to after the last written character
            tmp_buffer += chars_written;
    }
}

// Converts a string to a positive integer. If any character of the string is
// not a digit, return -1 as an error.
//
// Argument:
//   str  the string to be converted into a positive integer
//
// Returns:
//   0  if there exists a non-digit (not in 0-9) character in  str  string
//   The correspond positive integer on success
int ToPositiveInt(const char* str) {
    assert(str);

    for (size_t i = 0, len = strlen(str); i < len; i++)
        if (!isdigit(str[i])) return 0;

    return atoi(str);
}
