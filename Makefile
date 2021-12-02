CC = aarch64-buildroot-linux-gnu-gcc 
#CROSS-COMPILE ?= aarch64-buildroot-linux-gnu-
#CC = gcc

CFLAGS = -g -I. 
DEPS = pthread.h command.h

IP = 10.42.0.74
PORT = 5000



%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
create_server: TPCServerRasp.o command.o
	$(CC) -o TPCServerRasp.elf TPCServerRasp.o command.o -lpthread
	rm command.o
	
create_client: TPC_ClientPc.o command.o
	$(CC) -o TPC_ClientPc TPC_ClientPc.o command.o -lpthread -lrt
	rm command.o

send_default_msg:
	./send.out send HI everyone!
	
create_sendout:
	gcc sendmsg.c -o send.out -lrt
	
connect_client:
	./TPC_ClientPc $(IP) $(PORT) &
	



.PHONY: clean 
clean: 
	rm -f *.o TPCServerRasp.elf TPC_ClientPc
	
clean_client:
	rm -f TPC_ClientPc.o TPC_ClientPc command.o
	
clean_server:
	rm -f TPCServerRasp.elf TPCServerRasp.o command.o
	
transfer_server: 
	scp TPCServerRasp.elf led.ko root@$(IP):/etc
	
	
