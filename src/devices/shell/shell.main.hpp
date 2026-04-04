#pragma once

#include "shell.h"

#define printf(...) shellPrint(ch_shell, ##__VA_ARGS__)
#define getchar() shellGetChar(ch_shell)

char shellGetChar(Shell *shell);
