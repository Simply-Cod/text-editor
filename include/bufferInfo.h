#pragma once
#include "buffer.h"
#include <stdbool.h>

enum InputMode {
    NORMAL,
    INSERT,
};

typedef struct BufferInfo {
    unsigned int bufferLength;
    unsigned int currentLineNumb;
    enum InputMode mode;
    bool buffIsDirty;
    bool hasFileName;
    char *fileName;
}BufferInfo;

void infoInit(BufferInfo *bInfo);
int getFileName(BufferInfo *bInfo);
int infoGetLineNumbers(BufferInfo *info, Buffer *buffer, LineBuffer *current);
