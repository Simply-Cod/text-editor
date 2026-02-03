#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


void bufferInit(Buffer *buffer) {
    buffer->head = NULL;
}

int bufferCreateFirstLine(Buffer *buffer) {

    if (buffer->head != NULL) return 0;

    LineBuffer *newLine = malloc(sizeof(LineBuffer));

    if (!newLine) {
        exit(1);
    }
    newLine->buffer = malloc(sizeof(char) * MAX_LINE_LENGTH);
    if (!newLine->buffer) {
        free(newLine);
        exit(1);
    }
    newLine->lineLength = 0;
    newLine->cursorPosition = 0;
    newLine->buffer[0] = '\0';
    newLine->next = NULL;
    newLine->previous = NULL;

    buffer->head = newLine;
    return 1;
}

int bufferAddLineBelow(Buffer *buffer, LineBuffer *currentLine) {

    LineBuffer *newLine = malloc(sizeof(LineBuffer));

    if (!newLine) {
        exit(1);
    }
    newLine->buffer = malloc(sizeof(char) * MAX_LINE_LENGTH);
    if (!newLine->buffer) {
        free(newLine);
        exit(1);
    }
    newLine->lineLength = 0;
    newLine->cursorPosition = 0;
    newLine->buffer[0] = '\0';
    newLine->previous = currentLine;

    if (currentLine->next == NULL) { // Add new line to end
        newLine->next = NULL;
    } else {
        newLine->next = currentLine->next;
    }

    if (currentLine->next != NULL) {
        currentLine->next->previous = newLine;
    }
    currentLine->next = newLine;
    return 1;
}

int bufferAddLineAbove(Buffer *buffer, LineBuffer *currentLine) {

    LineBuffer *newLine = malloc(sizeof(LineBuffer));

    if (!newLine) {
        exit(1);
    }
    newLine->buffer = malloc(sizeof(char) * MAX_LINE_LENGTH);
    if (!newLine->buffer) {
        free(newLine);
        exit(1);
    }
    newLine->lineLength = 0;
    newLine->cursorPosition = 0;
    newLine->buffer[0] = '\0';

    newLine->next = currentLine;
    newLine->previous = currentLine->previous;

    if (currentLine->previous != NULL) {
        currentLine->previous->next = newLine;
    } else {
        buffer->head = newLine;
    }
    currentLine->previous = newLine;
    return 1;
}

void bufferDeleteLine(Buffer *buffer, LineBuffer *toDelete) {

    if (buffer->head->next == NULL) return;

    if (toDelete->previous == NULL)  {
        buffer->head = toDelete->next;

    } else {
        toDelete->previous->next = toDelete->next;
    }

    if (toDelete->next != NULL) {
        toDelete->next->previous = toDelete->previous;
    }

    free(toDelete->buffer);
    free(toDelete);
}

void bufferFree(Buffer *buffer) {

    if (buffer->head == NULL) return;

    LineBuffer *current = buffer->head;
    LineBuffer *next;

    while (current != NULL) {
        next = current->next;

        free(current->buffer);
        free(current);
        current = next;
    }

    buffer->head = NULL;
}

// Line Functions

int lineInser1Byte(LineBuffer *current, char c) {

    if (current->lineLength >= MAX_LINE_LENGTH - 1) return 0;

    memmove(&current->buffer[current->cursorPosition + 1], &current->buffer[current->cursorPosition], current->lineLength - current->cursorPosition);

    current->buffer[current->cursorPosition] = c;
    current->lineLength++;
    current->cursorPosition++;
    current->buffer[current->lineLength] = '\0';

    return 1;
}

int lineInsert2Bytes(LineBuffer *current, char c1, char c2) {

    if (current->lineLength >= MAX_LINE_LENGTH - 2) return 0;

    memmove(&current->buffer[current->cursorPosition + 2], &current->buffer[current->cursorPosition], current->lineLength - current->cursorPosition);

    current->buffer[current->cursorPosition] = c1;
    current->buffer[current->cursorPosition + 1] = c2;
    current->lineLength += 2;
    current->cursorPosition += 2;
    current->buffer[current->lineLength] = '\0';

    return 1;
}

