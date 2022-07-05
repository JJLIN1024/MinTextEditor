#include "buffer.h"

#include <stdlib.h>
#include <string.h>

void abAppend(abuf *ab, const char *s, int len) {
  char *new = realloc(ab->b, ab->len + len);
  if (new == NULL) return;

  memcpy(&new[ab->len], s, len);
  ab->b = new;
  ab->len += len;
}

void abFree(abuf *ab) { free(ab->b); }