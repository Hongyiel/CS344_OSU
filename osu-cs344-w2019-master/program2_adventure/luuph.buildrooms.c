/**
 * luuph.buildrooms.c
 *
 * Phi Luu
 *
 * Oregon State University
 * CS_344_001_W2019 Operating Systems 1
 * Program 2: Adventure
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>

/**
 * Preprocessor directives to avoid long expressions and "magic" values
 */
#define PERM_755 (mode_t)(S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#define PERM_644 (mode_t)(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define ONID_USERNAME (char*)"luuph"
#define PID (int)getpid()
#define MAX_DIRNAME_LEN (int)32
#define NUM_ROOMS (int)7
#define MAX_ROOMS (int)10
#define MAX_NAME_LEN (int)9
#define MIN_OUTBOUNDS (int)3
#define MAX_OUTBOUNDS (int)6

/**
 * Define new types
 */
typedef enum RoomType {
    START_ROOM,
    MID_ROOM,
    END_ROOM
} RoomType;

typedef struct Room {
    char name[MAX_NAME_LEN];
    struct Room* outbounds[MAX_OUTBOUNDS];
    int num_outbounds;
    RoomType type;
} Room;

/**
 * Global, hard-coded constants
 */
const char* FILE_NAMES[NUM_ROOMS] = {
    "Administration_Room",
    "Archive_Room",
    "Garage_Room",
    "Lobby_Room",
    "Meeting_Room",
    "Press_Room",
    "Visa_Room"
};

const char* ROOM_NAMES[MAX_ROOMS] = {
    "Admin",
    "Archives",
    "CCTV",
    "Cafe",
    "Garage",
    "Lobby",
    "Meeting",
    "Press",
    "Tellers",
    "Visa"
};

const char* ROOM_TYPES[END_ROOM + 1] = {
    "START_ROOM",
    "MID_ROOM",
    "END_ROOM"
};

void GenerateRooms(Room* rooms);
bool IsGraphComplete(Room* rooms);
void AddRandomConnection(Room* rooms);
bool IsSameRoom(Room* x, Room* y);
void ConnectRooms(Room* x, Room* y);
bool CanAddOutboundFrom(Room* x);
bool ConnectionExists(Room* x, Room* y);
void MakeRoomDir(char* dirname);
void OutputRoomsToFiles(char* dirname, Room* rooms);
void ShuffleIntArray(int* arr, int size);

int main(void) {
    srand(time(0));

    /* make room directory */
    char room_dirname[MAX_DIRNAME_LEN];
    MakeRoomDir(room_dirname);

    /* generate  NUM_ROOMS  rooms and set up connections between them */
    Room rooms[NUM_ROOMS];
    GenerateRooms(rooms);

    /* make room files and write contents to each of the files */
    OutputRoomsToFiles(room_dirname, rooms);

    return 0;
}

/**
 * Makes  NUM_ROOMS  rooms, sets up their names, types, and connections,
 * and outputs the results in the  rooms  array.
 *
 * Argument:
 *   rooms  the array to hold generated rooms
 *
 * The  rooms  array will be modified to hold the newly generated rooms.
 */
void GenerateRooms(Room* rooms) {
    assert(rooms);

    int i, j;

    /* array containing random indices between [0..MAX_ROOMS - 1] */
    int rand_room_name[MAX_ROOMS];
    ShuffleIntArray(rand_room_name, MAX_ROOMS);

    /* array containing random indices between [0..NUM_ROOMS - 1] */
    int rand_room_idx[NUM_ROOMS];
    ShuffleIntArray(rand_room_idx, NUM_ROOMS);

    /* create  NUM_ROOMS  rooms */
    for (i = 0; i < NUM_ROOMS; i++) {
        /* randomly pick a name for this room */
        for (j = 0; j < MAX_NAME_LEN; j++) rooms[i].name[j] = '\0';
        strcpy(rooms[i].name, ROOM_NAMES[rand_room_name[i]]);

        /* randomly pick a type for this room */
        if (i == rand_room_idx[0]) {
            rooms[i].type = START_ROOM;
        } else if (i == rand_room_idx[NUM_ROOMS - 1]) {
            rooms[i].type = END_ROOM;
        } else {
            rooms[i].type = MID_ROOM;
        }

        /* not create any connection just yet */
        rooms[i].num_outbounds = 0;
        for (j = 0; j < MAX_OUTBOUNDS; j++) rooms[i].outbounds[j] = NULL;
    }

    /* keep adding random connections until the room graph is complete */
    while (!IsGraphComplete(rooms)) {
        AddRandomConnection(rooms);
    }
}

/**
 * Returns  true  if all rooms have [MIN_OUTBOUNDS..MAX_OUTBOUNDS] outbounds.
 * Returns  false  otherwise.
 */
bool IsGraphComplete(Room* rooms) {
    assert(rooms);

    /* search all rooms for illegal number of outbounds */
    int i;
    for (i = 0; i < NUM_ROOMS; i++) {
        int outbounds = rooms[i].num_outbounds;
        if (outbounds < MIN_OUTBOUNDS || MAX_OUTBOUNDS < outbounds)
            return false;
    }

    /* if no room has illegal number of outbounds, graph is complete */
    return true;
}

/**
 * Pick 2 random (and available) rooms from the  rooms  array and add a
 * connection between the two.
 */
