#ifndef NET_H
#define NET_H

#define NET_INPUT_UP 1<<0
#define NET_INPUT_DOWN 1<<1
#define NET_INPUT_LEFT 1<<2
#define NET_INPUT_RIGHT 1<<3
#define NET_INPUT_SPACE 1<<4
#define NET_INPUT_RETURN 1<<5

int net_init();
void net_cleanup();
int net_start_server(int port);
int net_start_client(const char *addr, int port);
int net_accept();
ssize_t net_recv(int socket, char *buf, int buf_len);
void net_set_input(unsigned input);
ssize_t net_send_input(int sockfd);
ssize_t net_send(int socket, char *buf);
void net_close(int socket);

#endif /*NET_H*/
