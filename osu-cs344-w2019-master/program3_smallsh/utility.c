// utility.c
//
// Phi Luu
//
// Oregon State University
// CS_344_001_W2019 Operating Systems 1
// Program 3: Smallsh
//
// This module contains constants, parameters, and functions that are useful
// for the program but might seem too general to be related to a certain module.
// They are essentially the "helpers" of the program.

#include "utility.h"
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Converts an integer  x  to a string and put results in the string  s .
void IntToString(int x, char* s) {
    assert(s);

    // special case: x = 0
    if (x == 0) {
        s[0] = '0';
        s[1] = '\0';
        return;
    }

    // keep track of how many character currently in  s
    int slen = 0;

    // if  x  is negative, prefix  s  with a negative sign and make  x  positive
    if (x < 0) {
        s[slen++] = '-';
        x *= -1;
    }

    // get a temporary string to store the reverse string version of x
    char srev[12];
    int srev_len = 0;
    // divide x by 10 and take the remainder to get digits from right to left
    while (x > 0) {
        srev[srev_len++] = '0' + (x % 10);  // convert single digit to char
        x /= 10;
    }
    // terminate the reverse of  s
    srev[srev_len] = '\0';

    // copy to  s  in the right order
    for (int i = srev_len - 1; i >= 0; i--) s[slen++] = srev[i];
    // terminate  s
    s[slen] = '\0';
}

// Redirects the specified source file descriptor to a destination path name,
// with the provided flags and mode.
//
// Arguments:
//   src_fd         the source file descriptor
//   dest_pathname  the name of the destination path/file
//   dest_flags     flags for  open()
//   dest_mode      mode for  open()
void RedirectFileDescriptor(int src_fd, char* dest_pathname, int dest_flags,
                            mode_t dest_mode) {
    // support stdin and stdout redirection
    assert(dest_pathname && (src_fd == 0 || src_fd == 1));

    // open a new file descriptor
    int target_fd = open(dest_pathname, dest_flags, dest_mode);
    // make sure  open()  was successful
    if (target_fd == -1) {
        if (src_fd == 0) {
            fprintf(stderr, "cannot open %s for input\n", dest_pathname);
        } else if (src_fd == 1) {
            fprintf(stderr, "cannot open %s for output\n", dest_pathname);
        }
        exit(1);
    }

    // redirect the source fd to this new file descriptor
    int dup2_status = dup2(target_fd, src_fd);
    // make sure  dup2()  was successful
    if (dup2_status == -1) {
        fprintf(stderr, "dup2() failure\n");
        exit(1);
    }

    // close on  exec()
    fcntl(target_fd, F_SETFD, FD_CLOEXEC);
}

// Dynamically allocates an array of strings structure with the given capacity.
//
// Argument:
//   arr       the structure of dynamic array of strings to be initialized
//   capacity  the initial capacity of the array
void InitDynStrArr(DynStrArr* arr, int capacity) {
    assert(arr && capacity > 0);

    // allocate dynamic memory for the array
    arr->strings = malloc(capacity * sizeof(*(arr->strings)));
    assert(arr->strings);  // make sure allocation was succesful

    // init the array's size and capacity
    arr->size = 0;
    arr->capacity = capacity;
}

// Pushes a string to the back of the dynamic array (double the capacity if
// needed).
//
// Arguments:
//   arr         the structure of dynamic array of strings
//   new_string  the string to be pushed to the back of  arr->strings
//
// The array of strings inside  arr  will have an additional string at its back.
// The capacity of the array will be doubled if necessary.
void PushBackDynStrArr(DynStrArr* arr, char* new_string) {
    assert(arr && arr->strings && new_string);

    // before adding, must double the capacity if array is full
    if (arr->size == arr->capacity) {
        arr->strings = realloc(arr->strings, arr->capacity * 2
                               * sizeof(*(arr->strings)));
        arr->capacity *= 2;
    }

    // allocate memory and copy the new string to the back of the array
    arr->strings[arr->size] = malloc((strlen(new_string) + 1)
                                     * sizeof(*(arr->strings[arr->size])));
    assert(arr->strings[arr->size]);
    strcpy(arr->strings[arr->size], new_string);
    arr->size++;
}

