#pragma once

#include "log.h"

namespace kernel
{

#define kerror(msg, ...) logError(msg, ##__VA_ARGS__)
#define kwarning(msg, ...) logWarning(msg, ##__VA_ARGS__)
#define kinfo(msg, ...) logInfo(msg, ##__VA_ARGS__)
#define kdebug(msg, ...) logDebug(msg, ##__VA_ARGS__)
#define kverbose(msg, ...) logVerbose(msg, ##__VA_ARGS__)

void error(const char* msg, ...);

void reboot();

}  // namespace kernel
