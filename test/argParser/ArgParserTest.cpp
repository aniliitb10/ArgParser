#include <argParser/ArgParser.h>
#include <gtest/gtest.h>
#include "../TestUtil.h"

struct ArgParserTest : public ::testing::Test
{
    std::string logFileShortOption{"-l"};
    std::string logFileLongOption{"--logFilePath"};
    std::string binaryPath{"/home/binary"};
    std::string logFilePath{"/home/logs/log.txt"};
    std::string logFileHelpMessage{"Log file path"};
    std::string logFilePathShortOption{concatenate(logFileShortOption, '=', logFilePath)};
    std::string logFilePathLongOption{concatenate(logFileLongOption, '=', logFilePath)};

    template <typename T>
    std::string to_string(T&& arg) noexcept
    {
        // std::decay to remove cv and reference qualifiers
        if constexpr(std::is_same_v<std::decay_t<T>, std::string>) return arg;
        else if constexpr(std::is_constructible_v<std::string, std::decay_t<T>>) return std::string{std::forward<T>(arg)};
        else if constexpr(std::is_same_v<char, std::decay_t<T>>) return {arg};
        else if constexpr(std::is_same_v<bool, std::decay_t<T>>) return arg ? "true" : "false";
        // otherwise all we support is arithmetic values, let std::to_string take care of issues
        else return std::to_string(std::forward<T>(arg)); // wow! this was fun! without else, return fails to compile!
    }

    template<typename ...Ts>
    std::string concatenate(Ts&& ...args) noexcept
    {
        // This is inefficient compared to fmt::format, but we need to keep this header only library
        // and btw, this happens only in error cases
        return (to_string(std::forward<Ts>(args)) + ...);
    }
};

TEST_F(ArgParserTest, ShortOptionConstructionTest)
{
    ArgParser argParser{};
    argParser.addArgument(logFileShortOption, logFileLongOption, logFileHelpMessage);
    char *argv[] = {&binaryPath[0], &logFilePathShortOption[0]};
    argParser.parse(2, argv);

    EXPECT_EQ(argParser.retrieve("l"), logFilePath);
    EXPECT_EQ(argParser.retrieve("logFilePath"), logFilePath);
}

TEST_F(ArgParserTest, LongOptionConstructionTest)
{
    ArgParser argParser{};
    argParser.addArgument(logFileShortOption, logFileLongOption, logFileHelpMessage);
    char *argv[] = {&binaryPath[0], &logFilePathLongOption[0]};
    argParser.parse(2, argv);

    EXPECT_EQ(argParser.retrieve("l"), logFilePath);
    EXPECT_EQ(argParser.retrieve("logFilePath"), logFilePath);
}

TEST_F(ArgParserTest, NotInCmdLineOptionTest)
{
    ArgParser argParser{};
    argParser.addArgument(logFileShortOption, logFileLongOption, logFileHelpMessage);
    char *argv[] = {&binaryPath[0], &logFilePathLongOption[0]};
    argParser.parse(2, argv);

    EXPECT_FALSE(argParser.retrieve("counter"));
    EXPECT_FALSE(argParser.retrieve("c"));
}

TEST_F(ArgParserTest, DefaultValueTest)
{
    ArgParser argParser{};
    std::string defaultPath{"/home/"};
    argParser.addArgumentWithDefault(logFileShortOption, logFileLongOption, logFileHelpMessage, defaultPath);
    argParser.addArgumentWithDefault("-e", "--enable", logFileHelpMessage, true);
    argParser.addArgumentWithDefault("-d", "--disable", logFileHelpMessage, false);
    std::string disableStr{"-d=true"};
    char *argv[] = {&binaryPath[0], &disableStr[0]};
    argParser.parse(2, argv); // no value was passed

    EXPECT_EQ(argParser.retrieve("l"), defaultPath);
    EXPECT_EQ(argParser.retrieve("logFilePath"), defaultPath);
    EXPECT_TRUE(argParser.retrieve("enable"));
    EXPECT_TRUE(argParser.retrieve("disable"));
}

TEST_F(ArgParserTest, DefaultValueOverrideTest)
{
    ArgParser argParser{};
    std::string defaultPath{"/home/"};
    argParser.addArgumentWithDefault(logFileShortOption, logFileLongOption, logFileHelpMessage, defaultPath);
    char *argv[] = {&binaryPath[0], &logFilePathLongOption[0]};
    argParser.parse(2, argv);

    EXPECT_EQ(argParser.retrieve("l"), logFilePath);
    EXPECT_EQ(argParser.retrieve("logFilePath"), logFilePath);
}

