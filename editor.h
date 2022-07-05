#ifndef __editor_h__
#define __editor_h__

#include <termios.h>
#include <time.h>

#include "buffer.h"

#define CTRL_KEY(k) ((k)&0x1f)
#define MIN_VERSION "0.0.1"

/* Data Buffer */
typedef struct erow {
  int size;
  char *chars;
} erow;

typedef struct editorConfig {
  int mode; /* true for normal mode, false for insert mode */
  int cx, cy;
  int rx;
  int rowoff;
  int coloff;
  int screenrows;
  int screencols;
  int numrows;
  erow *row;
  char *filename;
  struct termios orig_termios;
} editorConfig;

enum editorKey {
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN,

};

enum editorMode { NORMAL_MODE = 0, INSERT_MODE, VISUAL_MODE };

void initEditor(editorConfig *);
int getCursorPosition(int *, int *);
int getWindowSize(editorConfig *);
void editorAppendRow(char *, size_t, editorConfig *);
void editorOpen(char *, editorConfig *);
int editorReadKey(editorConfig *);
void editorMoveCursor(int, editorConfig *);
void editorProcessKeypress(editorConfig *);
void editorDrawRaws(editorConfig *, abuf *);
void editorRefreshScreen(editorConfig *);
#endif