// Deallocates the array of strings in the structure and resets the parameters.
//
// Argument:
//   arr  the structure of dynamic array of strings to be deallocated
//
// arr->strings  will be freed, and  arr->size  and arr->capacity  will be reset
// to zero.
void DeleteDynStrArr(DynStrArr* arr) {
    // do nothing if the structure or the array is NULL
    if (!arr || !arr->strings) return;

    // clean up the strings if needed
    for (int i = 0; i < arr->size; i++)
        if (arr->strings[i]) free(arr->strings[i]);

    // clean up the array and reset parameters
    free(arr->strings);
    arr->size = 0;
    arr->capacity = 0;
}

// Dynamically allocates an array of PIDs structure with the given capacity.
//
// Argument:
//   arr       the structure of dynamic array of PIDs to be initalized
//   capacity  the initial capacity of the array
void InitDynPidArr(DynPidArr* arr, int capacity) {
    assert(arr && capacity > 0);

    // allocate dynamic memory for the array
    arr->pids = malloc(capacity * sizeof(*(arr->pids)));
    assert(arr->pids);  // make sure allocation was succesful

    // init the array's size and capacity
    arr->size = 0;
    arr->capacity = capacity;
}

// Pushes a PID to the back of the dynamic array (double the capacity if
// needed).
//
// Arguments:
//   arr      the structure of dynamic array of PIDs
//   new_pid  the PID to be pushed to the back of  arr->pids
//
// The array of PIDs inside  arr  will have an additional PID at its back.
// The capacity of the array will be doubled if necessary.
void PushBackDynPidArr(DynPidArr* arr, pid_t new_pid) {
    assert(arr && arr->pids && new_pid > 0);

    // before adding, must double the capacity if array is full
    if (arr->size == arr->capacity) {
        arr->pids = realloc(arr->pids, arr->capacity * 2
                            * sizeof(*(arr->pids)));
        arr->capacity *= 2;
    }

    arr->pids[arr->size++] = new_pid;
}

// Pops the PID at the back of the dynamic array. Returns a pointer to the value
// of the PID and removes the PID from the array.
//
// Argument:
//   arr  the structure of dynamic array of PIDs
//
// Returns:
//   A pointer to the PID of the latest children (the back of the array)
//   NULL if array is empty
pid_t* PopBackDynPidArr(DynPidArr* arr) {
    assert(arr);

    // if array is empty, return NULL
    if (arr->size == 0) return NULL;

    // otherwise, return the address of the last PID and update size
    return &(arr->pids[--arr->size]);
}

// Pops the (index)-th PID from the dynamic array. No returns.
//
// Arguments:
//   arr    the structure of dynamic array of PIDs to remove the PID from
//   index  the index of the PID to be removed from the array (must be in range)
void PopDynPidArrAt(DynPidArr* arr, int index) {
    assert(arr && 0 <= index && index < arr->size);

    // order doesn't matter, so just overwrite with the final PID
    arr->pids[index] = arr->pids[--arr->size];
}

// Returns the index of the target PID, or -1 if not found.
int IndexOfDynPidArr(DynPidArr* arr, pid_t target) {
    assert(arr);

    for (int i = 0; i < arr->size; i++)
        if (arr->pids[i] == target) return i;

    return -1;
}

// Deallocates the array of PIDs in the structure and resets the parameters.
//
// Argument:
//   arr  the structure of dynamic array of PIDs to be deallocated
//
// arr->pids  will be freed, and  arr->size  and arr->capacity  will be reset
// to zero.
void DeleteDynPidArr(DynPidArr* arr) {
    // do nothing if the structure or the array is NULL
    if (!arr) return;

    // clean up the array and reset parameters
    free(arr->pids);
    arr->size = 0;
    arr->capacity = 0;
}
