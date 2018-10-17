#define  _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>

#include "debug.h"
#include "ipset.h"

#include "uipset.h"

#define MAXEVENTS 64

const char* const logident = "uipsetd";
const char* const version = "0.1.0";
const int logfacility = LOG_DAEMON;

const char* socket_path = 0;

void display_usage ()
{
	printf (
		"Usage: uipsetd [options]\n\noptions:\n"
		 "  -h            Print usage\n"
		 "  -v            Print version information\n"
		 "  -d <level>    Debug level\n"
		 "  -o            Log to stdout\n"
		 "  -s [filename] Use this socket patch\n" );
}

void display_version ()
{
	printf ( "version: %s\n", version );
}

int main ( int argc, char* argv[] )
{
	int rc = EXIT_FAILURE;
	int opt;
	int loop = 0;
	const char* const logprefix = "main: ";

	/* кол-во одновременных подключений к сокету ? */
	int backlog = 10;

	openlog ( logident, LOG_PID, logfacility );

	// параметры запуска
	while ( ( opt = getopt ( argc, argv, "?hvos:d:" ) ) != -1 )
	{
		switch ( opt )
		{
			case 'h':
			case '?':
				rc = EXIT_SUCCESS;
			default:
				display_usage ();
				argc = 0;
				break;
			case 'v':
				rc = EXIT_SUCCESS;
				display_version ();
				argc = 0;
				break;
			case 'd':
				debuglevel = atoi ( optarg );
				break;
			case 'o':
				outconsole = 1;
				break;
			case 's':
				socket_path = optarg;
				break;
		}
	}

	int fd = -1;
	struct sockaddr_un addr;
	int epollfd;
	struct epoll_event event;
	struct epoll_event *events;

	// проверка параметров
	if ( rc == EXIT_FAILURE )
	{
		if ( socket_path == 0 )
			debug ( LOG_ERR, "%spath to socket not specified", logprefix );
		else
		{
			// если прога завершилась некореектно, то удалить сокет
			unlink ( socket_path );

			if ( ( fd = socket ( AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0 ) ) == -1 )
				debug ( LOG_ERR, "%ssocket error", logprefix );
			else
			{
				memset ( &addr, 0, sizeof ( addr ) );

				addr.sun_family = AF_UNIX;

				strncpy ( addr.sun_path, socket_path, sizeof ( addr.sun_path ) - 1 );

				if ( bind ( fd, (struct sockaddr*)&addr, sizeof ( addr ) ) == -1 )
					debug ( LOG_ERR, "%sbind error", logprefix );
				else
				{
					chmod ( socket_path, S_IWOTH );

					if ( listen ( fd, backlog ) == -1 )
						debug ( LOG_ERR, "%slisten error", logprefix );
					else if ( ( epollfd = epoll_create ( backlog ) ) < 0 )
						debug ( LOG_ERR, "%sfailed to create epoll context: %s", logprefix, strerror ( errno ) );
					else
					{
						event.data.fd = fd;
						event.events = EPOLLIN | EPOLLET;
						int s;

						if ( ( s = epoll_ctl ( epollfd, EPOLL_CTL_ADD, fd, &event ) ) < 0 )
							debug ( LOG_ERR, "%sepoll_ctl", logprefix );
						else
						{
							events = calloc ( MAXEVENTS, sizeof event );
							loop = 1;
							debug ( LOG_INFO, "%sloop start", logprefix );
						}
					}
				}
			}
		}
	}

	// основной цикл
	while ( loop == 1 )
	{
		debug ( LOG_DEBUG, "%smain loop", logprefix );

		int i = 0;
		/* возвращает количество (один или более) файловых дескрипторов из списка наблюдения, у которых поменялось состояние
		(которые готовы к вводу-выводу). */
		// у нас всегда один?
		int n = epoll_wait ( epollfd, events, MAXEVENTS, -1 );

		for ( i = 0; i < n; i ++ )
		{
			debug ( LOG_DEBUG, "%smain loop: n: [%d] i: [%d] fd: [%d] events[i].data.fd: [%d]", logprefix, n, i, fd, events[i].data.fd );

			if ( ( events[i].events & EPOLLERR ) || ( events[i].events & EPOLLHUP ) || ( ! ( events[i].events & EPOLLIN ) ) )
			{
				/* An error has occured on this fd, or the socket is not ready for reading (why were we notified then?) */
				debug ( LOG_DEBUG, "%sepoll error", logprefix );
				close ( events[i].data.fd );
				continue;
			}
			else if ( fd == events[i].data.fd )
			{
				debug ( LOG_DEBUG, "%sloop: 1", logprefix );

				// обработка подключения
				while ( 1 )
				{
					debug ( LOG_DEBUG, "%loop: 2", logprefix );

					struct sockaddr in_addr;
					socklen_t in_len;
					in_len = sizeof ( in_addr );

					int cfd = accept ( fd, &in_addr, &in_len );

					debug ( LOG_DEBUG, "%loop: 3", logprefix );

					if ( cfd == -1 )
					{
						if ( (errno == EAGAIN) || (errno == EWOULDBLOCK) )
						{
							debug ( LOG_DEBUG, "%loop: 4", logprefix );
							/* We have processed all incoming connections. */
							break;
						}
						else
						{
							debug ( LOG_ERR, "%sloop: err: accept", logprefix );
							break;
						}
					}

					event.data.fd = cfd;
					event.events = EPOLLIN | EPOLLET;
					epoll_ctl ( epollfd, EPOLL_CTL_ADD, cfd, &event );
				}
			}
			else
			{
				debug ( LOG_DEBUG, "%sreceive and decode", logprefix );

				uipset_msg_request req = uipset_msg_request_init_zero;

				pb_istream_t input = pb_istream_from_socket ( events[i].data.fd );

				//  получить и декодировать сообщение
				if ( ! pb_decode_delimited ( &input, uipset_msg_request_fields, &req ) )
					debug ( LOG_ERR, "%spb_decode_delimited failed", logprefix );
				else
				{
					debug ( LOG_DEBUG, "%smain loop: request: cmd: [%d] set: [%s] has_ent: [%d] ent: [%s]", logprefix, req.cmd, req.set, req.has_ent, req.ent );

					int ir = -1;
					const char* lst = 0;

					uipset_msg_response rsp = uipset_msg_response_init_zero;

					rsp.ret = ir;
					rsp.has_msg = false;
					*rsp.msg = 0;

					switch ( req.cmd )
					{
						case UIPSET_CMD_ADD:
							debug ( LOG_INFO, "%sadd", logprefix );
							ir = ipset_add ( req.set, req.ent );
							break;
						case UIPSET_CMD_DEL:
							debug ( LOG_INFO, "%sdel", logprefix );
							ir = ipset_del ( req.set, req.ent );
							break;
						case UIPSET_CMD_TST:
							debug ( LOG_INFO, "%stst", logprefix );
							ir = ipset_tst ( req.set, req.ent );
							break;
						case UIPSET_CMD_LST:
							debug ( LOG_INFO, "%slst", logprefix );
							ir = ipset_lst ( req.set, &lst );

							if ( ir == 1 && lst != 0 )
							{
								debug ( LOG_DEBUG, "%slst: [%s]", logprefix, lst );
								debug ( LOG_DEBUG, "%slst:lst strlen: [%d]", logprefix, strlen ( lst ) );
								strcpy ( rsp.msg, lst );
								rsp.has_msg = true;
							}
							break;
						default:
							debug ( LOG_INFO, "%sunknown", logprefix );
							break;
					}

					rsp.ret = ir;

					debug ( LOG_DEBUG, "%srsp.ret: %d", logprefix, rsp.ret );

					pb_ostream_t output = pb_ostream_from_socket ( events[i].data.fd );

					if ( ! pb_encode_delimited ( &output, uipset_msg_response_fields, &rsp ) )
						debug ( LOG_ERR, "%spb_encode_delimited failed", logprefix );
					else
						debug ( LOG_INFO, "%srequest processed", logprefix );
				}
			}
		}
	}

	unlink ( socket_path );
	closelog ();

	exit ( rc );
}
