/* Name: Meghan Dougherty
 * Date: 5/7/2019
 * Description: First creates a new directory called 
 * dougherm.rooms.<Process id of rooms program>. Then it will create 7 files
 * within the new directory titled room_1 ... room_7. Each file will contain
 * information about that room including the room title, room type, and room 
 * connections.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

//room struct containing the necessary member variables
struct Room
{

	char* type;
	int numConnects;
	struct Room *connections[6];
	char* name;
};
/* roomInit: takes a room struct, a name, and a type as arguments.
 * initializes the appropriate member variables of the room .
 */ 
void roomInit(struct Room *newRoom, char* name, char* type)
{
	newRoom->name = name;
	newRoom->type = type;
	newRoom->numConnects = 0;
	int i = 0;

	//initialize the connector pointers to NULL
	do
	{
		newRoom->connections[i] = NULL;
		i++;
	}while(i<6);


}

/* roomCreate: takes an array of room structs, an array of room names, and an
 * array of room types as arguments. Fills the room array with fully 
 * initialized rooms.
 */ 
void roomCreate(struct Room* rooms[], char* roomName[], char* roomType[])
{
	int i =0;
	int type;
	int name;

	//an array to keep track of chosen names so that the same 
	//name is not selected twice
	int isChosen[7] = {0,0,0,0,0,0,0};

	//Create the 7 rooms for the adventure game. 
	while(i<7)
	{
		type=-1;
		//allocate the memory for the room.
		rooms[i] = malloc(sizeof (struct Room));
		
		//determine the room type. The first and last rooms
		//in the array are the start and end rooms respectively
		//otherwise, the room is a mid room.
		if(i==0)
		{
			type = 0;
		}
		else if (i == 6)
		{
			type = 2;
		}
		else
		{
			type = 1;
		}

		//Generate a random room name.
		//Check the chosen name against the already chosen room names
		//if it has been chosen, choose another. If not, continue.
		do
		{
			name = rand() %9;
		}while (isChosen[name] == 1);

		//update the chosen array for the generated name. 
		isChosen[name] = 1;

		//initialize the room.
		roomInit(rooms[i], roomName[name], roomType[type]);
		i++;
	}
}
/*isGraphFull(): takes a struct Room array as an argument. Returns 1 (true) 
 * if all rooms have  * 3 to 6 outbound connections, returns 0 (false) 
 * otherwise
 */ 
int isGraphFull(struct Room* rooms[])
{
	int i=0;
	

	//Loop through the rooms array checking if each room has more than 3 connections.
	while(i<7)
	{
		if(rooms[i]->numConnects < 3)
		{
	
			return 0;
		}
		else
			i++;
	}
	return 1;
		
		
}

/* getRandomRoom(): takes no arguments. Returns a random Room but provides
 * no validation checks.
 */ 
struct Room* getRandomRoom(struct Room* rooms[])
{
	
	//generate a random number between 0 and 6
	int i = rand() %7;
	
	//return the room at the random index. 
	return rooms[i];

}

/*canAddConnectionFrom(Room): takes a Room struct object as an argument.
 * Returns 1 (true) if a connection can be added from the room. Returns
 * 0(false) otherwise.
 */ 
int canAddConnectionFrom(struct Room* r)
{
	//Check if the number of room connections 6 or less.
	if(r->numConnects <6)
	{
		
		//return true
		return 1;
	}
	else
		//return false
		return 0;
}
/*connectionAlreadyExists(Room,Room): takes two Room struct objects as
 * arguments. Returns 1 (true) if a connection exists between the rooms.
 * returns 0(false) otherwise.
 */ 
int connectionAlreadyExists(struct Room* x, struct Room* y)
{
	int i=0;
	//Take the name of room y and compare it against the name of
	//all of the rooms x is connected to.
	while(i<6)
	{
		if(x->connections[i] == NULL)
		{
			return 0;
		}
		else if(strcmp(y->name, x->connections[i]->name)==0)
		{
			//a match is found! return true!
			return 1; 
		}
		else
			//no match, increment the index.
			i++;
	}
	//No name match found. Return false.
	return 0;
}

