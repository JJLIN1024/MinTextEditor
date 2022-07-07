#include <stdlib.h>
#include <string.h>

#include "data.h"

void insertRow(editorConfig* E, int at, char* s, size_t len) {
  if (at < 0 || at > E->numrows)
    return;
  E->data = realloc(E->data, sizeof(row) * (E->numrows + 1));
  memmove(&E->data[at + 1], &E->data[at], sizeof(row) * (E->numrows - at));

  E->data[at].size = len;
  E->data[at].chars = malloc(len + 1);
  memcpy(E->data[at].chars, s, len);
  E->data[at].chars[len] = '\0';

  E->numrows++;
  /* TODO: how dirty this file is?
  maybe write it back when dirtyness
  exceed some threshold? performance tuning */
  E->dirty++;
}

// void freeRow(row* row) {
//   free(row->size);
//   free(row->chars);
// }

// void delRow(editorConfig* E, int at) {
//   if (at < 0 || at >= E->numrows)
//     return;
//   /* error, leaked ?*/
//   // editorFrerow(&E->row[at]);
//   memmove(&E->row[at], &E->row[at] + 1, sizeof(row) * (E->numrows - at - 1));
//   E->numrows--;
//   E->dirty++;
// }

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

// void insertNewLine(editorConfig* E) {
//   if (E->cx == 0) {
//     editorInsertRow(E, E->cy, "", 0);
//   } else {
//     row* row = &E->row[E->cy];
//     editorInsertRow(E, E->cy + 1, &row->chars[E->cx], row->size - E->cx);
//     row = &E->row[E->cy];
//     row->size = E->cx;
//     row->chars[row->size] = '\0';
//     editorUpdatrow(row);
//   }
//   E->cy++;
//   E->cx = 0;
// }

// void rowAppendString(editorConfig* E, row* row, char* s, size_t len) {
//   row->chars = realloc(row->chars, row->size + len + 1);
//   memcpy(&row->chars[row->size], s, len);
//   row->size += len;
//   row->chars[row->size] = '\0';
//   editorUpdatrow(row);
//   E->dirty++;
// }

// void rowDelChar(editorConfig* E, row* row, int at) {
//   if (at < 0 || at >= row->size)
//     return;
//   memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
//   row->size--;
//   editorUpdatrow(row);
//   E->dirty++;
// }

// void delChar(editorConfig* E) {
//   if (E->cy == E->numrows)
//     return;
//   if (E->cx == 0 && E->cy == 0)
//     return;

//   row* row = &E->row[E->cy];
//   if (E->cx > 0) {
//     editorRowDelChar(E, row, E->cx - 1);
//     E->cx--;
//   } else {
//     E->cx = E->row[E->cy - 1].size;
//     editorRowAppendString(E, &E->row[E->cy - 1], row->chars, row->size);
//     editorDelRow(E, E->cy);
//     E->cy--;
//   }
// }

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
