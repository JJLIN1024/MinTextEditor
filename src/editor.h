#ifndef __editor_h__
#define __editor_h__

#include <termios.h>  // orig_termios
#include <time.h>

/* TODO: VIM-like normal mode jumping
e.g. w for word jump */
#define TAB_WIDTH 4
#define CTRL_KEY(k) ((k)&0x1f)
#define KEY_TIMEOUT 0.5
#define MIN_VERSION "0.0.1"
/* width & padding for display line number at the left of the screen */
#define LINE_NUMBER_DATA 3
#define LINE_NUMBER_PADDING 1
#define LINE_NUMBER_WIDTH (LINE_NUMBER_DATA + LINE_NUMBER_PADDING)
#define BUFFER_INIT \
  { NULL, 0 }

/* Data Buffer */
typedef struct row {
  int size;
  char* chars;
  int rsize;
  char* render;
  unsigned char* hl; /* syntax highlight */
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

/* buffer for STDOUT */
typedef struct buffer {
  char* start;
  int size;
} buffer;

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

/* Syntax Highlight */
enum highlight {
  HL_NORMAL = 0,
  HL_NUMBER,
  HL_MATCH,
};

enum editorMode { NORMAL_MODE = 0, INSERT_MODE, VISUAL_MODE };

// terminal setting
void enableRawMode(struct termios*);
void disableRawMode(struct termios*);
void enableMouseEvent();
void disableMouseEvent();

// editor
void initEditor(editorConfig*);
void updateEditor(editorConfig*);
int getCursorPosition(int*, int*);
int getWindowSize(int*, int*);
void editorOpen(editorConfig*, char*);
void editorSave(editorConfig*);
void editorQuit(editorConfig*);
/* use callback to lower time complexity */
void editorFindAll(editorConfig*, char*);
void editorFindQuit(editorConfig*, char*);
void editorFind(editorConfig*, char*);
void editorFindForward(editorConfig*, char*);
void editorFindBackward(editorConfig*, char*);

// int rowCxToRx(row*, int);
// data buffer
void insertChar(editorConfig*, int);
void insertRow(editorConfig*, int, char*, size_t);
void updateRow(editorConfig*, row*);
void rowInsertChar(editorConfig*, row*, int, int);
void insertNewLine(editorConfig*);
void deleteRow(editorConfig*, int);
void rowAppendString(editorConfig*, row*, char*, size_t);
void rowDelChar(editorConfig*, row*, int);
void freerow(row*);
void deleteChar(editorConfig*);
void changeWord(editorConfig*);
void deleteWord(editorConfig*);
void replaceChar(editorConfig*, int);
char* rowsToString(editorConfig*, int*);

void moveCursor(editorConfig*, int);

// events
int readInput(editorConfig*);
void processEvent(editorConfig*);
void processNormalCommand(editorConfig*, char);
void setStatusMessage(editorConfig*, const char*, ...);
char* promptInfo(editorConfig*, char*);

void bufferAppend(buffer*, const char*, int);
void bufferFree(buffer*);

// screen render
void renderRows(editorConfig*, buffer*);
void renderStatusBar(editorConfig*, buffer*);
int rowCxToRx(row*, int);
int rowRxToCx(row*, int);
void renderCursor(editorConfig*);
void renderMessageBar(editorConfig*, buffer*);
void renderScreen(editorConfig*);

// syntax highlight
int is_separator(int);
void updateSyntax(row*);
int syntaxToColor(int);

#endif
