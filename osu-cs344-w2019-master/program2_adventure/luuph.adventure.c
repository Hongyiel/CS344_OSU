/**
 * luuph.adventure.c
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
#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <assert.h>

/**
 * Preprocessor directives to avoid long expressions and "magic" values
 */
#define ONID_USERNAME (char*)"luuph"
#define MAX_DIRNAME_LEN (int)32
#define NUM_ROOMS (int)7
#define MAX_NAME_LEN (int)9
#define MAX_OUTBOUNDS (int)6
#define MAX_STR_INPUT_LEN (int)256
#define TIME_FILE_NAME (char*)"currentTime.txt"
#define MAX_TIME_STR_LEN (int)50

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

typedef struct DynamicIntArray {
    int* values;
    int size;
    int capacity;
} DynIntArr;

/**
 * Global mutex flag
 */
pthread_mutex_t time_mutex = PTHREAD_MUTEX_INITIALIZER;

void* MainTimeThread(void* argument);
void InitRooms(Room* rooms, int size);
void GetNewestRoomDir(char* dirname);
bool IsNewestRoomDir(char* dirname, time_t* current_newest);
void ReadRoomsFromDir(char* dirname, Room* rooms);
void ReadRoomFromFile(char* filepath, Room* rooms, int* size);
void PlayGame(Room* rooms);
int GetUserInput(Room* rooms, int index, bool is_short_menu);
void PrintTimeToFile(char* filename);
void GetTimeFromFile(char* filename, char* result);
int FindRoomByName(Room* rooms, int size, char* name);
int FindStartRoomIndex(Room* rooms, int size);
void InitDynIntArr(DynIntArr* arr, int capacity);
void PushBackDynIntArr(DynIntArr* arr, int value);
void DeleteDynIntArr(DynIntArr* arr);

int main(void) {
    /* find the newest room directory */
    char room_dirname[MAX_DIRNAME_LEN];
    GetNewestRoomDir(room_dirname);

    /* read the rooms in the directory into the  rooms  array */
    Room rooms[NUM_ROOMS];
    InitRooms(rooms, NUM_ROOMS);
    ReadRoomsFromDir(room_dirname, rooms);

    PlayGame(rooms);

    /* clean up */
    pthread_mutex_destroy(&time_mutex);

    return 0;
}

/**
 * This the startup function for the  time_thread  defined globally.
 */
void* MainTimeThread(void* argument) {
    pthread_mutex_lock(&time_mutex);

    char* arg = (char*)argument;
    PrintTimeToFile(arg);

    pthread_mutex_unlock(&time_mutex);

    return NULL;
}

/**
 * Initializes all rooms inside the  rooms  array of size  size  to default
 * values.
 *
 * The  rooms  array will be modified after this function returns.
 */
void InitRooms(Room* rooms, int size) {
    assert(rooms);

    int i, j;
    for (i = 0; i < size; i++) {
        /* init name */
        for (j = 0; j < MAX_NAME_LEN; j++) rooms[i].name[j] = '\0';

        /* init outbounds */
        for (j = 0; j < MAX_OUTBOUNDS; j++) rooms[i].outbounds[j] = NULL;
        rooms[i].num_outbounds = 0;

        /* init type. There's no default type, so put in a dummy one */
        rooms[i].type = MID_ROOM;
    }
}

/**
 * Finds the newest room directory created in the current directory and copies
 * its name to  dirname .
 *
 * Argument:
 *   dirname  a string to hold the name of the newest room directory
 *
 * dirname  will be modified to hold the name of the newest room directory.
 */
void GetNewestRoomDir(char* dirname) {
    assert(dirname);

    /* open the current directory and see what's in it */
    struct dirent* entry;
    DIR* root_dir = opendir(".");
    if (root_dir) {
        /* run through all entries of the directory */
        time_t newest_mod_time = 0;
        while ((entry = readdir(root_dir))) {
            /* if there is a newer directory found, mark that directory */
            if (IsNewestRoomDir(entry->d_name, &newest_mod_time)) {
                strcpy(dirname, entry->d_name);
            }
        }

        closedir(root_dir);
    }
}

