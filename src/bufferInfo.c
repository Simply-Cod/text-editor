#include "bufferInfo.h"
#include "buffer.h"
#include "input.h"
#include "render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void infoInit(BufferInfo *bInfo) {
    bInfo->bufferLength = 0;
    bInfo->currentLineNumb = 0;
    bInfo->mode = NORMAL;
    bInfo->buffIsDirty = false;
    bInfo->hasFileName = false;
    bInfo->fileName = NULL;
}

int infoGetLineNumbers(BufferInfo *info, Buffer *buffer, LineBuffer *current) {

    if (buffer->head == NULL) {
        info->bufferLength = 0;
        info->currentLineNumb = 0;
        return 1;
    }

    LineBuffer *line = buffer->head;

    int count = 0;
    while (line != NULL) {

        count++;
        if (line == current)
            info->currentLineNumb = count;

        line = line->next;
    }
    info->bufferLength = count;

    return 1;
}

int getFileName(BufferInfo *bInfo) {

    char fName[64];
    int cursPos = 0;
    int lineLen = 0;
    fName[0] = '\0';

    Vector2 termSize = renderGetTerminalSize();

    int inCommandLine = 1;

    while (inCommandLine) {
        int ch = readInput();

        switch (ch) {
            case 0:
            case UP:
            case DOWN:
            case '\t':
                break;
            case ESC:
                return 0;
            case LEFT:
                if (cursPos <= 0) break;

                cursPos--;
                for (;;) {
                    if ((unsigned char)fName[cursPos] >= 128 && (unsigned char)fName[cursPos] <= 191 && cursPos != 0)
                        cursPos--;
                    else
                        break;
                }
                break;
            case RIGHT:
                if (fName[cursPos] == '\0') break;

                cursPos++;
                for (;;) {
                    unsigned char tmp = fName[cursPos];
                    if (tmp == '\0') break;

                    if (tmp >= 128 && tmp <= 191)
                        cursPos++;
                    else
                        break;
                }
                break;
            case 127: // Backspace
                if (lineLen == 0 || cursPos == 0) break;

                unsigned char tmp = fName[cursPos - 1];

                if (tmp >= 128 && tmp <= 191) {
                    memmove(&fName[cursPos - 2], &fName[cursPos], lineLen - cursPos);
                    lineLen -= 2;
                    cursPos -= 2;
                    fName[lineLen] = '\0';
                } else {
                    memmove(&fName[cursPos - 1], &fName[cursPos], lineLen - cursPos);
                    lineLen--;
                    cursPos--;
                    fName[lineLen] = '\0';
                }
                break;
            case 13: // Enter
                if (fName[0] == '\0') return 0;

                bInfo->fileName = malloc(sizeof(char) * strlen(fName) + 1);
                if (bInfo->fileName == NULL) return 0;
                strncpy(bInfo->fileName, fName, strlen(fName));
                bInfo->fileName[lineLen] = '\0';
                bInfo->hasFileName = true;
                return 1;
                break;
            case 32 ... 126:
                if (lineLen >= 63) break;


                memmove(&fName[cursPos + 1], &fName[cursPos], lineLen - cursPos);
                fName[cursPos] = ch;
                lineLen++;
                cursPos++;
                fName[lineLen] = '\0';
                break;
            case 194:
            case 195:
                if (lineLen >= 62) break;

                unsigned char ch2;
                if (read(STDIN_FILENO, &ch2, 1) == 0) break;

                memmove(&fName[cursPos + 2], &fName[cursPos], lineLen - cursPos);
                fName[cursPos] = ch;
                fName[cursPos + 1] = ch2;
                lineLen += 2;
                cursPos += 2;
                fName[lineLen] = '\0';
                break;
            default:
                 break;
        }
        printf("\x1b[%d;%dH", termSize.y / 2, 10);
        printf("+--------------------------------------------+\x1b[1B\x1b[10G");
        printf("|        Please enter file name              |\x1b[1B\x1b[10G");
        printf("|                                            |\x1b[1B\x1b[10G");
        printf("|                                            |\x1b[1B\x1b[10G");
        printf("|                                            |\x1b[1B\x1b[10G");
        printf("+-----<CR> to confirm---<Esc> to cancel------+");
        printf("\x1b[%d;%dH > ", (termSize.y / 2) + 3, 12);
        printf("%s", fName);

        int visualCur = cursPos;
        for (int i = cursPos; i > 0; i--) {
            if ((unsigned char)fName[i] >= 128 && (unsigned char)fName[i] <= 191) {
                visualCur--;
            }
        }
        printf("\x1b[%dG", visualCur + 15);
        fflush(stdout);

    }

    return 0;
}
