#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "dbg.h"
#include "editor.h"

void enableRawMode(struct termios* orig_termios) {
  check(tcgetattr(STDIN_FILENO, orig_termios) == -1, "enableRawMode");
  /* Save original terminal attribute for disableRawMode() to revert the
   * terminal back to its original state when the program ends. Config raw to
   * suits our need when execute the editor.
   */
  struct termios raw = *orig_termios;

  /*
  termios attribute setting reference:
  https://man7.org/linux/man-pages/man0/termios.h.0p.html
  */
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); /* tradition */
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  /* TODO: will ISIG affect signal when dynamically adjust windows size? */
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  check(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1, "enableRawMode");
}

void disableRawMode(struct termios* orig_termios) {
  check(tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios) == -1,
        "disableRawMode");
}

void enableMouseEvent() {
  write(STDOUT_FILENO, "\e[?1000;1006;1015h", 24);
};

void disableMouseEvent() {
  write(STDOUT_FILENO, "\e[?1000;1006;1015l", 24);
};

void initEditor(editorConfig* E) {
  E->mode = NORMAL_MODE;
  E->cx = 0;
  E->cy = 0;
  E->rx = 0;
  E->ry = 0;
  E->tx = 1;
  E->ty = 1;
  E->data = NULL;
  E->keyStroke = ' ';
  E->dirty = 0;
  E->numrows = 0;
  E->searchResultRow = -1;
  E->searchResultCol = -1;
  E->rowoff = 0;
  E->coloff = 0;
  E->filename = NULL;
  E->statusmsg[0] = '\0';
  E->statusmsg_time = 0;
  E->keystroke_time = 0;
  check(getWindowSize(&E->screenrows, &E->screencols) == -1, "getWindowSize");

  /* for status bar */
  E->screenrows -= 2;
}

void updateEditor(editorConfig* E) {
  check(getWindowSize(&E->screenrows, &E->screencols) == -1, "getWindowSize");
  E->screenrows -= 2;
}

int getCursorPosition(int* rows, int* cols) {
  char buf[32];
  unsigned int i = 0;
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
    return -1;
  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1)
      break;
    if (buf[i] == 'R')
      break;
    i++;
  }
  buf[i] = '\0';
  if (buf[0] != '\x1b' || buf[1] != '[')
    return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
    return -1;
  return 0;
}

