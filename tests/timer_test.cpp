#include "bus.hpp"
#include <memory>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <gtest/gtest.h>


class CPUtest : public ::testing::Test
{
protected:
	//std::unique_ptr<BUS> bus;
	BUS bus;

	virtual void SetUp()
	{
		std::srand((unsigned int)std::time(0));
		//this->bus = std::make_unique<BUS>();

	};

	virtual void TearDown()
	{

	};
};



TEST_F(CPUtest, increment_div_timer)
{
	// in 64 m cycles, the div timer increments by 1
	for(int i = 0; i < 63; i++)
		bus.cpu.updateTimers(4);

	ASSERT_EQ(bus.getMemory(DIV, debug), 0xAB);

	bus.cpu.updateTimers(4);

	ASSERT_EQ(bus.getMemory(DIV, debug), 0xAC);
}

TEST_F(CPUtest, increment_timers_tima_256KHz)
{
	bus.setMemory(0xFF07, (0b11111100 | 0b00000001), debug);

	// in 64 m cycles, the div timer increments by 1
	for (int i = 0; i < 3; i++)
		bus.cpu.updateTimers(4);

	ASSERT_EQ(bus.getMemory(TIMA, debug), 0x00);

	bus.cpu.updateTimers(4);

	ASSERT_EQ(bus.getMemory(TIMA, debug), 0x01);
}

TEST_F(CPUtest, increment_timers_tima_256KHz_to_overflow)
{
	bus.setMemory(0xFF07, (0b11111100 | 0b00000001), debug);

	// in 64 m cycles, the div timer increments by 1
	for (int i = 0; i < (4*0xFF) - 1; i++)
		bus.cpu.updateTimers(4);

	ASSERT_EQ(bus.getMemory(TIMA, debug), 0xFE);

	bus.cpu.updateTimers(4);

	ASSERT_EQ(bus.getMemory(TIMA, debug), 0xFF);

	for (int i = 0; i < 4; i++)
		bus.cpu.updateTimers(4);

	ASSERT_EQ(bus.getMemory(TIMA, debug), 0x00);

}