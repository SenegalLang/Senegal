#ifndef SENEGAL_SSOCKET_H
#define SENEGAL_SSOCKET_H

#include "svm.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>

    #include <netinet/in.h>
    #include <netdb.h>
    #include <arpa/inet.h>

typedef unsigned char	u_char;
typedef unsigned short	u_short;
typedef unsigned int	u_int;
typedef unsigned long	u_long;

#define SOCKET u_int
#define SO_DONTLINGER			 (u_int)(~128)
#define SO_USELOOPBACK				  64

#define closesocket(sock) close(sock)
#endif

Constant initSocketLib(VM* vm, int arity, Constant* args);

#endif //SENEGAL_SSOCKET_H
