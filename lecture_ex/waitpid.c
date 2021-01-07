#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void main() {
	pid_t spawnPid = -5; 
	int childExitMethod = -5;

	spawnPid = fork(); 
	if (spawnPid == -1) {
		perror("Hull Breach!\n"); 
		exit(1);
	}
	else if (spawnPid == 0) { // Terminate the child process immediately 
		printf("CHILD: PID: %d, exiting!\n", spawnPid); 
		exit(0);
	}

	printf("PARENT: PID: %d, waiting...\n", spawnPid); 
	waitpid(spawnPid, &childExitMethod, 0); 
	printf("PARENT: Child process terminated, exiting!\n"); 
	exit(0);
}