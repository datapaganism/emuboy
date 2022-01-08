#pragma once
#include "config.hpp"
class BUS;

class DMA_CONTROLLER
{
public:
    bool dma_triggered = false;
    int cycle_counter = 0;
    const int max_cycles_t = 40;
    BUS* bus_parent = nullptr;

    Byte source_address_high_nibble = 0;
    int i = 0;

    void connect_to_bus(BUS* bus_ptr);

    void request_dma(const Byte address_high_nibble);

    void update_dma(int cyclesUsed);
};
