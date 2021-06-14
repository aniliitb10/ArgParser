#include <gtest/gtest.h>
#include <argParser/ParsedArg.h>

struct ParsedArgTest : public ::testing::Test
{};

TEST_F(ParsedArgTest, ConstructionTest)
{
    const auto parsedShortArg = ParsedArg::parse_arg("-l");
    EXPECT_EQ(ArgTypeEnum::Short, parsedShortArg.argType);
    EXPECT_STREQ("l", parsedShortArg.parsedArg.data());

    const auto parsedLongArg = ParsedArg::parse_arg("--logfilePath");
    EXPECT_STREQ("logfilePath", parsedLongArg.parsedArg.data());
    EXPECT_EQ(ArgTypeEnum::Long, parsedLongArg.argType);
}

TEST_F(ParsedArgTest, InvalidEnumTest)
{
    EXPECT_EQ(ArgTypeEnum::Invalid, ParsedArg::parse_arg("l").argType);
    EXPECT_EQ(ArgTypeEnum::Invalid, ParsedArg::parse_arg("---l").argType);
    EXPECT_EQ(ArgTypeEnum::Invalid, ParsedArg::parse_arg("-l-").argType);
    EXPECT_EQ(ArgTypeEnum::Invalid, ParsedArg::parse_arg("--l-").argType);
    EXPECT_EQ(ArgTypeEnum::Invalid, ParsedArg::parse_arg("-l-l").argType);
    EXPECT_EQ(ArgTypeEnum::Invalid, ParsedArg::parse_arg("-l--").argType);
    EXPECT_EQ(ArgTypeEnum::Invalid, ParsedArg::parse_arg("l--").argType);
    EXPECT_EQ(ArgTypeEnum::Invalid, ParsedArg::parse_arg("- l").argType);
    EXPECT_EQ(ArgTypeEnum::Invalid, ParsedArg::parse_arg("-- l").argType);
    EXPECT_EQ(ArgTypeEnum::Invalid, ParsedArg::parse_arg("-- ").argType);
    EXPECT_EQ(ArgTypeEnum::Invalid, ParsedArg::parse_arg("- ").argType);
    EXPECT_EQ(ArgTypeEnum::Invalid, ParsedArg::parse_arg(" ").argType);
    EXPECT_EQ(ArgTypeEnum::Invalid, ParsedArg::parse_arg("").argType);
}