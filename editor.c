#include "editor.h"

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h> /* winsize */
#include <unistd.h>

#include "dbg.h"

void initEditor(editorConfig *E) {
  E->cx = 0;
  E->cy = 0;
  E->row = NULL;
  E->numrows = 0;
  /* TODO */
  E->mode = NORMAL_MODE;
  check(getWindowSize(E) == -1, "getWindowSize");
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

int editorReadKey(editorConfig *E) {
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
  switch (key) {
    case ARROW_LEFT:
      E->cx--;
      break;
    case ARROW_RIGHT:
      E->cx++;
      break;
    case ARROW_UP:
      E->cy--;
      break;
    case ARROW_DOWN:
      E->cy++;
      break;
  }
}

void editorProcessKeypress(editorConfig *E) {
  int c = editorReadKey(E);

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
      E->cx = E->screencols - 1;
      break;
    case PAGE_UP:
    case PAGE_DOWN: {
      int times = E->screenrows;
      while (times--) editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN, E);
    } break;
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

void editorDrawRaws(editorConfig *E, abuf *ab) {
  int y;
  /* TODO: put more information into welcoming message, e.g. help, how to
   * quit..., see what vim & nvim does!! especially when window size change or
   * too small*/
  for (y = 0; y < E->screenrows; y++) {
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
      int len = E->row[y].size;
      if (len > E->screencols) len = E->screencols;
      abAppend(ab, E->row[y].chars, len);
    }

    abAppend(ab, "\x1b[K", 3); /* Erases from the current cursor position to
                              the end of the current line */
    if (y < E->screenrows - 1) {
      abAppend(ab, "\r\n", 2);
    }
  }
}

void editorRefreshScreen(editorConfig *E) {
  abuf ab = ABUF_INIT;
  abAppend(&ab, "\x1b[?25l", 6);

  abAppend(&ab, "\x1b[H", 3);

  editorDrawRaws(E, &ab);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E->cy + 1, E->cx + 1);
  abAppend(&ab, buf, strlen(buf));

  abAppend(&ab, "\x1b[?25h", 6);

  write(STDOUT_FILENO, ab.b, ab.len);
  abFree(&ab);
}