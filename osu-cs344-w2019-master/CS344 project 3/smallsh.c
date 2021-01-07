/*
Author :: Hongyiel Suh
Class :: CS 344
Project 3
File Discription
Making my own shell program each function's Discription is below the comments
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#define MAXLINE 2048 // to support command lines with a maximum length of 2048 characters
#define MAXTOK 512 // to support maximum of 512 arguments

int exit_status = -7;// this is default error signal ex) integer 32bit 1111111...11111000
//return flag 4 bit -> the highest bit is 1 case and lowest bits are all 0
int numpids = 0;
pid_t pidarray[100];//get child processor pid
int infileFD = 0;
int outfileFD = 0;
int exist_infile = 0;
int exist_outfile = 0;
int foreground_flag = 0;
int background_flag = 0;
int number_toks = 0;
int keeploop = 1;
int process_type = 0;
int ignore_signal = 0; // tp compare signal that comes out from z handler
/* Function Discription
* Status :: it will give signal that using exit value
* CatchSignal :: iterally compare signal and wait for child processor
* Ctrl_C_handler & Ctrl_Z_handler :: Handling Ctrl command (background job and foreground job)
* Tokenizing :: get tokens individually
* Command_Handler :: This is actual command Handler
 */

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

void CatchSignal(int _num) {
    // get this child's PID
    int exit_status = -7;
    int i;
    pid_t done_childpid = waitpid(-1, &exit_status, WNOHANG);  // non-blocking

    // if there is a finished child
    if (done_childpid > 0 && (exit_status != -7)) {
        for(i=0;i<numpids;i++){
            if(pidarray[i++] == done_childpid){
              pidarray[i] = 0;
              pid_t result = waitpid(done_childpid, &exit_status, WNOHANG);
              // kill(done_childpid, SIGKILL);
              kill(done_childpid,(int)WTERMSIG(exit_status));
              break;
            }
        }
        printf("background pid %d is done: ", done_childpid); Status(exit_status);
        fflush(stdout);fflush(stdin);
    }
}
void Ctrl_C_handler(int _num){
  printf("terminated by signal %d\n", _num );
}
void Ctrl_Z_handler(int _num){
  if(ignore_signal == 0){
    printf("Entering foreground-only mod (& is now ignored)\n");
    fflush(stdout);
    ignore_signal = 1;
  }else if(ignore_signal == 1){
    // go to Exiting forground mode only function
    printf("Exiting foreground-only mode\n");
    fflush(stdout);
    ignore_signal = 0;
  }
}

void Tokenizing(char** token){
    number_toks = 1;
    while((token[number_toks] = strtok(NULL, " \n\t")) != NULL){
      number_toks++; //pointing 2d arrays by pointer token
    }
}

void Kill_Process(pid_t kill_pid){
  int i;
  if((numpids > 0) && (kill_pid = 0)){
    for(i = 0; i < numpids; i++){
      pid_t result = waitpid(pidarray[i], &exit_status, WNOHANG);
      if(pidarray[i] != 0){
        if(result != 0){
          //proccess is done, kill and remove from array
          kill(pidarray[i], SIGKILL);
          pidarray[i] = 0;
        }
      }
    }
  }else if((kill_pid != 0) && (numpids > 0)) {
    for(i=0;i<numpids;i++){
        if(pidarray[i++] == kill_pid){
          pidarray[i] = 0;
          pid_t result = waitpid(kill_pid, &exit_status, WNOHANG);
          // kill(done_childpid, SIGKILL);
          kill(kill_pid,(int)WTERMSIG(exit_status));
        }else;
    }
  }else;
}


