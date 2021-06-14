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

    ArgParser& addArgument(const std::string& shortOpt, const std::string& longOpt, const std::string &helpMsg);

    template<typename T = std::string>
    ArgParser& addArgument(const std::string& shortOpt, const std::string& longOpt, const std::string &helpMsg,
                            const T& defaultValue);

    void parse(int argc, char * argv[]);
    std::string getAppName() const { return appName; };

    template <typename T = std::string>
    T retrieve(const std::string& arg);

    std::string show_args() const noexcept;

private:
    static std::pair<ParsedArg, std::string> argValueParser(const std::string& arg);
    const Arg& find_arg(const ParsedArg &arg);

    template <typename T>
    static T convert(const std::string& arg);

    std::unordered_set<Arg> allArgs{};
    std::unordered_map<Arg, std::string> parsedArgs{};
    std::string appName{};
};


ArgParser& ArgParser::addArgument(const std::string &shortOpt, const std::string &longOpt, const std::string &helpMsg)
{
    if(!allArgs.emplace(shortOpt, longOpt, helpMsg).second)
    {
        throw std::runtime_error{fmt::format("Duplicate arguments: {}, {}", shortOpt, longOpt)};
    }
    return *this;
}

template<typename T>
ArgParser& ArgParser::addArgument(const std::string &shortOpt, const std::string &longOpt, const std::string &helpMsg,
                                   const T& defaultValue)
{
    if(!allArgs.emplace(shortOpt, longOpt, helpMsg, fmt::to_string(defaultValue)).second)
    {
        throw std::runtime_error{fmt::format("Duplicate arguments: {}, {}", shortOpt, longOpt)};
    }
    return *this;
}

void ArgParser::parse(int argc, char * argv[])
{
    appName = argv[0];
    for (int i = 1; i < argc; ++i)
    {
        const auto argValPair = argValueParser(argv[i]);
        const auto arg = find_arg(argValPair.first);
        if (!parsedArgs.emplace(arg, argValPair.second).second)
        {
            throw std::runtime_error{fmt::format("Received multiple values for {}", arg.toString())};
        }
    }

    // now add those arguments which have default values
    for (const auto& arg: allArgs)
    {
        if (arg.hasDefaultValue())
        {
            parsedArgs.emplace(arg, arg.getDefaultValue()); // it won't override if arg already exists
        }
    }
}

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

const Arg& ArgParser::find_arg(const ParsedArg &arg)
{
    auto argItr = std::find_if(allArgs.cbegin(), allArgs.cend(),
                               [&arg](const Arg& eachArg){ return eachArg.match(arg);});
    if (argItr == allArgs.cend())
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
T ArgParser::convert(const std::string &arg)
{
    T result{};
    std::from_chars<T>(arg.data(), arg.data() + arg.size(), result);
    return result;
}

std::string ArgParser::show_args() const noexcept
{
    std::ostringstream os{};
    for (const auto& arg : allArgs)
    {
        os << arg.toVerboseString() << "\n";
    }
    return os.str();
}

template<>
std::string ArgParser::convert(const std::string &arg)
{
    return arg;
}