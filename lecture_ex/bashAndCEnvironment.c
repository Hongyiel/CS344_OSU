#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
int main(int argc, char* argv[]) {
	char array[1000]; 
	printf("Variable %s has value: %s\n", argv[1], getenv(argv[1])); 
	printf("Doubling it!\n"); 
	strcpy(array, getenv(argv[1])); 
	strcat(array, getenv(argv[1])); 
	printf("New value of %s will be: %s\n", argv[1], array); 
	setenv(argv[1], array, 1); 
	printf("Variable %s has value: %s\n", argv[1], getenv(argv[1]));
}