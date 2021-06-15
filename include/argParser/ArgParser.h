#pragma once

#include <string>
#include <argParser/Arg.h>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <exception>
#include <fmt/format.h>
#include <charconv>
#include <sstream>

class ArgParser
{
public:
    ArgParser() = default;
    explicit ArgParser(std::string description);

    ArgParser& addArgument(const std::string& shortOpt, const std::string& longOpt, const std::string &helpMsg);

    template<typename T = std::string>
    ArgParser& addArgument(const std::string& shortOpt, const std::string& longOpt, const std::string &helpMsg,
                            const T& defaultValue);

    void parse(int argc, char * argv[]);
    std::string getAppName() const { return appName; };

    template <typename T = std::string>
    T retrieve(const std::string& arg);

    std::string helpMsg() const noexcept;
    bool needHelp() const noexcept;
    const std::string& getDescription() const noexcept;
private:

    static std::pair<ParsedArg, std::string> argValueParser(const std::string& arg);
    const Arg& findArg(const ParsedArg &arg);
    static bool isHelpString(const std::string& arg) ;
    ArgParser& addArgumentImpl(Arg&& arg);

    template <typename T>
    static T convert(const std::string& arg);

    // to describe the app
    std::string description;

private:

    // this keeps the configured arguments
    // Adding -h, --help as this is the default arguments for help
    std::vector<Arg> configuredArgs{Arg{"-h", "--help", "to get this message"}}; // pre-populated with default values

    // this is used to ensure that any combination of configured arguments don't match
    std::unordered_set<std::string> argKeys{"h", "help"}; // pre-populated with default values

    // this is used to keep the configured argument against their passed values in command line arguments
    std::unordered_map<Arg, std::string> parsedArgs{};
    std::string appName{};
    bool isOnlyHelpString{false};
};

inline
ArgParser& ArgParser::addArgument(const std::string &shortOpt, const std::string &longOpt, const std::string &helpMsg)
{
    return addArgumentImpl(Arg{shortOpt, longOpt, helpMsg});
}

template<typename T>
ArgParser& ArgParser::addArgument(const std::string &shortOpt, const std::string &longOpt, const std::string &helpMsg,
                                  const T& defaultValue)
{
    return addArgumentImpl(Arg{shortOpt, longOpt, helpMsg, fmt::to_string(defaultValue)});
}

inline
ArgParser& ArgParser::addArgumentImpl(Arg&& arg)
{
    if(!argKeys.emplace(arg.getShortOpt()).second || !argKeys.emplace(arg.getLongOpt()).second)
    {
        throw std::runtime_error{fmt::format("Duplicate arguments: {}", arg.toString())};
    }
    configuredArgs.emplace_back(std::move(arg));
    return *this;
}

inline
void ArgParser::parse(int argc, char * argv[])
{
    if (argc < 1 || argv == nullptr)
    {
        throw std::runtime_error{"Invalid command line arguments"};
    }

    appName = argv[0];
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
    for (const auto& arg: configuredArgs)
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

    const auto parsedArg {ParsedArg::parse_arg(arg.substr(0, pos))};
    return std::make_pair(parsedArg, arg.substr(pos+1));
}

inline
const Arg& ArgParser::findArg(const ParsedArg &arg)
{
    auto argItr = std::find_if(configuredArgs.cbegin(), configuredArgs.cend(),
                               [&arg](const Arg& eachArg){ return eachArg.match(arg);});
    if (argItr == configuredArgs.cend())
    {
        throw std::runtime_error(fmt::format("{} is not a known argument", arg.parsedArg));
    }
    return *argItr;
}

template<typename T>
T ArgParser::retrieve(const std::string &arg)
{
    const auto itr = std::find_if(parsedArgs.cbegin(), parsedArgs.cend(),
                                  [&arg](const std::pair<Arg, std::string>& argValuePair)
                                  { return argValuePair.first.match(arg);});
    if (itr == parsedArgs.cend())
    {
        throw std::runtime_error(fmt::format("Couldn't find [{}] in arguments", arg));
    }
    return convert<T>(itr->second);
}

template<typename T>
inline
T ArgParser::convert(const std::string &arg)
{
    T result{};
    std::from_chars<T>(arg.data(), arg.data() + arg.size(), result);
    return result;
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

    for (const auto& arg : configuredArgs)
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
{}

const std::string &ArgParser::getDescription() const noexcept
{
    return description;
}

template<>
inline
std::string ArgParser::convert(const std::string &arg)
{
    return arg;
}

template<>
inline
bool ArgParser::convert(const std::string &arg)
{
    static const std::string TRUE{"true"};
    std::string argCopy{arg};
    std::transform(argCopy.begin(), argCopy.end(), argCopy.begin(),
                   [](unsigned char c) { return std::tolower(c);});
    return argCopy == TRUE;
}