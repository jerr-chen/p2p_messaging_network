#include "socket.h"


#include <stdio.h>
#include <stdlib.h>
// system logging
#include <string.h>
#include <syslog.h>
#include <errno.h>

Socket::Socket()
{
	(this->address).sin_family = AF_INET;
	(this->address).sin_addr.s_addr = INADDR_ANY;
	this->port = -1;
	this->socket_fd = -1;
}

Socket::~Socket()
{
	/* TODO: Properly close socketfd.
	 * 0: stop receiving data
	 * 1: stop transmissing data
	 * 2: stop receiving and transmitting
	 */
	shutdown(this->socket_fd, 2);
}

/* Server TCP socket methods */
int Socket::setup_server_socket(int port)
{
	int success = 0;
	this->port = port;
	if(create())
	{
		if(attach(port))
		{
			if(listen_port())
			{
				success = 1;
			}
		}
	}
	return success;
}

int Socket::create()
{
	// create socket file descriptor
	this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(this->socket_fd == 0)
	{
		syslog(LOG_ERR, "[Socket] Failed to create server socket: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return 1;
}

int Socket::attach(int port)
{
	int opt = 1;
	int ret_check;

	// setting socket options
	ret_check = setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
	if(ret_check)
	{
		syslog(LOG_ERR, "[Socket] Failed to set socket options: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// attach socket to port
	(this->address).sin_port = htons(port);
	ret_check = bind(this->socket_fd, (struct sockaddr*) &(this->address), sizeof(address));
	if(ret_check < 0)
	{
		syslog(LOG_ERR, "[Socket] Failed to bind to port %d: %s", port, strerror(errno));
		exit(EXIT_FAILURE);
	}

	return 1;
}

int Socket::listen_port()
{
	int ret_check;

	// TODO: Consider other max backlog values
	// 3 : max backlog of connections
	ret_check = listen(this->socket_fd, 3);
	if(ret_check < 0)
	{
		syslog(LOG_ERR, "[Socket] Failed to listen on port %d: %s", this->port, strerror(errno));
		exit(EXIT_FAILURE);
	}

	return 1;
}

int Socket::accept_conn()
{
	int sock, addrlen;

	// accept incoming connection
	addrlen = sizeof(this->address);
	sock = accept(this->socket_fd, (struct sockaddr*) &(this->address), (socklen_t*) &addrlen);
	if(sock < 0)
	{
		syslog(LOG_WARNING, "[Socket] Failed to accept connection: %s", strerror(errno));
	}

	return sock;
}
