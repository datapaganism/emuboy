#include "bus.hpp"
#include <iostream>
#include <chrono>
#include <memory>

const std::string rom_path = "..\\..\\..\\roms\\TETRIS.gb";
const std::string bios_path = "..\\..\\..\\bios\\bios.bin";

const int cycles = 10000;

int main()
{
	std::cout << "stack_vs_unique_ptr allocation of heap Bus\n";
	for (int i = 0; i < 3; i++)
	{
		std::cout << "stack\n";
		{
			BUS bus_stack = BUS(rom_path, bios_path);
			auto start = std::chrono::steady_clock::now();
			for (int i = 0; i < cycles; i++)
				bus_stack.cycleSystemOneFrame();
			auto end = std::chrono::steady_clock::now();
			std::chrono::duration<double> elapsed = end - start;
			std::cout << " : " << elapsed.count() << "s\n";
		}
		std::cout << "unique\n";
		{
			std::unique_ptr<BUS> bus_stack = std::make_unique<BUS>(rom_path, bios_path);
			auto start = std::chrono::steady_clock::now();
			for (int i = 0; i < cycles; i++)
				bus_stack->cycleSystemOneFrame();
			auto end = std::chrono::steady_clock::now();
			std::chrono::duration<double> elapsed = end - start;
			std::cout << " : " << elapsed.count() << "s\n";
		}
	}

	return 0;
}