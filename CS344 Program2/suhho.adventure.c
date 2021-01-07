//#############################################################################
//# NAME: HONGYIEL SUH
//# ONID: suhho@oregonstate.edu
//# CS344_PROGRAM 2
//# FILE NAME: suhho.adventure.c
//# Discription: this file will give User interface and User direction which
//#               means star game fucntion.
//# Read files with dirent header file and using struct function to access in
//# the dirent headers. It should be initialized during run the code because
//# if the user active "suhho.buildrooms.c" again, it will give another folder
//# that newest directory. So, it should be initialized which means that the file
//# header need recent directory. <- using S_ISDIR will give stat of directory
//# from stat functions.
//# The read files will read by fscanf fuction
//# The Thread will activate in time fuction, time fuction will active during
//# function activatinig ( main fucntion ) with mutex which call "key of thread"
//# it could be access in the thread to activate "time" function. So, we need to
//# setting the sync, when processing the files.
//# and it will give result (time)
//#############################################################################
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
// void getsomething();
// Room IsGetinfo();

// get list from folder
struct Room {
  int numOutboundConnections;
  char* outboundConnections[6];
  char room_name[100];
  char room_type[100];
};
struct Room roominit[7];
struct Room* room_ptr[7];

pthread_mutex_t set_mutex = PTHREAD_MUTEX_INITIALIZER;
// Reference: # https://linux.die.net/man/3/pthread_mutex_unlock
// initialized mutex time set
void get_directory(char* new_directory){
  char get_dir_file[30] = "suhho.rooms."; // the folder name will "suhho.rooms.<NUMBER:ID>"
  int is_time = -1;  // compare file is recent file
  DIR* dip;  // get directory pointer.
  struct dirent *files; // get files from Folder
  struct stat get_Attributes;
  dip = opendir(".");  // open directory that include "."
  // printf("%d\n",dip);
  // printf("Connect!\n");
  if (dip > 0){
    while ((files = readdir(dip)) != NULL) { // if folder is not empty loop
      if (strstr(files->d_name, get_dir_file) != NULL) { // if suhho.rooms is include in files d_name
            stat(files->d_name, &get_Attributes);
        if (S_ISDIR(get_Attributes.st_mode)){ //compare that " file in there or not"
          if ((int)get_Attributes.st_mode > is_time){
            is_time = (int)get_Attributes.st_mtime;
            // printf("%d\n", is_time );
            strcpy(new_directory, files->d_name);
            // printf("%d\n", is_time );
          }
        }
      }
    }
  }
  closedir(dip);
  // close directory
}
//Reference: Thread Example : lecture ppt
void *get_time(void *new_directory){
// https://www.geeksforgeeks.org/multithreading-c-2/
  while(1){
  pthread_mutex_lock(&set_mutex);
  usleep(300);
  FILE* gf;
  //ew_directory = "get suhho.~~"
  char file_path[30];
  sprintf( file_path, "%s/currentTime.txt", (char*)new_directory );
  // http://man7.org/linux/man-pages/man3/strftime.3.html
  time_t now;
  char MY_TIME[50];
  time(&now);
  struct tm *tm = localtime(&now);
  strftime(MY_TIME, sizeof(MY_TIME), "%l:%M%p, %A, %B %d, %Y",tm);
  gf = fopen(file_path, "w");
  fprintf(gf,"%s", MY_TIME);

  fclose(gf);

  pthread_mutex_unlock(&set_mutex);
  usleep(300);
  }

}
void ending_game(int game_steps, char* game_route, int current_room_num){
  if(strcmp(room_ptr[current_room_num]->room_type, "END_ROOM") == 0){
    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
    printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS: \n%s", game_steps, game_route);
    exit(0);
    //ENDING
  }
}
void start_game(int current_room_num, char* new_directory){

  static int game_steps = 0;
  static char game_route[500]="";
  // to keep this memory

  //lock memory

  int amount_room_connection = room_ptr[current_room_num]->numOutboundConnections;

    ending_game(game_steps, game_route, current_room_num); // Fallow the Endling game

    //START Game play
    int i;
    int j;
    char user_input[30];
    int user_index = -1 ;

    printf("CURRENT LOCATION:%s\n",room_ptr[current_room_num]->room_name );
    printf("POSSIBLE CONNECTIONS: ");

    for(i=0;i<amount_room_connection; i++) {
      if (i==(amount_room_connection - 1))
      printf("%s.\n", room_ptr[current_room_num]->outboundConnections[i]);
      else
      printf("%s, ", room_ptr[current_room_num]->outboundConnections[i]);
    }
    // get amount of room ptr
    printf("WHERE TO? >");
    scanf("%s", user_input);

    if(strcmp(user_input,"time")==0){
        printf("START time input \n");
        pthread_mutex_unlock(&set_mutex);
        usleep(300);
        pthread_mutex_lock(&set_mutex);
        //declare thread in this function
        printf("START time input \n");

        FILE* gf;

        char file_path[30];
        sprintf( file_path, "%s/currentTime.txt", new_directory );
        gf = fopen(file_path, "r");
        char time_buf1[10]; //give time
        char time_buf2[10]; //days of the month
        char time_buf3[10]; // months
        char time_buf4[10]; // day
        char time_buf5[10]; // year

        printf("\n");
        //"%l:%M%p, %A, %B %d, %Y"
        fscanf(gf,"%s %s %s %s %s",time_buf1,time_buf2,time_buf3,time_buf4,time_buf5);
        printf("  %s %s %s %s %s",time_buf1,time_buf2,time_buf3,time_buf4,time_buf5 );
        printf("\n\n");
        //print  10:38PM, Tuesday, February 04, 2020 ;

        fclose(gf);
        // Re asking user_input
        printf("WHERE TO? >");
        scanf("%s", user_input);
    }
    // compare if there are exist or not
    for(i=0;i<amount_room_connection;i++){
      int tmp_room_cmp = strcmp(room_ptr[current_room_num]->outboundConnections[i],user_input);
      if( tmp_room_cmp == 0){
        for(j=0;j<7;j++){
          if(strcmp(user_input,room_ptr[j]->room_name) == 0) // if user_inputs are same as room_name;
              user_index = j;
            }
            strcat(game_route,user_input);
            strcat(game_route,"\n");
            printf("\n");
            // get user input to route of result
      }
    }
    if(user_index == -1){
      printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN \n" );
      start_game(current_room_num, new_directory);
    }else{
      game_steps++;  //add 1 for steps;
      start_game(user_index, new_directory);
    }
    // compare user_input is same as connections room ptr
}
  // This will setting end point of the room path
  // This will setting by user input
  // this will chage during playing the game but not individually
  // memorytical indipendent

