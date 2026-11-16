#pragma once

namespace devices::oled {

// A 28 × 30 outline, inset from the right edge of the 128 × 32 OLED.
template <typename Display>
void drawRobotIcon(Display &display)
{
    constexpr int x = 98, y = 1;
    constexpr int segments[][4] = {
        // Curved top and its front seam.
        {0,5,3,2}, {3,2,10,0}, {10,0,19,0}, {19,0,25,2}, {25,2,27,5},
        {0,5,6,7}, {6,7,13,9}, {13,9,21,8}, {21,8,27,5},
        // Body, split front panel, and wheel arch.
        {0,5,0,16}, {0,16,13,19}, {13,9,13,20}, {13,20,15,21},
        {15,21,20,21}, {20,21,23,24}, {23,24,23,27}, {23,27,27,25},
        {27,25,27,5}, {1,16,1,25}, {1,25,2,27}, {2,27,13,29},
        {13,19,13,26}, {13,26,14,28}, {14,28,13,29},
        {1,21,13,24}, {1,23,13,26},
        // Wheel outline.
        {16,23,20,23}, {20,23,22,25}, {22,25,21,28},
        {21,28,19,29}, {19,29,16,29}, {16,29,14,27}, {14,27,16,23},
    };
    for (const auto &s : segments)
        display.line(x + s[0], y + s[1], x + s[2], y + s[3], true);
}

}  // namespace devices::oled
