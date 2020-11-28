#ifdef __WIN32__
# include <winsock2.h>
#include <ws2tcpip.h>
#else
# include <sys/socket.h>
#endif

#include "includes/siolib.h"
#include "includes/sapi.h"
#include "includes/sparser.h"

typedef struct socket_t {
    int sockfd;
    int family;
    int type;
    int mcast;
    int protocol;
    int listening;
} Socket;

static void initSocket(Socket* sock, int family, int type, int mcast, int protocol, int listening) {
  if (sock->sockfd == -1)
    return;

  int on = 1;
  setsockopt(sock->sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

  sock->family = family;
  sock->type = type;
  sock->mcast = mcast;
  sock->protocol = protocol;
  sock->listening = listening;
}

static Constant sglClock(VM* vm, int arity, Constant* args) {
  return NUM_CONST((double)clock());
}

static Constant sglReadLine(VM* vm, int arity, Constant* args) {
  char* line = NULL;
  size_t len;

  size_t lineLen = getline(&line, &len, stdin);

  return GC_OBJ_CONST(copyString(vm, NULL, line, lineLen));
}

static Constant sglSocketConnect(VM* vm, int arity, Constant* args) {
  int type = SOCK_STREAM;

  char sabuf[sizeof(struct sockaddr_in6)];
  struct sockaddr* sa = (struct sockaddr*)sabuf;
  socklen_t slen = sizeof(sabuf);

  int family = AF_INET;
  int protocol = 0;
  int mcast = 0;

  char* string = AS_CSTRING(args[0]);

  if (!strcmp(string, "")) {
    if (!strcasecmp(string, "udp")) {
      type = SOCK_DGRAM;
    } else if (!strcasecmp(string, "mcast")) {
      type = SOCK_DGRAM;
      mcast = 1;
    }
  }
  const char* addr = AS_CSTRING(args[1]);
  int port = AS_NUMBER(args[2]);
  int ttl = AS_NUMBER(args[3]);

  //  int err = _gethostaddr()...

  Socket sock;
  sock.sockfd = socket(family, type, protocol);
  initSocket(&sock, family, type, mcast, protocol, 0);

  if (mcast) {
    int ok;

    if (setsockopt(sock.sockfd, SOL_SOCKET, SO_BROADCAST, (char *) &mcast, sizeof(mcast)) < 0) {
      // TODO(Calamity210): Error
      return NUM_CONST(0);
    }

    if (family == AF_INET)
      ok = setsockopt(sock.sockfd, IPPROTO_IP, IP_MULTICAST_TTL, (char *) &ttl, sizeof(ttl));
    else
      ok = setsockopt(sock.sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (char *) &ttl, sizeof(ttl));

    if (ok < 0) {
      // TODO: Error
      return NUM_CONST(0);
    }
  }

    if (connect(sock.sockfd, sa, slen) > 0) {
      // TODO: Error
    }

    return NUM_CONST(1);
}

Constant initIoLib(VM* vm, int arity, Constant* args) {
  #ifdef __WIN32__
    WORD versionWanted = MAKEWORD(1, 1);
    WSADATA wsaData;
    WSAStartup(versionWanted, &wsaData);
  #endif

  defineGlobal(vm, "CLOCKS_PER_SEC", NUM_CONST(CLOCKS_PER_SEC));
  defineGlobalFunc(vm, "clock", sglClock);
  defineGlobalFunc(vm, "readln", sglReadLine);
  defineGlobalFunc(vm, "socketConnect", sglSocketConnect);

}