# osu-cs344-w2019

This repository contains my programs in the course CS 344 Operating Systems 1, offered by Oregon State University in Winter 2019.

## Program 1 - Matrix

This program is a matrix calculator written in Bash script. The program can perform the following matrix operations:

- `dims [MATRIX]` prints the dimensions of a matrix in the `rows columns` format. The `MATRIX` argument is the name of the file that contains the matrix and is optional. If the `MATRIX` argument is not provided, the program will use `stdin`.
- `transpose [MATRIX]` transposes a matrix, that is, printing a matrix reflected along the main diagonal. The `MATRIX` argument works the same way as in the `dims` operation.
- `mean [MATRIX]` takes in an M-by-N matrix and prints a 1-by-N row vector whose each element is the mean of the matrix's columns containing that element. The `MATRIX` argument works the same way as in the `dims` operation.
- `add MATRIX_LEFT MATRIX_RIGHT` takes in two M-by-N matrices and prints an M-by-N matrix formed by the addition of two input matrices. The `MATRIX_LEFT` and `MATRIX_RIGHT` arguments are the names of the files that contain the matrices and are required.
- `multiply MATRIX_LEFT MATRIX_RIGHT` takes in an M-by-N matrix and an N-by-K matrix and prints an M-by-K matrix formed by the multiplication of the two input matrices. The `MATRIX_LEFT` and `MATRIX_RIGHT` arguments work the same way as in the `add` operation.

Note that this program assumes that all matrices are valid.

## Program 2 - Adventure

This program generates an arbitrary network of rooms and lets the user play the adventure game on that room network. This program is written in C and utilizes concurrency and mutex. To play the game, the user must follow these steps:

1. Compile the two C programs by running `make`.
2. Generate an arbitrary network of rooms by running `./luuph.buildrooms`. This command will generate a directory named `luuph.rooms.PROCESS_ID`, where `PROCESS_ID` is the PID of the `luuph.buildrooms` program. So, every time the player executes `./luuph.buildrooms`, a different network of rooms will be generated.
3. Run `./luuph.adventure` to play the game.

## Program 3 - Smallsh

This program is a small shell written in C that supports 3 built-in commands: `exit`, `status`, and `cd`. Other commands will be executed via the `exec()` family. The shell can also handle `SIGINT` and `SIGTSTP` signals. To compile and run this program, see its [README](program3_smallsh/README).

## Program 4 - OTP

This program contains 5 small programs that encrypt and decrypt information using a one-time pad-like system:

- `otp_enc` acts as a client that provides the server a plaintext file and a key file and requests the server to encrypt the plaintext using the key.
- `otp_enc_d` acts as a daemon of the encryption process. This is where the actual encryption happens. The server sends the encrypted text (aka ciphertext) back to the client, i.e. `otp_enc`.
- `otp_dec` is similar to `otp_enc`, but this time it requests the server to decrypt a ciphertext.
- `otp_dec_d` is similar to `otp_enc_d`, but this time the server decrypts a ciphertext and sends the plaintext back to the client.
- `keygen` is a standalone program that generates a random key for the encryption and decryption processes.

To use this program, follow these steps:

1. Compile all 5 programs by running `./compileall`
2. Run the encryption daemon by executing `./otp_enc_d PORT &`, where `PORT` is a number between 55000 and 65000.
3. Run the decryption daemon by executing `./otp_dec_d PORT &`, where `PORT` is a number between 55000 and 65000 (must be different from the port that `otp_enc_d` uses).
4. The two daemons will then be run in the background. Next, generate a key by running `./keygen LENGTH key_file`, where `LENGTH` is the length of the key and `key_file` is the file to output the resulting key to.
5. Try encrypting the information in the `plaintext1` file by running `./otp_enc plaintext1 key_file PORT > ciphertext1`, where `PORT` is the port number assigned to `otp_enc_d` in step 2. The encrypted text will be output to a file named `ciphertext1`. Note that the ciphertext is nothing like the original text (which is good!).
6. Try decrypting the information in the `ciphertext1` file by running `./otp_dec ciphertext1 key_file PORT > plaintext1_a`, where `PORT` is the port number assigned to `otp_dec_d` in step 3. The decrypted text will be output to a file named `plaintext1_a`. Note that the plaintext is now the same as the original text in the `plaintext1` file.
7. The OTP will produce a different ciphertext and plaintext if the key file is changed. Try it!

## Program Py

This program serves as an introduction to programming in Python. This program focuses on files, strings, and integer multiplication.

To run the program, execute `python3 mypython.py`.