/**
 * Returns  true  if the directory named  dirname  satisfies both conditions:
 * - has a valid room directory name format, that is
 *   ONID_USERNAME.rooms.PROCESS_ID
 * - is the most recently modified directory in the scope, that is the time of
 *   last modified is greater than  current_newest
 * Returns  false  otherwise.
 *
 * Arguments:
 *   dirname         a string to hold the name of the current room directory
 *   current_newest  the current newest (largest) last-modified time
 *
 * Whenever a there is a new latest-modifed time, the value of  current_newest
 * will be updated.
 */
bool IsNewestRoomDir(char* dirname, time_t* current_newest) {
    assert(dirname && current_newest);

    /* get statistics about this directory */
    struct stat st;
    int stat_status = stat(dirname, &st);

    /* get a temporary string to hold dirname since it will be split later */
    char dirname_tmp[MAX_DIRNAME_LEN];
    strcpy(dirname_tmp, dirname);

    /* if  stat()  does not succeed or if not a directory, return false */
    if (stat_status != 0 || !S_ISDIR(st.st_mode)) return false;

    /* divide the name into tokens with '.' being the delimiter */
    char* name_tok = strtok(dirname_tmp, ".");  /* get 1st token */

    /* if 1st token does not exist or not match name format, return false */
    if (!name_tok || strcmp(name_tok, ONID_USERNAME) != 0) return false;

    /* continue getting the 2nd token */
    name_tok = strtok(NULL, ".");

    /* if 2nd token does not exist or not match name format, return false */
    if (!name_tok || strcmp(name_tok, "rooms") != 0) return false;

    /* continue getting the 3rd token */
    name_tok = strtok(NULL, ".");

    /* if 3rd token does not exist, return false */
    if (!name_tok) return false;

    int i, tok_len = strlen(name_tok);

    /* if 3rd token contains a non-digit character, return false */
    for (i = 0; i < tok_len; i++)
        if (!isdigit(name_tok[i])) return false;

    /* if there are more than 3 tokens, return false */
    if (strtok(NULL, ".")) return false;

    /* if pass all tests above,  dirname  is the name of a room directory
     * now check if this directory's time of last modified is greater than the
     * current newest time. If so, this directory is the newest.
     */
    if ((double)difftime(st.st_mtime, *current_newest) > 0) {
        *current_newest = st.st_mtime;
        return true;
    } else {
        return false;
    }
}

/**
 * Reads the room files in the provided  dirname  directory and parses the rooms
 * into the  rooms  array.
 *
 * Arguments:
 *   dirname  the name of the newest room directory
 *   rooms    an array of Room structures to hold  NUM_ROOMS  parsed rooms
 *
 * rooms  will be modified to hold the parsed rooms from the room files.
 */
void ReadRoomsFromDir(char* dirname, Room* rooms) {
    assert(dirname && rooms);

    struct dirent* entry;
    struct stat st;
    char roomfile_path[MAX_DIRNAME_LEN * 2];

    /* open the  dirname  directory */
    DIR* root_dir = opendir(dirname);

    /* do nothing if directory cannot be read */
    if (!root_dir) return;

    /* run through all entries of the directory */
    int num_rooms = 0;
    while ((entry = readdir(root_dir))) {
        /* construct a path to the entry */
        strcpy(roomfile_path, dirname);
        strcat(roomfile_path, "/");
        strcat(roomfile_path, entry->d_name);

        /* if this entry is is a regular file, it is a room file */
        if (stat(roomfile_path, &st) == 0 && S_ISREG(st.st_mode))
            ReadRoomFromFile(roomfile_path, rooms, &num_rooms);
    }

    closedir(root_dir);
}

