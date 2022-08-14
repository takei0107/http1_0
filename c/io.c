#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILEBUFFERSIZE 1024

char *readFile(char *path, int *fsize) {
  FILE *fp;
  char c;
  char buf[FILEBUFFERSIZE];
  char *body;
  int size = 0;

  fp = fopen(path, "r");
  if (fp == NULL) {
    printf("fopen() failed. path -> %s\n", path);
    return NULL;
  }

  memset(buf, 0, FILEBUFFERSIZE);
  while ((c = fgetc(fp)) != EOF) {
    *(buf + size) = c;
    size++;
  }

  body = (char *)malloc(size + 1);
  body = memcpy(body, buf, size);
  body[size] = '\0';

  *fsize = size;
  return body;
}
