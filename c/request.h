#ifndef REQUEST_H

#define PORT 8080
#define QUEUELIMIT 5
#define RECVBUFFERSIZE 1024

struct httpMessage {
  char *requestLine;
  char **header;
  char *body;
};

struct bufferOffset {
  char *buffer;
  int offset;
};
struct httpRequest {
  char *method;
  char *path;
  char *version;
  char **header;
  char *body;
};

struct httpRequest *handleHttpRequestMessage(char *buffer, int bufsize,
                                             int clntSock);

#endif
#define REQUEST_H
