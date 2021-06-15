#include <argParser/ArgParser.h>
#include <gtest/gtest.h>
#include "../TestUtil.h"

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
    char *argv[] = {binaryPath.data(), logFilePathShortOption.data()};
    argParser.parse(2, argv);

    EXPECT_EQ(argParser.retrieve("l"), logFilePath);
    EXPECT_EQ(argParser.retrieve("logFilePath"), logFilePath);
}

TEST_F(ArgParserTest, LongOptionConstructionTest)
{
    ArgParser argParser{};
    argParser.addArgument(shortOption, longOption, helpMessage);
    char *argv[] = {binaryPath.data(), logFilePathLongOption.data()};
    argParser.parse(2, argv);

    EXPECT_EQ(argParser.retrieve("l"), logFilePath);
    EXPECT_EQ(argParser.retrieve("logFilePath"), logFilePath);
}

TEST_F(ArgParserTest, NotInCmdLineOptionTest)
{
    ArgParser argParser{};
    argParser.addArgument(shortOption, longOption, helpMessage);
    char *argv[] = {binaryPath.data(), logFilePathLongOption.data()};
    argParser.parse(2, argv);

    EXPECT_EXCEPTION(argParser.retrieve("counter"), std::runtime_error, "Couldn't find [counter] in arguments");
    EXPECT_EXCEPTION(argParser.retrieve("c"), std::runtime_error, "Couldn't find [c] in arguments");
}

TEST_F(ArgParserTest, DefaultValueTest)
{
    ArgParser argParser{};
    std::string defaultPath{"/home/"};
    argParser.addArgument(shortOption, longOption, helpMessage, defaultPath);
    char *argv[] = {binaryPath.data()};
    argParser.parse(1, argv); // no value was passed

    EXPECT_EQ(argParser.retrieve("l"), defaultPath);
    EXPECT_EQ(argParser.retrieve("logFilePath"), defaultPath);
}

TEST_F(ArgParserTest, DefaultValueOverrideTest)
{
    ArgParser argParser{};
    std::string defaultPath{"/home/"};
    argParser.addArgument(shortOption, longOption, helpMessage, defaultPath);
    char *argv[] = {binaryPath.data(), logFilePathLongOption.data()};
    argParser.parse(2, argv);

    EXPECT_EQ(argParser.retrieve("l"), logFilePath);
    EXPECT_EQ(argParser.retrieve("logFilePath"), logFilePath);
}

TEST_F(ArgParserTest, IntRetrieveTest)
{
    ArgParser argParser{};
    argParser.addArgument("-c", "--counter", "to get the counter");
    std::string counterLongOption{"--counter=10"};
    char *argv[] = {binaryPath.data(), counterLongOption.data()};
    argParser.parse(2, argv);

    EXPECT_EQ(argParser.retrieve<int>("c"), 10);
    EXPECT_EQ(argParser.retrieve<int>("counter"), 10);
}

TEST_F(ArgParserTest, BoolRetrieveTest)
{
    ArgParser argParser{};
    argParser.addArgument("-al", "--allowLogging", "to allow logging");
    argParser.addArgument("-aw", "--allowWaiting", "to allow waiting");
    argParser.addArgument("-ar", "--allowInterrupting", "to allow interrupting");
    std::string loggingArg{"--allowLogging=true"};
    std::string waitingArg{"-aw=false"};
    std::string interruptingArg{"-ar=True"};
    char *argv[] = {binaryPath.data(), loggingArg.data(), waitingArg.data(), interruptingArg.data()};
    argParser.parse(4, argv);

    EXPECT_TRUE(argParser.retrieve<bool>("al"));
    EXPECT_FALSE(argParser.retrieve<bool>("aw"));
    EXPECT_TRUE(argParser.retrieve<bool>("ar"));
}

TEST_F(ArgParserTest, InvalidArgTest)
{
    ArgParser argParser{};
    argParser.addArgument("-al", "--allowLogging", "to allow logging");
    EXPECT_EXCEPTION(argParser.parse(0, nullptr), std::runtime_error, "Invalid command line arguments");
    char *argv[] = {binaryPath.data()};
    EXPECT_EXCEPTION(argParser.parse(0, argv), std::runtime_error, "Invalid command line arguments");
    EXPECT_EXCEPTION(argParser.parse(1, nullptr), std::runtime_error, "Invalid command line arguments");
}

TEST_F(ArgParserTest, RepeatedArgTest)
{
    ArgParser argParser{};
    argParser.addArgument("-a", "--al", "to allow logging");
    EXPECT_EXCEPTION(argParser.addArgument("-a", "--allow", "ambiguous option"),
                     std::runtime_error, "Duplicate arguments: -a, --allow");
    EXPECT_EXCEPTION(argParser.addArgument("-al", "--allow", "ambiguous option"),
                     std::runtime_error, "Duplicate arguments: -al, --allow");

    argParser.addArgument("-allow", "--allowLogging", "to allow logging");
    EXPECT_EXCEPTION(argParser.addArgument("-al", "--allow", "ambiguous option"),
                     std::runtime_error, "Duplicate arguments: -al, --allow");
    EXPECT_EXCEPTION(argParser.addArgument("-l", "--al", "ambiguous option"),
                     std::runtime_error, "Duplicate arguments: -l, --al");
}

