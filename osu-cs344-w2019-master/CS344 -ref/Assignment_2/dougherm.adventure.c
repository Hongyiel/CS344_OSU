/* Name: Meghan Dougherty
 * Date: 5/6/2019
 * Description: A text driven adventure style game that reads files created by
 * the dougherm.rooms program in order to build the map. The user can move
 * through this map by specifying the room they would like to travel to. 
 * The goal is to reach the end room.
 *
 */ 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>

//Initialize global variables
char mostRecentDir[50];
char* roomTypes [] ={"START_ROOM", "MID_ROOM", "END_ROOM"};
pthread_mutex_t timeMutex;
char * timeOutputFileName = "currentTime.txt";

//define the room struct. This is slightly different from the buildrooms struct.
struct Room
{

	char* type;
	int numConnects;
	//to make nagivation a little easier, the connections array is now an array 
	//of indices instead of an array of room pointers.
	int connections[6];
	char* name;
};

//create an array of room structs
struct Room* rooms[7];

/*arrayInit: takes no arguments. Allocates memory for the room strucs in the rooms array as well as
 * initializing some of the room member variables.
 */ 
void arrayInit()
{
	int i=0;
	while(i<7)
	{	int x=0;
		//allocate memory
		rooms[i] = malloc(sizeof(struct Room));
		//Initalize all of the rooms aray values to -1
		while (x<6)
		{
			rooms[i]->connections[x] = -1;
			x++;
		}
		//initialize the room connections to 0
		rooms[i]->numConnects = 0;
		i++;
	}
	
}
/*getRecentDir: Takes no arguments. Picks the most recently created rooms 
*directory. 
*/
void getRecentDir()
{	
	//create variables
	char *prefix = "dougherm.rooms.";
	struct dirent *dirPt = malloc(sizeof(struct dirent));
	struct stat *buff = malloc(sizeof(struct stat));
	time_t newestMod;
	char currentDir[50];
	DIR *dir;
	time_t isNew = 0;

	

	//get the current working directory 
	getcwd(currentDir, sizeof(currentDir));
	//assign the current directory to the directory object 
	dir = opendir(currentDir);


	//Loop through the directories in the current working directory
	if(dir != NULL)
	{
		while(dirPt = readdir(dir))
		{
			//See if the directory matches dougherm.rooms
			if(strstr(dirPt->d_name, prefix) != NULL)
			{
				stat(dirPt->d_name, buff);
				newestMod = buff->st_mtime;
				//if the time of last modification is newer than
				//previous iterations, assign the directory 
				//name to the global.
				if(newestMod > isNew)
				{
					isNew = newestMod;
					strcpy(mostRecentDir, dirPt->d_name);
				}
			}
		}
	}

	//free allocated memory
	free(dirPt);
	free(buff);
}
/*addCon, takes to ints which relate to room array indices. creates a connection
 * within the room structs. Only creates a one way connection. 
 */
void addCon(int room1, int room2)
{
	//get the index for the connections array	
	int index = rooms[room1]->numConnects;
	
	//make the connection and increment the number of connections.
	rooms[room1]->connections[index] = room2;
	rooms[room1]->numConnects++;
}

/* getRoomNames(): takes no arguments, pulls the file names from the most
 * recent directory of dougherm.rooms.xxxxx. If the name is more than 2 
 * characters, the name is copied to the rooms array. 
 */ 
void getRoomNames()
{
	//create variables and pointers
	DIR* curDir;
	struct dirent *entry;
	int fileCount=0;

	//Allocate memory for the array.
	arrayInit();
	
	//open the most recent directory
	if((curDir=opendir(mostRecentDir)) != NULL)
	{
		//while there are still files to read
		while((entry = readdir(curDir)) != NULL)
		{
			//if the file is more than 2 characters
			if(strlen(entry->d_name)>2)
			{
				//add the file name to the rooms array.
				rooms[fileCount]->name=entry->d_name;
				fileCount++;
			}
		}
	}
	
}

/* getLabel: takes two strings, one with info and one empty. Splits the filled
 * string into a label (Either "ROOM TYPE:" or "CONNECTION:" and the value 
 * (the room name). The firs string,lable, gets the lable, the second, val, gets
 * the room name.
 */ 
