/*Name: Meghan Dougherty
 * Date: 6/5/2019
 * Description: The header file for the helper.c functions. Thse functions are various helper
 * functions for the OTP program.
 */ 

void error(const char *msg);
int getNumChars(const char* filename);
int charToInt (char c);
char intToChar(int i);
void sendFile(int filelength,int socketFD, int fd,  char buffer [1024]);

void encode(char  message[], char key[]);
void decode(char  message[], char key[]);
