#ifndef UIPSET_COMMON_H
#define UIPSET_COMMON_H

#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <pb_encode.h>
#include <pb_decode.h>

pb_istream_t pb_istream_from_socket ( int );

pb_ostream_t pb_ostream_from_socket ( int );

#endif