/**
 * Reads a room file in the provided  filepath  and parses the room's info
 * into the  rooms  array.
 *
 * Arguments:
 *   filepath  the complete path to the file
 *   rooms     an array of Room structures to hold  NUM_ROOMS  parsed rooms
 *   size      the current size of the  rooms  array.
 *
 * Room file format:
 * ROOM NAME: <room name>
 * CONNECTION 1: <room name>
 * <...>
 * ROOM TYPE: <room type>
 *
 * rooms  and  size  will be modified to hold the parsed rooms from the room
 * files and to hold the size of the  rooms  array (respectively).
 */
void ReadRoomFromFile(char* filepath, Room* rooms, int* size) {
    assert(filepath && rooms);

    FILE* roomfile = fopen(filepath, "r");
    assert(roomfile);

    Room* current_room = NULL;
    char line[MAX_DIRNAME_LEN];

    /* read the file line by line */
    while (fgets(line, sizeof(line), roomfile)) {
        /* tokenize the line by whitespace */
        strtok(line, " ");  /* 1st token -> ignore */
        char* line_tok2 = strtok(NULL, " ");  /* 2nd token */
        char* line_tok3 = strtok(NULL, " ");  /* 3rd token */

        /* trim trailing colon off the 2nd token */
        line_tok2[strlen(line_tok2) - 1] = '\0';
        /* trim trailing EOL off the 3rd token */
        line_tok3[strlen(line_tok3) - 1] = '\0';

        /* read room information */
        if (strcmp(line_tok2, "NAME") == 0) {
            /* try to find the room with this name first */
            int room_idx = FindRoomByName(rooms, *size, line_tok3);

            /* if the room does not exist, this is a new room */
            if (room_idx == -1) {
                current_room = &rooms[*size];
                (*size)++;
            } else {
                current_room = &rooms[room_idx];
            }

            strcpy(current_room->name, line_tok3);
        } else if (strcmp(line_tok2, "TYPE") == 0) {
            /* room name is always the first to be read, so there must always be
             * a "current room" after that */
            assert(current_room);

            if (strcmp(line_tok3, "MID_ROOM") == 0) {
                current_room->type = MID_ROOM;
            } else if (strcmp(line_tok3, "START_ROOM") == 0) {
                current_room->type = START_ROOM;
            } else {
                current_room->type = END_ROOM;
            }
        } else {  /* not name & not type -> this is a connection */
            /* room name is always the first to be read, so there must always be
             * a "current room" after that */
            assert(current_room);

            int outbound_arr_idx = atoi(line_tok2) - 1;

            /* find the room with the same name as the connection name */
            int outbound_idx = FindRoomByName(rooms, *size, line_tok3);

            /* if this outbound target room does not exist, create one */
            if (outbound_idx == -1) {
                strcpy(rooms[*size].name, line_tok3);

                /* point the outbound of the current room to this new room */
                current_room->outbounds[outbound_arr_idx] = &rooms[*size];

                /* increase size */
                (*size)++;
            } else {
                /* if the outbound target exists, point to it */
                current_room->outbounds[outbound_arr_idx] = &rooms[outbound_idx];
            }

            /* increase the number of outbounds */
            current_room->num_outbounds++;
        }
    }

    fclose(roomfile);
}

/**
 * Plays the game with the provided  rooms  array consisting of rooms.
 * The game starts at a  START_ROOM  in the  rooms  array and won't stop until
 * the user reaches the  END_ROOM  in the  rooms  array.
 *
 * The  rooms  array is guaranteed to have exactly one  START_ROOM  and one
 * END_ROOM .
 */
