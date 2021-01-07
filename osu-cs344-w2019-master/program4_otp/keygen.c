// keygen.c
//
// Phi Luu
//
// Oregon State University
// CS_344_001_W2019 Operating Systems 1
// Program 4: OTP
//
// This program creates a key file of specified length. The characters in the
// file generated are any of the 27 allowed characters, generated using the
// standard UNIX randomization methods. Note that the randomization is very
// simple and is not secure. The last character keygen outputs is a newline.
//
// Syntax:
//
//   ./keygen keylength
//
// where
//   keylength  length of the key file in characters
//
// Outputs:
//   All output text is output to  stdout , if any.
//   All error text is output to  stderr , if any.
//
// Note: 27 allowed characters are 26 capital letters and the space character.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#define CHAR_RANGE (int)27  // 26 capital alphabet letters + 1 space

static const char* CHAR_POOL = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

int ToPositiveInt(const char* str);

int main(int argc, char** argv) {
    // ensure correct usage of command line arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s keylength\n", argv[0]);
        return 1;
    }

    // try convert 1st arg from string to int
    // atoi()  returns 0 on non-numeric string, but in this case  keylength
    // must be positive. So, if the return value is not positive, exit
    int keylength = ToPositiveInt(argv[1]);
    if (!keylength) {
        fprintf(stderr, "%s:  keylength  must be a positive integer\n", argv[0]);
        return 1;
    }

    // seed the random number generator
    srand(time(0));

    // create a random string character by character
    for (int i = 0; i < keylength; i++) {
        printf("%c", CHAR_POOL[rand() % CHAR_RANGE]);
    }

    // end the key with a newline
    printf("\n");

    return 0;
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
