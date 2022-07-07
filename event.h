#ifndef __event_h__
#define __event_h__

#include "editor.h"

#define CTRL_KEY(k) ((k)&0x1f)

int readInput(editorConfig*);
void processEvent(editorConfig*);

#endif