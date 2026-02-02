#pragma once

#define MAX_LINE_LENGTH 256

typedef struct LineBuffer {
    int lineLength;
    int cursorPosition;
    char *buffer;
    struct LineBuffer *next;
    struct LineBuffer *previous;
}LineBuffer;

typedef struct Buffer {
    LineBuffer *head;
}Buffer;

void bufferInit(Buffer *buffer);
int bufferCreateFirstLine(Buffer *buffer);
int bufferAddLineBelow(Buffer *buffer, LineBuffer *currentLine);
int bufferAddLineAbove(Buffer *buffer, LineBuffer *currentLine);
void bufferDeleteLine(Buffer *buffer, LineBuffer *toDelete);
void bufferFree(Buffer *buffer);


int lineGetVisualCursorPos(LineBuffer *line);
int lineMoveCursorLeft(LineBuffer *line);
int lineMoveCursorRight(LineBuffer *line);
int lineMoveCursorUp(LineBuffer *line);
int lineMoveCursorDown(LineBuffer *line);

// inserts a char at the cursor position
// returns the visual cursor position
int lineInser1Byte(LineBuffer *current, char c);

// Inserts 2 byte characters
// returns the visual cursor position
int lineInsert2Bytes(LineBuffer *current, char c1, char c2);


int lineRemoveChar(LineBuffer *current);

// Move part from cursor to end down one line
int lineMoveBuffDown(LineBuffer *line, int count);
int lineMoveBuffUp(LineBuffer *line);
