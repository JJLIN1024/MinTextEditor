#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "cursor.h"
#include "data.h"
#include "dbg.h"
#include "editor.h"
#include "event.h"

int readInput(editorConfig* E) {
  /* TODO: MacOS command key, now its control*/
  char c;
  int rc;
  while ((rc = read(STDIN_FILENO, &c, 1)) != 1) {
    check(rc == -1 && errno != EAGAIN, "read from input fail");
  }

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

      case '\r':
        // editorInsertNewLine(E);
        break;

      case CTRL_KEY('q'):
        // if (E->dirty) {
        //   editorSetStatusMessage(E, "Unsave change, press Ctrl-E to force
        //   quit."); return;
        // }
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
      case BACKSPACE:
      case CTRL_KEY('h'):
      case DEL_KEY:
        // if (c == DEL_KEY)
        //   moveCursor(ARROW_RIGHT, E);
        // editorDelChar(E);
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
      case '\x1b': {
        E->mode = NORMAL_MODE;
        break;
      }
      case ARROW_UP:
      case ARROW_DOWN:
      case ARROW_LEFT:
      case ARROW_RIGHT:
        moveCursor(E, c);
        break;
      default:
        insertChar(E, c);
        break;
    }

  } else {
  }
}
