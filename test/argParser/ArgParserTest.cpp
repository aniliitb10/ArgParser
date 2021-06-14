#include <argParser/ArgParser.h>
#include <gtest/gtest.h>

struct ArgParserTest : public ::testing::Test
{
    std::string shortOption{"-l"};
    std::string longOption{"--logFilePath"};
    std::string binaryPath{"/home/binary"};
    std::string logFilePath{"/home/logs/log.txt"};
    std::string helpMessage{"Log file path"};
    std::string logFilePathShortOption{fmt::format("{}={}", shortOption, logFilePath)};
    std::string logFilePathLongOption{fmt::format("{}={}", longOption, logFilePath)};
};

TEST_F(ArgParserTest, ShortOptionConstructionTest)
{
    ArgParser argParser{};
    argParser.addArgument(shortOption, longOption, helpMessage);
    char* argv[] = {binaryPath.data(), logFilePathShortOption.data()};
    argParser.parse(2, argv);

    EXPECT_EQ(argParser.retrieve("l"), logFilePath);
    EXPECT_EQ(argParser.retrieve("logFilePath"), logFilePath);
}

TEST_F(ArgParserTest, LongOptionConstructionTest)
{
    ArgParser argParser{};
    argParser.addArgument(shortOption, longOption, helpMessage);
    char* argv[] = {binaryPath.data(), logFilePathLongOption.data()};
    argParser.parse(2, argv);

    EXPECT_EQ(argParser.retrieve("l"), logFilePath);
    EXPECT_EQ(argParser.retrieve("logFilePath"), logFilePath);
}