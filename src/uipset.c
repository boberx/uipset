#include "uipset.h"

bool uipset_start ( struct uipset_client* client, const char* const path )
{
	int rc = false;

	memset ( client, 0, sizeof ( *client ) );

	client->sock = socket ( AF_UNIX, SOCK_STREAM, 0 );

	if ( client->sock < 0 )
		client->err = errno;
	else
	{
		struct timeval tv;
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		setsockopt ( client->sock, SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv, sizeof ( struct timeval ) );

		struct sockaddr_un addr = { .sun_family = AF_UNIX };
		memcpy ( addr.sun_path, path, strlen ( path ) );

		if ( connect ( client->sock, (struct sockaddr*) &addr, sizeof ( addr ) ) < 0 )
			client->err = errno;
		else
		{
			rc = true;
		}
	}

	return rc;
}

void uipset_stop ( struct uipset_client* client )
{
	if ( client->sock >= 0 )
	{
		close ( client->sock );
		client->sock = -1;
	}
}

bool uipset_request ( struct uipset_client* client, unsigned char cmd, const char* set, const char* ipa )
{
	int rc = false;

	if ( client->sock >= 0 )
	{
		uipset_msg_request req = uipset_msg_request_init_zero;

		req.cmd = cmd;

		strncpy ( req.set, set, sizeof ( req.set ) );

		strncpy ( req.ipa, ipa, sizeof ( req.ipa ) );

		pb_ostream_t output = pb_ostream_from_socket ( client->sock );

		if ( ! pb_encode_delimited ( &output, uipset_msg_request_fields, &req ) )
		{
		}
		else
		{
			uipset_msg_response rsp = uipset_msg_response_init_zero;
			pb_istream_t input = pb_istream_from_socket ( client->sock );

			if ( ! pb_decode_delimited ( &input, uipset_msg_response_fields, &rsp ) )
			{
			}
			else if ( rsp.error == 0 )
			{
				rc = true;
			}
		}
	}

	return rc;
}
