#ifndef RESPONSE_H

struct httpResponse {
  char *version;
  short statusCode;
  char *reason;
  char **header;
  char *body;
};

char *createHttpResponseMessage(struct httpRequest *req, int *size);

#endif
#define RESPONSE_H