TEST_F(ArgParserTest, IntRetrieveTest)
{
    ArgParser argParser{};
    argParser.addArgument("-c", "--counter", "to get the counter");
    argParser.addArgument("-w", "--waitTime", "to get the wait time");
    argParser.addArgument("-m", "--multiplier", "to get the multiplier");
    std::string counterLongOption{"--counter=10"};
    std::string waitTimeLongOption{"--waitTime=15abc"}; // bad number
    std::string multiplierLongOption{"--multiplier=-15"};
    char *argv[] = {&binaryPath[0], &counterLongOption[0], &waitTimeLongOption[0], &multiplierLongOption[0]};
    argParser.parse(4, argv);

    const auto c = argParser.retrieve<int>("c");
    EXPECT_TRUE(c);
    EXPECT_EQ(c, 10);

    const auto counter = argParser.retrieve<int>("counter");
    EXPECT_TRUE(counter);
    EXPECT_EQ(counter, 10);

    EXPECT_FALSE(argParser.retrieve<int>("w"));
    EXPECT_FALSE(argParser.retrieve<int>("waitTime"));

    const auto multiplier = argParser.retrieve<int>("m");
    EXPECT_TRUE(multiplier);
    EXPECT_EQ(multiplier, -15);
}

TEST_F(ArgParserTest, FloatRetrieveTest)
{
    ArgParser argParser{};
    argParser.addArgument("-c", "--counter", "to get the counter");
    argParser.addArgument("-w", "--waitTime", "to get the wait time");
    argParser.addArgument("-m", "--multiplier", "to get the multiplier");
    std::string counterLongOption{"--counter=10.4"};
    std::string waitTimeLongOption{"--waitTime=15.48abc"}; // bad number
    std::string multiplierLongOption{"--multiplier=-15.8"};
    char *argv[] = {&binaryPath[0], &counterLongOption[0], &waitTimeLongOption[0], &multiplierLongOption[0]};
    argParser.parse(4, argv);

    const auto c = argParser.retrieve<float>("c");
    EXPECT_TRUE(c);
    EXPECT_FLOAT_EQ(*c, 10.4);

    const auto counter = argParser.retrieve<float>("counter");
    EXPECT_TRUE(counter);
    EXPECT_FLOAT_EQ(*counter, 10.4);

    EXPECT_FALSE(argParser.retrieve<float>("w"));
    EXPECT_FALSE(argParser.retrieve<float>("waitTime"));

    const auto multiplier = argParser.retrieve<float>("m");
    EXPECT_TRUE(multiplier);
    EXPECT_FLOAT_EQ(*multiplier, -15.8);
}

TEST_F(ArgParserTest, BoolRetrieveTest)
{
    ArgParser argParser{};
    argParser.addArgument("-al", "--allowLogging", "to allow logging");
    argParser.addArgument("-aw", "--allowWaiting", "to allow waiting");
    argParser.addArgument("-ar", "--allowInterrupting", "to allow interrupting");
    argParser.addArgument("-ap", "--allowParsing", "to allow parsing");
    std::string loggingArg{"--allowLogging=true"};
    std::string waitingArg{"-aw=false"};
    std::string interruptingArg{"-ar=True"};
    std::string parsingArg{"-ap=falsed"}; // bad argument
    char *argv[] = {&binaryPath[0], &loggingArg[0], &waitingArg[0], &interruptingArg[0], &parsingArg[0]};
    argParser.parse(5, argv);

    auto al = argParser.retrieve<bool>("al");
    auto aw = argParser.retrieve<bool>("aw");
    auto ar = argParser.retrieve<bool>("ar");
    auto ap = argParser.retrieve<bool>("ap");

    EXPECT_TRUE(al && *al);
    EXPECT_TRUE(aw && !*aw);
    EXPECT_TRUE(!ar);
    EXPECT_FALSE(ap);
}

