#include "input.h"
#include "render.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>


unsigned int readInput() {
    unsigned char ch;

    if (read(STDIN_FILENO, &ch, 1) != 1) return 0;

    if (ch == ESC) {
        unsigned char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) == 0) return ESC;
        if (read(STDIN_FILENO, &seq[1], 1) == 0) return ESC;

        if (seq[0] == '[') {

            switch (seq[1]) {
                case 'A':
                    return UP;
                case 'B':
                    return DOWN;
                case 'C':
                    return RIGHT;
                case 'D':
                    return LEFT;
            }
        }

    }

    return (unsigned int)ch;
}

int getCommand() {

    char command[64];
    int cursPos = 0;
    int lineLen = 0;
    command[0] = '\0';

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
                    if ((unsigned char)command[cursPos] >= 128 && (unsigned char)command[cursPos] <= 191 && cursPos != 0)
                        cursPos--;
                    else
                        break;
                }
                break;
            case RIGHT:
                if (command[cursPos] == '\0') break;

                cursPos++;
                for (;;) {
                    unsigned char tmp = command[cursPos];
                    if (tmp == '\0') break;

                    if (tmp >= 128 && tmp <= 191)
                        cursPos++;
                    else
                        break;
                }
                break;
            case 127: // Backspace
                if (lineLen == 0 || cursPos == 0) break;

                unsigned char tmp = command[cursPos - 1];

                if (tmp >= 128 && tmp <= 191) {
                    memmove(&command[cursPos - 2], &command[cursPos], lineLen - cursPos);
                    lineLen -= 2;
                    cursPos -= 2;
                    command[lineLen] = '\0';
                } else {
                    memmove(&command[cursPos - 1], &command[cursPos], lineLen - cursPos);
                    lineLen--;
                    cursPos--;
                    command[lineLen] = '\0';
                }
                break;
            case 13: // Enter
                if (command[0] == '\0') return 0;

                int i = 0;
                while (command[i] != '\0') {

                    switch (command[i]) {
                        case 'q':
                            return 2000;
                        case 'w':
                            return 2001;
                        default:
                            return 0;
                    }
                    i++;
                }
                break;
            case 32 ... 126:
                if (lineLen >= 63) break;


                memmove(&command[cursPos + 1], &command[cursPos], lineLen - cursPos);
                command[cursPos] = ch;
                lineLen++;
                cursPos++;
                command[lineLen] = '\0';
                break;
            case 194:
            case 195:
                if (lineLen >= 62) break;

                unsigned char ch2;
                if (read(STDIN_FILENO, &ch2, 1) == 0) break;

                memmove(&command[cursPos + 2], &command[cursPos], lineLen - cursPos);
                command[cursPos] = ch;
                command[cursPos + 1] = ch2;
                lineLen += 2;
                cursPos += 2;
                command[lineLen] = '\0';
                break;
            default:
                 break;
        }
        printf("\x1b[%d;%dH", termSize.y - 1, 1);
        printf("-- \x1b[42mCOMMANDLINE\x1b[0m --\n");
        printf("\r\x1b[0K  \x1b[1;32m>\x1b[0m %s", command);

        int visualCur = cursPos;
        for (int i = cursPos; i > 0; i--) {
            if ((unsigned char)command[i] >= 128 && (unsigned char)command[i] <= 191) {
                visualCur--;
            }
        }
        printf("\x1b[%dG", visualCur + 5);
        fflush(stdout);

    }

    return 0;
}
