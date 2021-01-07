#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void main() {
	int childExitMethod; 
	pid_t childPID = wait(&childExitMethod);
	if (childPID == -1) {
		perror("wait failed"); 
		exit(1);
	}
	if (WIFEXITED(childExitMethod)) {
		printf("The process exited normally\n"); 
		int exitStatus = WEXITSTATUS(childExitMethod); 
		printf("exit status was %d\n", exitStatus);
	}
	else 
		printf("Child terminated by a signal\n");
}