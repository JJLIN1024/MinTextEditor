#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dbg.h"
#include "editor.h"
#include "event.h"
#include "render.h"
#include "terminal.h"

editorConfig E;

/* error: resizing window too often(?) cause segmentation fault
some row content will be missing, after resizing */
/* TODO: error handling */
static void sigwinchHandler(int sig) {
  if (SIGWINCH == sig) {
    getWindowSize(&E.screenrows, &E.screencols);
    renderScreen(&E);
  }
}

int main(int argc, char** argv) {
  if (argc >= 3) {
    log_info("Usage: kilo <filename>(optional)");
  }

  enableRawMode(&E.orig_termios);
  // enableMouseEvent();
  initEditor(&E);

  if (argc == 2) {
    editorOpen(&E, argv[1]);
  }

  /* listen for Window Size change */
  signal(SIGWINCH, sigwinchHandler);
  // event polling ?

  // setStatusMessage(&E, "HELP: Ctrl-S = save | Ctrl-Q = quit");

  while (1) {
    renderScreen(&E);
    processEvent(&E);
  }

  // disableMouseEvent();
  disableRawMode(&E.orig_termios);

  return 0;
}
