/*Name: Meghan Dougherty
 * Date: 5/17/19
 * Description: implements a shell similar to the bash shell. Supports 
 * redirection of standard input and output, both foreground and background 
 * processes, and three built in commands: exit, cd, and status. Also supports
 * comments by utilizing the # character at the beginnings of lines. 
 */ 

#include <unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/wait.h>
#include<fcntl.h>
#include <string.h>

//constant variables based on project requirements
#define BUFFER_LIM 2048
#define ARG_LIM 513

//global foreground flag variable.
int fgOnly=0;

/*sigstpFunc(int signno): called when the program is passed a sigtstp signla 
 * (CTR-Z) and acts as a toggle between foreground only mode and normal mode
 * If the program is not in forground only mode, the function switches the 
 * fgOnly flag and informs the user that the & command will now be ignored and
 * that no background processes will be allowed. If the program is in foreground
 * only mode, the function switches to normal mode and informs the user that 
 * they have exited foreground-only mode. 
 */ 
void sigtstpFunc(int signo)
{
	//If we are in normal mode
	if (fgOnly == 0)
	{
		fgOnly=1;	
		//print message
		char* fgMessage = "\nEntering foreground-only mode (& is now ignored)\n: ";

		write(STDOUT_FILENO, fgMessage, 52);
		fflush(stdout);
		
	}
	//if we are in foreground-only mode
	else
	{
		//switch flag
		fgOnly=0;
		//print message
		char* fgMessage = "\nExiting foreground-only mode\n: ";
		write(STDOUT_FILENO, fgMessage, 32);
		fflush(stdout);
		
	}
}


/*getStatus(int status): takes an int as an argument. prints the either the 
 * exit status or the terminating signal of the last forground command. If the
 * function is run before any foreground command is run, it will return 0
 */
void getStatus(int status)
{
	//If the child process was terminated normally
	if(WIFEXITED(status))
	{
		//print the exit status value of the child process
		printf("exit value %d \n", WEXITSTATUS(status));
		fflush(stdout);
	}
	//otherwise, the child was terminated by a signal.
	else
	{
		//print the signal that terminated the child
		printf("terminated by signal %d\n", status);
		fflush(stdout);
	}
}

/*changeDirectory(char** argArray): takes a pointer to an argument array as an
 * argument. The user will invoke this command to change the current directory
 * if argArray[1] is NULL, the user only entered cd, it changes the directory
 * to the one specified in the HOME environment variable. Otherwise, it changes
 * the directory to the one specified in argArray[1]
 */

void changeDirectory(char** argArray)
{
	char* dir;
	//If the user did not specify a directory
	if(argArray[1]== NULL)
	{
		//set dir to the HOME environment variable
		dir = getenv("HOME");	
		chdir(dir);
	}	
	//otherwise, set dir to the first argument after "cd"
	else
	{
		dir = argArray[1];
	}

	//if the change directory command was not successfull print an error
	//message. 
	if (chdir(dir) != 0)
	{
		printf("Invalid directory name.\n"); 
	}
}



/*char* readLine(): takes no arguments. Reads raw input from stdin. returns the
 * data read.
 */ 

void expandPid(char* token, int shPid)
{
	//set up necessary variables and strings
	char stringPid[255];
	char buffer[BUFFER_LIM];
	char* copy = token;

	//convert shell PID to a string
	sprintf(stringPid, "%d", shPid);

	//search through the string looking for a substring "$$"
	while((copy = strstr(copy,"$$")))
	{
		//copy the string up to the point of the $$ occurance
		strncpy(buffer, token, copy-token);
		buffer[copy-token]='\0';

		//append the PID to the end of the sub-string
		strcat(buffer, stringPid);
		strcat(buffer, copy + strlen("$$"));
		
		//copy the concatenated buffer to the original token string.
		strcpy(token, buffer);
		copy++;
	}
	
}

void killBackground(pid_t backPids[], int numPids)
{
	int i=0;

	while(i<numPids)
	{
		kill(backPids[i], SIGTERM);
		i++;
	}
}


