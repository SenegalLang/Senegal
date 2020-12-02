#ifndef SENEGAL_SWSOCKET_H
#define SENEGAL_SWSOCKET_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#include "svm.h"

Constant initSocketLib(VM* vm, int arity, Constant* args);
#endif

#endif //SENEGAL_SWSOCKET_H
