// main.c
//
// Phi Luu
//
// Oregon State University
// CS_344_001_W2019 Operating Systems 1
// Program 3: Smallsh
//
// This is the main module containing the main smallsh process and signal
// handlers.
//
// Credits to Benjamin Brewster for awesome lecture :). References:
// - 03.01_Processes
// - 03.02_Process_Management_and_Zombies
// - 03.03_Signals
// - 03.04_More_UNIX_IO

#include "parser.h"
#include "builtins.h"
#include "utility.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define INIT_CHILDREN_CAP (int)8
#define JUNK_VAL (int)-7

static DynPidArr bg_children;  // array of PIDs of background children
static pid_t fg_childpid;      // PID of current foreground child
static pid_t bg_childpid;      // PID of the latest finished background child
static int fg_exit_status;     // exit status of the latest foreground child
static int bg_exit_status;     // exit status of the latest background child
static bool is_fg_only_mode;   // toggle for the foreground-only mode

// Catches SIGINT (i.e. INTerrupt i.e. Ctrl-C) and let the current foreground
// child process terminate itself instead of exiting from the shell.
void CatchSIGINT(int signo) {
    // only interrupt running foreground children
    if (fg_childpid != JUNK_VAL) {
        waitpid(fg_childpid, &fg_exit_status, 0);
    } else {
        // make prompt more pretty
        write(STDOUT_FILENO, "\n: ", 3);

        fflush(stdout);
    }
}

// Catches SIGTSTP (i.e. Terminal SToP i.e. Ctrl-Z) and toggle the
// foreground-only mode.
void CatchSIGTSTP(int signo) {
    // must wait for foreground process to finish if any
    if (fg_childpid != JUNK_VAL) waitpid(fg_childpid, &fg_exit_status, 0);

    // toggle between the modes and print informative messages
    if (is_fg_only_mode) {
        write(STDOUT_FILENO, "\nExiting foreground-only mode\n", 30);
    } else {
        write(STDOUT_FILENO,
              "\nEntering foreground-only mode (& is now ignored)\n", 50);
    }
    is_fg_only_mode = !is_fg_only_mode;  // toggle

    // make prompt more pretty
    if (fg_childpid == JUNK_VAL) write(STDOUT_FILENO, ": ", 2);

    fflush(stdout);
}

// Catches SIGCHLD (i.e. CHiLD process finished) and proceed if the child was
// a background one.
void CatchSIGCHLD(int signo) {
    // get this child's PID
    int exit_status = (int)JUNK_VAL;
    pid_t done_childpid = waitpid(-1, &exit_status, WNOHANG);  // non-blocking

    // if there is a finished child
    if (done_childpid > 0 && exit_status != (int)JUNK_VAL) {
        // find this child in the background list
        int done_childpid_idx = IndexOfDynPidArr(&bg_children, done_childpid);
        if (done_childpid_idx != -1) {
            PopDynPidArrAt(&bg_children, done_childpid_idx);

            printf("background pid %d is done: ", done_childpid);
            Status(exit_status);

            // make prompt more pretty
            if (WIFEXITED(exit_status)) write(STDOUT_FILENO, ": ", 2);

            fflush(stdout);

            // reset background PID
            bg_childpid = JUNK_VAL;
        }
    }
}

