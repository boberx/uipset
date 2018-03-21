#include <stdio.h>

#include "ipsetacld.h"

int main ( int argc, char* argv[] )
{
	printf ( "start\n" );

	const char* const address = "/tmp/ipsetacld.sock";

	struct ipsetacld_client client;

	if ( ! ipsetacld_start ( &client, address ) )
		printf ( "error: ipsetacld_start: %s\n", strerror ( client.err ) );
	else
	{
		printf ( "connected\n" );

		if ( ! ipsetacld_request ( &client, IPSETACLD_CMD_ADD, "test", "127.0.0.1" ) )
			printf ( "error: ipsetacld_request: %s\n", strerror ( client.err ) );
		else
			printf ( "YES! YES!\n" );

		ipsetacld_stop ( &client );
	}

	return 1;
}
