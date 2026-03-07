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
SHELL_EXPORT_VAR(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_VAR_POINT), uartLog, &uartLog,
                 log object for all logs);

void init()
{
    logRegister(&uartLog, &shell::shell);
}

}  // namespace devices::logger
