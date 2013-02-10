CC = gcc

.c.o:
	$(CC) -c -g -O -Wall  $< -o $@

sender: sender.o udt_send.o tselect.o set_address.o in_cksum.o error.o
	$(CC) -o $@ sender.o udt_send.o tselect.o set_address.o in_cksum.o error.o
clean:
	${RM} sender  sender.o udt_send.o tselect.o set_address.o in_cksum.o error.o