TEST_F(ArgParserTest, CharRetrieveTest)
{
    ArgParser argParser{};
    argParser.addArgument("-a", "--ans", "answer");
    argParser.addArgument("-f", "--full_ans", "full answer");

    std::string ansKeyValueArg{"-a=y"};
    std::string fullAnsKeyValueArg{"--full_ans=yes"};
    char *argv[] = {binaryPath.data(), ansKeyValueArg.data(), fullAnsKeyValueArg.data()};
    argParser.parse(3, argv);
    EXPECT_EQ(argParser.retrieve<char>("a"), 'y');
    EXPECT_EQ(argParser.retrieve<char>("ans"), 'y');

    EXPECT_EQ(argParser.retrieve("f"), "yes");
    EXPECT_EQ(argParser.retrieve("full_ans"), "yes");

    // but retrieving following should fail because "yes" can't just be converted to a char
    EXPECT_FALSE(argParser.retrieve<char>("f"));
    EXPECT_FALSE(argParser.retrieve<char>("full_ans"));
}

TEST_F(ArgParserTest, InvalidArgTest)
{
    ArgParser argParser{};
    argParser.addArgument("-al", "--allowLogging", "to allow logging");
    EXPECT_EXCEPTION(argParser.parse(0, nullptr), std::runtime_error, "Invalid command line arguments");
    char *argv[] = {&binaryPath[0]};
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
    EXPECT_EQ(argParser.helpMsg(), concatenate(topLine, "-h, --help\n\tdescription: to get this message\n"));

    argParser.addArgument("-l", "--logfile", "logfile path");
    EXPECT_EQ(argParser.helpMsg(), concatenate(topLine, "-h, --help\n\tdescription: to get this message\n"
                                               "-l, --logfile\n\tdescription: logfile path\n"));

    // now with a default value
    argParser.addArgumentWithDefault("-c", "--counter", "to get the counter", 15);
    EXPECT_EQ(argParser.helpMsg(), concatenate(topLine, "-h, --help\n\tdescription: to get this message\n"
                                               "-l, --logfile\n\tdescription: logfile path\n"
                                               "-c, --counter\n\tdescription: to get the counter, default: 15\n"));

    // now with mandatory argument
    argParser.addArgument("-a", "--enableAll", "to enable all", true);
    EXPECT_EQ(argParser.helpMsg(), concatenate(topLine, "-h, --help\n\tdescription: to get this message\n"
                                               "-l, --logfile\n\tdescription: logfile path\n"
                                               "-c, --counter\n\tdescription: to get the counter, default: 15\n"
                                               "-a, --enableAll\n\tdescription: to enable all, mandatory: true\n"));

    argParser.addArgument("-b", "--blockAll", "to block all", false);
    EXPECT_EQ(argParser.helpMsg(), concatenate(topLine, "-h, --help\n\tdescription: to get this message\n"
                                               "-l, --logfile\n\tdescription: logfile path\n"
                                               "-c, --counter\n\tdescription: to get the counter, default: 15\n"
                                               "-a, --enableAll\n\tdescription: to enable all, mandatory: true\n"
                                               "-b, --blockAll\n\tdescription: to block all\n"));
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
    char * argv[] = {&binaryPath[0], &shortHelpString[0]};
    argParser.parse(2, argv);
    EXPECT_TRUE(argParser.needHelp());
}

TEST_F(ArgParserTest, LongHelpStringTest)
{
    ArgParser argParser{};
    std::string LongHelpString{"--help"};
    char * argv[] = {&binaryPath[0], &LongHelpString[0]};
    argParser.parse(2, argv);
    EXPECT_TRUE(argParser.needHelp());
}

TEST_F(ArgParserTest, DoesntNeedHelpWhenNoArgTest)
{
    ArgParser argParser{};
    char * argv[] = {&binaryPath[0]};
    argParser.parse(1, argv);
    EXPECT_FALSE(argParser.needHelp());
}

TEST_F(ArgParserTest, DoesntNeedHelpWhenArgsTest)
{
    ArgParser argParser{};
    argParser.addArgument(logFileShortOption, logFileLongOption, logFileHelpMessage);
    char * argv[] = {&binaryPath[0], &logFilePathLongOption[0]};
    argParser.parse(2, argv);
    EXPECT_FALSE(argParser.needHelp());
}

TEST_F(ArgParserTest, AppNameWhenNoArgsTest)
{
    ArgParser argParser{};
    char * argv[] = {&binaryPath[0]};
    argParser.parse(1, argv);
    EXPECT_EQ(binaryPath, argParser.getAppPath());
}

TEST_F(ArgParserTest, AppNameWhenArgsTest)
{
    ArgParser argParser{};
    argParser.addArgument(logFileShortOption, logFileLongOption, logFileHelpMessage);
    char * argv[] = {&binaryPath[0], &logFilePathLongOption[0]};
    argParser.parse(2, argv);
    EXPECT_EQ(binaryPath, argParser.getAppPath());
}

