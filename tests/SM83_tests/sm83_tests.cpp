#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "bus.hpp"
// ...

struct cycles
{
	Word address;
	Byte data;
	std::string memory_request_pins;
};

struct ram
{
	Word address;
	Byte data;
};

struct Test
{
	std::string test_name;
	Word init_pc;
	Word init_sp;
	Byte init_a;;
	Byte init_b;
	Byte init_c;
	Byte init_d;
	Byte init_e;
	Byte init_f;
	Byte init_h;
	Byte init_l;
	Byte init_ime;
	Byte init_ie;
	std::vector<ram> init_rams;

	Word final_pc;
	Word final_sp;
	Byte final_a;
	Byte final_b;
	Byte final_c;
	Byte final_d;
	Byte final_e;
	Byte final_f;
	Byte final_h;
	Byte final_l;
	Byte final_ime;
	Byte final_ie;
	std::vector<ram> final_rams;
	std::vector<cycles> cycles;
	int cycles_to_execute;

};

// https://blog.andreiavram.ro/gtest-parameterized-tests-json/

void generate_test_from_js(const nlohmann::json& j, Test& test)
{
	test.test_name = j["name"];
	
	test.init_pc = j["initial"]["pc"];
	test.init_sp = j["initial"]["sp"];
	test.init_a = j["initial"]["a"];
	test.init_b = j["initial"]["b"];
	test.init_c = j["initial"]["c"];
	test.init_d = j["initial"]["d"];
	test.init_e = j["initial"]["e"];
	test.init_f = j["initial"]["f"];
	test.init_h = j["initial"]["h"];
	test.init_l = j["initial"]["l"];
	test.init_ime = j["initial"]["ime"];
	test.init_ie = j["initial"]["ie"];

	for (auto test_ram : j["initial"]["ram"])
	{
		ram ramfjs;
		ramfjs.address = test_ram[0];
		ramfjs.data = test_ram[1];
		test.init_rams.push_back(ramfjs);
	}

	test.final_pc = j["final"]["pc"];
	test.final_sp = j["final"]["sp"];
	test.final_a = j["final"]["a"];
	test.final_b = j["final"]["b"];
	test.final_c = j["final"]["c"];
	test.final_d = j["final"]["d"];
	test.final_e = j["final"]["e"];
	test.final_f = j["final"]["f"];
	test.final_h = j["final"]["h"];
	test.final_l = j["final"]["l"];
	test.final_ime = j["final"]["ime"];
	if (!j["final"]["ie"].empty())
		test.final_ie = j["final"]["ie"];

	for (auto test_ram : j["final"]["ram"])
	{
		ram ramfjs;
		ramfjs.address = test_ram[0];
		ramfjs.data = test_ram[1];
		test.final_rams.push_back(ramfjs);
	}

	for (auto test_cycles : j["cycles"])
	{
		cycles cyclesfjs;
		cyclesfjs.address = test_cycles[0];
		cyclesfjs.data = test_cycles[1];
		cyclesfjs.memory_request_pins = test_cycles[2];

		test.cycles.push_back(cyclesfjs);
	}

	test.cycles_to_execute = test.cycles.size();
}

std::vector<Test> GetTests(const std::string& path)
{
	std::ifstream input(path);
	nlohmann::json j;
	input >> j;
	return j;
}


int main()
{
	std::ifstream f("C:\\dev\\datapaganism\\emuboy\\tests\\SM83_tests\\v1\\00.json");
	if (f.is_open())
	{
		json tests = json::parse(f);

		for (auto test : tests)
		{
			Test tfjs;
			tfjs.test_name = test["name"];
			tfjs.init_pc = test["initial"]["pc"];
			tfjs.init_sp = test["initial"]["sp"];
			tfjs.init_a = test["initial"]["a"];
			tfjs.init_b = test["initial"]["b"];
			tfjs.init_c = test["initial"]["c"];
			tfjs.init_d = test["initial"]["d"];
			tfjs.init_e = test["initial"]["e"];
			tfjs.init_f = test["initial"]["f"];
			tfjs.init_h = test["initial"]["h"];
			tfjs.init_l = test["initial"]["l"];
			tfjs.init_ime = test["initial"]["ime"];
			tfjs.init_ie = test["initial"]["ie"];

			for (auto test_ram : test["initial"]["ram"])
			{
				ram ramfjs;
				ramfjs.address = test_ram[0];
				ramfjs.data = test_ram[1];
				tfjs.init_rams.push_back(ramfjs);
			}

			tfjs.final_pc = test["final"]["pc"];
			tfjs.final_sp = test["final"]["sp"];
			tfjs.final_a = test["final"]["a"];
			tfjs.final_b = test["final"]["b"];
			tfjs.final_c = test["final"]["c"];
			tfjs.final_d = test["final"]["d"];
			tfjs.final_e = test["final"]["e"];
			tfjs.final_f = test["final"]["f"];
			tfjs.final_h = test["final"]["h"];
			tfjs.final_l = test["final"]["l"];
			tfjs.final_ime = test["final"]["ime"];
			if (!test["final"]["ie"].empty())
				tfjs.final_ie = test["final"]["ie"];

			for (auto test_ram : test["final"]["ram"])
			{
				ram ramfjs;
				ramfjs.address = test_ram[0];
				ramfjs.data = test_ram[1];
				tfjs.final_rams.push_back(ramfjs);
			}

			for (auto test_cycles : test["cycles"])
			{
				cycles cyclesfjs;
				cyclesfjs.address = test_cycles[0];
				cyclesfjs.data = test_cycles[1];
				cyclesfjs.memory_request_pins = test_cycles[2];

				tfjs.cycles.push_back(cyclesfjs);
			}

			tfjs.cycles_to_execute = tfjs.cycles.size();
		}

	}
	return 0;
}