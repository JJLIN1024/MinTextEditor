#ifndef __syntax__h__
#define __syntax__h__

/* Syntax Highlight */
enum highlight {
  HL_NORMAL = 0,
  HL_NUMBER,
  HL_MATCH,
};
int is_separator(int);
void updateSyntax(row*);
int syntaxToColor(int);
#endif