#ifndef BIPSET_DEBUG_H
#define BIPSET_DEBUG_H

#include <stdarg.h>
#include <syslog.h>
#include <stdio.h>

extern int debuglevel;
extern int outconsole;

void debug ( int, const char*, ... );

#endif
