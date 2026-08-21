#ifdef BARE_METAL

#include "logger.hpp"

#include "devices/shell/shell.hpp"
#include "log.h"

namespace devices::logger
{

void uartLogWrite(char *buffer, short len);

Log uartLog = {.write = uartLogWrite, .active = true, .level = LOG_ALL};

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
    asm(".global _printf_float");  // To enable float support in printf
    // https://github.com/platformio/platform-ststm32/issues/616

    logRegister(&uartLog, &shell::shell);
}

int get_loglevel()
{
    return logGetLevel(&uartLog);
}

void set_loglevel(int level)
{
    logSetLevel(&uartLog, static_cast<LogLevel>(level));
}

}  // namespace devices::logger

#endif
