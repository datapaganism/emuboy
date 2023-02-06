#include <gtest/gtest.h>
#include "cpu.hpp"

TEST(CPUTest, setRegisterA)
{
    CPU cpu;
    cpu.registers.a = 5;

    ASSERT_EQ(5, cpu.registers.a);
}



int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

