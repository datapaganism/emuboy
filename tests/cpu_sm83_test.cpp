#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>

//#include "calculator.hpp"
//#include "C:\dev\datapaganism\emuboy\tests\SM83_tests\CMakeLists.txtjson.hpp"


#include "bus.hpp";

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace {

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

    void from_json(nlohmann::json& j, Test& test)
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
		std::vector<Test> final_tests;
		std::ifstream f("C:\\dev\\datapaganism\\emuboy\\tests\\SM83_tests\\v1\\00.json");
		if (f.is_open())
		{
			json tests_js = json::parse(f);

			for (auto test_js : tests_js)
			{
				Test t;
				from_json(test_js, t);
				final_tests.push_back(t);
			}

		}

		return final_tests;
    }


}  // namespace

class CalculatorParameterizedTest : public testing::TestWithParam<Test> {
};

TEST_P(CalculatorParameterizedTest, Sum)
{
	BUS bus;
	bus.cpu.registers.a = GetParam().init_a;
	bus.cpu.registers.a = GetParam().final_a;
	const auto actual = bus.cpu.registers.a;
	const auto expected = bus.cpu.registers.a;
    EXPECT_EQ(actual, expected);
}



// Parameters from json file
INSTANTIATE_TEST_SUITE_P(Json, CalculatorParameterizedTest, testing::ValuesIn(GetTests("C:\\dev\\datapaganism\\emuboy\\tests\\SM83_tests\\v1\\00.json")));
