#ifndef IPSET_H
#define IPSET_H

#include <arpa/inet.h>
#include <libipset/session.h>
#include <libipset/types.h>
#include <string.h>

#include "debug.h"

int ipset_ip_add ( const char*, const char* );
int ipset_ip_del ( const char*, const char* );
int ipset_ip_lst ( char** );

#endif