int Command_Handler(char** token, char* line){
    pid_t pid;
    int i = 0;
     // command [...][...] [< file][>file] [&]
     // detect the special character on the commend n
    keeploop = 1;
    if(!strcmp(token[0],"exit")){
        //if(!strcmp(token[1],"$") ){
            Kill_Process(0); // current child process will be killed
            //printf("Exit Program by 'exit $'\n");
            //kill(pid,SIGKILL);
            keeploop = 0;
            return(1);
        //}else{
        //    keeploop = 1;
        //    return(-2);
        //}
    }
    else if(!(strcmp(token[0],"cd"))){
        if(token[1] == NULL){
            const char* ch = getenv("HOME"); // get Home directory
            chdir(ch);
            return(1);
        }else{
            if(chdir(token[1]) == -1){
                printf(" %s: no such directory\n", token[1]);
                return (-1);
            }else{
                return(1);
            }
        }
    }
    else if(strcmp(token[0],"#")==0){
        // printf("%s",line); //give status input
        return(1);
    }
    else if(strcmp(token[0],"status")==0){
        Status(exit_status);
        return(1);
    }
    else { // from here, when the line is not built in commend
       // confirm the redirection file description
       // existing in direct
        int end_index = MAXLINE;
        char infilename[128] = {0};
        char outfilename[128] = {0};
        infileFD = 0;
        outfileFD = 0;
        exist_infile = 0;
        exist_outfile = 0;
        background_flag = 0;
        for(i=0;i<number_toks;i++) { // analysis commends ,for detecting the special commends ">" , "<" and "&"
            if(!strcmp(token[i],"<")){
                strcpy(infilename,token[i+1]);
                exist_infile = 1;
                int cur_index = (token[i] - token[0]);
                if(cur_index < end_index) end_index = cur_index;
                token[i++] = NULL; token[i] = NULL;
            }
            else if(!strcmp(token[i],">")){
                strcpy(outfilename,token[i+1]);
                exist_outfile = 1;
                int cur_index = (token[i] - token[0]);
                if(cur_index < end_index) end_index = cur_index;
                token[i++] = NULL; token[i] = NULL;
            }
            else if(!strcmp(token[i],"&")&& ((i+1) == number_toks)){
                if(ignore_signal == 0) {
                    background_flag = 10;
                    strcpy(outfilename,"/dev/null");
                    exist_outfile = 1;
                }else{
                    background_flag = 0;
                    exist_outfile = 0;
                }
                int cur_index = (token[i] - token[0]);
                if(cur_index < end_index) end_index = cur_index;
                token[i] = NULL;
                // last line
            }else {
                background_flag = 0;
                //exec_line[z++] = token[i];
            }
        }
        for(i=end_index;i<MAXLINE;i++) line[i] = '\0'; // make sure invalid buffers
        // Built-in commend check
        // and go through it
        int dup_result;
        pid_t result;

        pid = fork();       // copy image for child

        if(pid<0) {
            printf("fork error %d",pid);
            exit(-1);
        }
        else if(pid==0){
            //child process call according commend
            if(exist_infile){
                infileFD = open(infilename,O_RDONLY);
                if(infileFD == -1){
                    printf("cannot open %s for input \n",infilename);
                    // need to signaling
                    exit(1);
                }else{
                    fcntl(infileFD,F_SETFD,FD_CLOEXEC);
                    dup_result = dup2(infileFD,0);

                }
            }
            if(exist_outfile){
                outfileFD = open(outfilename,O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if(outfileFD == -1){
                    printf("cannot open %s for input \n",outfilename);
                    // need to signaling
                    exit(1);
                }else{
                    fcntl(outfileFD,F_SETFD,FD_CLOEXEC);
                    dup_result = dup2(outfileFD,1);
                }
            }
            if (background_flag == 10) { // ignore cntl_c
                struct sigaction ignore_action = {{0}};
                ignore_action.sa_handler = SIG_IGN;
                sigaction(SIGINT, &ignore_action, NULL);
            }

            if (execvp(token[0],token)== -1){           // lunch the child process is invoked
                printf("%s: no such file or directory \n",token[0]);
                Kill_Process(pid);
                exit(-1);
            }else{                                       // the case of child process is invoked
                printf("child precess is running\n");   // it will not display by Child
            }
        }else{ // Parents Process should separate the process type
            if(background_flag == 10){
                    result = waitpid(pid, &exit_status, WNOHANG);
                    pidarray[numpids++] = pid;
                    printf("background pid is %d \n", (int)pid);
                    fflush(stdout);
            }else{
                result = waitpid(pid, &exit_status, 0);
                if (WIFSIGNALED(exit_status)) Status(exit_status);
            }
        }
    }

    return(1);
}

int main(){

    /* get compare signal ctrl z and c */
    ignore_signal = 0;
    // ctrl + c
    struct sigaction SIGINT_action = {{0}};
    SIGINT_action.sa_handler = Ctrl_C_handler;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &SIGINT_action, NULL);

    // ctrl + z
    struct sigaction SIGTSTP_action = {{0}};
    SIGTSTP_action.sa_handler = Ctrl_Z_handler;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    //SIGCHLD to blocking signal and register struct
    struct sigaction SIGCHLD_action = {{0}};
    SIGCHLD_action.sa_handler = CatchSignal;
    sigfillset(&SIGCHLD_action.sa_mask);
    SIGCHLD_action.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &SIGCHLD_action, NULL);

    char* line; // it's commend line
    char cpy_line[MAXLINE];
    char* token[MAXTOK];

  /* start shell activation */
    while (keeploop){

        // it's token pointer

        printf (": ");          // prompt

        int i;
        for(i=0;i<MAXLINE;i++) cpy_line[i] = '\0';
        for(i=0;i<MAXTOK;i++) token[i] = '\0';

        line = malloc(MAXLINE * sizeof(*line)); // get line array in memory alloc
        fflush(stdin);
        fflush(stdout);
        fgets(line, MAXLINE, stdin); // get line comman

        if(line!=NULL){
            int position_of_array = (strstr(line,"$$")-line); // current address - start address of line
            if(position_of_array>0){
                line[position_of_array]   = '%';        // replace the first $ to %
                line[position_of_array+1] = 'd';        // replace second $ to d
                sprintf(cpy_line, line, getpid());       // replace current process ID number to new buffer
                strncpy(line,cpy_line,MAXLINE);
             }else{
                 strncpy(cpy_line,line,MAXLINE);
             }
    }


    if((token[0] = strtok(line," \n\t")) == NULL){
      continue;
    }
    // nothing is input then loop will active again.
    Tokenizing(token); // tokenizing line comannd in idv

    int ret_val = Command_Handler(token,cpy_line);
    if(ret_val == -2) keeploop = 1;
    free(line);
  }// While END

  exit(0);
}// main END
