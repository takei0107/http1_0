#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define QUEUELIMIT 5

#define RECVBUFFERSIZE 1024

struct httpMessage {
  char *requestLine;
  char **header;
  char *body;
};

struct httpRequest {
  char *method;
  char *path;
  char *version;
  char **header;
  char *body;
};

struct httpResponse {
  char *version;
  short statusCode;
  char *reason;
  char **header;
  char *body;
};

struct bufferOffset {
  char *buffer;
  int offset;
};

static int getLineLenToCrlf(struct bufferOffset *bo, int bufsize) {
  char *buffer;
  int offset;
  int l;
  char c;
  int nc;
  int hasCrlf;

  buffer = bo->buffer;
  offset = bo->offset;

  l = 0;
  nc = 0;
  hasCrlf = 0;

  for (; offset < bufsize; offset++) {
    c = buffer[offset];

    if (hasCrlf) {
      break;
    }

    if (c == '\r') {
      if (nc > 0) {
        nc = 0;
      }
      nc++;
    } else if (c == '\n') {
      if (nc == 1) {
        hasCrlf = 1;
      }
      nc = 0;
    } else {
      l++;
      nc = 0;
    }
  }

  if (!hasCrlf) {
    l = -1;
  }

  bo->offset = offset;
  return l;
}

static char *getRequestLineToken(char *requestLine, int pos) {
  char *token;
  int i;
  int j;
  int k;
  int pc;
  int st;
  char c;

  j = 0;
  k = 0;
  pc = 0;
  st = 0;
  for (i = 0; i < strlen(requestLine) + 1; i++) {
    c = requestLine[i];
    if (c == ' ' || c == '\0') {
      pc++;
      if (pc == pos) {
        token = (char *)malloc(k + 1);
        for (j = 0; j < k; j++) {
          token[j] = requestLine[st + j];
        }
        token[k] = '\0';
      } else {
        k = 0;
      }
      st = i + 1;
    }
    k++;
  }
  return token;
}

static void handleRequestLine(char *requestLine, struct httpRequest *req) {
  char *method = getRequestLineToken(requestLine, 1);
  char *path = getRequestLineToken(requestLine, 2);
  char *version = getRequestLineToken(requestLine, 3);

  req->method = method;
  req->path = path;
  req->version = version;
}

static void handleHeader(char **header, struct httpRequest *req) {
  req->header = header;
}

static struct httpResponse *createResponse(struct httpRequest *req) {
  char *reason_200 = "OK";
  char *reason_405 = "Method Not Allowed";

  struct httpResponse *res;
  res = (struct httpResponse *)malloc(sizeof(struct httpResponse));

  if ((strcmp(req->method, "GET") != 0) && (strcmp(req->method, "HEAD"))) {
    res->statusCode = 405;
    res->reason = reason_405;
  } else {
    res->statusCode = 200;
    res->reason = reason_200;
  }
  res->version = req->version;
  return res;
}

static char *createHttpResponseMessage(struct httpResponse *res, int *size) {
  char *message;
  int s = 0;
  char scStr[3];
  char *crlf = "\r\n";
  char sp = ' ';
  int i = 0;
  int j;

  sprintf(scStr, "%d", res->statusCode);

  s += strlen(res->version);
  s += 1; // SP
  s += strlen(scStr);
  s += 1; // SP
  s += strlen(res->reason);
  s += strlen(crlf);
  s += strlen(crlf);

  message = (char *)malloc(s);

  for (j = 0; j < strlen(res->version); j++) {
    message[i++] = res->version[j];
  }
  message[i++] = sp;

  for (j = 0; j < strlen(scStr); j++) {
    message[i++] = scStr[j];
  }
  message[i++] = sp;

  for (j = 0; j < strlen(res->reason); j++) {
    message[i++] = res->reason[j];
  }
  message[i++] = crlf[0];
  message[i++] = crlf[1];
  message[i++] = crlf[0];
  message[i++] = crlf[1];

  *size = s;
  return message;
}

static void sendResponse(struct httpResponse *res, int clntSock) {
  char *httpResponseMessage;
  int size;

  httpResponseMessage = createHttpResponseMessage(res, &size);

  if (send(clntSock, httpResponseMessage, size, 0) != size) {
    printf("send() failed");
    exit(1);
  }

  close(clntSock);
}

static void handleHttpMessage(char *buffer, int bufsize, int clntSock) {
  int i;
  int j;
  int lineLen;
  int rowNum;
  int offset;
  char c;
  struct bufferOffset bo;
  struct httpMessage hm;
  char *requestLine;
  char **header;
  char *body;
  char *line;
  int bodysize;
  struct httpRequest *req;
  struct httpResponse *res;

  bo.offset = 0;
  bo.buffer = buffer;

  offset = 0;
  lineLen = 0;

  // 1行目(request-line)
  lineLen = getLineLenToCrlf(&bo, bufsize);
  offset = bo.offset;
  requestLine = (char *)malloc(lineLen + 1);
  for (i = 0; i < lineLen; i++) {
    c = buffer[i];
    requestLine[i] = c;
  }
  requestLine[lineLen] = '\0';
  hm.requestLine = requestLine;

  // 2行目 ~ 区切り行まで
  // 一旦区切り行までの行数を求める
  rowNum = 0;
  while (getLineLenToCrlf(&bo, bufsize) != 0) {
    rowNum++;
  }
  // offset戻す
  bo.offset = offset;
  // 行数分allocate
  header = (char **)malloc(rowNum);
  for (j = 0; j < rowNum; j++) {
    lineLen = getLineLenToCrlf(&bo, bufsize);
    line = (char *)malloc(lineLen + 1);
    for (i = 0; i < lineLen; i++) {
      c = buffer[offset + i];
      line[i] = c;
    }
    line[lineLen] = '\0';
    header[j] = line;
    offset = bo.offset;
  }
  hm.header = header;

  // 区切り行スキップ
  getLineLenToCrlf(&bo, bufsize);
  offset = bo.offset;
  bodysize = bufsize - offset;
  body = (char *)malloc(bufsize + 1);
  // ボディ読み取る
  for (i = offset, j = 0; i < bufsize; i++, j++) {
    body[j] = buffer[i];
  }
  body[bodysize] = '\0';
  hm.body = body;

  req = (struct httpRequest *)malloc(sizeof(struct httpRequest));
  handleRequestLine(hm.requestLine, req);
  free(requestLine);
  handleHeader(hm.header, req);
  res = createResponse(req);
  sendResponse(res, clntSock);
}

int main() {
  int servSock; // server socket
  int clntSock; // client socket

  struct sockaddr_in servAddr; // server address
  struct sockaddr_in clntAddr; // client address

  unsigned int clntLen; // size of clntAddr

  char **lines;

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

  handleHttpMessage(recvBuffer, recvSize, clntSock);

  close(servSock);
  exit(0);
}
