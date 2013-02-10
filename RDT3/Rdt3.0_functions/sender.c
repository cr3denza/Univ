#include "np.h"
#include "packet.h"

// Event Types
#define TIMEOUT 			1
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
	int s;
	int rc;
	int event;
	fd_set rset, allset;

	DEBUG_ON = 1; 	// print debug message
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
		if ( rc == 0 ) { 	// timeout event
			event |= TIMEOUT;
		}
		if (FD_ISSET(0, &rset)) { 		// app data arrived
			event |= RDT_SEND;
		}
		if (FD_ISSET( s, &rset)) {		// feed-back arrived
			event |= RDT_RCV;
		}
		// Actions taken by State and Event
		// ...
	}
}

