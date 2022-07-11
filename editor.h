#ifndef __editor_h__
#define __editor_h__

#include <termios.h>
#include <time.h>

/* Data Buffer */
typedef struct row {
  int size;
  char* chars;
  int rsize;
  char* render;
} row;

typedef struct editorConfig {
  int mode;   /* VIM-like: normal, insert, visual*/
  int cx, cy; /* (x, y) on data, 0 based */
  int rx, ry; /* (x, y) after rendering data(e.g. render TAB, etc.), 0 based */
  int tx, ty; /* (x, y) on screen(terminal), 1 based */
  int rowoff; /* data row number of the first row on screen */
  int coloff; /* data column number of the first column on screen */
  int screenrows;
  int screencols;
  int searchResultRow;
  int searchResultCol;
  int numrows; /* number of rows read in from disk */
  row* data;   /* pointer of data read in from disk */
  int dirty;
  char keyStroke;
  char* filename;
  char statusmsg[80];
  time_t statusmsg_time;
  time_t keystroke_time;
  struct termios orig_termios; /* terminal(STDIN) attribute */
} editorConfig;

enum event {
  TAB = 9,
  BACKSPACE = 127,
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN,
  MOUSE_UP,
  MOUSE_DOWN
};

enum editorMode { NORMAL_MODE = 0, INSERT_MODE, VISUAL_MODE };

void initEditor(editorConfig*);
void updateEditor(editorConfig*);
int getCursorPosition(int*, int*);
int getWindowSize(int*, int*);
void editorOpen(editorConfig*, char*);
void editorSave(editorConfig*);
void editorQuit(editorConfig*);
void editorFind(editorConfig*, char*);
void editorFindForward(editorConfig*, char*);
void editorFindBackward(editorConfig*, char*);
#endif
