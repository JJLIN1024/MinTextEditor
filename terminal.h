#ifndef __terminal_h__
#define __terminal_h__

#include <termios.h>

#include "dbg.h"

struct termios orig_termios;

void enableRawMode(struct termios *);
void disableRawMode(struct termios *);
void enableMouseEvent();
void disableMouseEvent();
#endif