// }

int main(void){
  pthread_mutex_lock(&set_mutex); // unlock get_time;
  //create empty file directory from get_directory()
  char new_directory[300];
  int start_init; //start room number
  int i;
  for (i = 0; i < 7; i++) {
    room_ptr[i] = &roominit[i];
  }
  get_directory(new_directory); //recent directory get
  // printf("%s\n", new_directory ); //Success get folder directory name (suhho.rooms.<NUMBER>)

  char* file_name; // get files name in the folder


  DIR* dip;
  struct dirent *files;
  dip = opendir(new_directory);  // open directory dip2

  char file_path[300];

  int current_room_num = 0;

  while((files = readdir(dip)) != NULL){
    file_name = files->d_name;

    if ( (strcmp(files->d_name, ".") == 0) || (strcmp(files->d_name, "..") == 0) || (strcmp(files->d_name, "currentTime.txt") == 0) ) {
        //Skip
        continue;
        // Found from stackoverflow : to compare not include "." , ".." files
    }else
    strcpy(file_path, new_directory);
    strcat(file_path, "/");
    strcat(file_path, file_name);
    // snprintf(file_path,sizeof(file_path),"/%s",file_name );
    // get file path except "." , "..", "currentTime.txt"
    // printf("%s\n",file_path ); //Success!
    FILE* pFile = NULL;

    pFile = fopen(file_path, "r");
    if(pFile == NULL){
      printf("This is wrong file Path\n");
      exit(1);
    }
    // https://www.geeksforgeeks.org/scanf-and-fscanf-in-c-simple-yet-poweful/
    char buf1[100];
    char buf2[100];
    char buf_type[100];
    char* tmp_type;
    int get_return_str;
    char* tmp_roominit;
    fscanf(pFile,"%*s %*s %s", buf1);
    // printf("This is room Name: %s\n", buf1); //get room Name

    strcpy(roominit[current_room_num].room_name, buf1);
    // printf("%s\n",roominit[current_room_num].room_name );
    roominit[current_room_num].numOutboundConnections = 0;
    int* amount_connection = &roominit[current_room_num].numOutboundConnections;

    while(fscanf(pFile, "%s %s %s", buf1, buf2, buf_type) == 3){
      if(strcmp(buf1 , "CONNECTION") == 0){
        // printf("This is connection successful! %s\n",buf1 );
        // printf("This is connection bound 123123123%s\n",buf1 );
        roominit[current_room_num].outboundConnections[(*amount_connection)] = calloc(1, sizeof(char) * 100);
        //if do not (*amount_connection) -> it will give coredump
        //Reference by stackoverflow;
        strcpy(roominit[current_room_num].outboundConnections[(*amount_connection)], buf_type);
        (*amount_connection)++;
      }
    }
    int start_tmp_init;
    strcpy(roominit[current_room_num].room_type, buf_type);
    // if START_ROOM user_input start_game

    // printf("GETTING room type successful! %s\n",roominit[current_room_num].room_type );

    start_tmp_init = strcmp(roominit[current_room_num].room_type ,"START_ROOM");

    if(start_tmp_init == 0){
      start_init = current_room_num;
    }
    // printf("ROOM TYPE: %s\n", roominit[current_room_num].room_type); // roomtype successful!
    // printf("%d\n",current_room_num );
    fclose(pFile);

    current_room_num++;
  }
  closedir(dip);
  // printf("ReTest : roominit %s\n", roominit[start_init].room_type );
  //
  // printf("This is start of  function files\n");
  //
  // printf("This is start_init %d\n", start_init );
  //
  // printf("This is new_directory --- before inside game%s\n", new_directory);
  pthread_t ptid;      // Creating a new thread
  int get_thread;
  get_thread = pthread_create(&ptid, NULL, get_time, (void*)new_directory);
  assert(get_thread == 0);
// Reference : https://www.geeksforgeeks.org/thread-functions-in-c-c/
// Reference : http://man7.org/linux/man-pages/man3/pthread_create.3.html
// Reference : https://www.geeksforgeeks.org/print-1-2-3-infinitely-using-threads-in-c/
  start_game(start_init, new_directory);
  int a,b;
  for (a=0;a<7;a++) {
    for (b=0;b<roominit[a].numOutboundConnections; b++) {
      free(roominit[a].outboundConnections[b]);
    }
  }
  pthread_mutex_destroy(&set_mutex);
  // free array
  return 0;
}