/*connectRoom(Room, Room): takes two Room struct objects as arguments.
 * connects the rooms together. Does not validate connection.
 */ 
void connectRoom(struct Room* x, struct Room* y)
{
	//find the next available room pointer in each room's 
	//room connection array
	int i = 0;
	

	while(x->connections[i]!= NULL && i<6)
	{
		i++;
	}
	

	//Connect the rooms by assigning the second room
	//to the free index of the first room's connection array, and vice
	//versa.
	x->connections[i] = y;
	
	
	x->numConnects++;

}

/*isSameRoom(Room, Room): takes two Room struct objects as arguments. 
 * Returns 1 (true) if the rooms are the same room. Returns 0 (false)
 * otherwise
 */ 
int isSameRoom(struct Room* x, struct Room* y)
{
	//Uses the string compare function to compare the room names
	//of the provided room objects. Returns true if the room names
	//match (same room) returns false otherwise
		
	if(strcmp(x->name, y->name)==0)
	{
		return 1;
	}
	else
		return 0;
}

/* addRoomConnection(): takes no arguments.  Adds a random, valid outbound 
 * connection from one room to another
 */ 
void addRandomConnection(struct Room* rooms[])
{

	//create two room pointers
	struct Room* x;
	struct Room* y;
	//iterator
	int i=1;
	//randomly assign x to one of the available rooms. Re-assign x if we cannot add a connection
	//from the room chosen. Loop through the random selection until we find a room that can be assigned.
	
	do
	{
		x = getRandomRoom(rooms);
		
	}while(canAddConnectionFrom(x) == 0);

	//randomly assign y to one of the available rooms. ensure that y: can connect to x, is not the same room
	//as x, and isn't already connected to x. If any of these conditions are not met, re assign y until they are.
	do
	{	
	
		y = getRandomRoom(rooms);
		
		
	}while(canAddConnectionFrom(y) == 0 || isSameRoom(x,y)==1 || connectionAlreadyExists(x,y));
	
	//connect the rooms together. 
	connectRoom(x,y);
	connectRoom(y,x);
		

}


int main()
{
	srand(time(NULL));
	struct stat st = {0};
	//hard code the room types.
	char* roomType[3] = {"START_ROOM", "MID_ROOM", "END_ROOM"};

	//hard code the room names. I chose them from a list of adjectives
	//most commonly found in H.P. Lovecraft's works.
	char* roomNames[10] = {"foetid", "putrid", "atonic", "sthenic", "discoid", 
			"eldritch", "accursed", "dank", "decadent", "stygian"};
	//an array to hold the created rooms.
	struct Room* rooms[7];

	//get the Process id
	int pid = getpid();
	//create the file prefix
	char prefix[] = "dougherm.rooms.";
	char fileName[30];

	//initialize the rooms
	roomCreate(rooms, roomNames, roomType);
	
	//generate the graph.
	while(isGraphFull(rooms)==0)
	{
		
		addRandomConnection(rooms);
	}

	//generate the directory name
	sprintf(fileName, "%s%d", prefix,pid);
	

	//If the directory does not exist, create it
	if(stat(fileName, &st) == -1)
	{
		mkdir(fileName,0755);
	}
	int i=0;
	//create the room files
	while(i<7)
	{
		//buffer for the room file name
		char roomFileName[40]={0};

		//create the room file name by combining the directory name
		//with the room name followed by the pid.
		sprintf(roomFileName, "%s/%s", fileName,rooms[i]->name);
		
		//open a new file with read and write priviledges
		FILE *file = fopen(roomFileName, "w+");
		//write the room name to the file
		fprintf(file, "ROOM NAME: %s\n", rooms[i]->name);
		int c =0;
		//loop through the room connections for the selected room.
		while(c<rooms[i]->numConnects)
		{
			//print all the rooms this room is connected to.
			fprintf(file, "CONNECTION %d: %s\n", c,rooms[i]->connections[c]->name);
			//increment the iterator.
			c++;
		}
		//print the room type.
		fprintf(file, "ROOM TYPE: %s", rooms[i]->type);
		//increment the iterator
		i++;
		//close the file.
		fclose(file);
		
	}

 
 return 0;
}
