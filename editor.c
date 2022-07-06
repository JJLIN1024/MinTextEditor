#include "editor.h"

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h> /* winsize */
#include <time.h>
#include <unistd.h>

#include "dbg.h"

void initEditor(editorConfig *E) {
  E->cx = 0;
  E->cy = 0;
  E->rx = 0;
  E->row = NULL;
  E->numrows = 0;
  E->rowoff = 0;
  E->coloff = 0;
  E->filename = NULL;
  E->statusmsg[0] = '\0';
  E->statusmsg_time = 0;
  /* TODO */
  E->mode = NORMAL_MODE;
  check(getWindowSize(E) == -1, "getWindowSize");

  E->screenrows -= 2;
}

int getCursorPosition(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    if (buf[i] == 'R') break;
    i++;
  }
  buf[i] = '\0';
  if (buf[0] != '\x1b' || buf[1] != '[') return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;
  return 0;
}

int getWindowSize(editorConfig *E) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    return getCursorPosition(&E->screenrows, &E->screencols);
  } else {
    E->screencols = ws.ws_col;
    E->screenrows = ws.ws_row;
    return 0;
  }
}

int editorRowCxToRx(erow *row, int cx) {
  int rx = 0;
  int j = 0;
  for (j = 0; j < cx; j++) {
    if (row->chars[j] == '\t') {
      /* count how many bytes do I
      have to move to the right to get
      to one byte before the tab stop,
      One can use this technique to implement
      varios jumping scheme, such as what "w"
      in normal mode in VIM does. */
      rx += (MIN_TAP_STOP - 1) - (rx % MIN_TAP_STOP);
    }
    rx++;
  }
  return rx;
}

void editorUpdateRow(erow *row) {
  int tabs = 0;
  int j;

  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') tabs++;
  }

  free(row->render);
  row->render = malloc(row->size + tabs * MIN_TAP_STOP + 1);

  /* core idea: put every TAB on tab stop, which is
  index that are divisible by 8, then use editorRowCxToRx()
  to move to tab stop position if encountered a TAB */
  int idx = 0;
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      row->render[idx++] = ' ';
      while (idx % MIN_TAP_STOP != 0) row->render[idx++] = ' ';
    } else {
      row->render[idx++] = row->chars[j];
    }
  }

  row->render[idx] = '\0';
  row->rsize = idx;
}

void editorAppendRow(char *s, size_t len, editorConfig *E) {
  E->row = realloc(E->row, sizeof(erow) * (E->numrows + 1));

  int at = E->numrows;
  E->row[at].size = len;
  E->row[at].chars = malloc(len + 1);
  memcpy(E->row[at].chars, s, len);
  E->row[at].chars[len] = '\0';

  editorUpdateRow(&E->row[at]);

  E->numrows++;
}

void editorOpen(char *filename, editorConfig *E) {
  free(E->filename);
  E->filename = strdup(filename);

  FILE *fp = fopen(filename, "r");
  check(!fp, "fopen");

  char *line = NULL;
  size_t linecap = 0;
  ssize_t linelen;

  while ((linelen = getline(&line, &linecap, fp)) != -1) {
    while (linelen > 0 &&
           (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) {
      linelen--;
    }
    editorAppendRow(line, linelen, E);
  }

  free(line);
  fclose(fp);
}

int editorReadInput(editorConfig *E) {
  char c;
  int rc;
  while ((rc = read(STDIN_FILENO, &c, 1)) != 1) {
    check(rc == -1 && errno != EAGAIN, "read from input fail");
  }

  if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
        if (seq[2] == '~') {
          switch (seq[1]) {
            case '1':
              return HOME_KEY;
            case '3':
              return DEL_KEY;
            case '4':
              return END_KEY;
            case '5':
              return PAGE_UP;
            case '6':
              return PAGE_DOWN;
            case '7':
              return HOME_KEY;
            case '8':
              return END_KEY;
          }
        }
      } else {
        switch (seq[1]) {
          case 'A':
            return ARROW_UP;
          case 'B':
            return ARROW_DOWN;
          case 'C':
            return ARROW_RIGHT;
          case 'D':
            return ARROW_LEFT;
          case 'H':
            return HOME_KEY;
          case 'F':
            return END_KEY;
        }
      }
    } else if (seq[0] == 'O') {
      switch (seq[1]) {
        case 'H':
          return HOME_KEY;
        case 'F':
          return END_KEY;
      }
    } else {
      return '\x1b';
    }
  }
  return c;
}

void editorMoveCursor(int key, editorConfig *E) {
  erow *row = (E->cy >= E->numrows) ? NULL : &E->row[E->cy];
  switch (key) {
    case ARROW_LEFT:
      if (E->cx != 0) {
        E->cx--;
      } else if (E->cy > 0) {
        E->cy--;
        E->cx = E->row[E->cy].size;
      }
      break;
    case ARROW_RIGHT:
      if (row && E->cx < row->size) {
        E->cx++;
      } else if (row && E->cx == row->size) {
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
      if (E->cy < E->numrows) {
        E->cy++;
      }
      break;
  }

  row = (E->cy >= E->numrows) ? NULL : &E->row[E->cy];
  int rowlen = row ? row->size : 0;
  if (E->cx > rowlen) {
    E->cx = rowlen;
  }
}

void editorProcessEvent(editorConfig *E) {
  int c = editorReadInput(E);

  switch (c) {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;
    case HOME_KEY:
      E->cx = 0;
      break;
    case END_KEY:
      if (E->cy < E->numrows) {
        E->cx = E->row[E->cy].size;
      }
      break;
    case PAGE_UP:
    case PAGE_DOWN: {
      if (c == PAGE_UP) {
        E->cy = E->rowoff;
      } else if (c == PAGE_DOWN) {
        E->cy = E->rowoff + E->screenrows + 1;
        if (E->cy > E->numrows) E->cy = E->numrows;
      }
      int times = E->screenrows;
      while (times--) editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN, E);
    }
    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT:
      editorMoveCursor(c, E);
      break;
    default:
      break;
  }
}

