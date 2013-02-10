#include "np.h"
static float pktErrorRate = 0.1;
static float pktLossRate = 0.1;

static float
prob() {
	struct timeval tv;
	static unsigned int seed;

	if ( !seed ) {
		gettimeofday(&tv, NULL);
		seed = tv.tv_sec * 1000 + tv.tv_usec/1000;  // in msec unit
		srandom(seed);
	}
	return ( (float) random() / RAND_MAX );
}

// Unreliable data transfer
// 'sockfd' should be connected UDP socket
int
udt_send(int sockfd, char *pkt, unsigned int pktLength)
{
	char tmp[4096]; 
	if ( prob() <= pktLossRate )   {
//		debug("Packet lost\n");
		return (0);		// simulate packet loss
	}
	if ( prob() <= pktErrorRate ) {
		int l;
		memcpy(&tmp, pkt, pktLength); 
		l =  (int) (prob() * pktLength);	
		*(tmp+l) += 1;        // Cause bit error in l-th byte of the packet
//		debug("Packet corrupted\n");
		return(send(sockfd, &tmp, pktLength, 0));
	}
	return(send(sockfd, pkt, pktLength, 0));
}
