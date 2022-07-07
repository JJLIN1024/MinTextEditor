#include <stdlib.h>
#include <unistd.h>

#include "dbg.h"
#include "editor.h"
#include "terminal.h"

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
