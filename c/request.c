#include <stdlib.h>
#include <string.h>

#include "./request.h"

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
        st = i + 1;
        continue;
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

static void handleBody(char *body, struct httpRequest *req) {
  req->body = body;
}

struct httpRequest *handleHttpRequestMessage(char *buffer, int bufsize,
                                             int clntSock) {
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

  bo.offset = 0;
  bo.buffer = buffer;

  offset = 0;
  lineLen = 0;

  // 1??????(request-line)
  lineLen = getLineLenToCrlf(&bo, bufsize);
  offset = bo.offset;
  requestLine = (char *)malloc(lineLen + 1);
  for (i = 0; i < lineLen; i++) {
    c = buffer[i];
    requestLine[i] = c;
  }
  requestLine[lineLen] = '\0';
  hm.requestLine = requestLine;

  // 2?????? ~ ??????????????????
  // ?????????????????????????????????????????????
  rowNum = 0;
  while (getLineLenToCrlf(&bo, bufsize) != 0) {
    rowNum++;
  }
  // offset??????
  bo.offset = offset;
  // ?????????allocate
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

  // ????????????????????????
  getLineLenToCrlf(&bo, bufsize);
  offset = bo.offset;
  bodysize = bufsize - offset;
  body = (char *)malloc(bufsize + 1);
  // ?????????????????????
  for (i = offset, j = 0; i < bufsize; i++, j++) {
    body[j] = buffer[i];
  }
  body[bodysize] = '\0';
  hm.body = body;

  req = (struct httpRequest *)malloc(sizeof(struct httpRequest));
  handleRequestLine(hm.requestLine, req);
  free(requestLine);
  handleHeader(hm.header, req);
  handleBody(hm.body, req);

  return req;
}
