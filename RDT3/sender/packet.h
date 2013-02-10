#ifndef __PACKET_H__
#define __PACKET_H__

// Packet Structure
#define HEADER_LEN		8
#define MAX_DATA		1400
#define DATA			1
#define ACK				2
#define END				4

typedef struct {
	unsigned char type;		// DATA or ACK
	unsigned char seq;
	unsigned short len;		// network byte-order
	unsigned short checksum;	// host byte-order (not necessarily network-byte order
	unsigned short unused;	// should be cleared by 0
} Header;

typedef struct {		// for data packet
	Header header;
	char data[MAX_DATA];
} Packet;

typedef struct {		// for ACK, END
	Header header;
} CPacket;

typedef int BOOL;

#define TRUE 1
#define FALSE 0

void make_pkt(Packet* dataPacket, unsigned char seq, int data_len);
void make_cpkt(CPacket* controlPacket, unsigned char type, int seq);
char checkCPacket(CPacket* rcvpkt, int s_seq);


#endif  /* __PACKET_H__ */
