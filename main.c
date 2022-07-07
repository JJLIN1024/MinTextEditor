#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "data.h"
#include "dbg.h"
#include "editor.h"
#include "event.h"
#include "render.h"
#include "terminal.h"

editorConfig E;

static void sigwinchHandler(int sig) {
  if (SIGWINCH == sig) {
    char* filename = E.filename;
    initEditor(&E);
    if (filename != NULL) {
      editorOpen(&E, filename);
    }
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
  } else {
    insertRow(&E, E.cy, "", 1);
  }

  /* listen for Window Size change */
  signal(SIGWINCH, sigwinchHandler);

  setStatusMessage(&E, "HELP: Ctrl-S = save | Ctrl-Q = quit");

  while (1) {
    renderScreen(&E);
    processEvent(&E);
  }

  // disableMouseEvent();
  disableRawMode(&E.orig_termios);

  return 0;
}
