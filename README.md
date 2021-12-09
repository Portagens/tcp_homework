
# Raspberry Group Chat

This project is a simple group chat which use Raspberry PI 4 as a server and 
the Pc as a client/clients.




## Download Project

For ssh connection:
```bash
    git clone git@github.com:HugoRibeiro-A88287-UM/GroupChatHomework.git
```

For https connection:
```bash
    git clone https://github.com/HugoRibeiro-A88287-UM/GroupChatHomework.git
```

Go to the project directory:

```bash
    cd GroupChatHomework
```
## Installation

To install this project it's simple. 

* Open the command line into the project folder

* Type nano Makefile

* Change the RASPIP to yours.

* Save Makefile

* Type: 
```bash
	make all
```

NOTE: 
    This tutorial is to use raspberry pi 4 as a server. All the makefile was done to that purpose. 
    If you want to have your PC as client and server, all you need to do is to change the compiler
    for the server.


## Run 

To run the project, please first connect your raspberry to the computer with
a ssh connection. It's also possible to use UART connection. 
That option won't be approached here.


* Go to your local project folder

* Type make transfer_server to send the executable to the raspberry;

* Start executing the server on raspberry with the port number.
    Use the command: ./TPCServerRasp.elf <PORT> 

* Type make connect_client to connect the client to the server

* Start communicate with your friends :)


To send messages there are two way:

To send a default message to all clients:


        make send_default_msg 
    
To send specific messages:

        ./send.out <command> <message> 




## Command list

The available command list is:

* send 

        To send a message for all the clients

* name

        Change the name on the group chat

* help

        Show all the available commands

* kill

        Disconnect the group chat




