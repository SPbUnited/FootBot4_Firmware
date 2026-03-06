#include "shell.hpp"

extern "C"
{
#include "shell.h"
}

namespace devices::shell
{

Shell shell;

signed short shellWrite(char *data, unsigned short len)
{
    for (unsigned short i = 0; i < len; i++)
    {
        drivers::uart4.putc(data[i]);
    }
    return len;
}

signed short shellRead(char *data, unsigned short len)
{
    // UNUSED(data);
    // UNUSED(len);
    for (unsigned short i = 0; i < len; i++)
    {
        drivers::uart4.getc(&data[i]);
    }
    return len;
}

char shellBuffer[512];

void my_shellInit()
{
    shell.read = shellRead;
    shell.write = shellWrite;
    shellInit(&shell, shellBuffer, 512);
}

void my_shellLoop()
{
    shellTask(&shell);
}

}  // namespace devices::shell
