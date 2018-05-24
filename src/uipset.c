#include "uipset.h"

static void uipset_setlastmessage ( struct uipset_client* client, const char* format, ... )
{
	va_list vlist;
	va_start ( vlist, format );
	vsnprintf ( client->m_lastmessage, sizeof ( client->m_lastmessage ) / sizeof ( *client->m_lastmessage ), format, vlist );
	va_end ( vlist );
}

int uipset_start ( struct uipset_client* client, const char* const path )
{
	int rc = 0;
	const char* const logprefix = "uipset_start: ";

	memset ( client, 0, sizeof ( *client ) );

	client->m_sock = socket ( AF_UNIX, SOCK_STREAM, 0 );

	if ( client->m_sock < 0 )
		uipset_setlastmessage ( client, "%s%s", logprefix, strerror ( errno ) );
	else
	{
		struct timeval tv;
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		setsockopt ( client->m_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv, sizeof ( struct timeval ) );

		struct sockaddr_un addr = { .sun_family = AF_UNIX };

		memcpy ( addr.sun_path, path, strlen ( path ) );

		if ( connect ( client->m_sock, (struct sockaddr*) &addr, sizeof ( addr ) ) < 0 )
			uipset_setlastmessage ( client, "%s%s", logprefix, strerror ( errno ) );
		else
			rc = true;
	}

	return rc;
}

void uipset_stop ( struct uipset_client* client )
{
	*client->m_lastmessage = 0;

	if ( client->m_sock >= 0 )
	{
		close ( client->m_sock );
		client->m_sock = -1;
	}
}

int uipset_request ( struct uipset_client* client, unsigned char cmd, const char* const set, const char* const ent, uipset_msg_response* r )
{
	int rc = 0;
	const char* const logprefix = "uipset_request: ";

	*client->m_lastmessage = 0;

	if ( client->m_sock < 0 )
		uipset_setlastmessage ( client, "%sclient->m_sock < 0", logprefix );
	else
	{
		// Allocate space on the stack to store the message data.
		uipset_msg_request req = uipset_msg_request_init_zero;

		req.cmd = cmd;

		strncpy ( req.set, set, sizeof ( req.set ) );

		strncpy ( req.ent, ent, sizeof ( req.ent ) );

		if ( *req.ent != 0 )
			req.has_ent = true;

		// Create a stream that will write to our socket.
		pb_ostream_t output = pb_ostream_from_socket ( client->m_sock );

		// создаёт и отправляет сообщение
		if ( ! pb_encode_delimited ( &output, uipset_msg_request_fields, &req ) )
			uipset_setlastmessage ( client, "%sb_encode_delimited failed: %s", logprefix, PB_GET_ERROR ( &output ) );
		else
		{
			uipset_msg_response rsp = uipset_msg_response_init_zero;

			pb_istream_t input = pb_istream_from_socket ( client->m_sock );

			// принимает и декодирует сообщение
			if ( ! pb_decode_delimited ( &input, uipset_msg_response_fields, &rsp ) )
				uipset_setlastmessage ( client, "%spb_decode_delimited failed", logprefix );
			else
			{
				rc = 1;
				*r = rsp;
			}
			
		}
	}

	return rc;
}
