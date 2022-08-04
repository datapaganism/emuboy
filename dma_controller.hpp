#pragma once
#include "config.hpp"
class BUS;

class DMA_Controller
{
public:
    bool dma_triggered = false;
    int cycle_counter = 0;
    const int max_tcycles = 40;
    BUS* bus_parent = nullptr;

    Byte source_address_high_nibble = 0;
    int current_byte_pos = 0;

    void connectToBus(BUS* bus_ptr);

    void requestDMA(const Byte address_high_nibble);

    void updateDMA(int cycles_used);
};
