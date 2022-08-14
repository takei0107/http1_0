#include <stdio.h>
#include <stdlib.h>

char *ltoa(unsigned long l) {
  int i = 0;
  int b = 1; // 桁数
  long ll = l;
  char c;
  char *s;
  char *tmp;

  if (l == 0) {
    return "0";
  }

  // 桁数求める
  while ((ll /= 10) != 0) {
    b++;
  }

  tmp = (char *)malloc(b + 1);
  s = (char *)malloc(b + 1);

  for (i = 0; i < b; i++) {
    c = ((l % 10) + '0');
    tmp[i] = c;
    l/=10;
  }

  for (i = b - 1; i >= 0; i--) {
    s[i] = tmp[b - (i + 1)];
  }
  s[b] = '\0';

  free(tmp);
  return s;
}
