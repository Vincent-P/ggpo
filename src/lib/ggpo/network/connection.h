#pragma once
#include "types.h"

typedef struct _conn_Socket* conn_Socket; // SOCKET
typedef struct _conn_Address* conn_Address; // sockaddr_in

conn_Socket conn_open(uint16 port);
void conn_close(conn_Socket socket);

void conn_send(conn_Socket socket, conn_Address remote, void const *data, uint32 size, int flags);
int conn_receive(conn_Socket socket, uint8 *buf, uint32 size, conn_Address *out_address);

bool conn_support_ip_port();
conn_Address conn_address_from_ip_port(char *ip, uint16 port);
bool conn_addr_is_equal(conn_Address a, conn_Address b);
