#include "render.h"
#include "buffer.h"
#include "input.h"
#include <asm-generic/ioctls.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>


Vector2 renderGetTerminalSize() {
    struct winsize win;
    Vector2 termSize = {0, 0};

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) == -1) {
        return termSize;
    }

    termSize.x = win.ws_col;
    termSize.y = win.ws_row;

    return termSize;
}

int renderDraw(Buffer *buffer, LineBuffer *currentLine, enum InputMode mode) {
    
    Vector2 termSize = renderGetTerminalSize();
    if (termSize.x == 0) return 0;

    write(STDOUT_FILENO, "\x1b[H\x1b[2J", 7); // Move to beginning and clear

    LineBuffer *printPtr = buffer->head;
    int row = 0;
    int targetRow = 0;


    for (int i = 0; i < termSize.y - 2; i++) {

        if (printPtr != NULL) {
            printf("%-3d %s\x1b[1E", i + 1, printPtr->buffer);

            if (printPtr == currentLine)
                targetRow = row;
            printPtr = printPtr->next;
            row++;
        } else {
            printf("~\x1b[1E");
        }
    }

    // Draw Status Bar
    printf("-- %s MODE\x1b[0m -- Line: %d, Col: %d",
            (mode == INSERT ? "\x1b[41mINSERT" : "\x1b[44mNORMAL"), targetRow + 1, lineGetVisualCursorPos(currentLine));

    // Set cursor
    printf("\x1b[%d;%dH", targetRow + 1, lineGetVisualCursorPos(currentLine) + 5);
    return 1;
}
