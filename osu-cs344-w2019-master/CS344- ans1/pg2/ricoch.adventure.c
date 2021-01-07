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
#define NUM_ROOMS
/*********************************************
  Room Struct
**********************************************/
struct room
{
  char name[100];
  char room_type[100];
  int numConnections;
  struct room* outConnections[NUM_POSSIBLE_CONNECTS];
};

/*********************************************
  Program globals
**********************************************/
pthread_mutex_t mutex;
struct room roomList[NUM_ROOMS];


/*****************************************************************************
                            Game Setup Functions
*****************************************************************************/
/***************************************************************
   getRoomDirectory:
   Finds the most recently created room directory.
   Uses the file names inside it to name the room structs.
   Calls buildRoom for each room to build outgoing connections.
***************************************************************/
void readFileNames()
{
  // Modified timestamp of newest subdir examined
  int newestDirTime = -1;
  // Prefix we're looking for
  char targetDirPrefix[32] = "ricoch.rooms.";
  // Holds the name of the newest dir that contains prefix
  char newestDirName[256];
  memset(newestDirName, '\0', (sizeof(newestDirName)) );
  // Holds the directory we're starting in
  DIR* dirToCheck;
  // Holds the current subdir of the starting dir
  struct dirent *fileInDir;
  // Holds information we've gained about subdir
  struct stat dirAttributes;

  // Open up the directory this program was run in
  dirToCheck = opendir(".");


  // Make sure the current directory could be opened
  if (dirToCheck > 0)
  {
  // Check each entry in dir
   while ((fileInDir = readdir(dirToCheck)) != NULL)
   {
     //if entry has prefix
     if (strstr(fileInDir->d_name, targetDirPrefix) != NULL)
     {
       // Get attributes of the entry
       stat(fileInDir->d_name, &dirAttributes);

       // If this time is bigger
       if ((int)dirAttributes.st_mtime > newestDirTime)
       {
         newestDirTime = (int)dirAttributes.st_mtime;
         memset(newestDirName, '\0', sizeof(newestDirName));
         strcpy(newestDirName, fileInDir->d_name);
       }
      }
    }
  }
  // Close the directory we opened
  closedir(dirToCheck);


  //now we pull each filename to name the room structs
  int i=0;
  char filePath[100];
  //concatenate the filename into a useable filepath
  sprintf(filePath, "./%s", newestDirName);

  //create directory object and open filepath that was passed in
  DIR *d;
  struct dirent *dir;
  d = opendir(filePath);

  //read name of each room file
  //name each room struct in struct array
  if(d)
  {
    while ((dir = readdir(d)) != NULL)
    {
      if( strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..") )
      {
        strcpy(roomList[i].name, dir->d_name);
        i++;
      }
    }
  }
  //close the directory
  closedir(d);

  //rebuild the outgoing connections for each room;
  for (i=0; i<NUM_ROOMS; i++)
  {
    buildRoom(filePath, roomList[i].name, i);
  }

  return;
}


/***************************************************************
  buildRoom:
  Takes c-string of a file path, name of room file to open, index of room in roomList
  Opens room file, reads which outgoing connections it should have,
  Rebuilds those connections and returns
***************************************************************/
void buildRoom(char* parentPath, char* roomName, int roomIdx)
{
  int i=0;
  char filePath[100], line[100];

  //concatenate the path and  into a useable filepath
  sprintf(filePath, parentPath);
  strcat(filePath, "/");
  strcat(filePath, roomName);
  //create file pointer and open file in read mode
  FILE* myFile;
  myFile = fopen(filePath, "r");

  if(myFile == NULL)
    exit(1);


  //read the file line by line
  while (fgets(line, 100, myFile) != NULL)
  {
    char* search = strtok(line, " ");

    //if the string "CONNECTION" is encountered, break apart at a space or newline
    if(strcmp(search, "CONNECTION") == 0)
    {
      search = strtok(NULL, " ");
      search = strtok(NULL, "\n");
      //search roomList for a room name that matches the tokenized string
      for(i=0; i<NUM_ROOMS; i++)
      {
        //if found, build outgoing connection and increment # of connections
        if (strcmp(search, roomList[i].name) == 0)
        {
          roomList[roomIdx].outConnections[roomList[roomIdx].numConnections] = &roomList[i];
          roomList[roomIdx].numConnections++;
        }
      }
    }

    //else search for the string "ROOM TYPE"
    else //if(strcmp(search, "ROOM TYPE") == 0)
    {
      search = strtok(NULL, " ");
      search = strtok(NULL, "\n");
      //set room type based on found string
      strcpy(roomList[roomIdx].room_type, search);
    }
  }


  //close the file
  fclose(myFile);
  return;
}


/***************************************************************
 getStartRoom:
 Returns a pointer to the start room of the game
***************************************************************/
struct room* getStartRoom()
{
  int i;
  struct room* start;
  for (i=0; i<NUM_ROOMS; i++)
  {
    if ( strcmp(roomList[i].room_type, "START_ROOM") == 0 )
      start = &roomList[i];
  }

  return start;
}



/*****************************************************************************
                            Gameplay Functions
*****************************************************************************/
/***************************************************************
  printRoom:
  Takes pointer to a room.
  Prints room information (name, outgoing connections, prompt line) to screen.
***************************************************************/
void printRoom(struct room* a)
{
  //print the current room name
  printf("\n\nCURRENT LOCATION: %s", a->name);

  //print the outgoing connections with proper punctuation
  printf("\nPOSSIBLE CONNECTIONS:");
  int i;
  for (i=0; i < a->numConnections; i++)
  {
    //if the connection is the final one, print its name and a period
    if (i == (a->numConnections - 1))
      printf(" %s.", a->outConnections[i]->name);
    //otherwise, print its name and a comma
    else
      printf(" %s,", a->outConnections[i]->name);
   }

  printf("\nWHERE TO? >");

  return;
}


/***************************************************************
  searchRooms:
  Takes search string and pointer to room.
  Searches through room's outgoing connections.
  Returns -1 for room does not exist/connection does not exist,
  or returns index of outgoing room in current room's outConnection array.
***************************************************************/
int searchRooms(char* search, struct room* a)
{
  int i, ret= -1;

  for (i=0; i<a->numConnections; i++)
  {
    //if the search string matches the name of an outgoing room, return index
    if ( strcmp(search, a->outConnections[i]->name) == 0 )
    {
      ret = i;
      break;
    }
  }

  //otherwise, return -1
  return ret;
}


/***************************************************************
  getTime:
  Calculates current time and outputs to a .txt file.
***************************************************************/
void* getTime()
{
  struct tm *sTm;
  char buffer[100];
  FILE* timeFile;
  //create and write the file with the current time
  timeFile = fopen("currentTime.txt", "w+");

  //get the time from the system
  time_t timeNow = time(0);
  sTm = gmtime(&timeNow);

  //format the time string
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", sTm);
  //output the time string to the file and close it
  fputs(buffer, timeFile);
  fclose(timeFile);

  return;
}


/***************************************************************
  outputTime:
  Reads the time file from the current directory and prints to screen.
***************************************************************/
void outputTime()
{
  char buffer[100];
  FILE* timeFile;

  //open the file and read from it
  timeFile = fopen("currentTime.txt", "r");

  //if the time file was not readable or did not exist, say so
  if (timeFile == NULL)
    perror("Time file not found\n");

  //otherwise, open it, read the contents, and output to screen
  else
  {
    fgets(buffer, 100, timeFile);
    printf("\n%s\n", buffer);
    fclose(timeFile);
  }
}


/***************************************************************
  threadFun:
  Creates second thread that can be activated by calling getTime.
***************************************************************/
void threadFun()
{
  // I used this source to help me with this function: http://www.yolinux.com/TUTORIALS/LinuxTutorialPosixThreads.html
  pthread_t threading;
  pthread_mutex_init(&mutex, NULL);
  pthread_mutex_lock(&mutex);

  int threadID = pthread_create (&threading, NULL, getTime, NULL);
  pthread_mutex_unlock(&mutex);
  pthread_mutex_destroy(&mutex);

  usleep(50);
}



int main()
{
  int i,
      roomIdx,
      stepsTaken = 0,
      bufsize = 25;

  char* victoryPath[100];
  char roomString[100];


  //read the files in from the most recent room directory, rebuild the room map
  readFileNames();
  //get the start room
  struct room* curRoom;
  curRoom = getStartRoom();


  //as long as current room is not end room, keep looping
  while ( strcmp(curRoom->room_type, "END_ROOM") )
  {
    //as long as user does not enter a valid room name, keep looping
    do
    {
      //print the room data and prompt user for input
      printRoom(curRoom);

      //get user input, give them time as long as they ask for time
      do{
        scanf("%s", roomString);
        if ( strcmp(roomString, "time") == 0 )
        {
          //lock this thread, activate the other thread to make time file
          threadFun();
          outputTime();
          printf("\nWHERE TO? >");
        }
      } while ( strcmp(roomString, "time") == 0);


      //search outgoing connections for user-input room name
      roomIdx = searchRooms(roomString, curRoom);
      //if user entered invalid string, re-prompt for input
      if (roomIdx < 0)
        printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.");

    } while(roomIdx < 0);


    //move to new room
    curRoom = curRoom->outConnections[roomIdx];
    //record name of new room to print later and increment number of steps
    victoryPath[stepsTaken] = curRoom->name;
    stepsTaken++;
  }


  //user wins! print victory message
  printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!");
  printf("\nYOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", stepsTaken);

  //and print the victory path
  for(i=0; i<stepsTaken; i++)
  {
    printf("%s\n", victoryPath[i]);
  }


  return 0;
}
   