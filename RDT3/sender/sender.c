#include "np.h"
#include "packet.h"

// Event Types
#define TIMEOUT 		1
#define RDT_SEND 		2
#define RDT_RCV			4


//Packet buffers
Packet dataPacket;
CPacket controlPacket;

extern int DEBUG_ON;	// print debug messages

// Sender					
int main( int argc, char **argv )
{

	struct sockaddr_in peer;
	int clnt_addr_size;
	int s;
	int rc;
	int event;
	unsigned short read_byte;
	int s_seq=0;
	char state=0;
	char check;
	int end =0;

	fd_set rset, allset;

	DEBUG_ON = 1; 	// print debug message

	////////////////////////// argv 입력 오류 확인 해주는 내용 추가//////////////////////////

	set_address( argv[ 1 ], argv[ 2 ], &peer, "udp" ); // Set peer's IP and port
	if ( (s = socket( AF_INET, SOCK_DGRAM, 0 )) < 0)
		error(1, errno, "socket creation failed" );
	connect(s, (struct sockaddr *) &peer, sizeof(peer)); // connected UDP socket

	FD_ZERO(&allset);
	FD_SET( 0, &allset);
	FD_SET( s, &allset);


	for (;;) {
		rset = allset;
		rc = tselect( s + 1, &rset, NULL, NULL ); // Wait for data from above, packet from udt, or timeout
		if ( rc < 0 )
			error(1, errno, "select error");
		if ( (rc == 0) && (state == 1) ) { 	// timeout event
			event |= TIMEOUT;
			debug("timeout\n");

			if(end==0){
				debug("[DATA] send Packet: type[%d] seq[%d] len[%d] checksum[%x]\n", dataPacket.header.type, s_seq, dataPacket.header.len, dataPacket.header.checksum);
				udt_send(s, &dataPacket, sizeof(dataPacket));
				start_timer(TIMEOUT);
			}
			else{
				debug("[END] send Packet: type[%d] seq[%d] len[%d] checksum[%x]\n", controlPacket.header.type, s_seq, controlPacket.header.len, controlPacket.header.checksum);
				memset(&controlPacket, 0, sizeof(controlPacket)); // memory 0 set
				make_cpkt( &controlPacket, END, s_seq);
				udt_send(s, &controlPacket, sizeof(controlPacket));
				start_timer(TIMEOUT);
			}

		}
		else if (FD_ISSET(0, &rset) && (state == 0) ) { // app data arrived
			//ACK 상태에서 대기하기위해 타임아웃기다려주거나 하는 조건 추가	

			event |= RDT_SEND;
			if(s_seq==0){
				// time check
			}

			memset(&dataPacket, 0, sizeof(dataPacket)); // memory 0 set
			read_byte = read(0, dataPacket.data, MAX_DATA);//read line
			debug("read seq = %d  ", s_seq);

			if(read_byte > 0){
				make_pkt(&dataPacket, s_seq, read_byte);
				udt_send(s, &dataPacket, sizeof(dataPacket));
				
				debug("[DATA] send Packet: type[%d] seq[%d] len[%d] checksum[%x]\n", dataPacket.header.type, s_seq, dataPacket.header.len, dataPacket.header.checksum);
				
				s_seq++;
				start_timer(TIMEOUT);
				state = 1;
			}
			else if(read_byte == 0){
				memset(&controlPacket, 0, sizeof(controlPacket)); // memory 0 set
				make_cpkt( &controlPacket, END, s_seq);
				udt_send(s,&controlPacket,sizeof(controlPacket));

				debug("[END] send Packet: type[%d] seq[%d] len[%d] checksum[%x]\n", controlPacket.header.type, s_seq, controlPacket.header.len, controlPacket.header.checksum);
				
				end=1;
				s_seq++;
				start_timer(TIMEOUT);
				state = 1;
			}
			else if(read_byte < 0)
				error(1, errno, "Data Read Fault");
		}

		else if (FD_ISSET( s, &rset) && (state==1)) {	// feed-back arrived
			event |= RDT_RCV;
			read_byte = recvfrom(s,&controlPacket, 8, 0, (struct sockaddr *)&peer, &clnt_addr_size);

			check = checkCPacket(&controlPacket, s_seq);

			if(check == ACK){
				stop_timer();
				debug("[ACK] rcv Packet : type[%d] seq[%d] len[%d] checksum[%x]\n", controlPacket.header.type, s_seq, controlPacket.header.len, dataPacket.header.checksum);
				state = 0;
			}
			else if(check == END){
				stop_timer();
				debug("[END] rcv Packet : type[%d] seq[%d] len[%d] checksum[%x]\n", controlPacket.header.type, s_seq, controlPacket.header.len, controlPacket.header.checksum);
				debug("-------------END-----------\n");
				return 0;
			}
			else if(check == FALSE){
				debug("packet error\n");
			}
		}
	}
	// Actions taken by State and Event
	// ...
}

void make_pkt(Packet* dataPacket, unsigned char seq, int data_len){
	dataPacket->header.type = DATA;
	dataPacket->header.seq = seq%256;
	dataPacket->header.len = data_len + HEADER_LEN;
	dataPacket->header.checksum = in_cksum((uint16_t *)dataPacket , dataPacket->header.len);

}

void make_cpkt(CPacket* controlPacket, unsigned char type, int seq){

	controlPacket->header.type = type; // ACK or END
	controlPacket->header.seq = seq%256;
	controlPacket->header.len = HEADER_LEN;
	controlPacket->header.checksum = in_cksum((uint16_t *)controlPacket , controlPacket->header.len);

}

char checkCPacket(CPacket* rcvpkt, int r_seq){
	uint16_t check;
	check = rcvpkt->header.checksum;
	rcvpkt->header.checksum = 0;
	rcvpkt->header.checksum = in_cksum((uint16_t *)rcvpkt,8);

	debug(" %x\t%x \n",check,rcvpkt->header.checksum);
	
	if(rcvpkt->header.checksum != check){
		return -1;
	}
	else{
		if(rcvpkt->header.seq == (r_seq%256)){
			if(rcvpkt->header.type == ACK)
				return ACK;
			else if(rcvpkt->header.type == END)
				return END;
		}
		else
			return -1; // 기다리고 있지 않은 패킷이 온경우
	}
}
