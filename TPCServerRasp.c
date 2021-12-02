#include <sys/socket.h>
#include <sys/types.h>
#include <resolv.h>
#include <pthread.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>


#include "command.h"


/* DEFINES AND CONST VARIABLES */
void panic(char *msg);
#define panic(m)	{perror(m); abort();}
#define MAX_CLIENT_NUM			8
#define MAX_NAME_LEN			16
const char send_header[] = " said -> ";
const char array_status[][16] = {"Online", "AFK", "Death"};
const char ledStatus[] = { '0' , '1' };

/* STRUCTS */
typedef struct client_socket_into client_socket_info_t;
struct client_socket_into
{
	int socket;
	int status;
	int index;
	char name[MAX_NAME_LEN];
}; 


/* VARIABLES */
client_socket_info_t socket_table[MAX_CLIENT_NUM];
struct sockaddr_in addr;
int listen_sd, port;
int threads=0;
int led_id = 0;
int led_index = 0;




void *sendReceiveFunction(void *arg)
{
	/*
		This function is responsable for comunication between the server 
		and the client. It have the client socket info so it is possible to identify 
		what client is. 
		After receiving info, it is decoded because it can be different things, like change name,
		send message, status, etc.
		By default, it return a ERROR message to the client

	*/

	client_socket_info_t *info = (client_socket_info_t *)arg;
	command_t cmd;
	char sendBuffer[MSG_LEN];
	int j;

	while(1) // With this while cycle the thread never ends
	{		 //So, the server is always online

		if( recv(info->socket, &cmd ,sizeof(cmd),0) > 0 )
		{
			//Turn on the led on raspberry. Also applys a mask to always stay into the array limits
			write(led_id, &ledStatus[ (led_index++) & 1 ], 1);

			switch (cmd.type)
			{
				
				case cmd_send:
					strcpy(sendBuffer,info->name);
					strcat(sendBuffer,send_header);
					strcat(sendBuffer,cmd.message);
					strcpy(cmd.message,sendBuffer);

					cmd.type = cmd_receive;

					//printf("To send: %s  \n index: %d \n", cmd.message, info->index);

					j = info->index + 1;
					while( (j & (MAX_CLIENT_NUM - 1) ) != info->index ) 
					{
						//It probably will send to clients that not exist but
						//	this is done to avoid using an IF
						send(socket_table[ j & (MAX_CLIENT_NUM - 1) ].socket,&cmd,sizeof(cmd),0);
						j++;

					}

					break;

				case cmd_name:
					strcpy(info->name,cmd.message);
					strcpy(cmd.message,"\n Name Changed \n");
					cmd.type = cmd_receive;

					send(info->socket,&cmd,sizeof(cmd),0);

					break;

				case cmd_status:

					info->status = cmd.status;

					break;

				case cmd_kill:

					info->status = death;

					strcpy(cmd.message,"\n KILL SUCCESS \n");	
					cmd.type = cmd_receive;

					send(info->socket,&cmd,sizeof(cmd),0);
					break;

				case cmd_help:
					command_help(cmd.message);
					cmd.type = cmd_receive;

					send(info->socket,&cmd,sizeof(cmd),0);
					break;

				default:
					strcpy(cmd.message,"\n ERROR \n");
					cmd.type = cmd_receive;
					send(info->socket,&cmd,sizeof(cmd),0);

					break;

			}	
				
		}
		
	}
		
	return 0;
}

void *newConnectionFuction(void *arg)
{
	/*
		This function is in responsable to add new connection when they apper
		To do that, it is lisening to see if there is a new socket available
		If there is a new connection, it creats a thread to that connection
		with the info of that new conncetion "socket_table[threads]"
	*/
	int sd;
	int len = sizeof(addr);
	pthread_t send_receive[MAX_CLIENT_NUM]; 

	while (1)
	{
		while( (sd =  accept(listen_sd, (struct sockaddr*)&addr, &len) ) == -1 )
		{}
		
		socket_table[threads].socket=sd;
		socket_table[threads].status=online;		/*means connection opened*/
		socket_table[threads].index=threads;
		strcpy(socket_table[threads].name,"Anonymus");
		
		printf("New connection\n");

		pthread_create(&send_receive[threads], 0, sendReceiveFunction, &socket_table[threads]);
		pthread_detach(send_receive[threads]);

		threads++;
		

	}
	
	return 0;
}

