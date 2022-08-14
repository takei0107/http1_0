#ifndef REQUEST_H

#define PORT 8080
#define QUEUELIMIT 5
#define RECVBUFFERSIZE 1024

#include "./httpType.h"

struct bufferOffset {
  char *buffer;
  int offset;
};

struct httpRequest *handleHttpRequestMessage(char *buffer, int bufsize,
                                             int clntSock);

#endif
#define REQUEST_H
