#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dbg.h"
#include "render.h"

void bufferAppend(buffer* buf, const char* s, int len) {
  char* new = realloc(buf->start, buf->size + len);
  check((new == NULL), "Fail to append buffer");

  memmove(&new[buf->size], s, len);
  buf->start = new;
  buf->size += len;
}

void bufferFree(buffer* buf) {
  free(buf->start);
}

void scrollScreen(editorConfig* E) {
  if (E->cy < E->rowoff) {
    E->rowoff = E->cy;
  }
  if (E->cy >= E->rowoff + E->screenrows) {
    E->rowoff = E->cy - E->screenrows + 1;
  }
  if (E->cx < E->coloff) {
    E->coloff = E->cx;
  }
  if (E->cx >= E->coloff + E->screencols - LINE_NUMBER_WIDTH) {
    E->coloff = E->cx - E->screencols + LINE_NUMBER_WIDTH + 1;
  }
}

void renderRows(editorConfig* E, buffer* buf) {
  /* TODO: a extra line will be displayed at the end
  of the file, fix it. e.g: 9/8 in the status bar.
  should be 8/8. Related: cx, numrows */
  int y;
  /* TODO: put more information into welcoming message, e.g.
   * help, how to quit..., see what vim & nvim does!! especially
   * when window size change or too small*/
  for (y = 0; y < E->screenrows; y++) {
    int filerow = y + E->rowoff;

    if (y >= E->numrows) {
      /* no file displayed */
      if (E->numrows == 0 && y == E->screenrows / 3) {
        char welcome[80];
        int welcomeLen =
            snprintf(welcome, sizeof(welcome), "Min Editor v%s", MIN_VERSION);
        if (welcomeLen > E->screenrows)
          welcomeLen = E->screenrows;
        int padding = (E->screencols - welcomeLen) / 2;
        if (padding) {
          bufferAppend(buf, "~", 1);
        }
        padding--;

        while (padding--) {
          bufferAppend(buf, " ", 1);
        }
        bufferAppend(buf, welcome, welcomeLen);
      } else if (E->numrows == 0) {
        bufferAppend(buf, "~", 1);
      } else {
        ;
      }

    } else {
      /* line number section */
      bufferAppend(buf, "\x1b[36m", 5);
      char lineNumber[10];
      int numberLen =
          snprintf(lineNumber, sizeof(lineNumber), "%d", filerow + 1);
      int lineLen = LINE_NUMBER_DATA;
      while (lineLen-- > numberLen) {
        bufferAppend(buf, " ", 1);
      }
      bufferAppend(buf, lineNumber, numberLen);
      bufferAppend(buf, "\x1b[m", 3);
      bufferAppend(buf, " ", LINE_NUMBER_PADDING);

      /* TODO: rightmost content are missing */
      /* adjust screencol and coloff */
      int rowDataLen = E->data[filerow].size - E->coloff;
      if (rowDataLen < 0)
        rowDataLen = 0;
      if (rowDataLen > E->screencols - LINE_NUMBER_WIDTH)
        rowDataLen = E->screencols - LINE_NUMBER_WIDTH;
      bufferAppend(buf, &E->data[filerow].chars[E->coloff], rowDataLen);
    }

    bufferAppend(buf, "\x1b[K", 3); /* Erases from the current cursor position
                              to the end of the current line */
    bufferAppend(buf, "\r\n", 2);
  }
}

void renderStatusBar(editorConfig* E, buffer* buf) {
  bufferAppend(buf, "\x1b[7m", 4);

  char status[80], rstatus[80];
  int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
                     E->filename ? E->filename : "[No Name]", E->numrows,
                     E->dirty ? "(modified)" : "");
  int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", E->cy + 1, E->numrows);
  if (len > E->screencols)
    len = E->screencols;
  bufferAppend(buf, status, len);

  while (len < E->screencols) {
    if (E->screencols - len == rlen) {
      bufferAppend(buf, rstatus, rlen);
      break;
    } else {
      bufferAppend(buf, " ", 1);
      len++;
    }
  }
  bufferAppend(buf, "\x1b[m", 3);
  bufferAppend(buf, "\r\n", 2);
}

void renderMessageBar(editorConfig* E, buffer* buf) {
  bufferAppend(buf, "\x1b[K", 3);
  int msglen = strlen(E->statusmsg);
  if (msglen > E->screencols)
    msglen = E->screencols;
  if (msglen && time(NULL) - E->statusmsg_time < 5)
    bufferAppend(buf, E->statusmsg, msglen);
}

void renderCursor(editorConfig* E) {
  E->rx = E->cx - E->coloff + 1 + LINE_NUMBER_WIDTH;
  E->ry = E->cy - E->rowoff + 1;
}

void setStatusMessage(editorConfig* E, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(E->statusmsg, sizeof(E->statusmsg), fmt, ap);
  va_end(ap);
  E->statusmsg_time = time(NULL);
}

void renderScreen(editorConfig* E) {
  scrollScreen(E);
  renderCursor(E);

  buffer buf = BUFFER_INIT;
  /* set cursor invisible to avoid flicker effect */
  bufferAppend(&buf, "\x1b[?25l", 6);

  /* Set cursor back to home. Do NOT comment out this line, will cause the whole
   * program to break :( */
  bufferAppend(&buf, "\x1b[H", 3);

  renderRows(E, &buf);
  renderStatusBar(E, &buf);
  renderMessageBar(E, &buf);

  /* render cursor */
  char tmp[32];
  snprintf(tmp, sizeof(tmp), "\x1b[%d;%dH", E->ry, E->rx);
  bufferAppend(&buf, tmp, strlen(tmp));

  /* set cursor to the corresponding shape
  Block -> Normal mode
  Line -> Insert mode */
  if (E->mode == INSERT_MODE) {
    bufferAppend(&buf, "\033[5 q", 5);
  } else {
    bufferAppend(&buf, "\033[0 q", 5);
  }
  /* set cursor visible */
  bufferAppend(&buf, "\x1b[?25h", 6);

  write(STDOUT_FILENO, buf.start, buf.size);
  bufferFree(&buf);
}