TEST_F(ArgParserTest, DescriptionWhenNoArgsTest)
{
    ArgParser argParser{"An app for testing"};
    EXPECT_EQ("An app for testing", argParser.getDescription());

    char * argv[] = {&binaryPath[0]};
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

    argParser.addArgument(logFileShortOption, logFileLongOption, logFileHelpMessage);
    char * argv[] = {&binaryPath[0], &logFilePathLongOption[0]};
    argParser.parse(2, argv);
    EXPECT_EQ("An app for testing", argParser.getDescription()); // must be same even after parsing
    EXPECT_EQ(argParser.helpMsg(), "An app for testing\n"
                                   "Following is a list of configured arguments:\n"
                                   "-h, --help\n\tdescription: to get this message\n"
                                   "-l, --logFilePath\n\tdescription: Log file path\n");
}

TEST_F(ArgParserTest, ContainsTest)
{
    ArgParser argParser{};
    argParser.addArgument("-l", "--logFilePath", "to get log file path");
    argParser.addArgumentWithDefault("-c", "--counter", "to get the counter", 10);
    argParser.addArgument("-w", "--waitTime", "to get the wait time");
    std::string counterLongOption{"--counter=10"};

    char * argv[] = {&binaryPath[0], &logFilePathLongOption[0]};
    argParser.parse(2, argv);

    EXPECT_TRUE(argParser.contains("l"));
    EXPECT_TRUE(argParser.contains("logFilePath"));

    // because it has default value, it must return true
    EXPECT_TRUE(argParser.contains("c"));
    EXPECT_TRUE(argParser.contains("counter"));

    // although configured but it was not run with, hence, it must return false
    EXPECT_FALSE(argParser.contains("w"));
    EXPECT_FALSE(argParser.contains("waitTime"));

    // with '-' or '--' should return false
    EXPECT_FALSE(argParser.contains(logFileShortOption));
    EXPECT_FALSE(argParser.contains(logFileLongOption));

    // any other random string query should return false
    EXPECT_FALSE(argParser.contains(""));
    EXPECT_FALSE(argParser.contains(" "));
    EXPECT_FALSE(argParser.contains("  "));
    EXPECT_FALSE(argParser.contains("unexpected"));

    // 'h' or 'help' also should return false as there is no corresponding value
    EXPECT_FALSE(argParser.contains("h"));
    EXPECT_FALSE(argParser.contains("help"));
}

TEST_F(ArgParserTest, BadRetrivalTest)
{
    ArgParser argParser{};
    argParser.addArgument(logFileShortOption, logFileLongOption, logFileHelpMessage);
    EXPECT_EXCEPTION(argParser.retrieve("l"), std::runtime_error,
                     "parse() must be called with command line arguments before retrieving values");
    EXPECT_EXCEPTION(argParser.retrieve("logFilePath"), std::runtime_error,
                     "parse() must be called with command line arguments before retrieving values");
    EXPECT_EXCEPTION(argParser.retrieve(""), std::runtime_error,
                     "parse() must be called with command line arguments before retrieving values");
    EXPECT_EXCEPTION(argParser.retrieve("anyArgument"), std::runtime_error,
                     "parse() must be called with command line arguments before retrieving values");

    std::string helpArg{"-h"};
    char * argv[] = {&binaryPath[0], &helpArg[0]};
    argParser.parse(2, argv);
    EXPECT_EXCEPTION(argParser.retrieve("l"), std::runtime_error,
                     "Application was run with '-h' or '--help', retrieving values is not allowed. "
                     "Should call @helpMsg and return");

    // in fact, same behavior for any argument
    EXPECT_EXCEPTION(argParser.retrieve("anyArgument"), std::runtime_error,
                     "Application was run with '-h' or '--help', retrieving values is not allowed. "
                     "Should call @helpMsg and return");
    EXPECT_EXCEPTION(argParser.retrieve(""), std::runtime_error,
                     "Application was run with '-h' or '--help', retrieving values is not allowed. "
                     "Should call @helpMsg and return");
    EXPECT_EXCEPTION(argParser.retrieve(" "), std::runtime_error,
                     "Application was run with '-h' or '--help', retrieving values is not allowed. "
                     "Should call @helpMsg and return");
}

