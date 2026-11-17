#pragma once
#include <stdint.h>

namespace devices::oled {

// Include the first two columns of the robot outline so the stripes meet it.
constexpr unsigned waveWidth = 100;
constexpr unsigned waveFrames = 12;
constexpr unsigned waveDurationMs = 2000;

template <typename Display>
void drawWave(Display &display, unsigned first, unsigned last, int top, int spacing)
{
    static constexpr uint8_t period[] = {
        0, 0, 1, 1, 2, 3, 3, 4, 4, 4, 4, 4,
        4, 4, 4, 3, 3, 2, 1, 1, 0, 0, 0, 0,
    };
    for (unsigned x = first; x < last; ++x)
    {
        for (int stripe = 0; stripe < 3; ++stripe)
        {
            const int y = top + stripe * spacing + period[x % sizeof(period)];
            display.line(x, y, x, y + 2, true);
        }
    }
}

}  // namespace devices::oled
