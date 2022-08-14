#include <stdio.h>
#include <stdlib.h>

#include "./request.c"
#include "./io.c"

char *docroot = "./www";

struct httpResponse {
  char *version;
  short statusCode;
  char *reason;
  char **header;
  char *body;
};

static struct httpResponse *createResponse(struct httpRequest *req) {
  char *reason_200 = "OK";
  char *reason_405 = "Method Not Allowed";
  char *reason_500 = "Internal Server Error";
  char *path;
  char *body;
  int strLen = 0;
  int contentLength = 0;

  struct httpResponse *res;
  res = (struct httpResponse *)malloc(sizeof(struct httpResponse));

  if ((strcmp(req->method, "GET") != 0) && (strcmp(req->method, "HEAD"))) {
    res->statusCode = 405;
    res->reason = reason_405;
  } else {
    // GET
    if (strcmp(req->method, "GET") == 0) {
      strLen = strlen(docroot);
      strLen += strlen(req->path);
      path = (char *)malloc(strLen);
      memset(path, 0, strLen);
      path = strcat(path, docroot);
      path = strcat(path, req->path);
      if ((body = readFile(path, &contentLength)) == NULL) {
        res->statusCode = 500;
        res->reason = reason_500;
        res->body = NULL;
      } else {
        res->statusCode = 200;
        res->reason = reason_200;
        res->body = body;
      }
    } else {
      res->statusCode = 200;
      res->reason = reason_200;
    }
  }
  res->version = req->version;
  return res;
}

char *createHttpResponseMessage(struct httpRequest *req, int *size) {
  char *message;
  int s = 0;
  char scStr[3];
  char *crlf = "\r\n";
  char sp = ' ';
  int i = 0;
  int j;

  struct httpResponse *res;
  res = createResponse(req);

  sprintf(scStr, "%d", res->statusCode);

  s += strlen(res->version);
  s += 1; // SP
  s += strlen(scStr);
  s += 1; // SP
  s += strlen(res->reason);
  s += strlen(crlf);
  s += strlen(crlf);
  if (res->body != NULL) {
    s += strlen(res->body);
  }

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
  if (res->body != NULL) {
    for(j = 0; j < strlen(res->body); j++) {
      message[i++] = res->body[j];
    }
  }

  *size = s;
  return message;
}