void getLabel(char *label, char *val)
{
	
	int i=0;
	//tokenize the string by splitting off at the first ":"
	strtok(label, ":");
	//copy the rest of the buffer to the val string
	strcpy(val, strtok(NULL,""));

	//add '\0' to the end of both the new strings
	val[strlen(val)-1] = '\0';
	label[strlen(label)-1]='\0';

	//remove the leading " " in the value string.
	while(i<strlen(val))
	{
		val[i]=val[i+1];
		i++;
	}
	
}


/*findIndex: takes a room name as an argument. returns the index of that room
 * within the rooms array.
 */ 
int findIndex(char* room)
{
	int i=0;

	//search through the rooms array to find a room that matches the given name
	while(i<7)
	{
		if(strcmp(rooms[i]->name, room) == 0)
		{
			return i;
		}
		i++;
	}
	
}

/*formRooms(): takes no arguments. re creates the room structs based on the 
 * information stored in the room files.
 */ 
void formRooms()
{
	//create buffers
	char lineBuff[256];
	char valBuff[256];

	//create file pointer
	FILE *roomFile;
	int i=0;
	
	//fill the rooms array
	getRoomNames();
	chdir(mostRecentDir);

	while(i<7)
	{
		//open the file that matches the room name in the room array for reading
		roomFile = fopen(rooms[i]->name, "r");

		//clear the buffers
		memset(lineBuff, '\0', sizeof(lineBuff));
		memset(valBuff, '\0', sizeof(valBuff));

		//read the file line by line
		while (fgets(lineBuff, sizeof(lineBuff), roomFile) != NULL)
		{
			//tokenize the line 
			getLabel(lineBuff,valBuff);

			//Check if the line is a room type line.
			//I have used substring comparisons instead of strcmp because I was struggling 
			//with strtok inconsistently cutting off the end of lines 
			if(strstr(lineBuff, "ROOM TYP") != NULL)
			{
				//Assign the appropriate room type based on the file data.
				if(strstr(valBuff, "START")!=NULL)
				{	
					rooms[i]->type = roomTypes[0];
				}
				else if(strstr(valBuff, "MID") != NULL)
					rooms[i]->type = roomTypes[1];
				else
					rooms[i]->type = roomTypes[2];
			}
			//Check if the line is a connection line
			else if(strstr(lineBuff, "CONN") != NULL)
			{
				//find the index of the room and add the connection
				int index = findIndex(valBuff);
				addCon(i,index);
				
			}
		}
		//close the file
		fclose(roomFile);
		i++;
	}
}
/*finStartRoom(): takes no arguments. Returns the index of the room with type
 * START_ROOM.
 */ 
int findStartRoom()
{
	int i=0;
	int index= -1;

	//loop through the array to find the room that has a room type "START_ROOM"
	while(i<7)
	{
		if(strcmp(rooms[i]->type, roomTypes[0])==0)
			return i;
		else
			i++;
	}
}
/*outputPath: takes an integer array representing room indices and an integer 
 * representing the total steps taken by the player. Prints out room names
 * in the order the player visited them during the game.
 */ 
void outputPath(int path[], int totStep)
{
	int i=0;
	//Print the room names that the user went through in order
	while(i<totStep+1)
	{
		printf("%s\n", rooms[path[i]]->name);
		i++;
	}
	
}
/*getTimeFile(): takes no arguments, returns a NULL pointer. Creates and writes the
 * currentTime.txt file based on the current system time.
 */ 
