//#############################################################################
//# NAME: HONGYIEL SUH
//# ONID: suhho@oregonstate.edu
//# CS344_PROGRAM 2
//# FILE NAME: suhho.buildroom.c
//# Discription: this file will create rooms that made by files and folder,
//# folder will make with process id
//# files will connect each other with signal that lost signal will give -1 arrays.
//#############################################################################
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct stat st = {0};
struct Room{
  // this is for random room for using in AddRandomConnection
  int numOutboundConnections;
  int outboundConnections[6];
  // char room_name[];
};
// Need to initialized
struct Room roominit[7] = {
  //  -1 will mean that no connection.
    {.outboundConnections = {-1,-1,-1,-1,-1,-1}},
    {.outboundConnections = {-1,-1,-1,-1,-1,-1}},
    {.outboundConnections = {-1,-1,-1,-1,-1,-1}},
    {.outboundConnections = {-1,-1,-1,-1,-1,-1}},
    {.outboundConnections = {-1,-1,-1,-1,-1,-1}},
    {.outboundConnections = {-1,-1,-1,-1,-1,-1}},
    {.outboundConnections = {-1,-1,-1,-1,-1,-1}}
};


struct Room* room_ptr[7];

// Create all connections in graph
void ConnectRoom(int x, int y);

int GetRandomRoom();
bool IsSameRoom(int x, int y);

bool CanAddConnectionFrom(int roomIndex);
bool ConnectionAlreadyExists(int x, int y);



// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
bool IsGraphFull()
{
    int i;
    for( i = 0; i < 7; i++){
        //give message if numberof room values are less than 3
        //it will give message return false;
        if((*room_ptr[i]).numOutboundConnections < 3)
        return false;
        }
        return true;
}

// Adds a random, valid outbound connection from a Room to another Room
void AddRandomConnection()
{
  // Room A;  // Maybe a struct, maybe global arrays of ints
  // Room B;
  int random_room1;
  int random_room2;
  while(true)
  {
    random_room1 = GetRandomRoom();
    // printf("Hi\n");
    if (CanAddConnectionFrom(random_room1) == true)
      break;
  }

  do
  {
    random_room2 = GetRandomRoom();
  }
  while(CanAddConnectionFrom(random_room2) == false || IsSameRoom(random_room1, random_room2) == true || ConnectionAlreadyExists(random_room1, random_room2) == true);

  ConnectRoom(random_room1, random_room2);  // TODO: Add this connection to the real variables,
  ConnectRoom(random_room2, random_room1);  //  because this A and B will be destroyed when this function terminates


  // which means that connected room each others room1 - room2
  // Also, Backward room2 - room1
}

// Returns a random Room, does NOT validate if connection can be added
int GetRandomRoom()
{
  return rand() % 7;
  //get random numbers
}

// Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
bool CanAddConnectionFrom(int x)
{
  // printf("%d",(*room_ptr[x]).numOutboundConnections);

  if ((*room_ptr[x]).numOutboundConnections < 6)
    return true;
      else
    return false;
// Compare Add connection available or not
}
// Returns true if a connection from Room x to Room y already exists, false otherwise
bool ConnectionAlreadyExists(int x, int y)
{
  int i;
  for (i = 0; i < 6; i++) {
    if((*room_ptr[x]).outboundConnections[i] == y)
    return true;
    //if room is already exist give true
  }
  return false;
  // if not give false
}

// Connects Rooms x and y together, does not check if this connection is valid
void ConnectRoom(int x, int y)
{
  int i;
  for (i=0; i< 6; i++) {
    if((*room_ptr[x]).outboundConnections[i] == -1){
      //if Room[x][y] same as -1
      (*room_ptr[x]).outboundConnections[i] = y;
      //Room[x][y] will connect number "y" room
      (*room_ptr[x]).numOutboundConnections++;
      // printf("%d\n",(*room_ptr[x]).numOutboundConnections );
      //And generate next room and prepare to compare
      break;
    }
  }
}

// Returns true if Rooms x and y are the same Room, false otherwise
bool IsSameRoom(int x, int y)
{
  return( x == y );
  //if same return
}


void make_file(FILE** files){
  int i=0;
  int j=0;
  int k=9;
  int p=0;
  int number=0;
  int num_file=0;
  int size=10;
  char make_folder[30];
  char temp_folder[30];
  char file_get_folder[30];
  char* Room_list[7];
  char* number_j[7];
  char* Room_full_list[] = {"my_Room","Mcdonald","Car","classroom","Bus","Texi","Train","Road","Plain","Boat"};
  int get_id = getpid();


  for(i=0;i<7;i++){

    j = rand() % size;
    Room_list[i] = Room_full_list[j];
    Room_full_list[j] = Room_full_list[k];
    // get last array value on j;
    Room_full_list[k] = 0;
    // init last array
    // does not need but just in case;
    k--;
    size--;
    //narrow arrays and catch ther room arrays
  }
// Make direction of Folder
  sprintf(make_folder, "suhho.rooms.%d/",get_id );
  sprintf(temp_folder, "/%s",make_folder );
  if(stat(temp_folder, &st) == -1 ){
    int makeDirectory = mkdir(make_folder, 0755); // compare error
    if (makeDirectory == -1) {
      perror("ERROR"); //get error
      exit(0);
    }
  }
  //makefile

  for (i = 0; i < 7; i++) {
    // printf("%s\n",room_ptr[i] );
    strcpy(file_get_folder, make_folder);
    strcat(file_get_folder, Room_list[i]);
    //folder and files gets direction
    files[i] = fopen(file_get_folder, "w");
    //input files in the folder
    if (files[i] == NULL){
      perror("ERROR code: make files"); // get error
      exit(0);
    }

fprintf(files[i], "ROOM NAME: %s\n", Room_list[i]);
//get room name in the each room
for(p=0;p<6;p++){
  if((*room_ptr[i]).outboundConnections[ p ] != -1)
    fprintf(files[i], "CONNECTION %d: %s\n", p+1, Room_list[(*room_ptr[i]).outboundConnections[ p ] ]);
  else
    //get message
  break;
}
if (i == 0){
  fprintf(files[0], "ROOM TYPE: START_ROOM\n");
}else if (i == 6) {
  fprintf(files[6], "ROOM TYPE: END_ROOM\n");
}else
  fprintf(files[i], "ROOM TYPE: MID_ROOM\n");

  //get flags 0-> strat_Room
  //get flag 6 -> END_ROOM
  //get flag i -> MID_ROOM
fclose(files[i]); // file close
  }
//make folder

//http://man7.org/linux/man-pages/man2/getpid.2.html --> getpid();

}



int main() {
  int i=0;
  srand(time(NULL));

  for(i=0; i < 7;i++){
  room_ptr[i] = &roominit[i];
}
  // create File that include room# names
  // FILE** roomFiles = malloc(sizeof(FILE*)* 7);
  // create_file(roomFiles);
  // printf("hahaha");
  // create_file();
  FILE** roomFiles = malloc(sizeof(FILE*)*7); //make files in to room files

  while (IsGraphFull() == false)
  {
    AddRandomConnection();
  }

  make_file(roomFiles);
  return 0;
}
