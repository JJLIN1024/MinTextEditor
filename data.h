#ifndef __data_h__
#define __data_h__

#include "editor.h"

// #define TAP_STOP 8

// int rowCxToRx(row*, int);

void insertChar(editorConfig*, int);
void insertRow(editorConfig*, int, char*, size_t);
void rowInsertChar(editorConfig*, row*, int, int);

// void delRow(editorConfig*, int);

// void rowAppendString(editorConfig*, row*, char*, size_t);
// void rowDelChar(editorConfig*, row*, int);
// void freerow(row*);

// void delChar(editorConfig*);

// void insertNewLine(editorConfig*);

char* rowsToString(editorConfig*, int*);

#endif