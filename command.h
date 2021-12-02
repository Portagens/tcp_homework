#ifndef COMMAND_H
#define COMMAND_H

#include <stdio.h>

#define MSG_LEN 128

typedef enum {cmd_send = 0 , cmd_name , cmd_help, cmd_kill,cmd_status,cmd_receive} cmdType_t;
typedef enum { online = 0 , afk, death} status_t;

typedef struct command command_t;
struct command
{
	unsigned int type;
	char message[MSG_LEN];
	status_t status;
}; 

/*
*   @info: Compare the array to the available commands
*
*   @paramenters: *array: Array to be compared
*   
*   @return -EINVAL if not match
*           Command index (0,1,2,..)           
*/
int is_command(const char *array);


/*
*   @info: Gets the comand list
*
*   @paramenters: *array: Array to get the list
*           
*/
void command_help(char * array);


#endif //COMMAND_H