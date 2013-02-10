CC = gcc

.c.o:
	$(CC) -c -g -O -Wall  $< -o $@

receiver: receiver.o udt_send.o tselect.o set_address.o in_cksum.o error.o
	$(CC) -o $@ receiver.o udt_send.o tselect.o set_address.o in_cksum.o error.o
# receiver: receiver.o in_cksum.o error.o
#	$(CC) -o $@ receiver.o error.o
clean:
	${RM} receiver  receiver.o udt_send.o tselect.o set_address.o in_cksum.o error.o
