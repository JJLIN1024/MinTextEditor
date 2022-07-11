#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "dbg.h"
#include "render.h"
#include "syntax.h"

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

int rowCxToRx(row* row, int cx) {
  int rx = 0;
  int j;
  for (j = 0; j < cx; j++) {
    if (row->chars[j] == '\t')
      rx += (TAB_WIDTH - 1) - (rx % TAB_WIDTH);
    rx++;
  }
  return rx;
}

int rowRxToCx(row* row, int rx) {
  int cur_rx = 0;
  int cx;
  for (cx = 0; cx < row->size; cx++) {
    if (row->chars[cx] == '\t') {
      cur_rx += (TAB_WIDTH - 1) - (cur_rx % TAB_WIDTH);
    }
    cur_rx++;
    if (cur_rx > rx)
      return cx;
  }
  return cx;
}

void scrollScreen(editorConfig* E) {
  E->rx = 1;
  if (E->cy < E->numrows) {
    E->rx = rowCxToRx(&E->data[E->cy], E->cx);
  }
  E->ry = E->cy;

  if (E->ry < E->rowoff) {
    E->rowoff = E->ry;
  }
  if (E->ry >= E->rowoff + E->screenrows) {
    E->rowoff = E->ry - E->screenrows + 1;
  }
  if (E->rx < E->coloff) {
    E->coloff = E->rx;
  }
  if (E->rx >= E->coloff + E->screencols - LINE_NUMBER_WIDTH) {
    E->coloff = E->rx - E->screencols + LINE_NUMBER_WIDTH + 1;
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

        while (padding--) {
          bufferAppend(buf, " ", 1);
        }
        bufferAppend(buf, welcome, welcomeLen);
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

      /* Data section */
      int rowDataLen = E->data[filerow].rsize - E->coloff;
      if (rowDataLen < 0)
        rowDataLen = 0;
      if (rowDataLen > E->screencols - LINE_NUMBER_WIDTH)
        rowDataLen = E->screencols - LINE_NUMBER_WIDTH;

      char* data = &E->data[filerow].render[E->coloff];
      unsigned char* hl = &E->data[filerow].hl[E->coloff];
      int current_color = -1;
      for (int j = 0; j < rowDataLen; j++) {
        if (hl[j] == HL_NORMAL) {
          if (current_color != -1) {
            bufferAppend(buf, "\x1b[39m", 5);
            current_color = -1;
          }

          bufferAppend(buf, &data[j], 1);
        } else {
          int color = syntaxToColor(hl[j]);
          if (color != current_color) {
            current_color = color;
            char tmp[16];
            int clen = snprintf(tmp, sizeof(tmp), "\x1b[%dm", color);
            bufferAppend(buf, tmp, clen);
          }

          bufferAppend(buf, &data[j], 1);
        }
      }
      bufferAppend(buf, "\x1b[39m", 5);
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
  E->tx = E->rx - E->coloff + 1 + LINE_NUMBER_WIDTH;
  E->ty = E->ry - E->rowoff + 1;
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
  snprintf(tmp, sizeof(tmp), "\x1b[%d;%dH", E->ty, E->tx);
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