#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dbg.h"
#include "editor.h"

editorConfig E;

static void sigwinchHandler(int sig) {
  if (SIGWINCH == sig) {
    updateEditor(&E);
    renderScreen(&E);
  }
}

int main(int argc, char** argv) {
  if (argc >= 3) {
    log_info("Usage: kilo <filename>(optional)");
  }

  // enableMouseEvent();
  enableRawMode(&(E.orig_termios));

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

  disableRawMode(&(E.orig_termios));
  // disableMouseEvent();

  return 0;
}
