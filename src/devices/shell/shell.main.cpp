#include "shell.main.hpp"

char shellGetChar(Shell *shell)
{
    char data;
    if (shell->read && shell->read(&data, 1) == 1)
    {
        return data;
    }
    else
    {
        return -1;
    }
}
