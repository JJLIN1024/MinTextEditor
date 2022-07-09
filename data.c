#include <stdlib.h>
#include <string.h>

#include "data.h"

/* TODO: use a more efficient data structure, e.g. gap buffer or rope(?), array
 * is too slow */

void insertRow(editorConfig* E, int at, char* s, size_t len) {
  if (at < 0 || at > E->numrows)
    return;
  E->data = realloc(E->data, sizeof(row) * (E->numrows + 1));
  memmove(&E->data[at + 1], &E->data[at], sizeof(row) * (E->numrows - at));

  E->data[at].size = len;
  E->data[at].chars = malloc(len + 1);
  memcpy(E->data[at].chars, s, len);
  E->data[at].chars[len] = '\0';

  /* For render TAB */
  E->data[at].rsize = 0;
  E->data[at].render = NULL;
  updateRow(E, &E->data[at]);

  E->numrows++;
  /* TODO: how dirty this file is?
  maybe write it back when dirtyness
  exceed some threshold? performance tuning */
  E->dirty++;
}

void updateRow(editorConfig* E, row* row) {
  int tabs = 0;
  int j;
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t')
      tabs++;
  }

  free(row->render);
  row->render = malloc(row->size + tabs * (TAB_WIDTH - 1) + 1);

  int idx = 0;
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      row->render[idx++] = ' ';
      while (idx % TAB_WIDTH != 0)
        row->render[idx++] = ' ';
    } else {
      row->render[idx++] = row->chars[j];
    }
  }
  row->render[idx] = '\0';
  row->rsize = idx;
}

void insertNewLine(editorConfig* E) {
  if (E->cx == 0) {
    insertRow(E, E->cy, "", 0);
  } else {
    row* row = &E->data[E->cy];
    insertRow(E, E->cy + 1, &row->chars[E->cx], row->size - E->cx);
    row = &E->data[E->cy];
    row->size = E->cx;
    row->chars[row->size] = '\0';
  }
  E->cy++;
  E->cx = 0;
}

void rowInsertChar(editorConfig* E, row* row, int at, int c) {
  if (at < 0 || at >= row->size)
    at = row->size - 1;
  row->chars = realloc(row->chars, row->size + 1);
  memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
  row->size++;
  row->chars[at] = c;
  E->dirty++;
}

void insertChar(editorConfig* E, int c) {
  if (E->cy == E->numrows) {
    insertRow(E, E->numrows, "", 0);
  }
  rowInsertChar(E, &E->data[E->cy], E->cx, c);
  E->cx++;
}

void freerow(row* row) {
  free(row->chars);
}

void deleteRow(editorConfig* E, int at) {
  if (at < 0 || at >= E->numrows)
    return;

  freerow(&E->data[at]);
  memmove(&E->data[at], &E->data[at] + 1, sizeof(row) * (E->numrows - at - 1));
  E->numrows--;
  E->dirty++;
}

void rowAppendString(editorConfig* E, row* row, char* s, size_t len) {
  row->chars = realloc(row->chars, row->size + len + 1);
  memcpy(&row->chars[row->size], s, len);
  row->size += len;
  row->chars[row->size] = '\0';
  E->dirty++;
}

void rowdeleteChar(editorConfig* E, row* row, int at) {
  if (at < 0 || at >= row->size)
    return;
  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
  row->size--;
  E->dirty++;
}

void deleteChar(editorConfig* E) {
  if (E->cy == E->numrows)
    return;
  if (E->cx == 0 && E->cy == 0)
    return;

  row* row = &E->data[E->cy];
  if (E->cx > 0) {
    rowdeleteChar(E, row, E->cx - 1);
    E->cx--;
  } else {
    E->cx = E->data[E->cy - 1].size;
    rowAppendString(E, &E->data[E->cy - 1], row->chars, row->size);
    deleteRow(E, E->cy);
    E->cy--;
  }
}

char* rowsToString(editorConfig* E, int* buflen) {
  int totlen = 0;
  int j;
  for (j = 0; j < E->numrows; j++) {
    /* size + 1 is because we strip off \n
    when read it in the buffer */
    totlen += E->data[j].size + 1;
  }
  *buflen = totlen;

  char* buf = malloc(totlen);
  char* p = buf;
  for (j = 0; j < E->numrows; j++) {
    memcpy(p, E->data[j].chars, E->data[j].size);
    p += E->data[j].size;
    *p = '\n';
    p++;
  }
  return buf;
}
