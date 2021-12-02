#include <stdio.h>
#include <mqueue.h>   
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

/* DEFINES AND CONST VARIABLES*/
void panic(char *msg);
#define panic(m)	{perror(m); abort();}
#define BUFFER_LEN 256
#define MSGQ_NAME "/MQ_client_send"

/* VARIABLES */
mqd_t msgq_id;
int priority = 1;

int main( int argc, char *argv[] )
{   
    char buffer[BUFFER_LEN] = "\0";
    int i = 1;

    if(argc < 2 ){
    	printf("Usage: %s \"Message to send\"\n", argv[0]);
    	exit(1);
    }

    /* opening the queue using default attributes  --  mq_open() */
    msgq_id = mq_open(MSGQ_NAME, O_RDWR | O_CREAT , S_IRWXU | S_IRWXG, NULL);

    if (msgq_id == (mqd_t)-1) 
    {
        panic("Opening MQ sendmsg")
        exit(1);
    }
   
    //Copy all the message to one buffer
   for( ; i < argc ; i++)
   {

       strcat( buffer , argv[i] );
       strcat( buffer, " ");

   }

   
    /* sending the message      --  mq_send() */
    mq_send(msgq_id, buffer , BUFFER_LEN , priority );

    /* closing the queue        -- mq_close() */
    mq_close(msgq_id);

    return 0;
}


