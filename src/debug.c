#include "debug.h"

int debuglevel = LOG_INFO;
int outconsole = 0;

void debug ( int level, const char* format, ... )
{
	va_list vlist;

	if ( debuglevel >= level )
	{
		va_start ( vlist, format );
		vsyslog ( level, format, vlist );

		if ( outconsole == 1 )
		{
			printf ( format, vlist );
			printf ( "\n" );
		}

		va_end ( vlist );
	}
}
