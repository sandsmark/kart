#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef __MINGW32__
#include <winsock2.h>
#include "windowssucks/inet_v6defs.h"
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

#include "net.h"

static unsigned input_mask = 0;

static void die(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

int net_start_server(int port)
{
	int sockfd, err;
	struct sockaddr_in serv;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		die("Failed to create socket");
    int reuseaddr = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = INADDR_ANY;
	serv.sin_port = htons(port);
	if ((err = bind(sockfd, (struct sockaddr *)&serv, sizeof(serv))) < 0)
		die("Failed to bind to port");
	if ((err = listen(sockfd, 10)) < 0)
		die("Failed to call listen on socket");
	printf("Started server on port %d\n", port);
	return sockfd;
}

int net_start_client(const char *addr, int port)
{
	int sockfd, err;
	struct sockaddr_in serv;
	struct timeval tv;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		die("Failed to create socket");
	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);
	if ((err = inet_pton(AF_INET, addr, &serv.sin_addr)) < 1)
		die("inet_ptons failed, inet address probably not valid");
	if ((err = connect(sockfd, (struct sockaddr *)&serv, sizeof(serv))) < 0)
		die("Failed to connect to server");
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&tv, sizeof(tv));
	printf("Connected to server on %s:%d\n", addr, port);
	return sockfd;
}

int net_init()
{
    // fuck you, windows
#ifdef __MINGW32__
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error: %d\n", err);
        exit(1);
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        exit(1);
    }
    else
        printf("The Winsock 2.2 dll was found okay\n");

#endif
    return 0;
}

void net_cleanup()
{
    // fuck windows for realz, do I have to clean up everything after you
#ifdef __MINGW32__
    WSACleanup();
#endif
}

int net_accept(int sockfd)
{
	int clientfd = accept(sockfd, (struct sockaddr*)NULL, NULL);
	printf("Got connection: %d\n", clientfd);
	if (clientfd < 0)
		printf("accept failed\n");
	return clientfd;
}

ssize_t net_recv(int sockfd, char *buf, int buf_len)
{
	ssize_t n;
	n = recv(sockfd, buf, buf_len, 0);
	if (n > 0)
		buf[n] = '\0';
	return n;
}

void net_set_input(unsigned input)
{
	input_mask |= input;
}

ssize_t net_send_input(int sockfd)
{
	ssize_t n;
	char send_buf[64];
	snprintf(send_buf, 64, "%d", input_mask);
	n = send(sockfd, send_buf, strlen(send_buf), 0);
	input_mask = 0;
	return n;
}

ssize_t net_send(int sockfd, char *msg)
{
	return send(sockfd, msg, strlen(msg), 0);
}

void net_close(int sockfd)
{
	close(sockfd);
}
