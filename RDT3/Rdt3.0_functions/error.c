#include "np.h"

/* error - print a diagnostic and optionally quit */
void error( int status, int err, const char *fmt, ... )
{
	va_list ap;

	va_start( ap, fmt );
	vfprintf( stderr, fmt, ap );
	va_end( ap );
	if ( err )
		fprintf( stderr, ": %s (%d)\n", strerror( err ), err );
	if ( status )
		exit( status );
}

/* Print a message only if DEBUG_ON is set. */
int DEBUG_ON;

void debug(const char *fmt, ...)
{
	va_list		ap;

	if ( !DEBUG_ON )
		return;
	va_start(ap, fmt);
	vfprintf( stderr, fmt, ap );
	va_end(ap);
	return;
}
