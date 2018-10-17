#include "debug.h"

int debuglevel = LOG_INFO;
int outconsole = 0;

void debug ( int level, const char* format, ... )
{
	if ( debuglevel >= level )
	{
		va_list vlist;
		va_start(vlist, format);

		if ( outconsole == 1 )
		{
			vprintf ( format, vlist );
			printf ( "\n" );
		}

		vsyslog ( level, format, vlist );

		va_end(vlist);
	}
}
