#ifndef __buffer_h__
#define __buffer_h__

typedef struct abuf {
  char *b;
  int len;
} abuf;

#define ABUF_INIT \
  { NULL, 0 }

void abAppend(abuf *, const char *, int);
void abFree(abuf *);
#endif