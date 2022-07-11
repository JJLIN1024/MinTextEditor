#ifndef __event_h__
#define __event_h__

#include "editor.h"

#define CTRL_KEY(k) ((k)&0x1f)
#define KEY_TIMEOUT 0.5

int readInput(editorConfig*);
void processEvent(editorConfig*);
void processNormalCommand(editorConfig*, char);
void setStatusMessage(editorConfig*, const char*, ...);
char* promptInfo(editorConfig*, char*);

#endif