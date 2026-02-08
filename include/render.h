#pragma once
#include "buffer.h"
#include "bufferInfo.h"
#include <stdbool.h>
#include <unistd.h>

typedef struct Vector2{
    int x;
    int y;
}Vector2;

typedef struct ViewPort {
    int height;
    int topLine;
    int oldTopLine;
    int cursorRow;
    int cursorCol;
}ViewPort;

typedef enum {
    HL_NORMAL,
    HL_COMMENT,
    HL_STRING,
    HL_NUMBER,
    HL_OPERATOR,
    HL_DATA,
    HL_KEYWORD,
    HL_FUNCTION,
}Highlight;



#define CLR_RESET "\x1b[0m"
#define CLR_RESET_LEN 4

#define CLR_DATA "\x1b[33m"
#define CLR_DATA_LEN 5

#define CLR_COMMENT "\x1b[38;5;102m"
#define CLR_COMMENT_LEN 11

#define CLR_STRING "\x1b[32m"
#define CLR_STRING_LEN 5

#define CLR_NUMBER "\x1b[38;5;104m"
#define CLR_NUMBER_LEN 11

#define CLR_OPERATOR "\x1b[31m"
#define CLR_OPERATOR_LEN 5

#define CLR_KEYWORDS "\x1b[38;5;183m"
#define CLR_KEYWORDS_LEN 11

#define CLR_FUNCTION "\x1b[34m"
#define CLR_FUNCTION_LEN 5

Vector2 renderGetTerminalSize();


int renderDraw(Buffer *buffer, LineBuffer *currentLine, BufferInfo *bInfo, ViewPort *ViewPort);


void parseCHighlight(LineBuffer *line, Highlight *hl);
void printHlLine(LineBuffer *line);
