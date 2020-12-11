#include "includes/swsocket.h"
#include "includes/sapi.h"
#include "includes/sparser.h"
#include "includes/stable_utils.h"

void defineSocketConstants(VM* vm) {
  // Address/protocol families
  defineGlobal(vm,"AF_UNIX", NUM_CONST(AF_UNIX));
  defineGlobal(vm,"AF_INET", NUM_CONST(AF_INET));
  defineGlobal(vm,"AF_INET6", NUM_CONST(AF_INET6));
  defineGlobal(vm,"PF_INET", NUM_CONST(PF_INET));

  // Socket types
  defineGlobal(vm,"SOCK_STREAM", NUM_CONST(SOCK_STREAM));
  defineGlobal(vm,"SOCK_DGRAM", NUM_CONST(SOCK_DGRAM));
  defineGlobal(vm,"SOCK_RAW", NUM_CONST(SOCK_RAW));
  defineGlobal(vm,"SOCK_RDM", NUM_CONST(SOCK_RDM));
  defineGlobal(vm,"SOCK_SEQPACKET", NUM_CONST(SOCK_SEQPACKET));

  defineGlobal(vm,"SO_DEBUG", NUM_CONST(SO_DEBUG));
  defineGlobal(vm,"SO_ACCEPTCONN", NUM_CONST(SO_ACCEPTCONN));
  defineGlobal(vm,"SO_REUSEADDR", NUM_CONST(SO_REUSEADDR));
  defineGlobal(vm,"SO_KEEPALIVE", NUM_CONST(SO_KEEPALIVE));
  defineGlobal(vm,"SO_DONTROUTE", NUM_CONST(SO_DONTROUTE));
  defineGlobal(vm,"SO_BROADCAST", NUM_CONST(SO_BROADCAST));
  defineGlobal(vm,"SO_USELOOPBACK", NUM_CONST(SO_USELOOPBACK));
  defineGlobal(vm,"SO_LINGER", NUM_CONST(SO_LINGER));
  defineGlobal(vm,"SO_OOBINLINE", NUM_CONST(SO_OOBINLINE));
  defineGlobal(vm,"SO_DONTLINGER", NUM_CONST(SO_DONTLINGER));
  defineGlobal(vm,"SO_SNDBUF", NUM_CONST(SO_SNDBUF));
  defineGlobal(vm,"SO_RCVBUF", NUM_CONST(SO_RCVBUF));
  defineGlobal(vm,"SO_SNDLOWAT", NUM_CONST(SO_SNDLOWAT));
  defineGlobal(vm,"SO_RCVLOWAT", NUM_CONST(SO_RCVLOWAT));
  defineGlobal(vm,"SO_SNDTIMEO", NUM_CONST(SO_SNDTIMEO));
  defineGlobal(vm,"SO_RCVTIMEO", NUM_CONST(SO_RCVTIMEO));
  defineGlobal(vm,"SO_ERROR", NUM_CONST(SO_ERROR));
  defineGlobal(vm,"SO_TYPE", NUM_CONST(SO_TYPE));
  defineGlobal(vm,"SOMAXCONN", NUM_CONST(SOMAXCONN));

  defineGlobal(vm,"SOL_SOCKET", NUM_CONST(SOL_SOCKET));

  defineGlobal(vm,"IPPROTO_TCP", NUM_CONST(IPPROTO_TCP));

}

void defineSocketAddrClass(VM* vm) {
  GCClass* sockAddrClass = newClass(vm, copyString(vm, NULL, "SockAddress", 11), false);
  defineClassNativeField(vm, "family", NULL_CONST, sockAddrClass);
  defineClassNativeField(vm, "port", NULL_CONST, sockAddrClass);
  defineClassNativeField(vm, "address", NULL_CONST, sockAddrClass);
  defineClassNativeField(vm, "zero", NULL_CONST, sockAddrClass);

  defineGlobal(vm, "SockAddress", GC_OBJ_CONST(sockAddrClass));
}