void* getTimeFile()
{
	//create the file pointer and open the file
	FILE* timeFile;
	timeFile = fopen(timeOutputFileName, "w+");

	//create a buffer
	char buff[256];
	//create a time object
	struct tm *sTime;
	
	//get the current time.
	time_t now = time(0);

	//fill the time struct with the current time information
	sTime = gmtime(&now);

	//format the time information and store it in the buffer
	strftime (buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", sTime);
	
	//write the time info to the file
	fputs(buff, timeFile);
	
	//close the file
	fclose(timeFile);

	return NULL;
}
/*printTime: takes no arguments. Prints the current time by reading the 
 * information from the currentTime.txt file
 */ 
void printTime()
{
	//create a buffer and a file pointer
	char buff[256];
	FILE * timeFile;

	//clear the buffer
	memset(buff, '\0', sizeof(buff));

	//open the time file for reading
	timeFile= fopen(timeOutputFileName,"r");

	//print error if file was unable to be opened.
	if(timeFile == NULL)
	{
		printf("%s cound not be accessed\n", timeOutputFileName);
		return;
	}

	else 
	{
		//get the time information from the file and write it to the
		//buffer
		fgets(buff, 256, timeFile);
	
		//print the buffer
		printf("%s\n", buff);
	
		//close the file.
		fclose(timeFile);
	}

}

/* createNewThread(): takes no arguments. creates a new thread to write a file 
 * called currentTime.txt which stores the local time.
 */ 
int createNewThread()
{
	//create the thread
	pthread_t timeThread;

	//Initialize the mutex
	pthread_mutex_init (&timeMutex, NULL);
	//Lock until the thread completes
	pthread_mutex_lock(&timeMutex);

	//write the time file 
	int tid = pthread_create(&timeThread, NULL, getTimeFile, NULL);
	
	//join the thread to prevent runaway processes
	pthread_join(timeThread, NULL);

	//unlock the mutex
	pthread_mutex_unlock(&timeMutex);

	//destroy the mutex
	pthread_mutex_destroy(&timeMutex);



}
/* gamePlay(): takes no arguments. Runs the game play process.
 */ 
void gamePlay()
{
	//set up necessary variables
	int curStep =0;
	int path[1028];
	struct Room* curRoom;
	int roomFound=0;
	int curRoomIndex;
	char userInput[256];
	int i=0;

	//set the first step of the path to the start room
	path[curStep]=findStartRoom();

	do
	{
		//get the current room and assign the pointer
		curRoomIndex=path[curStep];
		curRoom=rooms[curRoomIndex];
		
		//print current room and its connections
		printf("CURRENT LOCATION: %s\n", curRoom->name);
		printf("POSSIBLE CONNECTIONS:");
		i=0;
		while(i<(curRoom->numConnects)-1)
		{
			printf(" %s ", rooms[curRoom->connections[i]]->name);
			
			i++;
		}
		//print the last connection separately to get the proper
		//formatting.		
		printf(" %s.\n", rooms[curRoom->connections[i]]->name);
		
		//clear the input buffer
		memset(userInput, '\0', sizeof(userInput));
		
		//Get the user input of where to move ot.
		printf("WHERE TO? >");
		scanf("%255s", userInput);
		printf("\n");

		roomFound=0;
		i=0;
		//If the user entered time		
		if (strcmp(userInput, "time") ==0)
		{
			//create the time file by starting a new thread
			createNewThread();
			//print the result and a newline for formatting
			printTime();
			printf("\n");
		}
		else
		{
			while(i<curRoom->numConnects)
			{
				//if the inputted name matches a connected room
				if(strcmp(userInput, rooms[curRoom->connections[i]]->name)==0)
				{
					//update the current step and the path
					curStep++;
					path[curStep] = curRoom->connections[i];
					//update the current room and current room index
					curRoomIndex = path[curStep];
					curRoom = rooms[curRoomIndex];
					//indicate a room was found
					roomFound=1;
					//if the current room is an end room
					if(strcmp(curRoom->type, roomTypes[2])==0)
					{
						//print victory statement, number of steps
						//and the path taken
						printf("YOU HAVE FOUND THE END ROOM!\n");
						printf("YOU TOOK %d STEPS\n", curStep+1);
						printf("Your Path was: \n");
						outputPath(path,curStep);
						return;
					}
				}
				i++;	
			}
			
			//if input does not match a room, print error and re prompt
			if(roomFound==0)
			{
			
				printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
			}
		}	
				
		
	}while(1);
}

/*cleanUp: takes no arguments. frees allocated memory.
 */ 
void cleanUp()
{
	int i=0;
	while (i<7)
	{
		free(rooms[i]);
		i++;
	}
}
int main()
{
	//create the room map
	getRecentDir();
	formRooms();
	
	//start the game
	gamePlay();

	//clean up memory
	cleanUp();
	return 0;
}

