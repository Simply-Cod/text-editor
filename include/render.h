#pragma once
#include "buffer.h"
#include "input.h"
#include <stdbool.h>

typedef struct Vector2{
    int x;
    int y;
}Vector2;


Vector2 renderGetTerminalSize();


int renderDraw(Buffer *buffer, LineBuffer *currentLine, enum InputMode mode, bool fIsDirty);