TEST_F(ArgParserTest, ArgParserChainingAndRetrieveMayThrowTest)
{
    auto argParser = ArgParser{}
            .addArgument(logFileShortOption, logFileLongOption, logFileHelpMessage)
            .addArgumentWithDefault("-c", "--counter", "to get the counter", 10)
            .addArgument("-w", "--waitTime", "to get the wait time");
    std::string counterLongOption{"--counter=10"};

    std::string waitOption{"-w=15.5"};
    char* argv[] = {&binaryPath[0], &logFilePathShortOption[0], &waitOption[0]};
    argParser.parse(3, argv);

    EXPECT_EQ(argParser.retrieveMayThrow("l"), logFilePath);
    EXPECT_EQ(argParser.retrieveMayThrow("logFilePath"), logFilePath);

    EXPECT_EQ(argParser.retrieveMayThrow<int>("c"), 10);
    EXPECT_EQ(argParser.retrieveMayThrow<int>("counter"), 10);

    EXPECT_FLOAT_EQ(argParser.retrieveMayThrow<float>("w"), 15.5);
    EXPECT_FLOAT_EQ(argParser.retrieveMayThrow<float>("waitTime"), 15.5);
}

TEST_F(ArgParserTest, ManadatoryArgumentTest)
{
    ArgParser argParser{};
    argParser.addArgument(logFileShortOption, logFileLongOption, logFileHelpMessage, true);
    char * argv[] = {&binaryPath[0]};

    EXPECT_EXCEPTION(argParser.parse(1, argv), std::runtime_error,
                     "Couldn't find [-l, --logFilePath] mandatory argument in passed arguments");
}

TEST_F(ArgParserTest, MultiManadatoryArgumentTest)
{
    ArgParser argParser{};
    argParser.addArgument(logFileShortOption, logFileLongOption, logFileHelpMessage, true);
    argParser.addArgument("-c", "--counter", "to get the counter", true);

    std::string counterArg{"-c=10"};
    char * argv[] = {&binaryPath[0], &logFilePathLongOption[0]};

    EXPECT_EXCEPTION(argParser.parse(2, argv), std::runtime_error,
                     "Couldn't find [-c, --counter] mandatory argument in passed arguments");
}

TEST_F(ArgParserTest, CombinedManadatoryArgumentTest)
{
    ArgParser argParser{};
    argParser.addArgument(logFileShortOption, logFileLongOption, logFileHelpMessage, true);
    argParser.addArgument("-c", "--counter", "to get the counter", true);
    argParser.addArgumentWithDefault("-w", "--waitTime", "to get the wait time", 10);
    argParser.addArgument("-n", "--number", "to get the number");
    argParser.addArgument("-a", "--enableAll", "to enable all");
    argParser.addArgument("-b", "--blockAll", "to block all", false);

    std::string counterArg{"-c=10"};
    std::string waitArg{"-w=12"};
    std::string numberArg{"-n=6"};

    char * argv[] = {&binaryPath[0], &logFilePathLongOption[0], &counterArg[0], &waitArg[0], &numberArg[0]};

    argParser.parse(5, argv);
    EXPECT_EQ(argParser.retrieveMayThrow("l"), logFilePath);
    EXPECT_EQ(argParser.retrieveMayThrow<int>("c"), 10);
    EXPECT_EQ(argParser.retrieveMayThrow<int>("w"), 12);
    EXPECT_EQ(argParser.retrieveMayThrow<int>("n"), 6);
    EXPECT_FALSE(argParser.contains("a"));
    EXPECT_FALSE(argParser.contains("b"));
}

