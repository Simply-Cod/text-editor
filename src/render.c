#include "render.h"
#include "buffer.h"
#include "bufferInfo.h"
#include <asm-generic/ioctls.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
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

    if (viewPort->oldTopLine != viewPort->topLine) {
        info->renderFull = true;
        viewPort->oldTopLine = viewPort->topLine;
    }

    return 1;
}

int renderDraw(Buffer *buffer, LineBuffer *currentLine, BufferInfo *bInfo, ViewPort *viewPort) {


    if (!updateViewPort(viewPort, bInfo, currentLine)) return 0;

    if (viewPort->topLine != viewPort->oldTopLine) {
        bInfo->renderFull = true;
        viewPort->oldTopLine = viewPort->topLine;
    }


    LineBuffer *printPtr = buffer->head;
    int n = 0;
    int index = 0;
    char lineNumb[32];



    if (!bInfo->renderFull) { // redraw current line

        if (printPtr != currentLine) {
            while (printPtr != currentLine && printPtr->next != NULL) {
                printPtr = printPtr->next;
                index++;
            }
        }
        printPtr = currentLine;

        char mv[32];
        int nMv = snprintf(mv, sizeof(mv), "\x1b[%d;0H\x1b[0m\x1b[0K", viewPort->cursorRow);

        write(STDOUT_FILENO, mv, nMv); // Move to beginning and clear line
        n = snprintf(lineNumb, sizeof(lineNumb), "\x1b[38;5;102m%4d\x1b[0m ", (index + 1));
        write(STDOUT_FILENO, lineNumb, n);
        printHlLine(printPtr);

    } else { // redraw all lines

    if (viewPort->topLine >= 1) {

        while (printPtr != NULL && index < viewPort->topLine) {
            printPtr = printPtr->next;
            index++;
        }
    }

        write(STDOUT_FILENO, "\x1b[H\x1b[2J\x1b[?25l", 13); // Move to beginning and clear And hide cursor
        for (int i = 0; i < viewPort->height; i++) {

            n =snprintf(lineNumb, sizeof(lineNumb), "\x1b[38;5;102m%4d\x1b[0m ", (index + i) + 1);

            if (printPtr != NULL) {
                write(STDOUT_FILENO, lineNumb, n);
                printHlLine(printPtr);
                write(STDOUT_FILENO, "\r\n", 2);
                printPtr = printPtr->next;
            } else {
                write(STDOUT_FILENO, "~\r\n", 3);
            }
        }

        bInfo->renderFull = false;
    }

        char cur[32];
    // Move the cursor to status line
    n = snprintf(cur, sizeof(cur), "\x1b[%d;0H", viewPort->height + 1);
    write(STDOUT_FILENO, cur, n);

    // clear
    write(STDOUT_FILENO, "\x1b[0K", 4);

    // Print mode
    char status[64];
    n = snprintf(status, sizeof(status), "-- %s MODE\x1b[0m -- Line: %d, Col: %d",
            (bInfo->mode == INSERT ? "\x1b[41mINSERT" : "\x1b[44mNORMAL"), bInfo->currentLineNumb, viewPort->cursorCol);
    write(STDOUT_FILENO, status, n);

    // Print file name
    n = snprintf(status, sizeof(status), "\t%s", bInfo->hasFileName ? bInfo->fileName : "<unnamed>");
    write(STDOUT_FILENO, status, n);

    if (bInfo->buffIsDirty)
        write(STDOUT_FILENO, " [+]", 4);

    const int offset = 6;
    // Set cursor and show
    n = snprintf(cur, sizeof(cur), "\x1b[%d;%dH\x1b[?25h", viewPort->cursorRow, viewPort->cursorCol + offset);
    write(STDOUT_FILENO, cur, n);
    // Cursor look

    if (bInfo->mode == NORMAL) {
        write(STDOUT_FILENO, "\x1b[2 q", 5); // Block
    } else if (bInfo->mode == INSERT) {
        write(STDOUT_FILENO, "\x1b[6 q", 5); // I 
    }
    return 1;
}