TEST_F(ArgParserTest, HelpStringTest)
{
    ArgParser argParser{};
    const std::string topLine{"Following is a list of configured arguments:\n"};
    EXPECT_EQ(argParser.helpMsg(), fmt::format("{}-h, --help\n\tdescription: to get this message\n", topLine));

    argParser.addArgument("-l", "--logfile", "logfile path");
    EXPECT_EQ(argParser.helpMsg(), fmt::format("{}-h, --help\n\tdescription: to get this message\n"
                                               "-l, --logfile\n\tdescription: logfile path\n", topLine));

    // now with a default value
    argParser.addArgument("-c", "--counter", "to get the counter", 15);
    EXPECT_EQ(argParser.helpMsg(), fmt::format("{}-h, --help\n\tdescription: to get this message\n"
                                               "-l, --logfile\n\tdescription: logfile path\n"
                                               "-c, --counter\n\tdescription: to get the counter, default: 15\n", topLine));
}

TEST_F(ArgParserTest, ForbiddenHelpArgs)
{
    // This is to test the behavior that user is not allowed to enter 'h' or 'help' as arguments
    // as these are reserved arguments for getting help
    ArgParser argParser{};
    EXPECT_EXCEPTION(argParser.addArgument("-h", "--help", "some description"), std::runtime_error,
            "Duplicate arguments: -h, --help");

    EXPECT_EXCEPTION(argParser.addArgument("-h", "--hl", "some description"), std::runtime_error,
                     "Duplicate arguments: -h, --hl");

    EXPECT_EXCEPTION(argParser.addArgument("-hl", "--help", "some description"), std::runtime_error,
                     "Duplicate arguments: -hl, --help");
}

TEST_F(ArgParserTest, ShortHelpStringTest)
{
    ArgParser argParser{};
    std::string shortHelpString{"-h"};
    char * argv[] = {binaryPath.data(), shortHelpString.data()};
    argParser.parse(2, argv);
    EXPECT_TRUE(argParser.needHelp());
}

TEST_F(ArgParserTest, LongHelpStringTest)
{
    ArgParser argParser{};
    std::string LongHelpString{"--help"};
    char * argv[] = {binaryPath.data(), LongHelpString.data()};
    argParser.parse(2, argv);
    EXPECT_TRUE(argParser.needHelp());
}

TEST_F(ArgParserTest, DoesntNeedHelpWhenNoArgTest)
{
    ArgParser argParser{};
    char * argv[] = {binaryPath.data()};
    argParser.parse(1, argv);
    EXPECT_FALSE(argParser.needHelp());
}

TEST_F(ArgParserTest, DoesntNeedHelpWhenArgsTest)
{
    ArgParser argParser{};
    argParser.addArgument(shortOption, longOption, helpMessage);
    char * argv[] = {binaryPath.data(), logFilePathLongOption.data()};
    argParser.parse(2, argv);
    EXPECT_FALSE(argParser.needHelp());
}

TEST_F(ArgParserTest, AppNameWhenNoArgsTest)
{
    ArgParser argParser{};
    char * argv[] = {binaryPath.data()};
    argParser.parse(1, argv);
    EXPECT_EQ(binaryPath, argParser.getAppName());
}

TEST_F(ArgParserTest, AppNameWhenArgsTest)
{
    ArgParser argParser{};
    argParser.addArgument(shortOption, longOption, helpMessage);
    char * argv[] = {binaryPath.data(), logFilePathLongOption.data()};
    argParser.parse(2, argv);
    EXPECT_EQ(binaryPath, argParser.getAppName());
}

TEST_F(ArgParserTest, DescriptionWhenNoArgsTest)
{
    ArgParser argParser{"An app for testing"};
    EXPECT_EQ("An app for testing", argParser.getDescription());

    char * argv[] = {binaryPath.data()};
    argParser.parse(1, argv);
    EXPECT_EQ("An app for testing", argParser.getDescription()); // must be same even after parsing
    EXPECT_EQ(argParser.helpMsg(), "An app for testing\n"
                                   "Following is a list of configured arguments:\n"
                                   "-h, --help\n\tdescription: to get this message\n");
}
TEST_F(ArgParserTest, DescriptionWhenArgsTest)
{
    ArgParser argParser{"An app for testing"};
    EXPECT_EQ("An app for testing", argParser.getDescription());

    argParser.addArgument(shortOption, longOption, helpMessage);
    char * argv[] = {binaryPath.data(), logFilePathLongOption.data()};
    argParser.parse(2, argv);
    EXPECT_EQ("An app for testing", argParser.getDescription()); // must be same even after parsing
    EXPECT_EQ(argParser.helpMsg(), "An app for testing\n"
                                   "Following is a list of configured arguments:\n"
                                   "-h, --help\n\tdescription: to get this message\n"
                                   "-l, --logFilePath\n\tdescription: Log file path\n");
}