#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "cursor.h"
#include "data.h"
#include "dbg.h"
#include "editor.h"
#include "event.h"
#include "render.h"

int readInput(editorConfig* E) {
  /* TODO: MacOS command key, in linux it's Ctrl*/
  char c;
  int rc;
  while ((rc = read(STDIN_FILENO, &c, 1)) != 1) {
    check(rc == -1 && errno != EAGAIN, "read from input fail");
  }

  if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) {
      return '\x1b';
    }
    if (read(STDIN_FILENO, &seq[1], 1) != 1) {
      return '\x1b';
    }
    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) {
          return '\x1b';
        }

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

void processEvent(editorConfig* E) {
  int number = 1;
  int c = readInput(E);

  char prevKeyStroke = E->keyStroke;
  E->keyStroke = (char)c;
  time_t prevKeystrokeTime = E->keystroke_time;
  E->keystroke_time = time(NULL);

  if (E->mode == NORMAL_MODE) {
    switch (c) {
      case 'i':
        E->mode = INSERT_MODE;
        break;
      case 'c':
      case 'd': {
        int next = readInput(E);
        if (next == 'w') {
          c == 'c' ? changeWord(E) : deleteWord(E);
        }
        break;
      }

      case 'r': {
        int next = readInput(E);
        replaceChar(E, next);
        break;
      }

      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        break;
      case ':':
      case '/':
        processNormalCommand(E, c);
        break;
      case CTRL('f'):
        break;
      case 'x':
        moveCursor(E, ARROW_RIGHT);
        deleteChar(E);
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
      /* gg to the top */
      case 'g':
        if (prevKeyStroke == 'g' &&
            (E->keystroke_time - prevKeystrokeTime) < KEY_TIMEOUT) {
          E->cy = 0;
          if (E->cx > E->data[E->cy].size - 1) {
            E->cx = E->data[E->cy].size - 1;
          }
        }
        break;
      /* to the buttom */
      case 'G':
        E->cy = E->numrows - 1;
        if (E->cx > E->data[E->cy].size - 1) {
          E->cx = E->data[E->cy].size - 1;
        }
        break;
      case 'z':
        if (prevKeyStroke == 'z' &&
            (E->keystroke_time - prevKeystrokeTime) < KEY_TIMEOUT) {
          int diff = (E->cy - E->rowoff) - (E->screenrows / 2);
          if ((diff > 0) && (E->rowoff + diff < E->numrows)) {
            E->rowoff += diff;
          } else if ((diff < 0) && (E->rowoff + diff > 0)) {
            E->rowoff += diff;
          }
        }
        break;
      case CTRL_KEY('q'):
        editorQuit(E);
        break;

      case CTRL_KEY('s'):
        editorSave(E);
        break;

      case '0':
      case HOME_KEY:
        E->cx = 0;
        break;
      case '$':
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
      /* Better escape: jk to escape insert-mode, Esc is too far*/
      case 'k':
        if (prevKeyStroke == 'j' &&
            (E->keystroke_time - prevKeystrokeTime) < KEY_TIMEOUT) {
          deleteChar(E);
          E->mode = NORMAL_MODE;
          break;
        } else {
          insertChar(E, c);
          break;
        }
      case 'j':
        if (prevKeyStroke == 'j' &&
            (E->keystroke_time - prevKeystrokeTime) < KEY_TIMEOUT) {
          deleteChar(E);
          E->mode = NORMAL_MODE;
          break;
        } else {
          insertChar(E, c);
          break;
        }
      case CTRL_KEY('c'):
        break;
      case CTRL_KEY('v'):
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
      case '\r':
        insertNewLine(E);
        break;
      case '\t':
        insertChar(E, c);
        break;
      default:
        if (!iscntrl(c))
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

void processNormalCommand(editorConfig* E, char c) {
  size_t bufsize = 128;
  char* buf = malloc(bufsize);

  size_t buflen = 1;
  buf[0] = c;
  buf[buflen] = '\0';

  while (1) {
    setStatusMessage(E, "%s", buf);
    renderScreen(E);

    int c = readInput(E);
    if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) {
      if (buflen != 0) {
        buf[--buflen] = '\0';
        if (buflen == 0) {
          setStatusMessage(E, "%s", "");
          free(buf);
          return;
        }
      }
    } else if (c == '\x1b') {
      setStatusMessage(E, "");
      free(buf);
      return;
    } else if (c == '\r') {
      if (buflen != 0) {
        if (strcmp(buf, ":w") == 0) {
          editorSave(E);
          free(buf);
          return;
        } else if (strcmp(buf, ":q") == 0) {
          editorQuit(E);
          free(buf);
          return;
        } else if (strcmp(buf, ":wq") == 0) {
          editorSave(E);
          editorQuit(E);
          free(buf);
          return;
        } else if (strcmp(buf, ":q!") == 0) {
          free(buf);
          write(STDOUT_FILENO, "\x1b[2J", 4);
          write(STDOUT_FILENO, "\x1b[H", 3);
          exit(0);
        } else if (buf[0] == '/') {
          setStatusMessage(E, "");
          int qlen = strlen(buf) - 1;
          char* query = malloc(qlen);
          memcpy(query, &buf[1], qlen);
          query[qlen] = '\0';
          editorFind(E, query);

          free(buf);
          free(query);
          return;
        } else {
          /* TODO: warning message should be red*/
          setStatusMessage(E, "Unknown command");
          free(buf);
          return;
        }
      }
    } else if (!iscntrl(c) && c < 128) {
      if (buflen == bufsize - 1) {
        /* TODO: amortized cost */
        bufsize *= 2;
        buf = realloc(buf, bufsize);
      }
      buf[buflen++] = c;
      buf[buflen] = '\0';
    }
  }
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