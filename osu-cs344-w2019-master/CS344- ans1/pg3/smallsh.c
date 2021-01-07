/*********************************************
Assignment 3: smallsh
Christopher Rico
CS344
OSU ID: 933239746
**********************************************/

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>


/*********************************************
              Function Prototypes
**********************************************/
void printStatus(int stats);
void catchSIGINT(int signo);
void catchSIGSTOP(int signo);
int getInput(char* arg[], char** fileIn, char** fileOut);
void replacePID(char *target, const char *needle, const char *replacement);


// global flag to toggle foreground-only mode (start in standard)
int ignoreBg = 0;


int main()
{
  char* argument[513];
  char* inFile = NULL;
  char* outFile = NULL;

  int foregroundProc;
  int i;
  int status = 0;
  int cpid;
  int fileIn = -1;
  int fileOut = -1;


  //Set up actions and handlers for ctrl+z and ctrl+c
  //Taken from lecture
  struct sigaction SIGINT_action = {0};
  struct sigaction SIGSTOP_action = {0};

  //for catching Ctrl-C
  SIGINT_action.sa_handler = catchSIGINT;
  SIGINT_action.sa_flags = 0;
  sigfillset(&(SIGINT_action.sa_mask));
  sigaction(SIGINT, &SIGINT_action, NULL);
  //for catching Ctrl-Z
  SIGSTOP_action.sa_handler = catchSIGSTOP;
  SIGSTOP_action.sa_flags = 0;
  sigfillset(&(SIGSTOP_action.sa_mask));
  sigaction(SIGTSTP, &SIGSTOP_action, NULL);


  //keep looping until user exits
  while(1)
  {
    //collect user input, parse into argument array and in/out files
    //and whether user wants process to be backgrounded or not
    foregroundProc = getInput(argument, &inFile, &outFile);
    //if the user is in foreground-only mode
    if(ignoreBg)
      foregroundProc = 1;

    /*********************************************
           User entered a built in-command:
           exit, cd, or status
     **********************************************/
    //line is null or user made a comment line
    if(argument[0] == NULL || strcmp(argument[0], "#") == 0)
    {
      continue;
    }

    //if user wants to change directory
    else if(strcmp(argument[0], "cd") == 0)
    {
      //if no argument, take user to their home directory
      if(argument[1] == NULL)
      {
        chdir(getenv("HOME"));
      }
      //otherwise, take user to the specified directory
      else
      {
        chdir(argument[1]);
      }
    }

    //if user wants status, print it
    else if(strcmp(argument[0], "status") == 0)
    {
      printStatus(status);
    }

    //if user wants to exit, let them
    else if(strcmp(argument[0], "exit") == 0)
    {
      exit(0);
    }

    /*********************************************
          User entered a non-built-in command
     **********************************************/
    //fork a new process and run switch on pid
    else
    {
      cpid = fork();
      switch(cpid)
      {
        //child process
        case 0:
          //allow the process to be killed by cmd-c
          SIGINT_action.sa_handler = SIG_DFL;
          sigaction(SIGINT, &SIGINT_action, NULL);

          //if user specified input redirection, open input file
          if(inFile != NULL)
          {
            fileIn = open(inFile, O_RDONLY);
            //if input file cannot be opened, let the user know
            if(fileIn == -1)
            {
              printf("\nsmallsh: cannot open %s for input\n", inFile);
              fflush(stdout);
              _Exit(1);
            }

            if(dup2(fileIn, 0) == -1)
            {
              perror("dup2");
              _Exit(1);
            }
            close(fileIn);
          }

          //if user specified this should be a background process
          else if(!foregroundProc)
          {
            fileIn = open("/dev/null", O_RDONLY);
            if(fileIn == -1)
            {
              perror("open");
              _Exit(1);
            }

            if(dup2(fileIn, 0) == -1)
            {
              perror("dup2");
              _Exit(1);
            }
          }

          //if user specified output redireciton, open output file
          if(outFile != NULL)
          {
            fileOut = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0744);
            if(fileOut == -1)
            {
              printf("\nsmallsh: cannot open file %s\n", outFile);
              fflush(stdout);
              _Exit(1);
            }

            if(dup2(fileOut, 1) == -1)
            {
              perror("dup2");
              _Exit(1);
            }
            close(fileOut);
          }

          //fire up the new process
          if(execvp(argument[0], argument))
          {
            //if it returns non-zero, it didn't work
            printf("\n%s: no such file or directory\n", argument[0]);
            fflush(stdout);
            _Exit(1);
          }
          break;


        //error with forking, record it
        case -1:
          perror("fork you error!");
          status = 1;
          break;


        //parent process
        default:
          if(foregroundProc)
          {
            //wait for foreground process to finish
            //before returning control to user
            waitpid(cpid, &status, 0);
          }
          else
          {
            //otherwise, print that the process is backgrounded
            //and return control to the user
            printf("\n%i is running in the background\n", cpid);
            break;
          }
      } // end of switch statement
    } // end of 'non-built-in command' else statment


    //clean up for next iteration of loop
    //clear argument list for next command user will enter
    for(i = 0; argument[i] != NULL; i++)
    {
        free(argument[i]);
    }

    //clear any allocated mem
    free(inFile);
    inFile = NULL;
    free(outFile);
    outFile = NULL;

    //Check if any process has completed
    //Returns 0 if no terminated processes
    cpid = waitpid(-1, &status, WNOHANG);
    while(cpid > 0)
    {
        printf("background process %i complete: ", cpid);
        printStatus(status);
        cpid = waitpid(-1, &status, WNOHANG);
    }

  } // end outer while loop

  return 0;

} //end main



