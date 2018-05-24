#include "ipset.h"

/*!
*	\brief вроде libipset не безопасна без блокировок, не хочу долго мучаться
*/
static pthread_mutex_t uipset_mutex = PTHREAD_MUTEX_INITIALIZER;
static char last_members[1024] = { 0 };

static int __attribute__ ((format (printf, 1, 2))) interout ( const char* fmt, ... )
{
	va_list args;
	va_start ( args, fmt );

	debug ( LOG_DEBUG, fmt, args );

	char* buffer = 0;

	vasprintf ( &buffer, fmt, args );

	char* p = strstr ( buffer, "Members:" );

	if ( p != 0 )
	{
	//	free ( last_members );
		*last_members = 0;

		debug ( LOG_DEBUG, "interout: 1" );
		debug ( LOG_DEBUG, "interout: 2: [%s]",  &p[9] );

		//last_members = (char*) malloc ( strlen ( &p[9] ) + 1 );
		//*last_members = 0;
		strcpy ( last_members, &p[9] );

		debug ( LOG_DEBUG, "interout: 3: [%s]",  last_members );
	}

	va_end ( args );

	free ( buffer );

	debug ( LOG_DEBUG, "interout: 4: [%s]",  last_members );

	return 1;
}

/*!
*	\brief Перехват сообщений от libipset
*/
static int __attribute__ ((format (printf, 1, 2))) ipset_print_file ( const char* fmt, ... )
{
	int len = 0;
	va_list args;
	va_start ( args, fmt );

	debug ( LOG_DEBUG, fmt, args );

	va_end ( args );

	return len;
}

int ipset_add ( const char* const set, const char* const ent )
{
	int rv = 0;

	pthread_mutex_lock ( &uipset_mutex );

	ipset_load_types ();

	struct ipset_session* session;

	session = ipset_session_init ( ipset_print_file );

	if ( session == NULL )
		debug ( LOG_ERR, "cannot initialize ipset session, aborting" );
	else
	{
		static enum ipset_cmd cmd = IPSET_CMD_ADD;
		const struct ipset_type* type = NULL;

		if ( ipset_envopt_parse ( session, IPSET_ENV_EXIST, NULL ) != 0 )
			debug ( LOG_ERR, "ipset_envopt_parse failed: %s", ipset_session_error ( session ) );
		else if ( ipset_parse_setname ( session, IPSET_SETNAME, set ) != 0 )
			debug ( LOG_ERR, "ipset_parse_setname failed: %s", ipset_session_error ( session ));
		else if ( ( type = ipset_type_get ( session, cmd ) ) == NULL )
			debug ( LOG_ERR, "ipset_parse_setname failed: %s", ipset_session_error ( session ) );
		else if ( ipset_parse_elem ( session, type->last_elem_optional, ent ) != 0 )
			debug ( LOG_ERR, "ipset_parse_elem failed: %s", ipset_session_error ( session ) );
		else if ( ipset_cmd ( session, cmd, 0 ) != 0 )
			debug ( LOG_ERR, "ipset_cmd failed: %s", ipset_session_error ( session ) );
		else
		{
			debug ( LOG_INFO, "ipset: added %s to %s", ent, set );
			rv = 1;
		}

		ipset_session_fini ( session );
	}

	pthread_mutex_unlock ( &uipset_mutex );

	return rv;
}

int ipset_del ( const char* const set, const char* const ent )
{
	int rv = 0;

	pthread_mutex_lock ( &uipset_mutex );

	ipset_load_types ();

	struct ipset_session* session;

	session = ipset_session_init ( ipset_print_file );

	if ( session == NULL )
		debug ( LOG_ERR, "cannot initialize ipset session, aborting: %s", ipset_session_error ( session ) );
	else
	{
		static enum ipset_cmd cmd = IPSET_CMD_DEL;
		const struct ipset_type* type = NULL;

		if ( ipset_envopt_parse ( session, IPSET_ENV_EXIST, NULL ) != 0 )
			debug ( LOG_ERR, "ipset_envopt_parse failed: %s", ipset_session_error ( session ) );
		else if ( ipset_parse_setname ( session, IPSET_SETNAME, set ) != 0 )
			debug ( LOG_ERR, "ipset_parse_setname failed: %s", ipset_session_error ( session ) );
		else if ( ( type = ipset_type_get ( session, cmd ) ) == NULL )
			debug ( LOG_ERR, "ipset_parse_setname failed: %s", ipset_session_error ( session ) );
		else if ( ipset_parse_elem ( session, type->last_elem_optional, ent ) != 0 )
			debug ( LOG_ERR, "ipset_parse_elem failed: %s", ipset_session_error ( session ) );
		else if ( ipset_cmd ( session, cmd, 0 ) != 0 )
			debug ( LOG_ERR, "ipset_cmd failed: %s", ipset_session_error ( session ) );
		else
		{
			debug ( LOG_INFO, "ipset: deleted %s from %s", ent, set );
			rv = 1;
		}

		ipset_session_fini ( session );
	}

	pthread_mutex_unlock ( &uipset_mutex );

	return rv;
}

