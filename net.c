#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __MINGW32__
#include <winsock2.h>
#include "windowssucks/inet_v6defs.h"
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

static void die(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

int start_server(int port)
{
	int sockfd, err;
	struct sockaddr_in serv_addr;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		die("Failed to create socket");
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if ((err = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
		die("Failed to bind to port");
	if ((err = listen(sockfd, 10)) < 0)
		die("Failed to call listen on socket");
	printf("Started server on port %d\n", port);
	return sockfd;
}

int start_client(const char *addr, int port)
{
	int sockfd, err;
	struct sockaddr_in serv_addr;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		die("Failed to create socket");
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	if ((err = inet_pton(AF_INET, addr, &serv_addr.sin_addr)) < 1)
		die("inet_ptons failed, inet address probably not valid");
	if ((err = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
		die("Failed to connect to server");
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