int getWindowSize(int* rows, int* cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
      return -1;
    return getCursorPosition(rows, cols);
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

void editorOpen(editorConfig* E, char* filename) {
  free(E->filename);
  E->filename = strdup(filename);

  FILE* fp = fopen(filename, "r");
  check(!fp, "Fail to open %s", filename);

  char* line = NULL;
  size_t linecap = 0;
  ssize_t linelen;

  while ((linelen = getline(&line, &linecap, fp)) != -1) {
    while (linelen > 0 &&
           (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) {
      linelen--;
    }
    insertRow(E, E->numrows, line, linelen);
  }

  free(line);
  fclose(fp);
  E->dirty = 0;
}

void editorSave(editorConfig* E) {
  /* TODO: more advance saving technique,
  write to a temp file, then rename it as current file */
  if (E->dirty == 0) {
    setStatusMessage(E, "No write since last change.");
    return;
  }
  if (E->filename == NULL) {
    E->filename = promptInfo(E, "Save as: %s (ESC to cancel)");
    if (E->filename == NULL) {
      setStatusMessage(E, "Save aborted");
      return;
    }
  }
  int len;
  char* buf = rowsToString(E, &len);

  int fd = open(E->filename, O_RDWR | O_CREAT, 0644);
  if (fd != -1) {
    if (ftruncate(fd, len) != -1) {
      if (write(fd, buf, len) == len) {
        close(fd);
        free(buf);
        E->dirty = 0;
        setStatusMessage(E, "%d bytes written to disk", len);
        return;
      }
    }
    close(fd);
  }
  free(buf);
  setStatusMessage(E, "Can't save! I/O error: %s", strerror(errno));
}

void editorQuit(editorConfig* E) {
  if (E->dirty) {
    setStatusMessage(E,
                     "Unsave change. :w <filename> -> save; :q! -> force quit");
    return;
  }
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  exit(0);
}

void editorFind(editorConfig* E, char* query) {
  int saved_cx = E->cx;
  int saved_cy = E->cy;
  int saved_coloff = E->coloff;
  int saved_rowoff = E->rowoff;

  editorFindAll(E, query);
  editorFindForward(E, query);
  while (1) {
    int c = readInput(E);
    if (c == 'q') {
      break;
    } else if (c == 'n') {
      editorFindForward(E, query);
    } else if (c == 'p') {
      editorFindBackward(E, query);
    } else if (c == 'i') {
      /* TODO: to insert mode */
      break;
    }
  }
  editorFindQuit(E, query);
  E->cx = saved_cx;
  E->cy = saved_cy;
  E->coloff = saved_coloff;
  E->rowoff = saved_rowoff;
  E->searchResultRow = -1;
}

void editorFindQuit(editorConfig* E, char* query) {
  for (int i = 0; i < E->numrows; i++) {
    row* row = &E->data[i];
    char* match = strstr(row->render, query);
    if (match) {
      /* match highlight */
      memset(&row->hl[match - row->render], HL_NORMAL, strlen(query));
    }
  }
  renderScreen(E);
}

void editorFindAll(editorConfig* E, char* query) {
  for (int i = 0; i < E->numrows; i++) {
    row* row = &E->data[i];
    char* match = strstr(row->render, query);
    if (match) {
      /* match highlight */
      memset(&row->hl[match - row->render], HL_MATCH, strlen(query));
    }
  }
  renderScreen(E);
}

void editorFindForward(editorConfig* E, char* query) {
  for (int i = E->searchResultRow + 1; i < E->numrows; i++) {
    row* row = &E->data[i];
    char* match = strstr(row->render, query);
    if (match) {
      E->searchResultRow = i;
      E->cy = i;
      E->cx = rowRxToCx(row, match - row->render);
      int diff = (E->cy - E->rowoff) - (E->screenrows / 2);
      if ((diff > 0) && (E->rowoff + diff < E->numrows)) {
        E->rowoff += diff;
      } else if ((diff < 0) && (E->rowoff + diff > 0)) {
        E->rowoff += diff;
      }
      /* match highlight */
      memset(&row->hl[match - row->render], HL_MATCH, strlen(query));
      renderScreen(E);
      break;
    }
  }
}

void editorFindBackward(editorConfig* E, char* query) {
  for (int i = E->searchResultRow - 1; i >= 0; i--) {
    row* row = &E->data[i];
    char* match = strstr(row->render, query);
    if (match) {
      E->searchResultRow = i;
      E->cy = i;
      E->cx = rowRxToCx(row, match - row->render);
      int diff = (E->cy - E->rowoff) - (E->screenrows / 2);
      if ((diff > 0) && (E->rowoff + diff < E->numrows)) {
        E->rowoff += diff;
      } else if ((diff < 0) && (E->rowoff + diff > 0)) {
        E->rowoff += diff;
      }
      renderScreen(E);
      break;
    }
  }
}

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

  E->data[at].hl = NULL;
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

  updateSyntax(row);
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
  updateRow(E, &E->data[E->cy]);
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
  updateRow(E, &E->data[E->cy]);
  E->cx++;
}

void freerow(row* row) {
  free(row->chars);
  free(row->render);
  free(row->hl);
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
  updateRow(E, row);
  E->dirty++;
}

void rowdeleteChar(editorConfig* E, row* row, int at) {
  if (at < 0 || at >= row->size)
    return;
  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
  row->size--;
  updateRow(E, row);
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

void changeWord(editorConfig* E) { /* regex */
  ;
}

void deleteWord(editorConfig* E) {
  ;
}

void replaceChar(editorConfig* E, int ch) {
  ;
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
      case 'o':
      case 'O':
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
      case CTRL_KEY('f'):
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

void moveCursor(editorConfig* E, int key) {
  row* row = (E->cy >= E->numrows) ? NULL : &E->data[E->cy];
  switch (key) {
    case ARROW_LEFT:
      if (E->cx != 0) {
        E->cx--;
      } else if (E->cx == 0) {
        if (E->cy > 0) {
          E->cy--;
          E->cx = E->data[E->cy].size - 1;
        }
      }
      break;
    case ARROW_RIGHT:
      if (row && E->cx < row->size) {
        E->cx++;
        if (E->cx == row->size) {
          if (E->cy < E->numrows - 1) {
            E->cy++;
            E->cx = 0;
          }
        }
      } else if (E->cx == row->size) {
        if (E->cy < E->numrows - 1) {
          E->cy++;
          E->cx = 0;
        }
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
  int rowlen = row ? row->size - 1 : 0;
  if (E->cx > rowlen) {
    E->cx = rowlen;
  }
  if (E->cx < 0) {
    E->cx = 0;
  }
}

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

int is_separator(int c) {
  return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

void updateSyntax(row* row) {
  row->hl = realloc(row->hl, row->rsize);
  memset(row->hl, HL_NORMAL, row->rsize);

  int i;
  while (i < row->rsize) {
    char c = row->render[i];
    if (isdigit(c)) {
      row->hl[i] = HL_NUMBER;
    }
    i++;
  }
}

int syntaxToColor(int hl) {
  switch (hl) {
    case HL_NUMBER:
      return 31;
    case HL_MATCH:
      return 34;
    default:
      return 37;
  }
}