void editorScroll(editorConfig *E) {
  // TAB
  E->rx = 0;
  if (E->cy < E->numrows) {
    E->rx = editorRowCxToRx(&E->row[E->cy], E->cx);
  }

  if (E->cy < E->rowoff) {
    E->rowoff = E->cy;
  }
  if (E->cy >= E->rowoff + E->screenrows) {
    E->rowoff = E->cy - E->screenrows + 1;
  }
  if (E->rx < E->coloff) {
    E->coloff = E->rx;
  }
  if (E->rx >= E->coloff + E->screencols) {
    E->coloff = E->rx - E->screencols + 1;
  }
}

void editorDrawRaws(editorConfig *E, abuf *ab) {
  /* TODO: a extra line will be displayed at the end
  of the file, fix it. e.g: 9/8 in the status bar.
  should be 8/8. Related: cx, numrows */
  int y;
  /* TODO: put more information into welcoming message, e.g. help, how to
   * quit..., see what vim & nvim does!! especially when window size change or
   * too small*/
  for (y = 0; y < E->screenrows; y++) {
    int filerow = y + E->rowoff;

    /* TODO: turn off "~" displayed when file is small,
    which does not fill the entire screen */
    if (y >= E->numrows) {
      if (E->numrows == 0 && y == E->screenrows / 3) {
        char welcome[80];
        int welcomeLen =
            snprintf(welcome, sizeof(welcome), "Min Editor v%s", MIN_VERSION);
        if (welcomeLen > E->screencols) welcomeLen = E->screencols;
        int padding = (E->screencols - welcomeLen) / 2;
        if (padding) {
          abAppend(ab, "~", 1);
          padding--;
        }
        while (padding--) {
          abAppend(ab, " ", 1);
        }
        abAppend(ab, welcome, welcomeLen);
      } else {
        abAppend(ab, "~", 3);
      }
    } else {
      /* Fix size line number section,
      number are right-aligned */
      /* TODO: cursor position, cursor are not suppose to
      scroll on line number and its section, and padding */
      /* color */
      // abAppend(ab, "\x1b[7m", 4);

      // char lineNumber[10];
      // int numberLen = snprintf(lineNumber, sizeof(lineNumber), "%d",
      // filerow); int lineLen = MIN_LINE_NUMBER_SECTION_LEN; while (lineLen-- >
      // numberLen + 1) {
      //   abAppend(ab, " ", 1);
      // }
      // abAppend(ab, lineNumber, numberLen);
      // /* revert back to original color */
      // abAppend(ab, "\x1b[m", 3);

      // abAppend(ab, " ", 1);

      /* TODO: rightmost content are missing */
      /* adjust screencol and coloff */
      int len = E->row[filerow].rsize - E->coloff;
      if (len < 0) len = 0;
      if (len > E->screencols) len = E->screencols;
      abAppend(ab, &E->row[filerow].render[E->coloff], len);
    }

    abAppend(ab, "\x1b[K", 3); /* Erases from the current cursor position to
                              the end of the current line */
    abAppend(ab, "\r\n", 2);
  }
}

void editorDrawStatusBar(editorConfig *E, abuf *ab) {
  abAppend(ab, "\x1b[7m", 4);

  char status[80], rstatus[80];
  int len = snprintf(status, sizeof(status), "%.20s - %d lines",
                     E->filename ? E->filename : "[No Name]", E->numrows);
  int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", E->cy + 1, E->numrows);
  if (len > E->screencols) len = E->screencols;
  abAppend(ab, status, len);

  while (len < E->screencols) {
    if (E->screencols - len == rlen) {
      abAppend(ab, rstatus, rlen);
      break;
    } else {
      abAppend(ab, " ", 1);
      len++;
    }
  }
  abAppend(ab, "\x1b[m", 3);
  abAppend(ab, "\r\n", 2);
}

void editorDrawMessageBar(editorConfig *E, abuf *ab) {
  abAppend(ab, "\x1b[K", 3);
  int msglen = strlen(E->statusmsg);
  if (msglen > E->screencols) msglen = E->screencols;
  if (msglen && time(NULL) - E->statusmsg_time < 5)
    abAppend(ab, E->statusmsg, msglen);
}

void editorSetStatusMessage(editorConfig *E, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(E->statusmsg, sizeof(E->statusmsg), fmt, ap);
  va_end(ap);
  E->statusmsg_time = time(NULL);
}

void editorRefreshScreen(editorConfig *E) {
  editorScroll(E);

  abuf ab = ABUF_INIT;
  abAppend(&ab, "\x1b[?25l", 6);

  /* Set cursor back to home. Do NOT comment out this line, will cause the whole
   * program to break :( */
  abAppend(&ab, "\x1b[H", 3);

  editorDrawRaws(E, &ab);
  editorDrawStatusBar(E, &ab);
  editorDrawMessageBar(E, &ab);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E->cy - E->rowoff) + 1,
           (E->rx - E->coloff) + 1);
  abAppend(&ab, buf, strlen(buf));

  abAppend(&ab, "\x1b[?25h", 6);

  write(STDOUT_FILENO, ab.b, ab.len);
  abFree(&ab);
}