TEST_F(ArgParserTest, BadArgTest)
{
    ArgParser argParser{};
    EXPECT_EXCEPTION(argParser.addArgument("-c", "-c", "help"), std::runtime_error, "Invalid long option: -c");
    EXPECT_EXCEPTION(argParser.addArgument("--count", "--count", "Counter"), std::runtime_error, "Invalid short option: --count");
    EXPECT_EXCEPTION(argParser.addArgument("-c", "--c", "Counter"), std::runtime_error, "Short option [-c] must be shorter than Long option [--c]");

    // what if one of them is empty
    EXPECT_EXCEPTION(argParser.addArgument("", "", "Counter"), std::runtime_error, "Invalid short option: ");
    EXPECT_EXCEPTION(argParser.addArgument("", "--c", "Counter"), std::runtime_error, "Invalid short option: ");
    EXPECT_EXCEPTION(argParser.addArgument("-c", "", "Counter"), std::runtime_error, "Invalid long option: ");

    // too many '-'
    EXPECT_EXCEPTION(argParser.addArgument("-c", "---c", "Counter"), std::runtime_error, "Invalid long option: ---c");
    EXPECT_EXCEPTION(argParser.addArgument("---c", "cc", "Counter"), std::runtime_error, "Invalid short option: ---c");
    EXPECT_EXCEPTION(argParser.addArgument("---c", "---cc", "Counter"), std::runtime_error, "Invalid short option: ---c");

    // swap short and long options
    EXPECT_EXCEPTION(argParser.addArgument("--counter", "-c", "Counter"), std::runtime_error, "Invalid short option: --counter");
    EXPECT_EXCEPTION(argParser.addArgument("-counter", "-c", "Counter"), std::runtime_error, "Invalid long option: -c");
    EXPECT_EXCEPTION(argParser.addArgument("-counter", "--c", "Counter"), std::runtime_error, "Short option [-counter] must be shorter than Long option [--c]");

    // bad arguments with placement of spaces randomly
    EXPECT_EXCEPTION(argParser.addArgument("-c ", "--counter", "Counter"), std::runtime_error, "Invalid short option: -c ");
    EXPECT_EXCEPTION(argParser.addArgument(" -c", "--counter", "Counter"), std::runtime_error, "Invalid short option:  -c");
    EXPECT_EXCEPTION(argParser.addArgument("- c", "--counter", "Counter"), std::runtime_error, "Invalid short option: - c");
    EXPECT_EXCEPTION(argParser.addArgument("-c", " --counter", "Counter"), std::runtime_error, "Invalid long option:  --counter");
    EXPECT_EXCEPTION(argParser.addArgument("-c", "- -counter", "Counter"), std::runtime_error, "Invalid long option: - -counter");
    EXPECT_EXCEPTION(argParser.addArgument("-c", "-- counter", "Counter"), std::runtime_error, "Invalid long option: -- counter");
    EXPECT_EXCEPTION(argParser.addArgument("-c", "--count er", "Counter"), std::runtime_error, "Invalid long option: --count er");

    // bad argument test with placement of '-' randomly
    EXPECT_EXCEPTION(argParser.addArgument("-c-", "--counter", "Counter"), std::runtime_error, "Invalid short option: -c-");
    EXPECT_EXCEPTION(argParser.addArgument("-c--", "--counter", "Counter"), std::runtime_error, "Invalid short option: -c--");
    EXPECT_EXCEPTION(argParser.addArgument("-c", "-counter", "Counter"), std::runtime_error, "Invalid long option: -counter");
    EXPECT_EXCEPTION(argParser.addArgument("-c", "--counter-", "Counter"), std::runtime_error, "Invalid long option: --counter-");
    EXPECT_EXCEPTION(argParser.addArgument("-c", "--count-er", "Counter"), std::runtime_error, "Invalid long option: --count-er");

    // empty argument test
    EXPECT_EXCEPTION(argParser.addArgument("", "--counter", "Counter"), std::runtime_error, "Invalid short option: ");
    EXPECT_EXCEPTION(argParser.addArgument(" ", "--counter", "Counter"), std::runtime_error, "Invalid short option:  ");
    EXPECT_EXCEPTION(argParser.addArgument("\t", "--counter", "Counter"), std::runtime_error, "Invalid short option: \t");
    EXPECT_EXCEPTION(argParser.addArgument("-c", "", "Counter"), std::runtime_error, "Invalid long option: ");
    EXPECT_EXCEPTION(argParser.addArgument("-c", " ", "Counter"), std::runtime_error, "Invalid long option:  ");
    EXPECT_EXCEPTION(argParser.addArgument("-c", "\t", "Counter"), std::runtime_error, "Invalid long option: \t");
}

TEST_F(ArgParserTest, ValidArgsTest)
{
    try
    {
        ArgParser argParser{};
        // if any of following function call fails, it will raise exception which will
        // take execution to catch block which will lead to the failure of this test case!
        argParser.addArgument("-l", "--long_option", "help");
        argParser.addArgument("-l1", "--long_option1", "help");
        argParser.addArgument("-l_1", "--long_option_1", "help");
        argParser.addArgument("-_", "--__", "help");
        argParser.addArgument("-___", "--____", "help");
        argParser.addArgument("-_1", "--__2", "help");
        argParser.addArgument("-1", "--2_", "help");
    }
    catch (...)
    {
        EXPECT_TRUE(false);  // because this should not be executed
    }
}