void PlayGame(Room* rooms) {
    assert(rooms);

    /* start at  START_ROOM */
    int room_idx = FindStartRoomIndex(rooms, NUM_ROOMS);  /* current room */
    int next_room_idx;  /* hold user-input next room name */

    /* keep track of the room index path
     * use dynamic array as the user can go into an uncountable loop */
    DynIntArr room_idx_path;
    InitDynIntArr(&room_idx_path, NUM_ROOMS);

    while (true) {
        /* save this room's index */
        PushBackDynIntArr(&room_idx_path, room_idx);

        /* if this room is the  END_ROOM , game is over*/
        if (rooms[room_idx].type == END_ROOM) break;

        /* keep prompting the user for outbound room name until valid input */
        do {
            next_room_idx = GetUserInput(rooms, room_idx, next_room_idx == -2);

            /* if invalid input, print error message */
            if (next_room_idx == -1) {
                printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
            } else if (next_room_idx == -2) {
                /* if user wants to see time, get time on a new thread */
                pthread_t time_thread;
                assert(pthread_create(&time_thread, NULL, MainTimeThread,
                                      (void*)TIME_FILE_NAME) == 0);

                /* wait for time thread to complete */
                assert(pthread_join(time_thread, NULL) == 0);

                /* get time from the file and print to terminal */
                char time_str[MAX_TIME_STR_LEN];
                GetTimeFromFile(TIME_FILE_NAME, time_str);
                printf("%s\n\n", time_str);
            }
        } while (next_room_idx == -1 || next_room_idx == -2);

        /* being here means the user has entered valid outbound room name,
         * so move to the next room */
        room_idx = next_room_idx;
    }

    /* game is over, print path taken, number of steps, and victory message */
    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
    printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n",
           room_idx_path.size - 1);
    int i;
    for (i = 1; i < room_idx_path.size; i++)
        printf("%s\n", rooms[room_idx_path.values[i]].name);

    DeleteDynIntArr(&room_idx_path);
}

/**
 * Gets the user's raw choice of outbound for the current rooms[index].
 *
 * Arguments:
 *   rooms          the array consisting of room structures
 *   index          the index of the current room the user is in
 *   is_short_menu  if only needs to print "WHERE TO? >" prompt
 *
 * Returns:
 *   The index of the outbound room if the user enters correctly, OR
 *   -1 if the user enters invalid room outbound name.
 */
int GetUserInput(Room* rooms, int index, bool is_short_menu) {
    assert(rooms && index >= 0);

    int i;
    int num_outbounds = rooms[index].num_outbounds;

    /* print menu */
    if (!is_short_menu) {
        printf("CURRENT LOCATION: %s\n", rooms[index].name);

        printf("POSSIBLE CONNECTIONS:");
        for (i = 0; i < num_outbounds; i++) {
            printf(" %s", rooms[index].outbounds[i]->name);
            /* print separator ',' if not finished, otherwise print '.' and EOL */
            if (i < num_outbounds - 1) {
                printf(",");
            } else {
                printf(".\n");
            }
        }
    }

    printf("WHERE TO? >");

    /* get user's raw input */
    char name_input[MAX_STR_INPUT_LEN];
    fgets(name_input, sizeof(name_input), stdin);

    printf("\n");

    /* trim the trailing EOL character */
    unsigned int name_input_len = strlen(name_input);
    if (name_input[name_input_len - 1] == '\n')
        name_input[--name_input_len] = '\0';

    /* if user types in "time", return -2 */
    if (strcmp(name_input, "time") == 0) return -2;

    /* illegal input if more than  MAX_NAME_LEN  characters */
    if (strlen(name_input) > MAX_NAME_LEN) return -1;

    /* only find the room with this name if the name is in the outbound list */
    for (i = 0; i < num_outbounds; i++) {
        if (strcmp(name_input, rooms[index].outbounds[i]->name) == 0) {
            return FindRoomByName(rooms, NUM_ROOMS, name_input);
        }
    }

    /* if cannot find the name in the outbound list, illegal input */
    return -1;
}

/**
 * Prints the current local time to a file named  filename .
 */
void PrintTimeToFile(char* filename) {
    FILE* timefile = fopen(filename, "w");
    assert(timefile);

    time_t t;
    struct tm* thetime;
    char time_str[MAX_TIME_STR_LEN];

    time(&t);
    thetime = localtime(&t);

    strftime(time_str, sizeof(time_str), "%l:%M%P, %A, %B %e, %Y", thetime);

    fprintf(timefile, "%s\n", time_str);

    fclose(timefile);
}

