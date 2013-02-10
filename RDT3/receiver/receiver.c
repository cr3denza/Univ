#include "np.h"
#include "packet.h"
#include <time.h>
#include <sys/types.h>

//Event Types
#define TIMEOUT 		1
#define RDT_SEND 		2
#define RDT_RCV			4

//Packet buffers
Packet dataPacket;
CPacket controlPacket;

extern int DEBUG_ON;	// print debug messages

// reciever			
int main( int argc, char **argv )
{
	int min=0,sec =0 ,msec =0;
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	int serv_sock;
	int clnt_addr_size;


	int result = 0;
	uint16_t check;
	int r_seq = 0, ack = 1, pre_seq=0;
	int correct_pkt =0, dup_pkt=0, rcv_pkt=0;
	int rcv_byte=0;
	struct timeval ti1,ti2;
	int read_byte;
	int rcv_start =0, rcv_end=0;

	DEBUG_ON = 1; 	// print debug message

	if(argc != 2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if ((serv_sock = socket( AF_INET, SOCK_DGRAM, 0 )) < 0)
		error(1, errno, "socket creation failed" );

	memset(&serv_addr, 0 , sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));	

	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error(1, errno, "socket bind failed");

	while(1) {
		memset(&dataPacket,0,sizeof(dataPacket));
		memset(&controlPacket,0,sizeof(controlPacket));

		clnt_addr_size = sizeof(clnt_addr);	
		read_byte = recvfrom(serv_sock, &dataPacket, sizeof(dataPacket), 0, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
		debug("========================================================\n");
		debug("[DATA] rcv Packet: type[%d] seq[%d] len[%d] checksum[%x]\n", dataPacket.header.type, r_seq, dataPacket.header.len, dataPacket.header.checksum);

		if(read_byte > 0){
			if(rcv_start ==0){
				gettimeofday(&ti1,NULL);
				rcv_start =1;
			}

			if(dataPacket.header.type == DATA){ // DATA 인경우 잘못된 전송 혹은 제대로된 전송
				if(checkPacket(&dataPacket, r_seq)){
					connect(serv_sock, (struct sockaddr *)&clnt_addr,clnt_addr_size);
					make_cpkt(&controlPacket, ACK, ack);
					write(1, dataPacket.data,dataPacket.header.len-HEADER_LEN);
					if(udt_send(serv_sock, &controlPacket, sizeof(controlPacket)) <0 )
						error(1, errno, "ACK udt_send error\n");

					debug("[ACK] send Packet: type[%d] seq[%d] len[%d] checksum[%x]\n", controlPacket.header.type, ack, controlPacket.header.len, controlPacket.header.checksum);
					correct_pkt++;
					rcv_pkt++;
					
					rcv_byte=rcv_byte + dataPacket.header.len - HEADER_LEN;
					pre_seq=r_seq;
					r_seq++;
					ack = r_seq+1;
				}
				else{
					if(dataPacket.header.seq == pre_seq) // error 인 duplicate 도 포함시킨다
						dup_pkt++;

					connect(serv_sock, (struct sockaddr *)&clnt_addr,clnt_addr_size);
					make_cpkt(&controlPacket, ACK, r_seq);
					if(udt_send(serv_sock, &controlPacket, sizeof(controlPacket)) <0 )
						error(1, errno, "ACK udt_send error\n");
					rcv_pkt++;
					debug("[ReACK] send Packet: type[%d] seq[%d] len[%d] checksum[%x]\n", controlPacket.header.type, r_seq, controlPacket.header.len, controlPacket.header.checksum);
				}
			}		
			else if(dataPacket.header.type == END){ // END 인경우 잘못된 전송 혹은 제대로된 전송
				if(checkCPacket(&dataPacket, r_seq)){
					if(rcv_end == 0){
						gettimeofday(&ti2,NULL);
						if((ti2.tv_sec - ti1.tv_sec)>0){
							min = (ti2.tv_sec - ti1.tv_sec)/60;
							sec = (ti2.tv_sec - ti1.tv_sec)%60;
						}
						else if((ti1.tv_sec-ti2.tv_sec)>0){
							min = (ti1.tv_sec - ti2.tv_sec)/60;
							sec = (ti1.tv_sec - ti2.tv_sec)%60;
						}
						if((ti2.tv_usec - ti1.tv_usec)>0)
							msec = (ti2.tv_usec - ti1.tv_usec)/1000;
						else if((ti1.tv_usec - ti2.tv_usec)>0)
							msec = (ti1.tv_usec - ti2.tv_usec)/1000;

						debug("===============================================================\n");
					//	debug("start %d usec\n", ti1.tv_usec);
					//	debug("end% d usec\n", ti2.tv_usec);
						debug("===============================================================\n");
						debug("%d of DATA packets received (including retransmitted packets)\n",rcv_pkt);
						debug("%d of duplicated DATA packets received\n",dup_pkt);
						debug("%d of DATA packets correctly received\n",correct_pkt);
						debug("%d of bytes correctly received\n",rcv_byte);
						debug("Time duration : %d min %d sec %d msec\n",min,sec,msec);
						debug("Troughput : %d (byte/second)\n",rcv_byte/(ti2.tv_sec - ti1.tv_sec));
						debug("===============================================================\n");

						rcv_end =1;

					}
					connect(serv_sock, (struct sockaddr *)&clnt_addr,clnt_addr_size);
					make_cpkt(&controlPacket, END, ack);
					if(udt_send(serv_sock, &controlPacket, sizeof(controlPacket)) <0 )
						error(1, errno, "ACK udt_send error\n");
					debug("[END] send Packet: type[%d] seq[%d] len[%d] checksum[%x]\n", controlPacket.header.type, ack, controlPacket.header.len, controlPacket.header.checksum);
					return 0;
				}
				else{
					connect(serv_sock, (struct sockaddr *)&clnt_addr,clnt_addr_size);
					make_cpkt(&controlPacket, ACK, r_seq);
					if(udt_send(serv_sock, &controlPacket, sizeof(controlPacket)) <0 )
						error(1, errno, "ACK udt_send error\n");
					debug("[ReACK] send Packet: type[%d] seq[%d] len[%d] checksum[%x]\n", controlPacket.header.type, r_seq, controlPacket.header.len, controlPacket.header.checksum);
				}
			}
		}

		if(read_byte < 0)
			error(1, errno, "recvfrom failed");

	}
}

void make_pkt(unsigned char type, int seq, char* data, unsigned short checksum){
	if(data==NULL){
		memset(&controlPacket,0,sizeof(controlPacket));
		controlPacket.header.seq = seq%256;
		controlPacket.header.len = HEADER_LEN;
		controlPacket.header.checksum = checksum;
		controlPacket.header.type = type;
	}

	else{
		memset(&dataPacket,0,sizeof(dataPacket));
		dataPacket.header.seq = seq;
		dataPacket.header.len = HEADER_LEN + sizeof(data);
		dataPacket.header.checksum = checksum;
		dataPacket.header.type = type;
		memcpy(&dataPacket.data,data,strlen(data));
	}
}

void make_cpkt(CPacket* controlPacket, unsigned char type, int seq){

	controlPacket->header.type = type; // ACK or END
	controlPacket->header.seq = seq%256;
	controlPacket->header.len = HEADER_LEN;
	controlPacket->header.checksum = in_cksum((uint16_t *)controlPacket , controlPacket->header.len);

	return 0;
}

BOOL checkPacket(Packet* rcvpkt, int r_seq){
	uint16_t check;
	check = rcvpkt->header.checksum;
	rcvpkt->header.checksum = 0;
	rcvpkt->header.checksum = in_cksum((uint16_t *)rcvpkt,rcvpkt->header.len);
	debug(" %x\t%x \n",check,rcvpkt->header.checksum);
	if((rcvpkt->header.checksum == check) && (rcvpkt->header.seq == (r_seq%256))){
		return TRUE;
	}
	else
		return FALSE;
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
