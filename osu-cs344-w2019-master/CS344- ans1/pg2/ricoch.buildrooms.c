/*********************************************
Assignment 2: Adventure Game!
Christopher Rico
CS344
OSU ID: 933239746
**********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#define NUM_POSSIBLE_CONNECTS 6
#define NUM_ROOMS 7

/*********************************************
  Room Struct
**********************************************/
struct room
{
  int id;
  char* name;
  char* room_type;
  int numConnections;
  struct room* outConnections[NUM_POSSIBLE_CONNECTS];
};


/*********************************************
  Program globals
**********************************************/
// array to store the rooms
struct room roomList[NUM_ROOMS];
// name of the output directory
char directoryName[200];
// array of names to be randomly selected from
char* roomNames[10] = {"distrikt", "man", "center", "q", "tropical", "playa", "temple", "gerlach", "esp", "airport"};
// array to count which names have been assigned
int pulled[NUM_ROOMS] = {0, 0, 0, 0, 0 ,0, 0};
//array to store the room types to be assigned
char* roomType[3] = {"START_ROOM", "END_ROOM", "MID_ROOM"};


/*********************************************
  isGraphFull: checks to see if any rooms
  have fewer than three connections
**********************************************/
int isGraphFull()
{
  int i, rV = 1;

  // check each room struct,
  // if it has fewer than three connections the graph is not full yet
  for(i=0; i<NUM_ROOMS; i++)
  {
    if(roomList[i].numConnections < 3)
      rV = 0;
  }

  return rV;
}


/*********************************************
  addRandomConnection: adds a connection
  between two randomly selected rooms
**********************************************/
void addRandomConnection()
{
  int a, b;
  // as long as randomly selected room a cannot be added to, get another
  while(1)
  {
    a = getRandomRoom();

    //once we choose one that can have an outgoing connection, break out of loop
    if (canAddConnectionFrom(a) == 1)
      break;
  }

  //get a random room b
  //that can have an outgoing connection, is not the same as a, and does not
  //already have a connection to a
  do
  {
    b = getRandomRoom();

  } while(canAddConnectionFrom(b) == 0 || isSameRoom(a, b) == 1 || connectionAlreadyExists(a, b) == 1);

  //connect the rooms to each other
  connectRoom(a, b);
  connectRoom(b, a);
  return;
}


/*********************************************
  getRandomRoom: returns a room at random from
  roomList array
**********************************************/
int getRandomRoom()
{
  //returns the position of a random room in the roomList array
  int pos = rand() % NUM_ROOMS;

  return pos;
}


/*********************************************
  canAddConnectionFrom: checks to see that a
  room has fewer than 6 outgoing connections
**********************************************/
int canAddConnectionFrom(int a)
{
  //checks to make sure that a room has fewer than 6 outgoing connections
  if(roomList[a].numConnections < NUM_POSSIBLE_CONNECTS)
    return 1;
  else
    return 0;
}


/*********************************************
  connectionAlreadyExists: checks to see that
  a connection between two rooms already exists
**********************************************/
int connectionAlreadyExists(int a, int b)
{
  int i, rV=0;
  //checks room ID of room a against room IDs of outgoing connections from room b
  for (i=0; i<roomList[b].numConnections; i++)
  {
    if (roomList[a].id == roomList[b].outConnections[i]->id)
      rV=1;
  }

  //returns 0 for no connection, 1 for connection
  return rV;
}


/*********************************************
  connectRoom: points one room's outgoing connection
  at another room's address
**********************************************/
void connectRoom(int a, int b)
{
  //points an outgoing connection at one room at address of another
  roomList[a].outConnections[roomList[a].numConnections] = &roomList[b];
  //increments number of connections for room with new connection
  roomList[a].numConnections++;
  return;
}


/*********************************************
  isSameRoom: checks to see that two rooms are not
  the same room struct
**********************************************/
int isSameRoom(int a, int b)
{
  //checks that two room's IDs are not the same
  if (roomList[a].id == roomList[b].id)
    return 1;
  else
    return 0;
}


/*********************************************
  getRandomRoomName: randomly accesses room name,
  checks that it has not already been assigned,
  then returns the name
**********************************************/
char* getRandomRoomName()
{
  int pos;
  int checker = 1;

  //continue getting a random name from the list of names
  while (checker)
  {
    pos = rand() % 10;

    //if the name has not already been pulled
    if (pulled[pos] == 0)
    {
      //stop the loop
      checker = 0;
      //set the name to pulled
      pulled[pos]= 1;
      //return the name
      return roomNames[pos];
    }
  }
}


/*********************************************
  initializeRooms: assigns names, IDs, and room types to 7 rooms
  sets their number of outgoing connections to 0
**********************************************/
void initializeRooms()
{
  int i;
  for (i=0; i<NUM_ROOMS; i++)
  {
    //give each room an id 1-7
    roomList[i].id = i;
    //get a randomly assigned name for the room
    roomList[i].name = getRandomRoomName();

    //assign the room a room type
    // start room
    if (i == 0)
      roomList[i].room_type = roomType[0];
    // end room
    else if (i == NUM_POSSIBLE_CONNECTS)
      roomList[i].room_type = roomType[1];
    // mid room
    else
      roomList[i].room_type = roomType[2];

    roomList[i].numConnections = 0;
  }

  return;
}


/*********************************************
  makeFolder: creates the directory the rooms will be written to
**********************************************/
char* makeFolder()
{
  sprintf(directoryName, "ricoch.rooms.%d", getpid());
  mkdir(directoryName, 0700);  //Need to set permissions for folder. I forgot to do this at first and kept getting seg fault errors
  return directoryName;
}


/*********************************************
  putFiles: writes the files to a specified directory
**********************************************/
void putFiles(char* folder)
{
  //go into specified directory
  chdir(folder);

  //Make a file with the proper name for each room
  int i;
  for(i = 0; i < NUM_ROOMS; i++)
  {
    FILE* myFile = fopen(roomList[i].name, "a");
    fprintf(myFile, "ROOM NAME: %s\n", roomList[i].name);

    int j;
    //Print the connections for each file. We use j+ 1 because we want to print connections starting at 1, not 0
    for(j = 0; j < roomList[i].numConnections; j++)
    {
      fprintf(myFile, "CONNECTION %d: %s\n", j+1, roomList[i].outConnections[j]->name);
    }

    //Get the type and print it to the room
    fprintf(myFile, "ROOM TYPE: %s\n", roomList[i].room_type);
    fclose(myFile);
 }
 return;
}


int main()
{
  //seed random number generator
  srand(time(NULL));

  //generate and name each room, give it an ID, room type
  initializeRooms();

  //as long as there is one room with fewer than 3 connections, add connections
  while( isGraphFull() == 0 )
  {
    addRandomConnection();
  }

  //make the directory to store each file
  char* roomFolder = makeFolder();

  //put the room files in the directory
  putFiles(roomFolder);

  return 0;
}
                                          