Constant sglSocket(VM* vm, int arity, Constant* args) {
  expect(3, arity, "socket");

  int family = AS_NUMBER(args[0]);
  int type = AS_NUMBER(args[1]);
  int protocol = AS_NUMBER(args[2]);

  return NUM_CONST(socket(family, type, protocol));
}

Constant sglSetSocketOptions(VM* vm, int arity, Constant* args) {
  expect(5, arity, "setSocketOptions");

  SOCKET sock = AS_NUMBER(args[0]);
  int level = AS_NUMBER(args[1]);
  int optName = AS_NUMBER(args[2]);
  char* optVal = "";
  int optLen = AS_NUMBER(args[4]);

  int res = setsockopt(sock, level, optName, optVal, optLen);
  args[3] = GC_OBJ_CONST(copyString(vm, NULL, optVal, strlen(optVal)));

  return NUM_CONST(res);
}

Constant sglConnect(VM* vm, int arity, Constant* args) {
  expect(5, arity, "connect");

  SOCKET sock = AS_NUMBER(args[0]);
  short family = AS_NUMBER(args[1]);
  u_short port = AS_NUMBER(args[2]);
  GCList* address = AS_LIST(args[3]);
  int hLength = AS_NUMBER(args[4]);

  char addr[4] = {AS_NUMBER(address->elements[3]), AS_NUMBER(address->elements[2]),  AS_NUMBER(address->elements[1]), AS_NUMBER(address->elements[0])};

  struct sockaddr_in server;
  memset((char*)&server, 0, sizeof(server)); // Precaution
  memcpy( (void*)&server.sin_addr,addr, hLength);
  server.sin_family = family;
  server.sin_port = (unsigned short)htons(port);

  return NUM_CONST(connect(sock, (const struct sockaddr*)&server, sizeof(server)));
}

Constant sglINetNtoa(VM* vm, int arity, Constant* args) {
  expect(1, arity, "inetNtoa");

  GCList* address = AS_LIST(args[0]);

  char addr[4] = {AS_NUMBER(address->elements[3]), AS_NUMBER(address->elements[2]),  AS_NUMBER(address->elements[1]), AS_NUMBER(address->elements[0])};

  struct in_addr inAddr;
  memset((char*)&inAddr, 0, sizeof(inAddr)); // Precaution
  memcpy( (void*)&inAddr,addr, 4);

  char* res = inet_ntoa(inAddr);

  return GC_OBJ_CONST(copyString(vm, NULL, res, strlen(res)));
}

Constant sglSend(VM* vm, int arity, Constant* args) {
  expect(3, arity, "send");

  SOCKET sock = AS_NUMBER(args[0]);
  char* buf = AS_CSTRING(args[1]);
  int flags = AS_NUMBER(args[2]);

  return NUM_CONST(send(sock, buf, strlen(buf), flags));
}

Constant sglReceive(VM* vm, int arity, Constant* args) {
  expect(3, arity, "receive");

  SOCKET sock = AS_NUMBER(args[0]);
  int length = AS_NUMBER(args[1]);
  int flags = AS_NUMBER(args[2]);

  char buffer[length+1];

  recv(sock, buffer, length, flags);

  return GC_OBJ_CONST(copyString(vm, NULL, buffer, length));
}

Constant sglClose(VM* vm, int arity, Constant* args) {
  expect(1, arity, "closeSocket");

  SOCKET sock = AS_NUMBER(args[0]);

#ifdef _WIN32
  int ret = closesocket(sock);
#else
  int ret = close(sock);
#endif

  return AS_NUMBER(ret);
}

