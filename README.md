# GroupChatHomework
Group chat using the Raspberry PI as a server and the Pc as clients

In order to be able to put the group chat work, follow these steps.



1. Open the Makefile and check if CC = aarch64-buildroot-linux-gnu-gcc is uncomment
2. Comment the CC = gcc line
3. Open the terminal in the folder directory
4. Type make clean to clean all the executables and .o files
5. Type make create_server to create the server executable
6. Type make transfer_server to send the executable to the raspberry
7. Commment the CC = aarch64-buildroot-linux-gnu-gcc line and uncomment CC = gcc line
8. Type make create_client to create the client
9. Type make create_sendout to create the send program
10. Start executing the server on raspberry with a 5000 port
11. Type make connect_client

	And the group chat shoud be working.

To send messages type make send_default_msg or type ./send.out <command> <message>
All the messages will be displayed on the terminal.



NOTE: This tutorial is to use raspberry pi 4 as a server. If you don't it, please don't
	do the first two steps. Instead, comment the CC = aarch64-buildroot-linux-gnu-gcc
	and let the CC = gcc line always uncomment.
	Also, don't do step 6. 
