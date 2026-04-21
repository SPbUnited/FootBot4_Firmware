#pragma once

#include "log.h"
#include "scheduler/scheduler.hpp"

namespace kernel
{

void error(const char* msg, ...);

void reboot();

void delay_ms(uint32_t delay_ms);

void progress_start();
float progress_tick(float progress);
void progress_finish();

#define kerror(msg, ...) logError(msg, ##__VA_ARGS__)
#define kwarning(msg, ...) logWarning(msg, ##__VA_ARGS__)
#define kinfo(msg, ...) logInfo(msg, ##__VA_ARGS__)
#define kdebug(msg, ...) logDebug(msg, ##__VA_ARGS__)
#define kverbose(msg, ...) logVerbose(msg, ##__VA_ARGS__)

#define kprogress_start(msg, ...)                                                                \
    if (LOG_ENABLE)                                                                              \
    {                                                                                            \
        kernel::progress_start();                                                                \
        logWrite(LOG_ALL_OBJ, LOG_DEBUG, DEBUG_TEXT " " msg "" LOG_END, LOG_TIME_STAMP, LOG_TAG, \
                 ##__VA_ARGS__);                                                                 \
    }

#define kprogress_tick(progress)                                        \
    if (LOG_ENABLE)                                                     \
    {                                                                   \
        float current = kernel::progress_tick(progress);                \
        logWrite(LOG_ALL_OBJ, LOG_DEBUG, "...%.1f%%" LOG_END, current); \
    }

#define kprogress_finish(msg, ...)                                       \
    if (LOG_ENABLE)                                                      \
    {                                                                    \
        logWrite(LOG_ALL_OBJ, LOG_DEBUG, msg "" LOG_END, ##__VA_ARGS__); \
        kernel::progress_finish();                                       \
    }

}  // namespace kernel
