#include "dma_controller.hpp"
#include "bus.hpp"



void DMA_CONTROLLER::connect_to_bus(BUS* bus_ptr)
{
    this->bus_parent = bus_ptr;
}

void DMA_CONTROLLER::request_dma(const Byte address_high_nibble)
{
    this->dma_triggered = true;
    this->source_address_high_nibble = address_high_nibble;
    this->i = 0;
}

void DMA_CONTROLLER::update_dma(int cyclesUsed)
{
    if (this->dma_triggered)
    {
        int cycle_counter = cyclesUsed;

        while (this->cycle_counter != 0)
        {
            this->cycle_counter--;

            this->bus_parent->set_memory(OAM + this->i, this->bus_parent->get_memory((this->source_address_high_nibble << 8) + this->i, MEMORY_ACCESS_TYPE::dma_controller), MEMORY_ACCESS_TYPE::dma_controller);
            this->i++;
            if (i >= this->max_cycles_t * 4) // POTENTIAL BUG HERE, need to test later
            {
                this->dma_triggered = false;
                break;
            }

        }
        
    }
}
