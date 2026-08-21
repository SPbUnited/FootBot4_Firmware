#ifdef WEBOTS_INTEGRATION

#include "device_manager.hpp"

#include "iostream"
#include "webots.robot.hpp"
#include "webots/Robot.hpp"

namespace devices
{

const char init_message[] =
    "\n"
    "    ______            __  ____        __  __ __\n"
    "   / ____/___  ____  / /_/ __ )____  / /_/ // /\n"
    "  / /_  / __ \\/ __ \\/ __/ __  / __ \\/ __/ // /_\n"
    " / __/ / /_/ / /_/ / /_/ /_/ / /_/ / /_/__  __/\n"
    "/_/    \\____/\\____/\\__/_____/\\____/\\__/  /_/   \n"
    "\n"
    "Build:              " __DATE__ " " __TIME__
    "\n"
    "Firmware version:   " FIRMWARE_VERSION
    "\n"
    "Copyright:          (c) 2025 SPbUnited\n\n";

void init()
{
    // shell::my_shellInit();
    // logger::init();

    // shellWriteString(&shell::shell, init_message);
    // kinfo("Shell initialized");
    // kinfo("Logger initialized");
    std::cout << init_message << std::endl;

    wbrobot = new webots::Robot();
}

};  // namespace devices

#endif
