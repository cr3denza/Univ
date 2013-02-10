#include "np.h"

/* udp_server - set up a UDP server */
int udp_server( char *hname, char *sname )
{
	int s;
	struct sockaddr_in local;

	set_address( hname, sname, &local, "udp" );
	s = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( !isvalidsock( s ) )
		error( 1, errno, "socket call failed" );
	if ( bind( s, ( struct sockaddr * ) &local, sizeof( local ) ) )
		error( 1, errno, "bind failed" );
	return s;
}
