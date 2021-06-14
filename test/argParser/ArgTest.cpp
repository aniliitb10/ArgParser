
#include <argParser/Arg.h>
#include "../TestUtil.h"
#include <gtest/gtest.h>
#include <exception>

struct ArgTest : public ::testing::Test
{};

TEST_F(ArgTest, ContructionTest)
{
    const Arg arg{"-l", "--logfile", "logfile path"};
    EXPECT_EQ("l", arg.getShortOpt());
    EXPECT_EQ("logfile", arg.getLongOpt());
    EXPECT_EQ("-l, --logfile", arg.toString());
    EXPECT_EQ("-l, --logfile\n\tdescription: logfile path", arg.toVerboseString());
    EXPECT_FALSE(arg.hasDefaultValue());
}

TEST_F(ArgTest, DefaultValueTest)
{
    const Arg arg{"-c", "--count", "To get the counter", "58"};
    EXPECT_EQ("c", arg.getShortOpt());
    EXPECT_EQ("count", arg.getLongOpt());
    EXPECT_EQ("-c, --count", arg.toString());
    EXPECT_EQ("-c, --count\n\tdescription: To get the counter, default: 58", arg.toVerboseString());
    EXPECT_TRUE(arg.hasDefaultValue());
    EXPECT_EQ(arg.getDefaultValue(), "58");
}

TEST_F(ArgTest, BadArgumentTest)
{
    // same short and long options
    EXPECT_EXCEPTION(Arg("-c", "-c", "Counter"), std::runtime_error, "Invalid long option: -c");
    EXPECT_EXCEPTION(Arg("--count", "--count", "Counter"), std::runtime_error, "Invalid short option: --count");
    EXPECT_EXCEPTION(Arg("-c", "--c", "Counter"), std::runtime_error, "Short option [-c] must be shorter than Long option [--c]");

    // what if one of them is empty
    EXPECT_EXCEPTION(Arg("", "", "Counter"), std::runtime_error, "Invalid short option: ");
    EXPECT_EXCEPTION(Arg("", "--c", "Counter"), std::runtime_error, "Invalid short option: ");
    EXPECT_EXCEPTION(Arg("-c", "", "Counter"), std::runtime_error, "Invalid long option: ");

    // too many '-'
    EXPECT_EXCEPTION(Arg("-c", "---c", "Counter"), std::runtime_error, "Invalid long option: ---c");
    EXPECT_EXCEPTION(Arg("---c", "cc", "Counter"), std::runtime_error, "Invalid short option: ---c");
    EXPECT_EXCEPTION(Arg("---c", "---cc", "Counter"), std::runtime_error, "Invalid short option: ---c");

    // swap short and long options
    EXPECT_EXCEPTION(Arg("--counter", "-c", "Counter"), std::runtime_error, "Invalid short option: --counter");
    EXPECT_EXCEPTION(Arg("-counter", "-c", "Counter"), std::runtime_error, "Invalid long option: -c");
    EXPECT_EXCEPTION(Arg("-counter", "--c", "Counter"), std::runtime_error, "Short option [-counter] must be shorter than Long option [--c]");
}