void *statusClientFuntion(void *arg)
{
	//This function is to check the status of the clients.
	//Every 5 seconds will check the status
	//The server will ask to all the clients what status they are
	//The answers will be in sendreceivefuntion

	int i;

	command_t status_cmd;

	status_cmd.type = cmd_status;

	while (1)
	{

		sleep(5);

		for(i = 0; i < threads ; i++)
		{
			//Send a report request
			send(socket_table[i].socket,&status_cmd,sizeof(status_cmd),0);

		}	
	}
	return 0;
}

void *commandLineServerFuntion(void *arg)
{
	/*
		This function deals with the server command line
		At this version, it only can check the status of the clients
	*/

	char buffer[MSG_LEN];
	int i;

	while(1)
	{
		scanf("%[^\n]%*c", buffer);

		if(strcmp(buffer,"status") == 0)
		{
			for(i = 0 ; i < threads ; i++)
			{
				printf("\n Client %d aka %s -> %s \n", socket_table[i].index , socket_table[i].name , array_status[socket_table[i].status]);
			}
		}

	}

}

static void int_handler(int signo)
{		
	if(signo==SIGINT)
	{	/* handler of SIGNAL*/

		for(int i = 0 ; i < MAX_CLIENT_NUM ; i ++)
		{
			shutdown(socket_table[i].socket,SHUT_RD);
			shutdown(socket_table[i].socket,SHUT_WR);
			shutdown(socket_table[i].socket,SHUT_RDWR);
		}

		system("rmmod led");
		exit(1);
		
	}
}

int main(int count, char *args[])
{	

	if ( count != 2 )
	{
		printf("usage: %s <protocol or portnum>\n", args[0]);
		exit(0);
	}

	/*---Get server's IP and standard service connection--*/
	if ( !isdigit(args[1][0]) )
	{
		struct servent *srv = getservbyname(args[1], "tcp");
		if ( srv == NULL )
			panic(args[1]);
		printf("%s: port=%d\n", srv->s_name, ntohs(srv->s_port));
		port = srv->s_port;
	}
	else
		port = htons(atoi(args[1]));

	/*--- create socket ---*/
	listen_sd = socket(PF_INET, SOCK_STREAM, 0);
	if ( listen_sd < 0 )
		panic("socket");

	/*--- bind port/address to socket ---*/
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = port;
	addr.sin_addr.s_addr = INADDR_ANY;                   /* any interface */
	if ( bind(listen_sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
		panic("bind");

	/*LED Setup*/

	system("insmod led.ko");
	led_id = open("/dev/led0", O_WRONLY);
	system("echo none >/sys/class/leds/led0/trigger");

	/*Signal Handler */

	signal(SIGINT,  int_handler);

	/*--- make into listener with 10 slots ---*/
	if ( listen(listen_sd, MAX_CLIENT_NUM) != 0 )
		panic("listen")

	/*--- begin waiting for connections ---*/
	else
	{	
		//Create the threads to make the server work
		pthread_t newConnection, statusClient,commandLineServer;
 

		pthread_create(&newConnection,0,newConnectionFuction,&socket_table);
		pthread_create(&statusClient,0,statusClientFuntion,&socket_table);
		pthread_create(&commandLineServer,0,commandLineServerFuntion,NULL);

		pthread_join(commandLineServer, NULL);
		pthread_join(statusClient,&socket_table);
		pthread_join(newConnection,&socket_table);
		pthread_exit(NULL);

		
		
	}

	return 0;
}
