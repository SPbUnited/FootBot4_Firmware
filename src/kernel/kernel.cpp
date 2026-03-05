#include "kernel.hpp"

#include "drivers/driver_manager.hpp"

namespace kernel
{

void error(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    drivers::uart4.printf("[ERROR]: ");
    drivers::uart4.vprintf(msg, args);
    va_end(args);

    while (1)
        ;
}

}  // namespace kernel