// HostEnt {
//  var name;
//  var aliases;
//  var addrType;
//  var length;
//  var addresses;
Constant sglGetHostByName(VM* vm, int arity, Constant* args) {
  GCClass* hostEntClass = newClass(vm, copyString(vm, NULL, "HostEnt", 7), false);

  char* name = AS_CSTRING(args[0]);
  struct hostent* hostEnt = gethostbyname(name);

  if (!hostEnt)
    return NULL_CONST;

  tableInsert(vm, &hostEntClass->fields, copyString(vm, NULL, "name", 4),
              GC_OBJ_CONST(copyString(vm, NULL, hostEnt->h_name, strlen(hostEnt->h_name))));

  int aliasesLen = 0;

  while (hostEnt->h_aliases[aliasesLen] != 0)
    aliasesLen++;

  GCList* aliases = newList(vm, aliasesLen);

  for (int i = 0; i < aliasesLen; i++) {
    GCList* aliasesElementList = newList(vm, 4);

    aliasesElementList->elements[aliasesLen - 1] = NUM_CONST((unsigned char)hostEnt->h_aliases[i][3]);
    aliasesElementList->elements[aliasesLen - 2] = NUM_CONST((unsigned char)hostEnt->h_aliases[i][2]);
    aliasesElementList->elements[aliasesLen - 3] = NUM_CONST((unsigned char)hostEnt->h_aliases[i][1]);
    aliasesElementList->elements[aliasesLen - 4] = NUM_CONST((unsigned char)hostEnt->h_aliases[i][0]);
    aliasesElementList->elementC = 4;

    aliases->elements[i] = GC_OBJ_CONST(aliasesElementList);
  }

  aliases->elementC = aliasesLen;

  tableInsert(vm, &hostEntClass->fields, copyString(vm, NULL, "aliases", 7),GC_OBJ_CONST(aliases));
  tableInsert(vm, &hostEntClass->fields, copyString(vm, NULL, "addrType", 8), NUM_CONST(hostEnt->h_addrtype));
  tableInsert(vm, &hostEntClass->fields, copyString(vm, NULL, "length", 6), NUM_CONST(hostEnt->h_length));

  int hAddrLen = 0;

  while (hostEnt->h_addr_list[hAddrLen] != 0)
    hAddrLen++;

  GCList* hAddrList = newList(vm, hAddrLen);

  for (int i = 0; i < hAddrLen; i++) {
    GCList* hAddrElementList = newList(vm, 4);

    hAddrElementList->elements[aliasesLen - 1] = NUM_CONST((unsigned char)hostEnt->h_addr_list[i][3]);
    hAddrElementList->elements[aliasesLen - 2] = NUM_CONST((unsigned char)hostEnt->h_addr_list[i][2]);
    hAddrElementList->elements[aliasesLen - 3] = NUM_CONST((unsigned char)hostEnt->h_addr_list[i][1]);
    hAddrElementList->elements[aliasesLen - 4] = NUM_CONST((unsigned char)hostEnt->h_addr_list[i][0]);

    hAddrElementList->elementC = 4;

    hAddrList->elements[i] = GC_OBJ_CONST(hAddrElementList);
  }

  hAddrList->elementC = hAddrLen;

  tableInsert(vm, &hostEntClass->fields, copyString(vm, NULL, "addresses", 9),GC_OBJ_CONST(hAddrList));

  return GC_OBJ_CONST(newInstance(vm, hostEntClass));
}

Constant initSocketLib(VM *vm, int arity, Constant *args) {
#ifdef _WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2,0), &wsaData);
  atexit((void(*)())WSACleanup);
#endif

  defineSocketConstants(vm);
  defineSocketAddrClass(vm);

  defineGlobalFunc(vm, "socket", sglSocket);
  defineGlobalFunc(vm, "setSocketOptions", sglSetSocketOptions);
  defineGlobalFunc(vm, "connect", sglConnect);
  defineGlobalFunc(vm, "send", sglSend);
  defineGlobalFunc(vm, "receive", sglReceive);
  defineGlobalFunc(vm, "closeSocket", sglClose);
  defineGlobalFunc(vm, "getHostByName", sglGetHostByName);

  defineGlobalFunc(vm, "inetNtoa", sglINetNtoa);
}
