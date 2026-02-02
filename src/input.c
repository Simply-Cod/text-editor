#include "input.h"
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
