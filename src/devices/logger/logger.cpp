#include "logger.hpp"

#include "devices/shell/shell.hpp"

namespace devices::logger
{

void uartLogWrite(char *buffer, short len);

Log uartLog = {.write = uartLogWrite, .active = true, .level = LOG_DEBUG};

void uartLogWrite(char *buffer, short len)
{
    if (uartLog.shell)
    {
        shellWriteEndLine(uartLog.shell, buffer, len);
    }
}

void init()
{
    logRegister(&uartLog, &shell::shell);
}

}  // namespace devices::logger