int main(void) {
    // signal handler registration
    struct sigaction SIGINT_action = {{0}};   // deal with SIGINT (i.e Ctrl-C)
    SIGINT_action.sa_handler = CatchSIGINT;   // handler when caught
    sigfillset(&SIGINT_action.sa_mask);       // block all signal types
    SIGINT_action.sa_flags = SA_RESTART;      // restart syscalls
    sigaction(SIGINT, &SIGINT_action, NULL);  // register the struct

    struct sigaction SIGTSTP_action = {{0}};    // deal with SIGTSTP
    SIGTSTP_action.sa_handler = CatchSIGTSTP;   // handler when caught
    sigfillset(&SIGTSTP_action.sa_mask);        // block all signal types
    SIGTSTP_action.sa_flags = SA_RESTART;       // no flags
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);  // register the struct

    struct sigaction SIGCHLD_action = {{0}};    // deal with SIGCHLD
    SIGCHLD_action.sa_handler = CatchSIGCHLD;   // handler when caught
    sigfillset(&SIGCHLD_action.sa_mask);        // block all signal types
    SIGCHLD_action.sa_flags = SA_RESTART;       // restart syscalls
    sigaction(SIGCHLD, &SIGCHLD_action, NULL);  // register the struct

    // initialization of static global var
    InitDynPidArr(&bg_children, INIT_CHILDREN_CAP);
    fg_childpid = JUNK_VAL;
    fg_exit_status = 0;
    bg_childpid = JUNK_VAL;
    bg_exit_status = 0;
    is_fg_only_mode = false;

    // start smallsh and keep it running in a loop
    while (true) {
        // cmdline  contains the whole command line in a C string
        char* cmdline = PromptUser();
        if (cmdline[0] == '\0' || cmdline[0] == '#') continue;

        // cmdwords  contains the whole command line in an array of C strings,
        //           whose each element is a word
        DynStrArr* cmdwords = SplitCmdLineToWords(cmdline);

        // no need to parse for  execvp()  if command is one of the built-ins
        if (strcmp(cmdwords->strings[0], "exit") == 0) {
            // clean up before exiting
            free(cmdline);
            DeleteDynStrArr(cmdwords);
            free(cmdwords);

            // clean up all background children and exit with code 0
            Exit(&bg_children, 0);
        }

        if (strcmp(cmdwords->strings[0], "status") == 0) {
            Status(fg_exit_status);

            // clean up before ignoring everything below
            free(cmdline);
            DeleteDynStrArr(cmdwords);
            free(cmdwords);
            continue;
        }

        if (strcmp(cmdwords->strings[0], "cd") == 0) {
            // if no arg, pass  ${HOME}  in as path, otherwise pass  arg[1]
            Cd((cmdwords->size == 1) ? getenv("HOME") : cmdwords->strings[1]);

            // clean up before ignoring everything below
            free(cmdline);
            DeleteDynStrArr(cmdwords);
            free(cmdwords);
            continue;
        }

        // execvp_argv  contains the command line words from the beginning to
        //              the word before  < ,  > ,  and/or  &  symbols. This is
        //              an  execvp() -compatible array of C strings
        int stdin_redir_idx = -1;
        int stdout_redir_idx = -1;
        bool is_bg = false;
        int execvp_argc = 0;
        char* execvp_argv[cmdwords->size + 1];  // +1 for NULL at the end
        ParseCmdWords(cmdwords, execvp_argv, &execvp_argc, &stdin_redir_idx,
                      &stdout_redir_idx, &is_bg);

        // fork off a child
        pid_t spawnpid = JUNK_VAL;
        spawnpid = fork();
        if (spawnpid == -1) {
            fprintf(stderr, "fork() failure\n");

            // clean up
            free(cmdline);
            DeleteDynStrArr(cmdwords);
            free(cmdwords);

            // clean up all children and exit with code 1
            Exit(&bg_children, 1);
        } else if (spawnpid == 0) {  // child
            // redirect stdin to file if the  <  symbol was found
            if (stdin_redir_idx != -1) {
                char* file = cmdwords->strings[stdin_redir_idx + 1];
                RedirectFileDescriptor(0, file, O_RDONLY, 0);
            } else if (is_bg) {
                // if the process is to run on background, suppress all stdin
                // if no input file is specified
                RedirectFileDescriptor(0, "/dev/null", O_RDONLY, 0);
            }

            // redirect stdout to file if the  >  symbol was found
            if (stdout_redir_idx != -1) {
                char* file = cmdwords->strings[stdout_redir_idx + 1];
                RedirectFileDescriptor(1, file, O_WRONLY | O_CREAT | O_TRUNC,
                                       0644);
            } else if (is_bg && !is_fg_only_mode) {
                // if the process is to run on background, suppress all stdout
                // if no output file is specified
                RedirectFileDescriptor(1, "/dev/null", O_WRONLY, 0);
            }

            // ignore SIGINT if backgrounded and not in fg-only mode
            if (is_bg && !is_fg_only_mode) {
                struct sigaction ignore_action = {{0}};
                ignore_action.sa_handler = SIG_IGN;
                sigaction(SIGINT, &ignore_action, NULL);
            }

            // ignore SIGTSTP
            struct sigaction ignore_action = {{0}};
            ignore_action.sa_handler = SIG_IGN;
            sigaction(SIGTSTP, &ignore_action, NULL);

            // use the  exec()  family for non-builtin commands
            execvp(execvp_argv[0], execvp_argv);

            // if execvp() fails, handle it
            fprintf(stderr, "%s: no such file or directory\n", execvp_argv[0]);
            exit(1);
        }

        // no  fork()  error + not child => must be the parent

        // add the child's PID to the children array if it's bg and continue on
        if (is_bg && !is_fg_only_mode) {
            PushBackDynPidArr(&bg_children, spawnpid);
            printf("background pid is %d\n", (int)spawnpid);
            fflush(stdout);
        } else {
            // otherwise, block the shell until the child exits (either normally
            // or by a signal)
            fg_childpid = spawnpid;
            waitpid(spawnpid, &fg_exit_status, 0);

            // when foreground process finishes, reset the foreground PID
            fg_childpid = JUNK_VAL;

            // print status if signal-terminated
            if (WIFSIGNALED(fg_exit_status)) Status(fg_exit_status);
        }

        // clean up heap used for this loop
        free(cmdline);
        DeleteDynStrArr(cmdwords);
        free(cmdwords);
    }

    return 0;
}
