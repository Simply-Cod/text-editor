#include "buffer.h"
#include "render.h"
#include "terminal.h"
#include "input.h"
#include "readAndWrite.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


Terminal term;
Buffer buff;

int main(int argc, char *argv[]) {
    bool quit = false;
    unsigned int ch = 0;
    enum InputMode inputMode = NORMAL;
    char *fName;
    bool havefName = false;
    bool fIsDirty = false;

    if (!TerminalEnableRaw(&term))exit(1);

    bufferInit(&buff);
    bufferCreateFirstLine(&buff);

    LineBuffer *currentLine = buff.head;
    // either read or create new buffer
    if (argc > 1) {
        if (loadFile(&buff, argv[1])) {
            fName = malloc(sizeof(char) * strlen(argv[1]));

            if (fName) havefName = true;
        }
    } else {
        currentLine->buffer[0] = '\0';
        currentLine->lineLength = 0;
    }
    currentLine->cursorPosition = 0;
    //=================================


    // Main loop
    // ================================
    while (!quit) {

        ch = readInput();

        if (inputMode == NORMAL) {

            switch (ch) {
                case 0:
                    break;
                case '0':
                    currentLine->cursorPosition = 0;
                    break;
                case '$':
                    currentLine->cursorPosition = currentLine->lineLength;
                    break;
                case 17: // Ctrl-Q
                    quit = true;
                    break;
                case 'i':
                    inputMode = INSERT;
                    lineMoveCursorLeft(currentLine);
                    write(STDOUT_FILENO, "\x1b[6 q", 5);
                    break;
                case 'a':
                    inputMode = INSERT;
                    write(STDOUT_FILENO, "\x1b[6 q", 5);
                    break;
                case UP:
                case 'k':
                    if (currentLine->previous == NULL) break;

                    currentLine = currentLine->previous;
                    break;
                case DOWN:
                case 'j':
                    if (currentLine->next == NULL) break;

                    currentLine = currentLine->next;
                    break;
                case RIGHT:
                case 'l':
                    lineMoveCursorRight(currentLine);
                    break;
                case LEFT:
                case 'h':
                    lineMoveCursorLeft(currentLine);
                    break;
                case 'o':
                    bufferAddLineBelow(&buff, currentLine);
                    currentLine = currentLine->next;
                    inputMode = INSERT;
                    break;
                case 'O':
                    bufferAddLineAbove(&buff, currentLine);
                    currentLine = currentLine->previous;
                    inputMode = INSERT;
                    break;
                case ':':
                    int cmd = getCommand();

                    switch (cmd) {
                        case 2000:
                            quit = true;
                            break;
                        case 2001:
                            if (havefName) {
                                writeFile(&buff, fName);
                                fIsDirty = false;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }

        } else if (inputMode == INSERT) {

            switch (ch) {
                case 0:
                    break;
                case ESC:
                    inputMode = NORMAL;
                    write(STDOUT_FILENO, "\x1b[2 q", 5); // Block
                    break;

                // Enter
                // =========================================
                case 13:
                    bufferAddLineBelow(&buff, currentLine);
                    fIsDirty = true;

                    if (currentLine->next == NULL) break;

                    if (currentLine->cursorPosition == currentLine->lineLength) {
                        currentLine = currentLine->next;
                    } else {
                        int count = currentLine->lineLength - currentLine->cursorPosition;
                        lineMoveBuffDown(currentLine, count);
                        currentLine = currentLine->next;

                    }
                    break;
                /*----------------------------------------*/

                case 16: // Ctrl-P
                    break;

                case 17: // Ctrl-Q
                    quit = true;
                    break;

                case '\t':
                    for (int i = 0; i < 4; i++) {
                        lineInser1Byte(currentLine, ' ');
                    }
                    fIsDirty = true;
                    break;

                // Ascii printable (1 byte)
                // =========================================
                case 32 ... 126:
                    lineInser1Byte(currentLine, ch);
                    fIsDirty = true;
                    break;
                /*----------------------------------------*/

                // Backspace
                // ========================================
                case 127:
                    if (currentLine->cursorPosition <= 0) {

                        if (currentLine->previous == NULL) break;

                        if (currentLine->lineLength > 0) {

                        lineMoveBuffUp(currentLine);
                        currentLine = currentLine->previous;
                        bufferDeleteLine(&buff, currentLine->next);
                        } else {
                            currentLine = currentLine->previous;
                            bufferDeleteLine(&buff, currentLine->next);
                        }

                    } else {
                        lineRemoveChar(currentLine);
                    }
                    fIsDirty = true;
                    break;
                /*----------------------------------------*/

                // UTF-8 2 byte range
                // ========================================
                case 194:
                case 195:
                    unsigned char seq[2];
                    seq[0] = ch;
                    if (read(STDIN_FILENO, &seq[1], 1) == 0) break;

                    lineInsert2Bytes(currentLine, seq[0], seq[1]);
                    fIsDirty = true;
                    break;
                /*----------------------------------------*/

                case UP:
                    if (currentLine->previous == NULL) break;

                    currentLine = currentLine->previous;
                    break;
                case DOWN:
                    if (currentLine->next == NULL) break;

                    currentLine = currentLine->next;
                    break;
                case RIGHT:
                    lineMoveCursorRight(currentLine);
                    break;
                case LEFT:
                    lineMoveCursorLeft(currentLine);
                    break;
                default:
                    printf("%d ", ch);
                    break;
            }

        } // Insert mode end
        // Draw
        // ===============================================
        renderDraw(&buff, currentLine, inputMode, fIsDirty);

        fflush(stdout);
        /*-----------------------------------------------*/
    }


    write(STDOUT_FILENO, "\x1b[H\x1b[2J", 7);
    bufferFree(&buff);
    TerminalDisableRaw(&term);
    return 0;
}
