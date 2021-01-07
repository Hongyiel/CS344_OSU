// parser.c
//
// Phi Luu
//
// Oregon State University
// CS_344_001_W2019 Operating Systems 1
// Program 3: Smallsh
//
// This module contains constants, parameters, and functions related to
// prompting user's input commands and parsing them into smallsh.

#include "parser.h"
#include "builtins.h"
#include "utility.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

// Displays the prompt and prompts the user for a smallsh command.
//
// Assumes a command is formatted as
//
//   command [arg1 arg2 ...] [< input_file] [> output_file] [&]
//
// where items in square brackets are optional.
//
// Assumes commands are made up of words separated by spaces. The special
// symbols-- < ,  > , and  & --are recognized, but they must be surrounded by
// spaces like other words.
//
// Assumes the syntax of the command is valid.
//
// Returns:
//   A new dynamically allocated C string containing the whole command line
char* PromptUser(void) {
    // dynamically allocate memory for command line and ensure it succeeds
    char* cmdline = malloc(MAX_CMDLINE_LEN * sizeof(*cmdline));
    assert(cmdline);  // make sure allocation was successful

    // get raw input
    printf(": ");
    fflush(stdout);
    fflush(stderr);
    fgets(cmdline, MAX_CMDLINE_LEN, stdin);

    // remove trailing newline(s)
    size_t cmdline_len = strlen(cmdline);
    while (cmdline_len > 0 && cmdline[cmdline_len - 1] == '\n')
        cmdline[--cmdline_len] = '\0';

    return cmdline;
}

// Splits the command line into words. Puts each of the words into a dynamic
// array of C strings.
//
// Argument:
//   cmdline  a pointer to the string that contains the command line
//
// Returns:
//   A new dynamically allocated dynamic array structure of strings containing
//   the split command line
DynStrArr* SplitCmdLineToWords(char* cmdline) {
    // ensure  cmdline  not NULL and not empty
    assert(cmdline && cmdline[0] != '\0');

    // a parsed command line will be split by space and put into an array
    DynStrArr* cmd_args = malloc(sizeof(*cmd_args));
    InitDynStrArr(cmd_args, INIT_CMDLINE_ARGS);

    // create a temporary copy of the command line used for splitting
    char cmdline_tmp[MAX_CMDLINE_LEN];
    strcpy(cmdline_tmp, cmdline);

    // divide the command line into tokens with " " being the delimiter
    char* raw_cmdline_tok = strtok(cmdline_tmp, " ");  // 1st word

    // split the command line until it cannot be split anymore
    while (raw_cmdline_tok != NULL) {
        char cmdline_tok[MAX_CMDLINE_LEN];
        memset(cmdline_tok, '\0', MAX_CMDLINE_LEN);

        // evaluate  $$  if there is one to current PID
        int raw_len = strlen(raw_cmdline_tok), len = 0;
        if (raw_len == 1) {
            strcat(cmdline_tok, raw_cmdline_tok);
        } else {
            for (int i = 0; i < raw_len - 1; ) {
                if (raw_cmdline_tok[i] == '$'
                        && raw_cmdline_tok[i + 1] == '$') {
                    // convert PID to string
                    char pid_str[12];
                    IntToString((int)getpid(), pid_str);
                    // concat the new string to the processed token
                    strcat(cmdline_tok, pid_str);
                    len = strlen(cmdline_tok);
                    // skip 1 index
                    i += 2;
                } else {
                    // concat char to the processed token normally
                    cmdline_tok[len++] = raw_cmdline_tok[i];
                    // don't miss the final character which isn't iterated over
                    if (i == raw_len - 2)
                        cmdline_tok[len++] = raw_cmdline_tok[raw_len - 1];
                    // increment normally
                    i++;
                }
            }
        }

        // push to results
        PushBackDynStrArr(cmd_args, cmdline_tok);
        raw_cmdline_tok = strtok(NULL, " ");
    }

    return cmd_args;
}

// Parses the command line based on the three special symbols:  < ,  > , and  &
// and outputs their word indices to corresponding parameters.
//
// Arguments:
//   cmdwords          a pointer to the string that contains the command line
//   execvp_argv       an array of C strings containing the command and args
//   execvp_argc       a pointer to the resulting size of  execvp_argv
//   stdin_redir_idx   a pointer to the word index of the  <  symbol
//   stdout_redir_idx  a pointer to the word index of the  >  symbol
//   is_bg             a pointer to a flag, whether the process is backgrounded
//
// The words in  cmdwords  will be parsed into  execvp_argv .
// The size of  execvp_argv  will be held by  execvp_argc .
// stdin_redir_idx ,  stdout_redir_idx , and  is_bg  will be modified after the
// function returns.
void ParseCmdWords(DynStrArr* cmdwords, char** execvp_argv, int* execvp_argc,
                   int* stdin_redir_idx, int* stdout_redir_idx, bool* is_bg) {
    assert(cmdwords && cmdwords->size >= 1 && execvp_argv && execvp_argc
           && stdin_redir_idx && stdout_redir_idx && is_bg);

    // init the output parameters
    *execvp_argc = 0;
    *stdin_redir_idx = -1;
    *stdout_redir_idx = -1;
    *is_bg = false;

    // find the  <  or  >  or trailing  &  symbols;  &  must appear at the end
    bool execvp_len_done = false;
    for (int i = 0; i < cmdwords->size; i++) {
        // if matches stdin redirection symbol, done with counting words
        if (strcmp(cmdwords->strings[i], "<") == 0) {
            *stdin_redir_idx = i;
            execvp_len_done = true;
        }
        // if matches stdout redirection symbol, done with counting words
        if (strcmp(cmdwords->strings[i], ">") == 0) {
            *stdout_redir_idx = i;
            execvp_len_done = true;
        }
        // if matches background symbol at the end (and only at the end),
        // done with counting words
        if (strcmp(cmdwords->strings[i], "&") == 0 && i == cmdwords->size - 1) {
            *is_bg = true;
            execvp_len_done = true;
        }
        // otherwise, increase the len of the array to be passed to  execvp()
        if (!execvp_len_done) (*execvp_argc)++;
    }

    (*execvp_argc)++;  // need 1 more slot to contain NULL
    for (int i = 0; i < *execvp_argc - 1; i++)
        execvp_argv[i] = cmdwords->strings[i];  // temp pointer, not hard copy
    execvp_argv[*execvp_argc - 1] = NULL;
}

