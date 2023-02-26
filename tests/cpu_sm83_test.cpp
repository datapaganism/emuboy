#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <fstream>


//#include "calculator.hpp"
//#include "C:\dev\datapaganism\emuboy\tests\SM83_tests\CMakeLists.txtjson.hpp"

//https://blog.andreiavram.ro/gtest-parameterized-tests-json/

#include "dummy_bus.hpp";

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
		Byte init_a;
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
		int final_ie;
		std::vector<ram> final_rams;

		std::vector<cycles> cycles;
		int cycles_to_execute;

	};

    void from_json(nlohmann::json& j, Test& test)
    {
		test.test_name = j["name"];
		std::replace(test.test_name.begin(), test.test_name.end(), ' ', '_');

		if (test.test_name == "C3_002B")
		{
			NO_OP;
		}

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
		(!j["final"]["ie"].empty()) ? test.final_ie = j["final"]["ie"] : test.final_ie = -1;

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
		std::ifstream f(path);
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

	//std::string PrintToString(const Test& test)
	//{
	//	std::stringstream ss;
	//	ss << "> " << test.test_name;// << " + " << test.b << " = " << test.sum;
	//	return ss.str();
	//}
	//
	std::string PrintToString(const Test& test)
	{
		return test.test_name;
	}

	//std::string PrintToStringParamName(const Test& test)
	//{
	//	return test.test_name;
	//}
}  // namespace


class SM83Test : public testing::TestWithParam<Test> {
};

std::string rom_path = "C:\\dev\\datapaganism\\emuboy\\roms\\empty.gb";
std::string bios_path = "..\\..\\..\\bios\\bios.binx";


TEST_P(SM83Test, opcode)
{

	
	// setup
	BUS bus;

	auto params = GetParam();

	bus.cpu.registers.a = params.init_a;
	bus.cpu.registers.b = params.init_b;
	bus.cpu.registers.c = params.init_c;
	bus.cpu.registers.d = params.init_d;
	bus.cpu.registers.e = params.init_e;
	bus.cpu.registers.f = params.init_f;
	bus.cpu.registers.h = params.init_h;
	bus.cpu.registers.l = params.init_l;

	bus.cpu.registers.pc = params.init_pc;
	bus.cpu.registers.sp = params.init_sp;

	bus.cpu.interrupt_master_enable = params.init_ime;

	//bus.setMemory(IF_REGISTER, 0xE1, debug);
	bus.setMemory(IE_REGISTER, params.init_ie, debug);
	
	for (ram ram : params.init_rams)
	{
		bus.setMemory(ram.address, ram.data, debug);
	}


	// exectute
	int cycles_executed = 0;

	if (GetParam().test_name == "E8_02B9")
	{
		NO_OP;
		//return;
	}

	cycles_executed += bus.cpu.mStepCPUOneInstruction();


	// compare
    EXPECT_EQ(cycles_executed, params.cycles_to_execute);
    
	EXPECT_EQ(bus.cpu.registers.a, params.final_a);
    EXPECT_EQ(bus.cpu.registers.b, params.final_b);
    EXPECT_EQ(bus.cpu.registers.c, params.final_c);
    EXPECT_EQ(bus.cpu.registers.d, params.final_d);
    EXPECT_EQ(bus.cpu.registers.e, params.final_e);
    EXPECT_EQ(bus.cpu.registers.f, params.final_f);
    EXPECT_EQ(bus.cpu.registers.h, params.final_h);
	EXPECT_EQ(bus.cpu.registers.l ,params.final_l);

    EXPECT_EQ(bus.cpu.registers.pc, params.final_pc);
    EXPECT_EQ(bus.cpu.registers.sp, params.final_sp);

    EXPECT_EQ(bus.cpu.interrupt_master_enable, params.final_ime);
	
	if (params.final_ie != -1)
		EXPECT_EQ(bus.getMemory(IE_REGISTER,debug), params.final_ie);

	for (ram ram : params.final_rams)
	{
		EXPECT_EQ(bus.getMemory(ram.address,debug), ram.data);
	}

}

const std::string folder_path = "C:\\dev\\datapaganism\\emuboy\\tests\\v1";
std::vector<std::string> pathsOfJSONTests(std::string folder_path)
{
	std::vector<std::string> paths;
	for (const auto& entry : std::filesystem::directory_iterator(folder_path))
	{
		if (entry.path().filename().extension() == ".json"
			&& entry.path().filename() != "10.json"
			&& entry.path().filename() != "76.json"
			)
			paths.push_back(entry.path().string());
	}

	return paths;
}

std::vector<Test> ManyTests(std::vector<std::string> paths)
{
	std::vector<Test> b;
	for (auto path : paths)
	{
		auto tests = GetTests(path);
		b.reserve(b.size() + tests.size());
		b.insert(b.end(), tests.begin(), tests.end());
	}
	return b;
};



//INSTANTIATE_TEST_SUITE_P(Json, SM83Test,
//	testing::ValuesIn(
//		GetTests(folder_path + "\\c3.json")
//	), testing::PrintToStringParamName()
//);


//INSTANTIATE_TEST_SUITE_P(Json, SM83Test,
//	testing::ValuesIn(
//		GetTests(pathsOfJSONTests(folder_path)[0xF8])
//	), testing::PrintToStringParamName()
//);

const std::vector<std::string> reduced_tests
{
	folder_path + "\\76.json",
	//folder_path + "\\C3.json",
	//folder_path + "\\CB 0C.json",
	//folder_path + "\\CB 61.json",
	//folder_path + "\\E8.json",
	//folder_path + "\\F2.json",
};

//INSTANTIATE_TEST_SUITE_P(, SM83Test,
//	testing::ValuesIn(
//		ManyTests(reduced_tests)
//	), testing::PrintToStringParamName()
//);

INSTANTIATE_TEST_SUITE_P(Json, SM83Test,
	testing::ValuesIn(
		ManyTests(pathsOfJSONTests(folder_path))
	), testing::PrintToStringParamName()
);


