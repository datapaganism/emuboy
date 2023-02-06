#include "dma_controller.hpp"
#include "bus.hpp"



void DMA_Controller::connectToBus(BUS* bus_ptr)
{
    this->bus_parent = bus_ptr;
}

void DMA_Controller::requestDMA(const Byte address_high_nibble)
{
    this->dma_triggered = true;
    this->source_address_high_nibble = address_high_nibble;
    this->current_byte_pos = 0;
}

void DMA_Controller::updateDMA(int cyclesUsed)
{
    if (this->dma_triggered)
    {
        this->cycle_counter = cyclesUsed;

        while (this->cycle_counter != 0)
        {
            this->cycle_counter--;

            this->bus_parent->setMemory(OAM + this->current_byte_pos, this->bus_parent->getMemory((this->source_address_high_nibble << 8) + this->current_byte_pos, eMemoryAccessType::dma_controller), eMemoryAccessType::dma_controller);
            this->current_byte_pos++;
            if (current_byte_pos >= this->max_tcycles * 4) // POTENTIAL BUG HERE, need to test later
            {
                this->dma_triggered = false;
                break;
            }

        }
        
    }
}
