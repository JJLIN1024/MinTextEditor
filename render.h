#ifndef __render_h__
#define __render_h__

#include "editor.h"

#define MIN_VERSION "0.0.1"
/* width & padding for display line number at the left of the screen */
#define LINE_NUMBER_DATA 3
#define LINE_NUMBER_PADDING 1
#define LINE_NUMBER_WIDTH (LINE_NUMBER_DATA + LINE_NUMBER_PADDING)
#define BUFFER_INIT \
  { NULL, 0 }

/* buffer for STDOUT */
typedef struct buffer {
  char* start;
  int size;
} buffer;

void bufferAppend(buffer*, const char*, int);
void bufferFree(buffer*);

void renderRows(editorConfig*, buffer*);
void renderStatusBar(editorConfig*, buffer*);
int rowCxToRx(row*, int);
void renderCursor(editorConfig*);
void renderMessageBar(editorConfig*, buffer*);
void renderScreen(editorConfig*);

#endif