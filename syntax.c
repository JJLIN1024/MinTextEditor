#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "editor.h"
#include "syntax.h"

int is_separator(int c) {
  return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

void updateSyntax(row* row) {
  row->hl = realloc(row->hl, row->rsize);
  memset(row->hl, HL_NORMAL, row->rsize);

  int i;
  while (i < row->rsize) {
    char c = row->render[i];
    if (isdigit(c)) {
      row->hl[i] = HL_NUMBER;
    }
    i++;
  }
}

int syntaxToColor(int hl) {
  switch (hl) {
    case HL_NUMBER:
      return 31;
    case HL_MATCH:
      return 34;
    default:
      return 37;
  }
}