int lineRemoveChar(LineBuffer *current) {
    if (current->lineLength <= 0) return 0;

    if ((unsigned char)current->buffer[current->cursorPosition - 1] >= 128 && (unsigned char)current->buffer[current->cursorPosition - 1] <= 191) {

        memmove(&current->buffer[current->cursorPosition - 2], &current->buffer[current->cursorPosition], current->lineLength - current->cursorPosition);
        current->lineLength -= 2;
        current->cursorPosition -= 2;
        current->buffer[current->lineLength] = '\0';
    } else {
        memmove(&current->buffer[current->cursorPosition - 1], &current->buffer[current->cursorPosition], current->lineLength - current->cursorPosition);
        current->lineLength--;
        current->cursorPosition--;
        current->buffer[current->lineLength] = '\0';
    }
    return 1;
}

int lineMoveCursorLeft(LineBuffer *line) {

    if (line->cursorPosition <= 0) {
        return 0;
    }

    line->cursorPosition--;
    for (;;) {

        if (line->cursorPosition <= 0) break;

        if ((unsigned char)line->buffer[line->cursorPosition] >= 128 && (unsigned char)line->buffer[line->cursorPosition] <= 191 && line->cursorPosition != 0)
            line->cursorPosition--;
        else
            break;
    }
    return 1;
}

int lineMoveCursorRight(LineBuffer *line) {
    if (line->buffer[line->cursorPosition] == '\0')
        return line->cursorPosition;

    line->cursorPosition++;
    for (;;) {

        if (line->buffer[line->cursorPosition] == '\0') break;

        if ((unsigned char)line->buffer[line->cursorPosition] >= 128 && (unsigned char)line->buffer[line->cursorPosition] <= 191)
            line->cursorPosition++;
        else
            break;
    }
    return 1;
}

int lineMoveCursorUp(LineBuffer *line) {
    if (line->previous == NULL) return lineGetVisualCursorPos(line);

    line = line->previous;
    line->cursorPosition = line->lineLength;

    return 1;
}

int lineMoveCursorDown(LineBuffer *line) {
    if (line->next == NULL) return lineGetVisualCursorPos(line);

    line = line->next;
    if (line->lineLength == 0) {
        line->cursorPosition = 0;
    } else {
        line->cursorPosition = line->lineLength;
    }

    return 1;
}

int lineGetVisualCursorPos(LineBuffer *line) {
    int visualCur = line->cursorPosition;

    for (int i = line->cursorPosition; i > 0; i--) {

        if ((unsigned char)line->buffer[i] >= 128 && (unsigned char)line->buffer[i] <= 191)
            visualCur--;
    }
    return visualCur;
}

int lineMoveBuffDown(LineBuffer *line, int count) {

    if (line->next == NULL) return 0;
    if (line->next->lineLength + count >= MAX_LINE_LENGTH) return 0;

    strncpy(&line->next->buffer[line->next->lineLength], &line->buffer[line->cursorPosition], count);
    line->lineLength -= count;
    line->buffer[line->lineLength] = '\0';

    line->next->lineLength += count;
    line->next->buffer[count] = '\0';

    return 1;
}

int lineMoveBuffUp(LineBuffer *line) {
    if (line->previous == NULL) return 0;
    if (line->previous->lineLength + line->lineLength >= MAX_LINE_LENGTH - 1) return 0;

    strncpy(&line->previous->buffer[line->previous->lineLength], &line->buffer[0], line->lineLength);
    line->previous->lineLength += line->lineLength;
    line->previous->buffer[line->previous->lineLength] = '\0';

    line->lineLength = 0;
    line->buffer[0] = '\0';

    return 1;
}
