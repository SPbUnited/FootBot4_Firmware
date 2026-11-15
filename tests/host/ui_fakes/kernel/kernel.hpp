#pragma once
namespace kernel { inline bool rebooted = false; inline void reboot() { rebooted = true; } }
