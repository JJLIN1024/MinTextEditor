#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "cursor.h"
#include "data.h"
#include "dbg.h"
#include "editor.h"
#include "event.h"
#include "render.h"

int readInput(editorConfig* E) {
  /* TODO: MacOS command key, now its control*/
  char c;
  int rc;
  while ((rc = read(STDIN_FILENO, &c, 1)) != 1) {
    check(rc == -1 && errno != EAGAIN, "read from input fail");
  }

  /* TODO: vim & nvim v.s. Vscode
  TAB behavior seems different, in vim,
  TAB in every file is a TAB, cursor will move
  to the next TAB STOP, but in Vscode, this behavior
  is observed only in Makefile type of file. */
  // if (c == '\t') {
  //   return TAB;
  // }

  if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1)
      return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1)
      return '\x1b';
    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1)
          return '\x1b';
        if (seq[2] == '~') {
          switch (seq[1]) {
            case '1':
              return HOME_KEY;
            case '3':
              return DEL_KEY;
            case '4':
              return END_KEY;
            case '5':
              /* TODO: implement MacOS & Linux support */
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

void processEvent(editorConfig* E) {
  int c = readInput(E);

  if (E->mode == NORMAL_MODE) {
    switch (c) {
      case 'i':
        E->mode = INSERT_MODE;
        break;
      case 'h':
        moveCursor(E, ARROW_LEFT);
        break;
      case 'j':
        moveCursor(E, ARROW_DOWN);
        break;
      case 'k':
        moveCursor(E, ARROW_UP);
        break;
      case 'l':
        moveCursor(E, ARROW_RIGHT);
        break;

      case CTRL_KEY('q'):
        if (E->dirty) {
          setStatusMessage(E, "Unsave change, press Ctrl-E to force quit.");
          return;
        }
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;
      /* TODO: force quit -> :!wq */
      case CTRL_KEY('e'):
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;

      case CTRL_KEY('s'):
        editorSave(E);
        break;

      case HOME_KEY:
        E->cx = 0;
        break;

      case END_KEY:
        if (E->cy < E->numrows) {
          E->cx = E->data[E->cy].size - 1;
        }
        break;

      case ARROW_UP:
      case ARROW_DOWN:
      case ARROW_LEFT:
      case ARROW_RIGHT:
        moveCursor(E, c);
        break;
      case CTRL_KEY('l'):
        break;
      default:

        break;
    }
  } else if (E->mode == INSERT_MODE) {
    switch (c) {
      case '\x1b':
        E->mode = NORMAL_MODE;
        break;
      case '\r':
        insertNewLine(E);
        break;
      case ARROW_UP:
      case ARROW_DOWN:
      case ARROW_LEFT:
      case ARROW_RIGHT:
        moveCursor(E, c);
        break;
      case BACKSPACE:
      case CTRL_KEY('h'):
      case DEL_KEY:
        if (c == DEL_KEY)
          moveCursor(E, ARROW_RIGHT);
        deleteChar(E);
        break;
      default:
        insertChar(E, c);
        break;
    }

  } else {
  }
}

void setStatusMessage(editorConfig* E, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(E->statusmsg, sizeof(E->statusmsg), fmt, ap);
  va_end(ap);
  E->statusmsg_time = time(NULL);
}

char* promptInfo(editorConfig* E, char* info) {
  size_t bufsize = 128;
  char* buf = malloc(bufsize);

  size_t buflen = 0;
  buf[0] = '\0';

  while (1) {
    setStatusMessage(E, info, buf);
    renderScreen(E);
    int c = readInput(E);
    if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) {
      if (buflen != 0)
        buf[--buflen] = '\0';
    } else if (c == '\x1b') {
      setStatusMessage(E, "");
      free(buf);
      return NULL;
    } else if (c == '\r') {
      if (buflen != 0) {
        setStatusMessage(E, "");
        return buf;
      }
    } else if (!iscntrl(c) && c < 128) {
      if (buflen == bufsize - 1) {
        bufsize *= 2;
        buf = realloc(buf, bufsize);
      }
      buf[buflen++] = c;
      buf[buflen] = '\0';
    }
  }
}