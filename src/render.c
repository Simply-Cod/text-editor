#include "render.h"
#include "buffer.h"
#include "input.h"
#include "bufferInfo.h"
#include <asm-generic/ioctls.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdbool.h>


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

int renderDraw(Buffer *buffer, LineBuffer *currentLine, BufferInfo *bInfo) {

    Vector2 termSize = renderGetTerminalSize();
    if (termSize.x == 0) return 0;

    write(STDOUT_FILENO, "\x1b[H\x1b[2J", 7); // Move to beginning and clear

    LineBuffer *printPtr = buffer->head;


    for (int i = 0; i < termSize.y - 2; i++) {

        if (printPtr != NULL) {
            printf("%3d  %s\x1b[1E", i + 1, printPtr->buffer);
            printPtr = printPtr->next;
        } else {
            printf("~\x1b[1E");
        }
    }

    // Draw Status Bar
    printf("-- %s MODE\x1b[0m -- Line: %d, Col: %d",
            (bInfo->mode == INSERT ? "\x1b[41mINSERT" : "\x1b[44mNORMAL"), bInfo->currentLineNumb, lineGetVisualCursorPos(currentLine));
    if (bInfo->buffIsDirty) {
        printf("\t%s [+]", bInfo->hasFileName ? bInfo->fileName : "<no name>");
    } else {
        printf("\t%s", bInfo->hasFileName ? bInfo->fileName : "<no name>");
    }
    printf("\t Total length: %d Line number: %d", bInfo->bufferLength, bInfo->currentLineNumb);

    // Set cursor
    printf("\x1b[%d;%dH", bInfo->currentLineNumb, lineGetVisualCursorPos(currentLine) + (bInfo->mode == INSERT ? 6 : 5));
    return 1;
}
