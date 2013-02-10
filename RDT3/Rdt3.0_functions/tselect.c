#include "np.h"

static struct timeval *active = NULL;		/* active timer */

void start_timer(unsigned int ms )
{
	static struct timeval timeout;		/* timeout in time of day */

	if ( gettimeofday( &timeout, NULL ) < 0 )
		error( 1, errno, "timeout: gettimeofday failure" );
	timeout.tv_usec += ms * 1000;
	if ( timeout.tv_usec > 1000000 )
	{
		timeout.tv_sec += timeout.tv_usec / 1000000;
		timeout.tv_usec %= 1000000;
	}
	active = &timeout;
	return;
}
/* end timeout */

/* cancel a timer */
void stop_timer()
{
	active = NULL;
	return;
}

/* tselect - select with a timer */
int tselect( int maxp1, fd_set *re, fd_set *we, fd_set *ee )
{
	struct timeval now;
	struct timeval tv;
	struct timeval *tvp;

	if ( active )			// if timer is running
	{
		if ( gettimeofday( &now, NULL ) < 0 )
			error( 1, errno, "tselect: gettimeofday failure" );
		if ( !timercmp(&now, active, <) )	// timeout elapsed
			return 0; 
		tv.tv_sec = active->tv_sec - now.tv_sec;;
		tv.tv_usec = active->tv_usec - now.tv_usec;
		if ( tv.tv_usec < 0 )
		{
			tv.tv_usec += 1000000;
			tv.tv_sec--;
		}
		tvp = &tv;		// get the time difference for timeout
	}
	else
		tvp = NULL;		// no timer
	return ( select( maxp1, re, we, ee, tvp ) );
}

