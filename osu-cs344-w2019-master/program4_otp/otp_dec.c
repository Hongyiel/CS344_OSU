// otp_dec.c
//
// Phi Luu
//
// Oregon State University
// CS_344_001_W2019 Operating Systems 1
// Program 4: OTP
//
// This program connects to  otp_dec_d  and asks it to decrypt ciphertext
// using a passed-in ciphertext and key, and otherwise performs exactly like
// otp_enc , and is runnable in the same three ways. This program is not able to
// connect to  otp_enc_d , even if it tries to connect on the correct port.
//
// Syntax--three ways to run:
//
//   ./otp_dec ciphertext key port
//   ./otp_dec ciphertext key port > plaintext
//   ./otp_dec ciphertext key port > plaintext &
//
// where
//   ciphertext  file containing the ciphertext to decrypt
//   key         file containing the encryption key to use to decrypt the text
//   port        port this program attempts to connect to  otp_dec_d  on
//   plaintext   file to output the decrypted text to
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

#define HOSTNAME (char*)"localhost"
#define MAX_BUFFER_SIZE (size_t)100000
#define ENCRYPTOR (unsigned char)0
#define DECRYPTOR (unsigned char)1

static char* prog;  // executable's name, for convenience

size_t ValidateAndReadFile(const char* fname, char* buffer, size_t size);
void ReadFromServer(int socket_fd, void* buffer, size_t len, int flags);
void WriteToServer(int socket_fd, void* buffer, size_t len, int flags);
int ToPositiveInt(const char* str);

int main(int argc, char** argv) {
    prog = argv[0];

    // ensure correct usage of command line arguments
    if (argc != 4) {
        fprintf(stderr, "Usage: %s ciphertext key port\n", prog);
        return 1;
    }

    // ensure the  port  argument is a positive integer
    int port = ToPositiveInt(argv[3]);
    if (!port) {
        fprintf(stderr, "%s:  port  must be a positive integer\n", prog);
        return 1;
    }

    // open, validate, and read from ciphertext file
    char ciphertext[MAX_BUFFER_SIZE];
    size_t ciphertext_len = ValidateAndReadFile(argv[1], ciphertext, MAX_BUFFER_SIZE);

    // open, validate, and read from key file
    char key[MAX_BUFFER_SIZE];
    size_t key_len = ValidateAndReadFile(argv[2], key, MAX_BUFFER_SIZE);

    // if key is shorter than ciphertext, terminate with code 1
    if (key_len < ciphertext_len) {
        fprintf(stderr, "%s: Key \"%s\" is too short\n", prog, argv[2]);
        return 1;
    }

    // set up the server address struct
    // convert machine name into a special form of address and ensure success
    struct hostent* server_host_info = gethostbyname(HOSTNAME);
    if (!server_host_info) {
        fprintf(stderr, "%s: gethostbyname() error: Could not get host %s\n",
                prog, HOSTNAME);
        return 1;
    }
    // initialize and clear out the address struct
    struct sockaddr_in server_address;
    memset((char*)&server_address, '\0', sizeof(server_address));
    server_address.sin_family = AF_INET;    // network-capable socket
    server_address.sin_port = htons(port);  // store port number
    // copy the special address to this address struct
    memcpy((char*)&server_address.sin_addr.s_addr,   // destination
           (char*)server_host_info->h_addr_list[0],  // source
           server_host_info->h_length);              // how much to copy

    // set up the socket and ensure successful setup
    // general-purpose socket, use TCP, normal behavior
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        fprintf(stderr, "%s: socket() error: Could not open socket\n", prog);
        return 1;
    }

    // connect to server and ensure successful connection
    if (connect(socket_fd,
                (struct sockaddr*)&server_address,
                sizeof(server_address)) < 0) {
        fprintf(stderr, "%s: connect() error: Could not connect to %s:%d\n",
                prog, HOSTNAME, port);
        close(socket_fd);
        return 1;
    }

    // specify the type of OTP (encryptor or decryptor) to server
    unsigned char otp_type = DECRYPTOR;
    WriteToServer(socket_fd, &otp_type, sizeof(otp_type), 0);

    // receive permission/rejection from server
    unsigned char can_connect_server = 0;
    ReadFromServer(socket_fd, &can_connect_server, sizeof(can_connect_server), 0);

    // print error messages and exit if server rejects connection
    if (!can_connect_server) {
        fprintf(stderr, "%s: Could not contact  %s_d  on port %d\n",
                prog, prog, port);
        close(socket_fd);
        return 2;
    }

    // send ciphertext to server
    // first tell server how much data is going to be sent
    WriteToServer(socket_fd, &ciphertext_len, sizeof(ciphertext_len), 0);
    // then send the ciphertext to server
    WriteToServer(socket_fd, ciphertext, ciphertext_len, 0);

    // send key to server
    // first tell server how much data is going to be sent
    WriteToServer(socket_fd, &key_len, sizeof(key_len), 0);
    // then send the key to server
    WriteToServer(socket_fd, key, key_len, 0);

    // receive plaintext from server
    // first read from server how much data is going to be sent
    size_t plaintext_len = 0;
    ReadFromServer(socket_fd, &plaintext_len, sizeof(plaintext_len), 0);
    // then read plaintext from server
    char plaintext[plaintext_len + 1];  // +1 for \0
    memset(plaintext, '\0', sizeof(plaintext));
    ReadFromServer(socket_fd, plaintext, plaintext_len, 0);

    // print the plaintext to stdout
    printf("%s\n", plaintext);

    // clean up
    close(socket_fd);

    return 0;
}

