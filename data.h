#ifndef __data_h__
#define __data_h__

#include "editor.h"

// #define MIN_TAP_STOP 8
// #define MIN_LINE_NUMBER_SECTION_LEN 5

void insertRow(editorConfig*, int, char*, size_t);
int rowCxToRx(row*, int);
void updatrow(row*);
void frerow(row*);
void delRow(editorConfig*, int);
void rowInsertChar(editorConfig*, row*, int, int);
void rowAppendString(editorConfig*, row*, char*, size_t);
void rowDelChar(editorConfig*, row*, int);
void insertChar(editorConfig*, int);
void insertNewLine(editorConfig*);
void delChar(editorConfig*);
char* rowsToString(editorConfig*, int*);

#endif