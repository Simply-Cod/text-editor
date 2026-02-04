#include "buffer.h"
#include <string.h>


int motion_x_RemoveChar(LineBuffer *line) {

    if (line->lineLength <= 0 || line->cursorPosition >= line->lineLength)
        return 0;

    int bytesToRm = 1;
    unsigned char c = line->buffer[line->cursorPosition];

    if (c >=192 && c <= 223) {
        bytesToRm = 2;
    }

    memmove(&line->buffer[line->cursorPosition], &line->buffer[line->cursorPosition + bytesToRm],
                line->lineLength - line->cursorPosition - bytesToRm + 1);

    line->lineLength -= bytesToRm;

    if (line->cursorPosition >= line->lineLength && line->lineLength >0)
        line->cursorPosition = line->lineLength - 1;

    return 1;
}

int motion_w_JumpWord(LineBuffer *line) {
    if (line->cursorPosition >= line->lineLength || line->buffer[line->cursorPosition] == '\0') return 0;

    for (;;) {
        char ch = line->buffer[line->cursorPosition];
        line->cursorPosition++;

        if (ch == ' ' || line->buffer[line->cursorPosition + 1] == '\0')
            break;
    }
    if (line->buffer[line->cursorPosition] == '\0')
        line->cursorPosition--;
    return 1;
}

int motion_b_JumbWordB(LineBuffer *line) {
    if (line->cursorPosition <= 0) return 0;

    line->cursorPosition--;
    for (;;) {
        char ch = line->buffer[line->cursorPosition - 1];
        if (ch == ' ' || line->cursorPosition <= 0)
            break;

        line->cursorPosition--;
    }
    return 1;

}