void AddRandomConnection(Room* rooms) {
    assert(rooms);

    /* x  and  y  hold addresses of two randomly picked available rooms */
    Room* x;
    Room* y;

    /* keep picking a random room until there is one available for connection */
    do {
        x = &rooms[rand() % NUM_ROOMS];
    } while (!CanAddOutboundFrom(x));

    /* do the same for y, but with more conditions */
    do {
        y = &rooms[rand() % NUM_ROOMS];
    } while (!CanAddOutboundFrom(y) || IsSameRoom(x, y) || ConnectionExists(x, y));

    ConnectRooms(x, y);  /* make x -> y and y -> x connections */
}

/**
 * Returns  true  if  x  and  y  exist and point to the same room structure.
 * Returns  false  otherwise.
 */
bool IsSameRoom(Room* x, Room* y) {
    return (x && y) && (x == y);
}

/**
 * Establishes outbounds x -> y and y -> x. Checking connection capability is
 * applied.
 */
void ConnectRooms(Room* x, Room* y) {
    assert(x && y);

    /* do nothing if either x or y cannot have any more connections */
    if (!CanAddOutboundFrom(x) || !CanAddOutboundFrom(y)) return;

    /* establish the connection otherwise */
    x->outbounds[x->num_outbounds++] = y;  /* x -> y */
    y->outbounds[y->num_outbounds++] = x;  /* y -> x */
}

/**
 * Returns  true  if number of outbounds of  x  is less than  MAX_OUTBOUNDS .
 * Returns  false  otherwise.
 */
bool CanAddOutboundFrom(Room* x) {
    assert(x);
    return x->num_outbounds < MAX_OUTBOUNDS;
}

/**
 * Returns  true  if (x -> y and y -> x). Returns  false  otherwise.
 *
 * Assumes that either (x -> y and y -> x) or there is no connection
 * (i.e. assumes no illegal half-connections).
 */
bool ConnectionExists(Room* x, Room* y) {
    assert(x && y);

    /* search through  from 's outbound connections */
    int i;
    for (i = 0; i < x->num_outbounds; i++) {
        /* assume that if x -> y then y -> x to simplify checking */
        if (x->outbounds[i] == y) return true;
    }

    /* if cannot find any connection, return false */
    return false;
}

/**
 * Generates a directory to contain room files using the provided name
 *
 * Format: Directory name = ONID_USERNAME.rooms.PROCESS_ID
 *
 * Argument:
 *   dirname  the string to contain the name of the directory
 *
 * The string  dirname  will be modified to the above format.
 */
void MakeRoomDir(char* dirname) {
    assert(dirname);

    /* Safely init empty  dirname  string */
    int i;
    for (i = 0; i < MAX_DIRNAME_LEN; i++) dirname[i] = '\0';

    /* ensure directory does not exist */
    struct stat st = {0};
    if (stat(dirname, &st) == -1) {
        /* construct the room directory name = ONID.rooms.PID */
        sprintf(dirname, "%s.%s.%d", ONID_USERNAME, "rooms", PID);
        /* create a directory with rwxr-xr-x permission to contain room files */
        mkdir(dirname, PERM_755);
    }
}

/**
 * Creates  NUM_ROOMS  room files in a directory with a given directory
 * name and writes contents from the  rooms  array to those files.
 *
 * Argument:
 *   dirname  the name of the directory that contains the room files.
 *   rooms    an array consisiting of  NUM_ROOMS  room structures.
 *
 * NUM_ROOMS  new files will be created inside directory  dirname .
 */
void OutputRoomsToFiles(char* dirname, Room* rooms) {
    assert(dirname && rooms);

    int i, j;

    /* generate  NUM_ROOMS  room files and write contents to them */
    for (i = 0; i < NUM_ROOMS; i++) {
        /* construct a file name for the room file */
        char filename[MAX_DIRNAME_LEN * 2];
        for (j = 0; j < MAX_DIRNAME_LEN * 2; j++) filename[j] = '\0';
        strcat(filename, dirname);
        strcat(filename, "/");
        strcat(filename, FILE_NAMES[i]);

        /* open the file for writing */
        FILE* roomfile = fopen(filename, "w");
        assert(roomfile);

        /* room name */
        fprintf(roomfile, "ROOM NAME: %s\n", rooms[i].name);

        /* room outbound connections */
        for (j = 0; j < rooms[i].num_outbounds; j++) {
            fprintf(roomfile, "CONNECTION %d: %s\n", j + 1,
                    rooms[i].outbounds[j]->name);
        }

        /* room type */
        fprintf(roomfile, "ROOM TYPE: %s\n", ROOM_TYPES[rooms[i].type]);

        /* clean up */
        fclose(roomfile);
    }
}

/**
 * Helper function: Shuffles the integers of an array.
 *
 * Arguments:
 *   arr   the array of integers to be shuffled, indexed [0..size - 1]
 *   size  the size of the array
 *
 * arr  will be modified (more accurately, shuffled).
 */
void ShuffleIntArray(int* arr, int size) {
    assert(arr);

    int i;

    /* init the array in natural order */
    for (i = 0; i < size; i++) arr[i] = i;

    /* shuffle the array */
    for (i = 0; i < size; i++) {
        /* generate a random number in range [i, size - 1] */
        int rand_idx = i + rand() % (size - i);
        /* swap the element at this index with one at i */
        int tmp = arr[i];
        arr[i] = arr[rand_idx];
        arr[rand_idx] = tmp;
    }
}
