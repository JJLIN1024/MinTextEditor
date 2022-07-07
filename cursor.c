#include "cursor.h"

void moveCursor(editorConfig* E, int key) {
  row* row = (E->cy >= E->numrows) ? NULL : &E->data[E->cy];
  switch (key) {
    case ARROW_LEFT:
      if (E->cx != 0) {
        E->cx--;
      } else if (E->cy > 0) {
        E->cy--;
        E->cx = E->data[E->cy].size;
      }
      break;
    case ARROW_RIGHT:
      if (row && E->cx < row->size - 1) {
        E->cx++;
      } else if (row && E->cx == row->size - 1) {
        E->cy++;
        E->cx = 0;
      }
      break;
    case ARROW_UP:
      if (E->cy != 0) {
        E->cy--;
      }
      break;
    case ARROW_DOWN:
      /* E->numrows is 1-based */
      if (E->cy < E->numrows - 1) {
        E->cy++;
      }
      break;
  }

  row = (E->cy >= E->numrows) ? NULL : &E->data[E->cy];
  int rowlen = row ? row->size : 0;
  if (E->cx > rowlen) {
    E->cx = rowlen;
  }
}