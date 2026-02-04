#pragma once

// default keybinds
#define CTRL_Q 17

#define ESC 27
#define ENTER 13
#define BACKSPACE 127

// Custom key binds
#define UP      1000
#define DOWN    1001
#define RIGHT   1002
#define LEFT    1003

#define ALT_J 1500
#define ALT_K 1501

// Command signals
#define CMD_QUIT 2000
#define CMD_WRITE 2001


unsigned int readInput();
int getCommand();