int ipset_lst ( const char* const set, const char** ent )
{
	int rv = -1;

	pthread_mutex_lock ( &uipset_mutex );

	ipset_load_types ();

	struct ipset_session* session;

	session = ipset_session_init ( ipset_print_file );

	if ( session == NULL )
		debug ( LOG_ERR, "cannot initialize ipset session, aborting: %s", ipset_session_error ( session ) );
	else
	{
		static enum ipset_cmd cmd = IPSET_CMD_LIST;

		// видимо от этого параметра здеь не холодно- не жарко, так как он только на добавление удаление
		if ( ipset_envopt_parse ( session, IPSET_ENV_EXIST, NULL ) != 0 )
			debug ( LOG_ERR, "ipset_envopt_parse failed: %s", ipset_session_error ( session ) );
		else if ( ipset_session_output ( session, IPSET_LIST_PLAIN ) != 0 )
			debug ( LOG_ERR, "ipset_session_output failed: %s", ipset_session_error ( session ) );
		else if ( ipset_parse_setname ( session, IPSET_SETNAME, set ) != 0 )
			debug ( LOG_ERR, "ipset_parse_setname failed: %s", ipset_session_error ( session ) );
		else if ( ipset_session_outfn ( session, interout ) != 0 )
			debug ( LOG_ERR, "ipset_session_outfn failed: %s", ipset_session_error ( session ) );
		else if ( ipset_cmd ( session, cmd, 0 ) != 0 )
		{
			if ( ipset_session_error ( session ) != NULL )
			{
				debug ( LOG_WARNING, "ipset_cmd failed: %s", ipset_session_error ( session ) );
				rv = 0;
			}
		}
		else
		{
			debug ( LOG_DEBUG, "ipset_lst: [%s]", last_members );
			*ent = last_members;
			rv = 1;
		}

		ipset_session_fini ( session );
	}

	pthread_mutex_unlock ( &uipset_mutex );

	return rv;
}

int ipset_tst ( const char* const set, const char* const ent )
{
	int rv = -1;

	pthread_mutex_lock ( &uipset_mutex );

	ipset_load_types ();

	struct ipset_session* session;

	session = ipset_session_init ( ipset_print_file );

	if ( session == NULL )
		debug ( LOG_ERR, "cannot initialize ipset session, aborting" );
	else
	{
		static enum ipset_cmd cmd = IPSET_CMD_TEST;
		const struct ipset_type* type = NULL;

		if ( ipset_envopt_parse ( session, IPSET_ENV_EXIST, NULL ) != 0 )
			debug ( LOG_ERR, "ipset_envopt_parse failed: %s", ipset_session_error ( session ) );
		else if ( ipset_parse_setname ( session, IPSET_SETNAME, set ) != 0 )
			debug ( LOG_ERR, "ipset_parse_setname failed: %s", ipset_session_error ( session ) );
		else if ( ( type = ipset_type_get ( session, cmd ) ) == NULL )
			debug ( LOG_ERR, "ipset_parse_setname failed: %s", ipset_session_error ( session ) );
		else if ( ipset_parse_elem ( session, type->last_elem_optional, ent ) != 0 )
			debug ( LOG_ERR, "ipset_parse_elem failed: %s", ipset_session_error ( session ) );
		else if ( ipset_cmd ( session, cmd, 0 ) != 0 )
		{
			// не одназначно, поэтому доп. проверка
			if ( ipset_session_error ( session ) == NULL )
			{
				debug ( LOG_INFO, "%s is NOT in set %s", ent, set );
				debug ( LOG_DEBUG, "ipset_cmd failed %s", ipset_session_warning ( session ) );
				rv = 0;
			}
			else
			{
				debug ( LOG_ERR, "ipset_cmd failed %s", ipset_session_warning ( session ) );
				debug ( LOG_ERR, "ipset_cmd failed %s", ipset_session_error ( session ) );
			}
		}
		else
		{
			rv = 1;
			debug ( LOG_INFO, "%s is in set %s", ent, set );
		}

		ipset_session_fini ( session );
	}

	pthread_mutex_unlock ( &uipset_mutex );

	return rv;
}
