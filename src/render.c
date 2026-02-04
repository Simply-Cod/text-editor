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

static int updateViewPort(ViewPort *viewPort, BufferInfo *info, LineBuffer *line) {
    Vector2 termSize = renderGetTerminalSize();
    if (termSize.x == 0 || termSize.y == 0) return 0;

    viewPort->height = termSize.y - 2;

    if (info->currentLineNumb <= viewPort->topLine) {
        viewPort->topLine = info->currentLineNumb - 1;
    } else if (info->currentLineNumb >= viewPort->topLine + viewPort->height) {
        viewPort->topLine = info->currentLineNumb - viewPort->height;
    }
    viewPort->cursorRow = info->currentLineNumb - viewPort->topLine;
    viewPort->cursorCol = lineGetVisualCursorPos(line);

    return 1;
}

int renderDraw(Buffer *buffer, LineBuffer *currentLine, BufferInfo *bInfo, ViewPort *viewPort) {


    if (!updateViewPort(viewPort, bInfo, currentLine)) return 0;


    write(STDOUT_FILENO, "\x1b[H\x1b[2J", 7); // Move to beginning and clear

    LineBuffer *printPtr = buffer->head;

    int index = 0;
    if (viewPort->topLine >= 1) {

        while (printPtr != NULL && index < viewPort->topLine) {
            printPtr = printPtr->next;
            index++;
        }
    }


    for (int i = 0; i < viewPort->height; i++) {

        if (printPtr != NULL) {
            printf("%3d  %s\x1b[1E", (index + i) + 1, printPtr->buffer);
            printPtr = printPtr->next;
        } else {
            printf("~\x1b[1E");
        }
    }

    // Draw Status Bar
    printf("-- %s MODE\x1b[0m -- Line: %d, Col: %d",
            (bInfo->mode == INSERT ? "\x1b[41mINSERT" : "\x1b[44mNORMAL"), bInfo->currentLineNumb, viewPort->cursorCol);
    if (bInfo->buffIsDirty) {
        printf("\t%s [+]", bInfo->hasFileName ? bInfo->fileName : "<no name>");
    } else {
        printf("\t%s", bInfo->hasFileName ? bInfo->fileName : "<no name>");
    }
    printf("arr cur position: %d, char under cursor: %c", currentLine->cursorPosition, currentLine->buffer[currentLine->cursorPosition]);

    int offset = 6;
    // Set cursor
    printf("\x1b[%d;%dH", viewPort->cursorRow, viewPort->cursorCol + offset);
    return 1;
}
