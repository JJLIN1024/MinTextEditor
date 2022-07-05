#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dbg.h"
#include "editor.h"
#include "terminal.h"

editorConfig E;

/* error: resizing window too often(?) cause segmentation fault */
/* TODO: error handling */
static void sigwinchHandler(int sig) {
  if (SIGWINCH == sig) {
    getWindowSize(&E);
    editorRefreshScreen(&E);
  }
}

int main(int argc, char **argv) {
  if (argc >= 3) {
    log_info("Usage: kilo <filename>(optional)");
  }

  enableRawMode(&E.orig_termios);
  // enableMouseEvent();
  initEditor(&E);

  if (argc == 2) {
    editorOpen(argv[1], &E);
  }

  /* listen for Window Size change */
  signal(SIGWINCH, sigwinchHandler);

  // event polling ?

  while (1) {
    editorRefreshScreen(&E);
    editorProcessEvent(&E);
  }

  // disableMouseEvent();
  disableRawMode(&E.orig_termios);

  return 0;
}