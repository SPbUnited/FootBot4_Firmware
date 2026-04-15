#include "system_clock.hpp"

#include "devices/shell/shell.hpp"

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), micros,
                 drivers::system_clock::micros, Robot uptime in microseconds);
