#ifndef HTTPTYPE_C

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

#endif
#define HTTPTYPE_C
