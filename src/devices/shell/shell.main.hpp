#pragma once

#include "shell.h"

#define getshell()                       \
    Shell *ch_shell = shellGetCurrent(); \
    if (!ch_shell)                       \
    {                                    \
        return -1;                       \
    }

#define printf(...) shellPrint(ch_shell, ##__VA_ARGS__)
#define getchar() shellGetChar(ch_shell)

char shellGetChar(Shell *shell);
