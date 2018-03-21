#include "ipset.h"

static int exit_error ( int e, struct ipset_session* sess )
{
	debug ( LOG_ERR, "ipset: %s", ipset_session_error ( sess ) );
	ipset_session_fini ( sess );

	return e;
}

static int ip_valid ( const char* ipaddr )
{
	unsigned int family = 0;
	unsigned int ret = 0;
	struct sockaddr_in sa;

	family = strchr ( ipaddr, '.' ) ? AF_INET : AF_INET6;

	ret = inet_pton ( family, ipaddr, &(sa.sin_addr) );

	return ret != 0;
}

static int ipset_do ( int c, const char* set, const char* elem )
{
	const struct ipset_type* type = NULL;
	enum ipset_cmd cmd = c;
	int ret = 0;
	struct ipset_session* sess;

	if ( ! ip_valid ( elem ) )
	{
		debug ( LOG_ERR, "ipset: %s is not a valid IP address", elem );
		return 1;
	}

	ipset_load_types ();

	sess = ipset_session_init ( printf );

	if ( sess == NULL )
	{
		debug ( LOG_ERR, "ipset: failed to initialize session" );
		return 1;
	}

	if ( cmd == IPSET_CMD_ADD )
	{
		ret = ipset_envopt_parse ( sess, IPSET_ENV_EXIST, NULL );

		if ( ret < 0 )
		{
			return exit_error ( 1, sess );
		}
	}

	/*! синтаксическая проверка назчания сета? и добавление его в сессию */
	ret = ipset_parse_setname ( sess, IPSET_SETNAME, set );

	if ( ret < 0 )
	{
		return exit_error ( 1, sess );
	}

	/*! возвращает тип сета, переданного в сессию выше */
	type = ipset_type_get ( sess, cmd );

	if ( type == NULL )
	{
		return exit_error ( 1, sess );
	}

	ret = ipset_parse_elem ( sess, type->last_elem_optional, elem );

	if ( ret < 0 )
	{
		return exit_error ( 1, sess );
	}

	ret = ipset_cmd ( sess, cmd, 0 );

	if ( ret < 0 )
	{
		return exit_error ( 1, sess );
	}

	ipset_session_fini ( sess );

	return 0;
}

int ipset_ip_add ( const char* set, const char* elem )
{
	int rc = ipset_do ( IPSET_CMD_ADD, set, elem );

	if ( rc == 0 )
	{
		debug ( LOG_INFO, "ipset: added %s to %s", elem, set );
	}

	return rc;
}

int ipset_ip_del ( const char* set, const char* elem )
{
	int rc = ipset_do ( IPSET_CMD_DEL, set, elem );

	if ( rc == 0 )
	{
		debug ( LOG_INFO, "ipset: deleted %s from %s", elem, set );
	}

	return rc;
}

int ipset_ip_lst ( char** set )
{
	struct ipset_session* sess = NULL;
	enum ipset_cmd cmd = IPSET_CMD_LIST;
	int ret = 0;

	ipset_load_types ();

	sess = ipset_session_init ( printf );

	if ( sess == NULL )
	{
		debug ( LOG_ERR, "ipset: failed to initialize session" );
		return 1;
	}

	ret = ipset_cmd ( sess, cmd, 0 );

	if ( ret < 0 )
	{
		return exit_error ( 1, sess );
	}

	ipset_session_fini ( sess );
}
