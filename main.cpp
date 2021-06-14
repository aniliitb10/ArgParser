#include <gtest/gtest.h>
//#include <argParser/ArgParser.h>

int main(int argc, char **argv)
{
    /*ArgParser parser{};
    parser.addArgument("-c", "--count", "counting message");
    parser.addArgument("-l", "--logfile", "logfile path", "/home/logs/log.txt");
    parser.parse(argc, argv);

    if (parser.needHelp())
    {
        std::cout << parser.helpMsg() << "\n";
        return 0;
    }*/

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}