#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void execute(char** argv) {
	if (execvp(*argv, argv) < 0) {
		perror("Exec failure!"); 
		exit(1);
	}
}

void main() {
	char* args[3] = { "ls", "-a", NULL }; 
	printf("Replacing process with: %s %s\n", args[0], args[1]); 
	execute(args);
}