// builtins.c
//
// Phi Luu
//
// Oregon State University
// CS_344_001_W2019 Operating Systems 1
// Program 3: Smallsh
//
// This module contains constants, parameters, and functions related to the
// built-in commands of the shell. Commands that are not in this module will
// be executed via the  exec()  family.

#include "builtins.h"
#include "utility.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Kills all background child processes, flushes stdout, and then terminates
// the shell.
//
// Argument:
//   bg_children  a dynamic array of PIDs containing the PIDs of bg children
//   exit_code    the exit code specified for the final  exit()  call
void Exit(DynPidArr* bg_children, int exit_code) {
    assert(bg_children);

    // kill all background child processes
    pid_t* childpid;
    while ((childpid = PopBackDynPidArr(bg_children)))
        kill(*childpid, SIGKILL);

    // clean up PID container
    DeleteDynPidArr(bg_children);

    exit(exit_code);
}

// Changes the current working directory to a directory specified by  path .
void Cd(char* path) {
    assert(path);

    // try going to the directory & handle errors if any
    if (chdir(path) == -1)
        fprintf(stderr, "%s: no such file or directory\n", path);
}

// Displays the exit code (or signal termination code) of previously terminated
// process.
void Status(int child_exit_status) {
    if (WIFEXITED(child_exit_status)) {
        // if terminated normally, get the exit status via macro
        printf("exit value %d\n", (int)WEXITSTATUS(child_exit_status));
        fflush(stdout);
    } else if (WIFSIGNALED(child_exit_status)) {
        // if terminated by a signal, get the exit signal via macro
        printf("terminated by signal %d\n", (int)WTERMSIG(child_exit_status));
        fflush(stdout);
    }
}