// Validates and reads the file with the provided file name.
//
// Argument:
//   fname   name of the file to be opened and read from
//   buffer  buffer containing text read from  file
//   size    the maximum size of the  buffer  array
//
// On success, this function returns size of  buffer  back to  main()  and lets
// main()  continue the program.
//
// On either of these error cases:
// - If the file named  fname  could not be opened
// - If  buffer  read from file is empty
// - If  buffer  read from file contains a bad character (neither an alphabet
//       nor a space)
// The program will print according error messages and exit with code 1.
//
// Notes: On all cases, file will be closed after it has been opened and read.
size_t ValidateAndReadFile(const char* fname, char* buffer, size_t size) {
    assert(fname && buffer && size > 0);

    FILE* file = fopen(fname, "r");

    // check non-opened files
    if (!file) {
        fprintf(stderr, "%s: Could not open file \"%s\"\n", prog, fname);
        exit(1);
    }

    // read until the end-of-line character or until running out of buffer
    memset(buffer, '\0', size);     // clear out buffer
    fgets(buffer, size - 1, file);  // size - 1 due to \0 termination
    buffer[strcspn(buffer, "\n")] = '\0';  // remove trailing newlines

    // check for empty buffer
    size_t len = strlen(buffer);
    if (len == 0) {
        fprintf(stderr, "%s: Empty buffer was read from \"%s\"\n", prog, fname);
        fclose(file);
        exit(1);
    }

    // check for bad characters
    for (size_t i = 0; i < len; i++) {
        if (!isupper(buffer[i]) && buffer[i] != ' ') {
            fprintf(stderr, "%s: File \"%s\" contains bad characters\n",
                    prog, fname);
            fclose(file);
            exit(1);
        }
    }

    // if passed all checks, reading from  file  was successful
    fclose(file);
    return len;
}

// Reads data from server to a buffer with the provided socket and flags.
//
// Arguments:
//   socket_fd  file descriptor of the socket
//   buffer     buffer to write the read data from the server into
//   len        length of the buffer
//   flags      flags used in the  recv()  function, use 0 for normal behavior
//
// If the data cannot be read from the server, the function will print error
// messages accordingly and then terminate the program with code 1.
void ReadFromServer(int socket_fd, void* buffer, size_t len, int flags) {
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

// Writes data from a buffer to server with the provided socket and flags.
//
// Arguments:
//   socket_fd  file descriptor of the socket
//   buffer     buffer containing the data to be written to server
//   len        length of the buffer
//   flags      flags used in the  send()  function, use 0 for normal behavior
//
// If the data cannot be written to the server, the function will print error
// messages accordingly and then terminate the program with code 1.
void WriteToServer(int socket_fd, void* buffer, size_t len, int flags) {
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