void printHlLine(LineBuffer *line) {

    if (line->lineLength <= 0) {
        write(STDOUT_FILENO, CLR_RESET, 4);
        return;
    }

    Highlight prev = HL_NORMAL;
    Highlight hl[MAX_LINE_LENGTH + 1];

    parseCHighlight(line, hl);
    write(STDOUT_FILENO, CLR_RESET, 4);

    for (int i = 0; i < line->lineLength; i++) {

        if (hl[i] != prev) {
            switch (hl[i]) {
                case HL_NORMAL:
                    write(STDOUT_FILENO, CLR_RESET, CLR_RESET_LEN);
                    break;
                case HL_COMMENT:
                    write(STDOUT_FILENO, CLR_COMMENT, CLR_COMMENT_LEN);
                    break;
                case HL_STRING:
                    write(STDOUT_FILENO, CLR_STRING, CLR_STRING_LEN);
                    break;
                case HL_NUMBER:
                    write(STDOUT_FILENO, CLR_NUMBER, CLR_NUMBER_LEN);
                    break;
                case HL_OPERATOR:
                    write(STDOUT_FILENO, CLR_OPERATOR, CLR_OPERATOR_LEN);
                    break;
                case HL_DATA:
                    write(STDOUT_FILENO, CLR_DATA, CLR_DATA_LEN);
                    break;
                case HL_KEYWORD:
                    write(STDOUT_FILENO, CLR_KEYWORDS, CLR_KEYWORDS_LEN);
                    break;
                case HL_FUNCTION:
                    write(STDOUT_FILENO, CLR_FUNCTION, CLR_FUNCTION_LEN);
                    break;
            }
            prev = hl[i];
        }

        write(STDOUT_FILENO, &line->buffer[i], 1);
    }
    write(STDOUT_FILENO, CLR_RESET, CLR_RESET_LEN);
}

static const char *c_dataTypes[] = {
    "int", "float", "char",
    "double", "void", "enum",
    "short", "struct", "union",
    "signed", "unsigned",
    NULL,
};

static int isDataType(const char *word, int len) {

    for (int i = 0; c_dataTypes[i]; i++) {
        if ((int)strlen(c_dataTypes[i]) == len && strncmp(c_dataTypes[i], word, len) == 0) {
            return 1;
        }

    }
    return 0;
}

static const char *c_keywords[] = {
    "if", "else", "else if",
    "switch", "case", "default",
    "while", "for", "do",
    "break", "return", "continue",
    "goto", "typedef", "static",
    "const",
    NULL,
};

static int isKeyword(const char *word, int len) {

    for (int i = 0; c_keywords[i]; i++) {
        if ((int)strlen(c_keywords[i]) == len && strncmp(c_keywords[i], word, len) == 0) {
            return 1;
        }

    }
    return 0;
}


void parseCHighlight(LineBuffer *line, Highlight *hl) {

    for (int i = 0; i < MAX_LINE_LENGTH; i++) {
        hl[i] = HL_NORMAL;
    }

    int i = 0;
    while (i < line->lineLength) {

        // Highlight comments
        if (line->buffer[i] == '/' && line->buffer[i + 1] == '/') {
            for (; i < line->lineLength; i++) {
                hl[i] = HL_COMMENT;
            }
            return;
        }

        // Highlight strings
        if (line->buffer[i] == '"') {
            hl[i++] = HL_STRING;

            while (i < line->lineLength) {
                hl[i] = HL_STRING;
                if (line->buffer[i] == '"' && line->buffer[i - 1] != '\\') {
                    i++;
                    break;
                }
                i++;
            }
            continue;
        }

        if (isdigit(line->buffer[i])) {
            hl[i] = HL_NUMBER;
            i++;
            continue;
        }

        // Operators
        switch(line->buffer[i]) {
            case '+':
            case '-':
            case '/':
            case '*':
            case '=':
            case '!':
            case '&':
            case '|':
            case '<':
            case '>':
                hl[i] = HL_OPERATOR;
                i++;
                continue;
        }

        // Data types
        if (isalpha(line->buffer[i]) || line->buffer[i] == '_') {
            int start = i;

            while (i < line->lineLength && (isalnum(line->buffer[i]) || line->buffer[i] == '_')) {
                i++;
            }

            int wordlen = i - start;

            if (isDataType(&line->buffer[start], wordlen)) {
                for (int j = start; j < i; j++) {
                    hl[j] = HL_DATA;
                }
            } else if (isKeyword(&line->buffer[start], wordlen)) {
                for (int j = start; j < i; j++) {
                    hl[j] = HL_KEYWORD;
                }
            }
            continue;
        }

        // Functions
        if (line->buffer[i] == '(') {
            if (i > 1 && (line->buffer[i - 1] != ' ' || line->buffer[i - 1] != '.') && hl[i - 1] == HL_NORMAL) {

                int b = i - 1;
                while (line->buffer[b] != ' ' && line->buffer[b] != ')') {
                    hl[b] = HL_FUNCTION;
                    b--;

                    if (b == 0) break;
                    if (line->buffer[b] == '(' || hl[b] != HL_NORMAL) break;
                }
            }
        }

        i++;
    }

}
