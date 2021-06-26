#pragma once

#include <string>
#include <argParser/Arg.h>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <exception>
#include <fmt/format.h>
#include <sstream>

/*
 * This is the main class to be included for argument parsing
 * Sample use case:
 *      ArgParser argParser{" a command line app"}; // description which also becomes part of help message
 *      argParser.add("-l", "--logFile", "log file path");
 *      argParser.add("-w", "--waitTime", "Wait time for application");
 *      argParser.parse(argc, argv);
 *      if (argParser.needHelp())
 *      {
 *          log(argParser.helpMsg()); // in this case, @parse() didn't parse anything as there was nothing to parse
 *          return 0;
 *      }
 *
 * Retrieving values:
 *      std::string logFile = argParser.retrieve("l").first; // or argParser.retrieve("logFile").first
 *      const auto valuePair = argParser.retrieve<int>(w); // or argParser.retrieve<int>("waitTime")
 *      assert(valuePair.second) // assert that conversion succeeded
 *      const auto waitTime = valuePair.first;
 *
 *      @Note that @retrieve returns std::pair<T, bool> where boolean represents
 *      if the status of conversion from std::string to T was successful
 *      So, the status MUST be checked before using the retrieved value
 *
 *      If retrieval fails then there is no point of using the retrieved value because it will
 *      definitely be different from the expected value
 *      If retrieval fails then check the documentation of std::istringstream::operator>> to check
 *      the content of pair.first
 *
 *      @retrieve has been specialized for: std::string and bool
 *      std::string -> no need to check the pair.second because conversion status will always be true
 *      bool -> if source string is (case insensitive) True/False, then conversion succeeds to corresponding values
 *              otherwise conversion fails and pair.second is set to false
 *
 * Help message:
 *      If application is run with -h or --help flag, then @needHelp returns true and
 *      user MUST call @helpMsg and then return because, in this case when parse() was called, no parsing was done
 *
 * */
class ArgParser
{
public:
    ArgParser();

    explicit ArgParser(std::string description);

    // to add an argument
    // e.g. argParser.addArgument("-w", "--waitTime", "Wait time");
    ArgParser &addArgument(const std::string &shortOpt, const std::string &longOpt, const std::string &helpMsg);

    // to add an argument, with default value
    // 1) argParser.addArgument("-w", "--waitTime", "Wait time", 10);
    // 2) argParser.addArgument("-l", "--logfile", "Log file", "/var/tmp/app.log");
    template<typename T = std::string>
    ArgParser &addArgument(const std::string &shortOpt, const std::string &longOpt, const std::string &helpMsg,
                           const T &defaultValue);

    // To parse the command line arguments
    // Should be called with the arguments of main function
    void parse(int argc, char *argv[]);

    // As the first command line argument is always application path, it returns the same
    std::string getAppPath() const
    { return appPath; };

    // To retrieve value corresponding to @arg
    // Check 2nd argument to ensure that conversion to type T succeeded
    // if the arg was not configured/passed, it will throw exception
    // all values of arguments are stored as std::string, hence conversion to std::string always succeeds
    template<typename T = std::string>
    std::pair<T, bool> retrieve(const std::string &arg);

    // throws if conversion to type T fails
    template<typename T>
    T retrieveMayThrow(const std::string &arg);

    // To get the help message for this application with configured arguments
    std::string helpMsg() const noexcept;

    // Returns true if application was called with -h or --help
    bool needHelp() const noexcept;

    // To get the description of application, the argument of the parameterized constructor
    const std::string &getDescription() const noexcept;

    // Returns true if the application was run with @arg as one of command line arguments
    bool contains(const std::string &arg) const noexcept;

private:
    // preprocess to ensure that help works as expected
    void init() noexcept;

    // ensures that parse() was called
    void validate_retrieval() const;

    static std::pair<ParsedArg, std::string> argValueParser(const std::string &arg);

    const Arg& findArg(const ParsedArg &arg);

    static bool isHelpString(const std::string &arg);

    ArgParser &addArgumentImpl(Arg &&arg);

    template<typename T>
    static std::pair<T, bool> convert(const std::string &arg);

    // to describe the app
    std::string description;

    // this keeps the configured arguments
    std::vector<Arg> configuredArgs{};

    // this is used to ensure that any combination of configured arguments don't match
    std::unordered_set<std::string> argKeys{}; // pre-populated with default values

    // this is used to keep the configured argument against their passed values in command line arguments
    std::unordered_map<Arg, std::string> parsedArgs{};
    std::string appPath{};
    bool isOnlyHelpString{false};
};

inline
ArgParser &ArgParser::addArgument(const std::string &shortOpt, const std::string &longOpt, const std::string &helpMsg)
{
    return addArgumentImpl(Arg{shortOpt, longOpt, helpMsg});
}

template<typename T>
ArgParser &ArgParser::addArgument(const std::string &shortOpt, const std::string &longOpt, const std::string &helpMsg,
                                  const T &defaultValue)
{
    return addArgumentImpl(Arg{shortOpt, longOpt, helpMsg, fmt::to_string(defaultValue)});
}

inline
ArgParser &ArgParser::addArgumentImpl(Arg &&arg)
{
    if (!argKeys.emplace(arg.getShortOpt()).second || !argKeys.emplace(arg.getLongOpt()).second)
    {
        throw std::runtime_error{fmt::format("Duplicate arguments: {}", arg.toString())};
    }
    configuredArgs.emplace_back(std::move(arg));
    return *this;
}

