#define  _GNU_SOURCE

// https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c/
// https://codereview.stackexchange.com/questions/98558/non-blocking-unix-domain-socket

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
	printf ( "usage:\n" );
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
//	int cl = -1;
	struct sockaddr_un addr;
	int epollfd;
	struct epoll_event event;
	struct epoll_event *events;

	// проверка параметров
	if ( socket_path != 0 )
	{
		// если прога завершилась некореектно, то удалить сокет
		unlink ( socket_path );

		if ( ( fd = socket ( AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0 ) ) == -1 )
		{
			debug ( LOG_ERR, "socket error" );
		}
		else
		{
			memset ( &addr, 0, sizeof ( addr ) );

			addr.sun_family = AF_UNIX;

			strncpy ( addr.sun_path, socket_path, sizeof ( addr.sun_path ) - 1 );

			if ( bind ( fd, (struct sockaddr*)&addr, sizeof ( addr ) ) == -1 )
				debug ( LOG_ERR, "bind error" );
			else
			{
				chmod ( socket_path, S_IWOTH );

				if ( listen ( fd, backlog ) == -1 )
					debug ( LOG_ERR, "listen error" );
				else if ( ( epollfd = epoll_create ( backlog ) ) < 0 )
					debug ( LOG_ERR, "failed to create epoll context: %s", strerror ( errno ) );
				else
				{
					event.data.fd = fd;
					event.events = EPOLLIN | EPOLLET;
					int s;

					if ( ( s = epoll_ctl ( epollfd, EPOLL_CTL_ADD, fd, &event ) ) < 0 )
						debug ( LOG_ERR, "epoll_ctl" );
					else
					{
						events = calloc ( MAXEVENTS, sizeof event );
						loop = 1;
						debug ( LOG_INFO, "start loop" );
					}
				}
			}
		}
	}

	// основной цикл
	while ( loop == 1 )
	{
		int i = 0;
		/* возвращает количество (один или более) файловых дескрипторов из списка наблюдения, у которых поменялось состояние
		(которые готовы к вводу-выводу). */
		// у нас всегда один?
		int n = epoll_wait ( epollfd, events, MAXEVENTS, -1 );

		for ( i = 0; i < n; i ++ )
		{
			debug ( LOG_ERR, "n: %d", n );
			debug ( LOG_ERR, "events[i].data.fd: %d", events[i].data.fd );

			if ( ( events[i].events & EPOLLERR ) || ( events[i].events & EPOLLHUP ) || ( ! ( events[i].events & EPOLLIN ) ) )
			{
				/* An error has occured on this fd, or the socket is not ready for reading (why were we notified then?) */
				debug ( LOG_ERR, "epoll error" );
				close ( events[i].data.fd );
				continue;
			}
			else if ( fd == events[i].data.fd )
			{
				// обработка подключения
				while ( 1 )
				{
				debug ( LOG_ERR, "eblishen" );

				struct sockaddr in_addr;
				socklen_t in_len;
				in_len = sizeof ( in_addr );

//				struct sockaddr_in caddr;
//				memset ( &caddr, 0, sizeof ( caddr ) );
			//	caddr.sun_family = AF_UNIX;
				int cfd = accept ( fd, &in_addr, &in_len );
				debug ( LOG_ERR, "nuuuuuuuuuuuu" );

				if ( cfd == -1 )
				{
					if ( (errno == EAGAIN) || (errno == EWOULDBLOCK) )
					{
						debug ( LOG_INFO, "tipa ok?");
						/* We have processed all incoming connections. */
						break;
					}
					else
					{
						debug ( LOG_ERR, "err: accept" );
						break;
					}
				}

				char hbuf[256], sbuf[256];
				getnameinfo ( &in_addr, in_len, hbuf, sizeof hbuf, sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV );

debug( LOG_ERR, "Accepted connection on descriptor %d "
                             "(host=%s, port=%s)\n", cfd, hbuf, sbuf);

				event.data.fd = cfd;
				event.events = EPOLLIN | EPOLLET;
				epoll_ctl ( epollfd, EPOLL_CTL_ADD, cfd, &event);
				}
			}
			else
			{
				// ********
				uipset_msg_request req = uipset_msg_request_init_zero;

				pb_istream_t input = pb_istream_from_socket ( events[i].data.fd );

				if ( ! pb_decode_delimited ( &input, uipset_msg_request_fields, &req ) )
				{
					debug ( LOG_INFO, "pidaras!" );
				}
				else
				{
					int ir = 1;

					switch ( req.cmd )
					{
						case UIPSET_CMD_ADD:
							debug ( LOG_INFO, "add" );
							ir = ipset_ip_add ( req.set, req.ipa );
							break;
						case UIPSET_CMD_DEL:
							debug ( LOG_INFO, "del" );
							ir = ipset_ip_del ( req.set, req.ipa );
							break;
						default:
							debug ( LOG_INFO, "chto za nahuy?" );
							break;
					}

					uipset_msg_response rsp = uipset_msg_response_init_zero;

					rsp.error = ir;

					pb_ostream_t output = pb_ostream_from_socket ( events[i].data.fd );

					if ( ! pb_encode_delimited ( &output, uipset_msg_response_fields, &rsp ) )
					{
					}
					else
					{
						debug ( LOG_INFO, "ja ja!" );
					}
				}

				// ********

				// обрабтка данных
//				char buf[512];
//				debug ( LOG_ERR, "o ja ebuntung" );
//				ssize_t count = read (events[i].data.fd, buf, sizeof buf);
//				debug ( LOG_INFO, buf );
//				if ( strcmp ( buf, "add" ) == 0 )
//				{
//					debug ( LOG_INFO, "add" );
//					ipset_ip_add ( "test", "127.0.0.1" );
//				}
//				recv ( cfd, &message, sizeof message, 0);
				//read ( events[i].data.fd, buf, sizeof ( buf ) );
			}
		}
	}

	unlink ( socket_path );
	closelog ();

	exit ( rc );
}
