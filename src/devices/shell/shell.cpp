#include "shell.hpp"

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

int func(int argc, char *argv[])
{
    drivers::uart4.printf("%dparameter(s)\r\n", argc);
    for (int i = 1; i < argc; i++)
    {
        drivers::uart4.printf("%s\r\n", argv[i]);
    }

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), func, func, test);