/**
 * Gets the time in string format in the file named  filename  and copy it to
 * the  result  string.
 *
 * The  result  string will be modified after the function returns.
 */
void GetTimeFromFile(char* filename, char* result) {
    FILE* timefile = fopen(filename, "r");
    assert(timefile);

    /* read the one (and supposedly the only) line in file */
    char time_str[MAX_TIME_STR_LEN];
    fgets(time_str, sizeof(time_str), timefile);

    /* trim the trailing newline if any */
    unsigned int time_str_len = strlen(time_str);
    if (time_str[time_str_len - 1] == '\n')
        time_str[--time_str_len] = '\0';

    /* copy to result */
    strcpy(result, time_str);

    fclose(timefile);
}

/**
 * Returns the index of the room with matched  name  in the  rooms  array.
 * Returns -1 if not found the room whose name is  name .
 */
int FindRoomByName(Room* rooms, int size, char* name) {
    assert(rooms && name);

    /* search through all the rooms */
    int i;
    for (i = 0; i < size; i++)
        if (strcmp(rooms[i].name, name) == 0) return i;

    /* return -1 if no name matches */
    return -1;
}

/**
 * Returns the index of the room with  START_ROOM  type in the  rooms  array.
 * Returns -1 if not found the  START_ROOM  (fail-safe).
 */
int FindStartRoomIndex(Room* rooms, int size) {
    assert(rooms);

    /* search through all the rooms */
    int i;
    for (i = 0; i < size; i++)
        if (rooms[i].type == START_ROOM) return i;

    /* return -1 if no starting room (fail-safe -- not supposed to happen) */
    return -1;
}

/**
 * Dynamically allocates an array of integers with the given capacity.
 *
 * Arguments:
 *   arr       the dynamic-array-of-integers structure that holds the array
 *   capacity  the initial capacity of the array
 *
 * The array inside  arr  will be given  capacity  slots of heap memory.
 */
void InitDynIntArr(DynIntArr* arr, int capacity) {
    assert(arr && capacity > 0);

    /* allocate  capacity  slots of memory for the array */
    arr->values = (int*)malloc(sizeof(int) * capacity);
    assert(arr->values);  /* make sure allocation was succesful */

    /* init the array's size and capacity */
    arr->size = 0;
    arr->capacity = capacity;
}

/**
 * Pushes a value to the back of the dynamic array (doubles the capacity if
 * needed).
 *
 * Arguments:
 *   arr    the dynamic-array-of-integers structure that holds the array
 *   value  the value to be pushed to the back of the array
 *
 * The array inside  arr  will have the additional value  value  at its back.
 * The capacity of the array will be doubled if necessary.
 */
void PushBackDynIntArr(DynIntArr* arr, int value) {
    assert(arr && arr->values);

    int i;

    /* before adding, must double the capacity if array is full */
    if (arr->size == arr->capacity) {
        /* init a new array with double capacity */
        int* new_values = (int*)malloc(sizeof(int) * arr->capacity * 2);
        assert(new_values);  /* make sure allocation was successful */

        /* move all values to the new array */
        for (i = 0; i < arr->size; i++) new_values[i] = arr->values[i];

        /* destroy the old array and set pointer to the new one */
        free(arr->values);
        arr->values = new_values;

        /* set the new capacity */
        arr->capacity *= 2;
    }

    /* add the new value to the back of the array */
    arr->values[arr->size++] = value;
}

/**
 * Deallocates the dynamic array  arr->values  from the heap memory,
 * only the array but not the container (the structure  arr ).
 */
void DeleteDynIntArr(DynIntArr* arr) {
    /* do nothing if the structure  arr  or the array  arr->values  is NULL */
    if (!arr || !arr->values) return;

    /* free up memory and reset parameters */
    free(arr->values);
    arr->size = 0;
    arr->capacity = 0;
}
