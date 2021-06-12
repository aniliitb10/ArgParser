#include <ArgParser.h>
#include <fmt/format.h>

int main(int argc, char** argv) {

    ArgParser argParser{};
    argParser.addArgument("-c", "--count", "to get the count");
    argParser.addArgument("-l", "--logfile", "log file path", "/home/");
    argParser.parse(argc, argv);
    fmt::print("App name: {}\n", argParser.getAppName());
    fmt::print("Count: {}\n", argParser.retrieve<int>("count"));
    fmt::print("log file path: {}\n", argParser.retrieve<>("l"));

    return 0;
}