inline
void ArgParser::parse(int argc, char *argv[])
{
    if (argc < 1 || argv == nullptr)
    {
        throw std::runtime_error{"Invalid command line arguments"};
    }

    appPath = argv[0];
    if (argc == 2 && isHelpString(argv[1]))
    {
        isOnlyHelpString = true;
        return;
    }

    for (int i = 1; i < argc; ++i)
    {
        const auto argValPair = argValueParser(argv[i]);
        const auto arg = findArg(argValPair.first);
        if (!parsedArgs.emplace(arg, argValPair.second).second)
        {
            throw std::runtime_error{fmt::format("Received multiple values for {}", arg.toString())};
        }
    }
    // now add those arguments which have default values
    for (const auto &arg: configuredArgs)
    {
        if (arg.hasDefaultValue())
        {
            parsedArgs.emplace(arg, arg.getDefaultValue()); // it won't override if arg already exists
        }
    }
}

inline
std::pair<ParsedArg, std::string> ArgParser::argValueParser(const std::string &arg)
{
    const static char SEP{'='};
    auto pos = arg.find(SEP);
    if (pos == 0 || pos == std::string::npos)
    {
        throw std::runtime_error(fmt::format("Separator [{}] is supposed to separate arg and value in: {}",
                                             SEP, arg));
    }

    const auto parsedArg{ParsedArg::parse_arg(arg.substr(0, pos))};
    return std::make_pair(parsedArg, arg.substr(pos + 1));
}

inline
const Arg &ArgParser::findArg(const ParsedArg &arg)
{
    auto argItr = std::find_if(configuredArgs.cbegin(), configuredArgs.cend(),
                               [&arg](const Arg &eachArg)
                               { return eachArg.match(arg); });
    if (argItr == configuredArgs.cend())
    {
        throw std::runtime_error(fmt::format("{} is not a known argument", arg.parsedArg));
    }
    return *argItr;
}

template<typename T>
std::pair<T, bool> ArgParser::retrieve(const std::string &arg)
{
    validate_retrieval();
    const auto itr = std::find_if(parsedArgs.cbegin(), parsedArgs.cend(),
                                  [&arg](const std::pair<Arg, std::string> &argValuePair)
                                  { return argValuePair.first.match(arg); });
    if (itr == parsedArgs.cend())
    {
        throw std::runtime_error(fmt::format("Couldn't find [{}] in arguments", arg));
    }
    return convert<T>(itr->second);
}

template<typename T>
inline
std::pair<T, bool> ArgParser::convert(const std::string &arg)
{
    // I am aware that std::istringstream is slow
    // but it is run for only few option suring startup and it does the job really well
    // so, it is a good choice for C++14 (of C++17 has std::from_chars)
    std::istringstream iss{arg};
    T result{};

    iss >> result;
    // it is important to check that iss has not failed and eof has been reached
    // e.g. in "123abc" to int, iss won't fail but eof won't reach which is an error!
    return std::pair<T, bool>(result, !iss.fail() && iss.eof());
}

inline
std::string ArgParser::helpMsg() const noexcept
{
    std::ostringstream os{};

    if (!getDescription().empty())
    {
        os << getDescription() << "\n";
    }

    os << "Following is a list of configured arguments:\n";

    for (const auto &arg : configuredArgs)
    {
        os << arg.toVerboseString() << "\n";
    }
    return os.str();
}

inline
bool ArgParser::isHelpString(const std::string &arg)
{
    static const std::string ShortHelpString{"-h"};
    static const std::string LongHelpString{"--help"};

    return arg == ShortHelpString || arg == LongHelpString;
}

inline
bool ArgParser::needHelp() const noexcept
{
    return isOnlyHelpString;
}

inline
ArgParser::ArgParser(std::string description) : description(std::move(description))
{
    init();
}

const std::string &ArgParser::getDescription() const noexcept
{
    return description;
}

template<typename T>
T ArgParser::retrieveMayThrow(const std::string &arg)
{
    const auto retrieved_pair = retrieve(arg);
    if (!retrieved_pair.second)
    {
        throw std::runtime_error{fmt::format("Type conversion failed for {}", arg)};
    }
    return retrieved_pair.first;
}

bool ArgParser::contains(const std::string &arg) const noexcept
{
    // although iterating a map is not ideal but not command line arguments are always only few
    return std::find_if(parsedArgs.cbegin(), parsedArgs.cend(), [&arg](const std::pair<Arg, std::string> &parsedArg)
    {
        return parsedArg.first.match(arg);
    }) != parsedArgs.cend();
}

void ArgParser::init() noexcept
{
    // the default help argument
    Arg helpArg{"-h", "--help", "to get this message"};
    argKeys.emplace(helpArg.getShortOpt());
    argKeys.emplace(helpArg.getLongOpt());
    configuredArgs.emplace_back(std::move(helpArg));
}

ArgParser::ArgParser()
{
    init();
}

void ArgParser::validate_retrieval() const
{
    if (appPath.empty())
    {
        // this means parse() was never called
        throw std::runtime_error{"parse() must be called with command line arguments before retrieving values"};
    }

    if (needHelp())
    {
        throw std::runtime_error{"Application was run with '-h' or '--help', retrieving values is not allowed. "
                                 "Should call @helpMsg and return"};
    }
}

template<>
inline
std::pair<std::string, bool> ArgParser::convert(const std::string &arg)
{
    return std::make_pair(arg, true);
}

template<>
inline
std::pair<bool, bool> ArgParser::convert(const std::string &arg)
{
    static const std::string TRUE{"true"};
    static const std::string FALSE{"false"};
    std::string argCopy{arg};
    std::transform(argCopy.begin(), argCopy.end(), argCopy.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });

    if (argCopy == TRUE)
    {
        return std::make_pair(true, true);
    }

    return std::make_pair(false, argCopy == FALSE);
}