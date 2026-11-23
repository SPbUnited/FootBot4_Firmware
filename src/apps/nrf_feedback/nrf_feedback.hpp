#pragma once

#include <cstdint>

namespace apps::nrf_feedback
{
struct TxStats
{
    bool ready;
    uint32_t completed;
    uint32_t lastCompletedMs;
};

const TxStats &txStats();
void init();
void send();
}