int main()
{
	//string for user input
	char* inputLine= malloc(BUFFER_LIM);

	//variables for input/output redirection
	char* inputFile = NULL;
	char* outputFile = NULL;

	//array for arguments.
	char* argArray[ARG_LIM]; 

	//array to store background process pids.
	pid_t backPids[512];
	char* curToken=NULL;
	char expLine[BUFFER_LIM];
	int status=0;	
	int myFile=-1;
	int pid;
	//flag for background process.
	int background=0;
	int charsEntered;
	int numPids=0;

	//Signal handler to ignore ^c
	struct sigaction sigintAct={0};
	sigintAct.sa_handler = SIG_IGN;
	sigaction(SIGINT, &sigintAct,NULL);

	//signal handler to redirect ^Z
	struct sigaction sigtstpAct={0};	
	sigtstpAct.sa_handler = &sigtstpFunc;
	sigfillset(&sigtstpAct.sa_mask);
	sigtstpAct.sa_flags=SA_RESTART;
	sigaction(SIGTSTP, &sigtstpAct, NULL);
	while(1)
	{	
		//set the background flag to 0
		background =0;
		

		//print the shell prompt
		printf(": ");
		fflush(stdout);

		//get user input.
		ssize_t size =0;
		charsEntered = getline(&inputLine, &size, stdin);
		if(charsEntered == -1)
			clearerr(stdin);
		
		//Begin parsing the user input by tokenizing the input string
		int args=0;		 

		curToken= strtok(inputLine, " \n");
	
		while(curToken != NULL)
		{
			//handle output redirection
			if(strcmp(curToken, ">") == 0)
			{
				//get the output file name and store it
				curToken=(strtok(NULL, " \n"));
				outputFile = strdup(curToken);
				
				curToken = strtok(NULL, " \n");
			}
			//handle input file redirection
			else if (strcmp(curToken, "<") ==0)
			{
				//get input file name and store it
				curToken=(strtok(NULL, " \n"));
				inputFile = strdup(curToken);

				curToken = strtok(NULL, " \n");
			}
			
			//handle expansion 
			else 
			{
				//check if part of the token contains $$
				if(strstr(curToken, "$$") != NULL)
				{
					//get the shell pid
					int shPid = getpid();
					//convert the $$ to the shell Pid.
					strcpy(expLine, curToken);
					expandPid(expLine, shPid);
					curToken=expLine;
				}
			
				//add the token to the argArray	
				argArray[args]=strdup(curToken);
				curToken=strtok(NULL, " \n");
				args++;
			}		
				
		}
		/*Check if the last argument is an ampersand. 
		This must be outside the tokenizing loop, 
		otherwise it will catch errantn ampersands 
		inside echo comments and interpret them inccorrectly.*/
		
		//decrement the argument iterator
		args--;
		//make sure the array position is not null and the final 
		//argument is a "&"
		if(argArray[args] != NULL && strcmp(argArray[args],"&")==0)
		{
			//remove the & in argument array. If this doesn't happen
			//sleep commands will return an error.
			argArray[args]='\0';
			//if background commands are allowed, set the background flag.
			if(fgOnly==0)
			{
				background=1;
			}
		}
		//re-increment the args iterater for later use
		args++;
		//set the final array value to NULL.	
		argArray[args] = NULL;	
		
		//Begin checking the argArray for built in commands
		
		//Check if the user entered a blank line or a comment
		if(argArray[0] == NULL || strncmp(argArray[0],"#",1)==0)
		{
			//do nothing
			
		}

		//if the user entered cd
		else if (strcmp(argArray[0],"cd")==0)
		{
			//call changeDir 
			changeDirectory(argArray);
			
		}

		//if the user entered exit
		else if(strcmp(argArray[0],"exit")==0)
		{
			//clean up any background processes exit the shell
			killBackground(backPids, numPids);
			exit(EXIT_SUCCESS);
		}

		//if the user entered status
		else if(strcmp(argArray[0],"status")==0)
		{
			//call getStatus
			getStatus(status);
			fflush(stdout);
		}

		//otherwise, execute command by forking a child process
		else	
		{
			pid = fork();
			//if there was an error forking the child process
			if(pid<0)
			{
				//print an error message, clean up and exit
				perror("Hull Breach!/n");
				killBackground(backPids, numPids);
				status =1;
				break;
			}
			//instructions for the child process
			else if(pid==0)
			{
				//if this is a foreground process
				if(background==0)
				{
					//reset the sigint handler so the 
					//child process can be killed.
					sigintAct.sa_handler = SIG_DFL;
					sigaction(SIGINT, &sigintAct, NULL);
				}

				//if input redirection was specified
				if(inputFile != NULL)
				{
					//open in read only mode
					myFile=open(inputFile, O_RDONLY);

					//if there is an error opening the file,
					//display a message and exit.
					if(myFile ==-1)
					{
						fprintf(stderr, "cannot open file %s for input\n", inputFile);
						fflush(stdout);
						exit(1);
					}
					//redirect input and print a message if
					//an error occurs
					else if(dup2(myFile,0)== -1)
					{
						fprintf(stderr, "error in redirecting input\n");
						fflush(stdout);
						exit(1);
					}
					close(myFile);
				}
				//if output redirection was specified
				if(outputFile!= NULL)
				{
					//open the file for write only, to create the
					//file if it doesn't exist, and to over write
					//the file if it does exist.
					myFile = open(outputFile, O_WRONLY|O_CREAT|O_TRUNC, 0644);
					//print an error message if there was an error
					//opening the file.
					if(myFile==-1)
					{
						fprintf(stderr, "cannot open %s for output\n", outputFile);
						fflush(stdout);
						exit(1);
					}			
					//otherwise, redirect the input and print
					//an error message if any occur
					else if(dup2(myFile,1)== -1)
					{
						fprintf(stderr, "error in redirecting output\n");
						fflush(stdout);
						exit(1);
					}
						//close the file.
						close(myFile);
				}
				//Check if this is a background process
				if(background==1)
				{
					//if no input was specified
					if (inputFile==NULL)
					{	
						//redirect the input to /dev/null and
						//check for errors
						myFile = open("/dev/null", O_RDONLY);
						if(myFile==-1)
						{
							fprintf(stderr, "cannot open /dev/null for input\n");
							fflush(stdout);
							exit(1);
						}
						//read the data, checking for read
						//errors
						else if(dup2(myFile,0)==-1)
						{
							fprintf(stderr, "error in redirecting input\n");
							fflush(stdout);
							exit(1);
						}
						close(myFile);
					}
					//if no output file was specified
					if (outputFile==NULL)
					{
						//redirec the output to /dev/null so 
						//that the input does not print
						//to the terminal and check for errors

						myFile = open("/dev/null", O_WRONLY);
						if(myFile==-1)
						{
							fprintf(stderr, "cannot open /dev/null for output\n");
							fflush(stdout);
							exit(1);
						}
						//write the output and check for errors
						else if(dup2(myFile,1)==-1)
						{
							fprintf(stderr, "error in redirecting output\n");
							fflush(stdout);
							exit(1);
						}
						close(myFile);
					}
				}
				//execute the command, and print an error message
				//if the command was not found.
				if(execvp(argArray[0], argArray))
				{
					fprintf(stderr, "%s: no such file or directory\n", argArray[0]);
					fflush(stdout);
					exit(1);
				}
			}
			//instructions for the parent process
			else
			{
				//if this is a foreground process
				if(background==0)
				{
					//wait for the process to complete
				
					pid=waitpid(pid, &status, 0);
					
					//if process was terminated, print an error 
					//message with terminating signal
					if(WIFSIGNALED(status))
					{
						printf("terminated by signal %d\n", status);
						fflush(stdout);
					}
				}
				//if this is a background process
				if(background == 1)
				{
					//do not wait for the process to complete
					waitpid(pid, &status, WNOHANG);
					//add PID to background PID array for later
					//clean up if necessary
					backPids[numPids] = pid;
					//print PID and increment background PID
					//counter
					printf("background pid is %d\n", pid);
					fflush(stdout);
					numPids++;		
					
				}

			}
		
		}

		//clean up the argument array for the next command
		int x=0;
		while(x<args)
		{
			argArray[x]=NULL;
			x++;
		}

		//clear out the input and output file names as well
		//as the user input string.
		inputFile = NULL;
		outputFile = NULL;
		inputLine=NULL;
		free(inputLine);

		//check for any background processes that have completed
		pid=waitpid(-1, &status, WNOHANG);
		while(pid>0)
		{
			//print the PID and exit value if the process ended normally
			if (WIFEXITED(status))
			{
				printf("background pid %d is done: exit value %d\n", pid,status);
				fflush(stdout);
			}
			//print the PID and signal if the process was terminated
			else
			{
				printf("background pid %d is done: terminated by singal %d\n", pid, status);
				fflush(stdout);
			}
			//continue checking
			pid = waitpid(-1,&status, WNOHANG);
				
				
		}

	}
	return 0;
}


