#pragma once

#define ESC 27

#define UP      1000
#define DOWN    1001
#define RIGHT   1002
#define LEFT    1003

enum InputMode {
    NORMAL,
    INSERT,
};

unsigned int readInput();
int getCommand();
