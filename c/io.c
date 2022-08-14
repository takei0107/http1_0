#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./io.h"

char *readFile(char *path, int *fsize) {
  FILE *fp;
  char c;
  char buf[FILEBUFFERSIZE];
  char *body;
  int size = 0;

  fp = fopen(path, "r");
  if (fp == NULL) {
    fclose(fp);
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

  fclose(fp);
  *fsize = size;
  return body;
}

char *createFile(char *path, char *body) {
  FILE *fp;

  fp = fopen(path, "w");
  if (fp == NULL) {
    fclose(fp);
    printf("fopen() failed. path -> %s\n", path);
    return NULL;
  }

  if (fputs(body, fp) == EOF) {
    fclose(fp);
    printf("fputs() failed. path -> %s\n", path);
    return NULL;
  }

  fclose(fp);
  return body;
}
