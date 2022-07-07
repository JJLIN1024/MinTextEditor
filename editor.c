#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

// #include "data.h"
#include "data.h"
#include "dbg.h"
#include "editor.h"
#include "render.h"

void initEditor(editorConfig* E) {
  E->mode = NORMAL_MODE;
  E->cx = 0;
  E->cy = 0;
  E->rx = 1;
  E->ry = 1;
  E->data = NULL;
  E->dirty = 0;
  E->numrows = 0;
  E->rowoff = 0;
  E->coloff = 0;
  E->filename = NULL;
  E->statusmsg[0] = '\0';
  E->statusmsg_time = 0;

  check(getWindowSize(&E->screenrows, &E->screencols) == -1, "getWindowSize");

  /* for status bar */
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
  if (E->filename == NULL)
    return;
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
