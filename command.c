#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "command.h"

#define PMAX 8
#define MSG_MAX 16

const char cmd_array[PMAX][MSG_MAX] = {"send","name","help","kill",0};
const char array_help[] = {"commands: send msg / name new_name / kill / help"};



int is_command(const char *array)
{
    int i = 0;

    int output = -EINVAL;

    for(i = 0 ; (char)cmd_array[i] != 0 ; i++)
    {
    
        if(strcmp(array,cmd_array[i]) == 0)
            return i;
        
    }

    return -EINVAL;
}

void command_help(char * array)
{
    strcpy(array,array_help);
}