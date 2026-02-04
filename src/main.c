#include "buffer.h"
#include "render.h"
#include "terminal.h"
#include "input.h"
#include "readAndWrite.h"
#include "bufferInfo.h"
#include "motions.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
void cleanup();


Terminal term;
Buffer buff;
BufferInfo bInfo;
ViewPort viewPort;

int main(int argc, char *argv[]) {
    atexit(cleanup);

    bool quit = false;
    unsigned int ch = 0;
    infoInit(&bInfo);
    viewPort.height = 0;
    viewPort.topLine = 0;
    viewPort.cursorRow = 0;

    int prefCurPos = 0;

    if (!TerminalEnableRaw(&term))exit(1);

    bufferInit(&buff);
    bufferCreateFirstLine(&buff);

    LineBuffer *currentLine = buff.head;
    // either read or create new buffer
    if (argc > 1) {
        bInfo.fileName = malloc(sizeof(char) * strlen(argv[1]) + 1);
        if (bInfo.fileName) {
            strncpy(bInfo.fileName, argv[1], strlen(argv[1]));
            bInfo.fileName[strlen(argv[1]) + 1] = '\0';
            bInfo.hasFileName = true;
        }

        if (loadFile(&buff, bInfo.fileName)) {

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


        if (bInfo.mode == NORMAL) {

            switch (ch) {
                case 0:
                    break;
                case '0':
                    currentLine->cursorPosition = 0;
                    prefCurPos = 0;
                    break;
                case '$':
                    if (currentLine->lineLength > 0)
                        currentLine->cursorPosition = currentLine->lineLength - 1;

                    prefCurPos = currentLine->lineLength;
                    break;
                case 17: // Ctrl-Q
                    quit = true;
                    break;
                case 'i':
                    bInfo.mode = INSERT;
                    prefCurPos = currentLine->cursorPosition;
                    break;
                case 'a':
                    if (currentLine->cursorPosition < currentLine->lineLength) {
                        if ((unsigned char)currentLine->buffer[currentLine->cursorPosition] >= 192) {
                            currentLine->cursorPosition += 2;
                        } else {
                            currentLine->cursorPosition++;
                        }
                    }
                    bInfo.mode = INSERT;
                    prefCurPos = currentLine->cursorPosition;
                    break;
                case 'x':
                    if (currentLine->lineLength > 0)
                        motion_x_RemoveChar(currentLine);
                    break;
                case 'w':
                    if (currentLine->lineLength <= 0)
                        break;

                    motion_w_JumpWord(currentLine);
                    prefCurPos = currentLine->cursorPosition;
                    break;
                case 'b':
                    if (currentLine->cursorPosition <= 1) break;

                    motion_b_JumbWordB(currentLine);
                    prefCurPos = currentLine->cursorPosition;
                    break;
                case UP:
                case 'k':
                    if (currentLine->previous == NULL) break;

                    currentLine = currentLine->previous;

                    if (prefCurPos > currentLine->lineLength) {
                        if (currentLine->lineLength > 0)
                            currentLine->cursorPosition = currentLine->lineLength - 1;
                        else
                            currentLine->cursorPosition = 0;
                    }
                    else {
                        currentLine->cursorPosition = prefCurPos;
                    }
                    break;
                case DOWN:
                case 'j':
                    if (currentLine->next == NULL) break;

                    currentLine = currentLine->next;

                    if (prefCurPos > currentLine->lineLength) {
                        if (currentLine->lineLength > 0)
                            currentLine->cursorPosition = currentLine->lineLength - 1;
                        else
                            currentLine->cursorPosition = 0;
                    }
                    else {
                        currentLine->cursorPosition = prefCurPos;
                    }
                    break;
                case RIGHT:
                case 'l':
                        lineMoveCursorRight(currentLine);

                    if (prefCurPos != currentLine->cursorPosition)
                        prefCurPos = currentLine->cursorPosition;
                    break;
                case LEFT:
                case 'h':
                    lineMoveCursorLeft(currentLine);
                    if (prefCurPos != currentLine->cursorPosition)
                        prefCurPos = currentLine->cursorPosition;
                    break;
                case 'o':
                    bufferAddLineBelow(&buff, currentLine);
                    currentLine = currentLine->next;
                    prefCurPos = currentLine->cursorPosition;
                    bInfo.mode = INSERT;
                    break;
                case 'O':
                    bufferAddLineAbove(&buff, currentLine);
                    currentLine = currentLine->previous;
                    prefCurPos = currentLine->cursorPosition;
                    bInfo.mode = INSERT;
                    break;
                case ':':
                    int cmd = getCommand();

                    switch (cmd) {
                        case CMD_QUIT:
                            quit = true;
                            break;
                        case CMD_WRITE:
                            if (bInfo.hasFileName) {
                                writeFile(&buff, bInfo.fileName);
                                bInfo.buffIsDirty = false;
                            } else {
                              if (getFileName(&bInfo)) {
                                  writeFile(&buff, bInfo.fileName);
                                  bInfo.buffIsDirty = false;
                              }
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }

        } else if (bInfo.mode == INSERT) {

            switch (ch) {
                case 0:
                    break;
                case ESC:
                    bInfo.mode = NORMAL;
                    lineMoveCursorLeft(currentLine);
                    break;

                case ENTER:
                    bufferAddLineBelow(&buff, currentLine);
                    bInfo.buffIsDirty = true;

                    if (currentLine->next == NULL) break;

                    if (currentLine->cursorPosition == currentLine->lineLength) {
                        currentLine = currentLine->next;
                    } else {
                        int count = currentLine->lineLength - currentLine->cursorPosition;
                        lineMoveBuffDown(currentLine, count);
                        currentLine = currentLine->next;
                    }
                    break;

                case CTRL_Q:
                    quit = true;
                    break;

                case '\t':
                    for (int i = 0; i < 4; i++) {
                        lineInser1Byte(currentLine, ' ');
                    }
                    bInfo.buffIsDirty = true;
                    break;

                // Ascii printable (1 byte)
                // =========================================
                case 32 ... 126:
                    lineInser1Byte(currentLine, ch);
                    bInfo.buffIsDirty = true;
                    prefCurPos = currentLine->cursorPosition;
                    break;
                /*----------------------------------------*/

                // Backspace
                // ========================================
                case BACKSPACE:
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
                        prefCurPos = currentLine->cursorPosition;

                    } else {
                        lineRemoveChar(currentLine);
                        prefCurPos = currentLine->cursorPosition;
                    }
                    bInfo.buffIsDirty = true;
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
                    bInfo.buffIsDirty = true;
                    prefCurPos = currentLine->cursorPosition;
                    break;
                /*----------------------------------------*/

                case UP:
                    if (currentLine->previous == NULL) break;

                    currentLine = currentLine->previous;

                    if (prefCurPos > currentLine->lineLength) {
                        if (currentLine->lineLength > 0)
                            currentLine->cursorPosition = currentLine->lineLength - 1;
                        else
                            currentLine->cursorPosition = 0;
                    }
                    else {
                        currentLine->cursorPosition = prefCurPos;
                    }
                    break;
                case DOWN:
                    if (currentLine->next == NULL) break;

                    currentLine = currentLine->next;

                    if (prefCurPos > currentLine->lineLength) {
                        if (currentLine->lineLength > 0)
                            currentLine->cursorPosition = currentLine->lineLength - 1;
                        else
                            currentLine->cursorPosition = 0;
                    }
                    else {
                        currentLine->cursorPosition = prefCurPos;
                    }
                    break;
                case RIGHT:
                    lineMoveCursorRight(currentLine);
                    if (prefCurPos != currentLine->cursorPosition)
                        prefCurPos = currentLine->cursorPosition;
                    break;
                case LEFT:
                    lineMoveCursorLeft(currentLine);
                    if (prefCurPos != currentLine->cursorPosition)
                        prefCurPos = currentLine->cursorPosition;
                    break;
                default:
                    printf("%d ", ch);
                    break;
            }

        } // Insert mode end

        infoGetLineNumbers(&bInfo, &buff, currentLine);

        // Draw
        // ===============================================

        // Cursor look

            if (bInfo.mode == NORMAL) {
                write(STDOUT_FILENO, "\x1b[2 q", 5); // Block
            } else if (bInfo.mode == INSERT) {
                write(STDOUT_FILENO, "\x1b[6 q", 5); // I 
            }

            renderDraw(&buff, currentLine, &bInfo, &viewPort);

            fflush(stdout);
        /*-----------------------------------------------*/
    }

    return 0;
}

void cleanup() {
    write(STDOUT_FILENO, "\x1b[H\x1b[2J", 7);

    if (bInfo.fileName != NULL) {
        free(bInfo.fileName);
    }
    if (buff.head != NULL) {
        bufferFree(&buff);
    }
    TerminalDisableRaw(&term);
}
