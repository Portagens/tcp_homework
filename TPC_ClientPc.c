
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <resolv.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <mqueue.h> 

#include "command.h"

/* DEFINES AND CONST VARIABLES*/
void panic(char *msg);
#define panic(m)	{perror(m); abort();}
#define MQ_BUFFER_LEN 8192 //Min_buff_size
#define AUX_BUFFER_LEN 256
#define MSGQ_NAME "/MQ_client_send"

/* VARIABLES */
status_t client_status,aux_status;
mqd_t msgq_id;
int priority = 1;
int sd;


void *sendfunction(void *arg)
{
	/*
		This function is reponsable for sending messages to the server

	*/
	int sd = *(int*)arg;           
	char aux_buffer[MQ_BUFFER_LEN];
	int mq_recv = 0;
	command_t cmd;

	while(1)
	{
		/* Receive message by ICP message queue*/
		mq_recv = mq_receive(msgq_id , aux_buffer , MQ_BUFFER_LEN ,  &priority );
		
		if (mq_recv == -1) 
		{
			panic("Abort receiving mq");
    	}

		char * token = strtok(aux_buffer," ");

		if(token != NULL)
		{
			cmd.type = is_command(token);

			//To continue the slip, the first parameter is NULL
			token = strtok(NULL,"\0");

			if(token != NULL)
			{			
				
				strcpy(cmd.message,token);

				send(sd,&cmd,sizeof(cmd),0);
			}
			else
			{
				cmd.type = -EINVAL;
				send(sd,&cmd,sizeof(cmd),0);
			}

			client_status = online;
			aux_status = online;

		}
	}

}

void *receivefunction(void *arg)
{
	/*
		This function is resposable for receiving the data from the server
		Frist, it need to decode what command did he received and then
		do the proper job.

		By default, the user gets a error warning that something went wrong
	*/

	int sd = *(int*)arg;   
	command_t cmd;         
	
	while(1)
	{
		if (recv(sd,&cmd,sizeof(cmd),0) > 0)
		{
			switch (cmd.type)
			{
			case cmd_receive:
				printf("%s \n", cmd.message);
				break;
			
			case cmd_status:
				cmd.type = cmd_status;
				cmd.status = client_status;

				send(sd,&cmd,sizeof(cmd),0);

				break;

			default:
				printf("\n ERROR \n");
				break;
			}

		}
	}

}

void *statusfunction(void *arg)
{       
	/*
		This function controlls the status of a client
		Every time a client send a msg, aux and client status goes online
		Every 60 seconds verify if the client send something
		If yes, aux and client will be online so no change
		If not, aux will be afk and so client will be afk too
	*/
	while(1)
	{
		sleep(60);

		client_status = aux_status;

		aux_status = afk;

	}

}


static void int_handler(int signo)
{		
	if(signo==SIGINT)
	{	/* handler of SIGNAL*/
		shutdown(sd,SHUT_RD);
		shutdown(sd,SHUT_WR);
		shutdown(sd,SHUT_RDWR);
		mq_close(msgq_id); /*Close queue*/
		exit(1);
		
	}
}

int main(int count, char *args[])
{	struct hostent* host;
	struct sockaddr_in addr;
	int port;

	/* opening the queue using default attributes  --  mq_open() */
  	msgq_id = mq_open(MSGQ_NAME, O_RDWR | O_CREAT , S_IRWXU | S_IRWXG, NULL);
	if (msgq_id == (mqd_t)-1) 
	{
		panic("Opening MQ Client");
		exit(1);
	}

	if ( count != 3 )
	{
		printf("usage: %s <servername> <protocol or portnum>\n", args[0]);
		exit(0);
	}

	signal(SIGINT,  int_handler);

	/*---Get server's IP and standard service connection--*/
	host = gethostbyname(args[1]);
	//printf("Server %s has IP address = %s\n", args[1],inet_ntoa(*(long*)host->h_addr_list[0]));
	if ( !isdigit(args[2][0]) )
	{
		struct servent *srv = getservbyname(args[2], "tcp");
		if ( srv == NULL )
			panic(args[2]);
		printf("%s: port=%d\n", srv->s_name, ntohs(srv->s_port));
		port = srv->s_port;
	}
	else
		port = htons(atoi(args[2]));

	/*---Create socket and connect to server---*/
	sd = socket(PF_INET, SOCK_STREAM, 0);        /* create socket */
	if ( sd < 0 )
		panic("socket");
	memset(&addr, 0, sizeof(addr));       /* create & zero struct */
	addr.sin_family = AF_INET;        /* select internet protocol */
	addr.sin_port = port;                       /* set the port # */
	addr.sin_addr.s_addr = *(long*)(host->h_addr_list[0]);  /* set the addr */

    //If the server exists, the connect function should return zero
    //Otherwise it means that there is no server available
	if ( connect(sd, (struct sockaddr*)&addr, sizeof(addr)) == 0)
	{
		pthread_t send,receive,status;

		client_status = online;
		aux_status = online;

		pthread_create (&receive,0,receivefunction,&sd);
		pthread_create (&send,0,sendfunction,&sd);
		pthread_create (&status,0,statusfunction,&sd);


		pthread_join(&receive,&sd);
		pthread_join(&send,&sd);
		pthread_join(&status,&sd);
	
        pthread_exit(NULL);

	}
	else
		panic("connect");
}
