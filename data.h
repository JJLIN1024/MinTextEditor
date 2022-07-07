#ifndef __data_h__
#define __data_h__

#include "editor.h"

/* TODO: VIM-like normal mode jumping
e.g. w for word jump */
// #define TAP_STOP 8

// int rowCxToRx(row*, int);

void insertChar(editorConfig*, int);
void insertRow(editorConfig*, int, char*, size_t);
void rowInsertChar(editorConfig*, row*, int, int);
void insertNewLine(editorConfig*);

void deleteRow(editorConfig*, int);
void rowAppendString(editorConfig*, row*, char*, size_t);
void rowDelChar(editorConfig*, row*, int);
void freerow(row*);
void deleteChar(editorConfig*);

char* rowsToString(editorConfig*, int*);

#endif