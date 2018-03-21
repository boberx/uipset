#include "common.h"

static bool read_callback ( pb_istream_t *stream, uint8_t* buf, size_t count )
{
	int fd = (intptr_t)stream->state;
	int result;

	result = recv(fd, buf, count, MSG_WAITALL);

	if ( result == 0 )
		stream->bytes_left = 0;

	return result == count;
}

static bool write_callback ( pb_ostream_t *stream, const uint8_t *buf, size_t count )
{
	int fd = (intptr_t)stream->state;

	return send ( fd, buf, count, 0 ) == count;
}

pb_ostream_t pb_ostream_from_socket ( int fd )
{
	pb_ostream_t stream = { &write_callback, (void*)(intptr_t) fd, SIZE_MAX, 0 };

	return stream;
}

pb_istream_t pb_istream_from_socket ( int fd )
{
	pb_istream_t stream = { &read_callback, (void*)(intptr_t) fd, SIZE_MAX };

	return stream;
}