/*********************************************
  showStatus:
  Prints the exit status of status to screen
**********************************************/
void printStatus(int stats)
{
  //Taken from lecture
  if(WIFEXITED(stats))
      printf("exit value %i\n", WEXITSTATUS(stats));
  else
      printf("terminated by signal %i\n", stats);

  return;
}


/*********************************************
  catchSIGINT:
  Catches (Ctrl-C) and causes it to do nothing
  to the smallsh process
**********************************************/
void catchSIGINT(int signo)
{

}


/*********************************************
  catchSigStop:
  Catches stop signal (Ctrl-Z)
  and toggles foreground-only mode
**********************************************/
void catchSIGSTOP(int signo)
{
  //toggles in or out of foreground-only mode
  if (ignoreBg)
  {
    ignoreBg = 0;
    char* message1 ="\nExiting foreground-only mode\n";
    write(STDOUT_FILENO, message1, 31);
  }
  else
  {
    ignoreBg = 1;
    char* message2 = "\nEntering foreground-only mode (& is now ignored)\n";
    write(STDOUT_FILENO, message2, 51);
  }

  return;
}


/*********************************************
  replacePID:
  Takes 3 pointers to char:
  - Input string (target)
  - Substr to search for (needle)
  - String to replace with (replacement)
  Referenced https://stackoverflow.com/questions/32413667/replace-all-occurrences-of-a-substring-in-a-string-in-c
**********************************************/
void replacePID(char *target, const char *needle, const char *replacement)
{
  char buffer[2050] = { 0 };
  char *insert_point = &buffer[0];
  const char *tmp = target;
  size_t needle_len = strlen(needle);
  size_t repl_len = strlen(replacement);

  while (1) {
      const char *p = strstr(tmp, needle);

      // walked past last occurrence of needle; copy remaining part
      if (p == NULL) {
          strcpy(insert_point, tmp);
          break;
      }

      // copy part before needle
      memcpy(insert_point, tmp, p - tmp);
      insert_point += p - tmp;

      // copy replacement string
      memcpy(insert_point, replacement, repl_len);
      insert_point += repl_len;

      // adjust pointers, move on
      tmp = p + needle_len;
  }

  // write altered string back to target
  strcpy(target, buffer);
}


/*********************************************
  getInput:
  Takes pointer to array of char pointers to store user args,
  Pointer to char for input and output file names

  Grabs user input from stdin and parses:
  Arguments
  Any input/output files using < >
  Whether the process should be background or not using &

  Returns 1 if command should be run in foreground, 0 if bg.
**********************************************/
int getInput(char* arg[], char** fileIn, char** fileOut)
{
  char userInput[2050];
  char pid[10];
  char* token;
  int argIdx = 0,
      fG = 1;

  fflush(stdout);
  fflush(stdin);
  printf(": ");
  fflush(stdin);

  if(fgets(userInput, sizeof(userInput), stdin) != NULL)
  {
    //replace any instances of $$ with shell PID
    sprintf(pid, "%d", getpid());
    replacePID(userInput, "$$", pid);

    //now parse user input for instructions
    token = strtok(userInput, " \n");
    while(token != NULL)
    {
      //if user wants an input file
      if(strcmp(token, "<") ==0 )
      {
        //parse the input and store the name of the file in fileIN
        token = strtok(NULL, " \n");
        *fileIn = strdup(token);
        token = strtok(NULL, " \n");
      }

      //if user wants an output file
      else if(strcmp(token, ">") == 0)
      {
        //perform same action and save name of file to fileOut
        token = strtok(NULL, " \n");
        *fileOut = strdup(token);
        token = strtok(NULL, " \n");
      }

      //if user wants process to be a background process
      else if(strcmp(token, "&") == 0)
      {
        fG = 0;
        break; //because this is the end of the input
      }

      //otherwise, save each argument in its own index
      else
      {
        arg[argIdx] = strdup(token);
        token = strtok(NULL, " \n");
        argIdx++;
      }
    }

    //save a null value at the last index in argument array
    arg[argIdx] = NULL;
  }

  //return whether or not the process should be foreground
  return fG;
}
