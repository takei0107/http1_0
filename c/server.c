#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "./response.c"

#define PORT 8080
#define QUEUELIMIT 5
#define RECVBUFFERSIZE 1024

void startServer() {
  int servSock; // server socket
  int clntSock; // client socket

  struct sockaddr_in servAddr; // server address
  struct sockaddr_in clntAddr; // client address

  unsigned int clntLen; // size of clntAddr

  char **lines;

  struct httpRequest *req;
  int resSize;

  char *httpResponseMessage;

  // create tcp-socket
  if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    fprintf(stderr, "socket() failed");
    exit(1);
  }

  // initialize server address
  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(PORT);

  // bind
  if (bind(servSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
    fprintf(stderr, "bind() failed");
    exit(1);
  }

  // listen
  if (listen(servSock, QUEUELIMIT) < 0) {
    fprintf(stderr, "listen() failed");
    exit(1);
  }

  clntLen = sizeof(clntAddr);

  // accept
  if ((clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &clntLen)) <
      0) {
    fprintf(stderr, "accept() failed");
    exit(1);
  }

  char recvBuffer[RECVBUFFERSIZE];
  memset(recvBuffer, 0, sizeof(recvBuffer));
  int recvSize;

  if ((recvSize = recv(clntSock, recvBuffer, RECVBUFFERSIZE, 0)) < 0) {
    fprintf(stderr, "recv() failed");
    exit(1);
  }

  req = handleHttpRequestMessage(recvBuffer, recvSize, clntSock);
  httpResponseMessage = createHttpResponseMessage(req, &resSize);

  if (send(clntSock, httpResponseMessage, resSize, 0) != resSize) {
    printf("send() failed");
    exit(1);
  }

  close(clntSock);
  close(servSock);
}
