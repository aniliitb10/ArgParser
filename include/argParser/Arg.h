#pragma once

#include <argParser/ParsedArg.h>

#include <fmt/format.h>
#include <functional>
#include <utility>
#include <string>
#include <algorithm>
#include <cctype>

class Arg
{
public:

    Arg(const std::string &shortArg, const std::string &longArg, std::string helpMsg);
    Arg(const std::string &shortArg, const std::string &longArg, std::string helpMsg, std::string defaultValue);

    bool operator==(const Arg &rhs) const;
    bool operator!=(const Arg &rhs) const;

    [[nodiscard]] const std::string &getShortOpt() const;
    [[nodiscard]] const std::string &getLongOpt() const;

    [[nodiscard]] bool match(const ParsedArg &arg) const;
    [[nodiscard]] bool match(const std::string &arg) const noexcept;

    [[nodiscard]] bool hasDefaultValue() const noexcept;
    [[nodiscard]] const std::string& getDefaultValue() const noexcept;

    [[nodiscard]] std::string toString() const;
    [[nodiscard]] std::string toVerboseString() const;

private:
    void init_args(const std::string &shortArg, const std::string &longArg);

    std::string shortOpt{};
    std::string longOpt{};
    std::string helpMsg{};
    std::string defaultValue{};
    bool hasDefault{false};
};

namespace std
{
    template<>
    struct hash<Arg>
    {
        std::size_t operator()(const Arg &key) const noexcept
        {
            return std::hash<std::string>{}(key.getShortOpt() + key.getLongOpt());
        }
    };
}

inline
Arg::Arg(const std::string &shortArg, const std::string &longArg, std::string helpMsg) :
        helpMsg(std::move(helpMsg)),
        hasDefault(false)
{
    init_args(shortArg, longArg);
}

inline
Arg::Arg(const std::string &shortArg, const std::string &longArg, std::string helpMsg, std::string defaultValue) :
        helpMsg(std::move(helpMsg)),
        defaultValue(std::move(defaultValue)),
        hasDefault(true)
{
    init_args(shortArg, longArg);
}

inline
bool Arg::operator==(const Arg &rhs) const
{
    return shortOpt == rhs.shortOpt && longOpt == rhs.longOpt;
}

inline
bool Arg::operator!=(const Arg &rhs) const
{
    return !(*this == rhs);
}

inline
const std::string& Arg::getShortOpt() const
{
    return shortOpt;
}

inline
const std::string& Arg::getLongOpt() const
{
    return longOpt;
}

inline
bool Arg::match(const ParsedArg &arg) const
{
    switch (arg.argType)
    {
        case ArgTypeEnum::Short:
            return arg.parsedArg == this->shortOpt;

        case ArgTypeEnum::Long:
            return arg.parsedArg == this->longOpt;

        case ArgTypeEnum::Invalid:
            throw std::runtime_error{fmt::format("Attempting to match with Invalid argument")};
    }
    return false; // just to make compiler happy
}

inline
bool Arg::match(const std::string &arg) const noexcept
{
    return arg == shortOpt || arg == longOpt;
}

inline
std::string Arg::toString() const
{
    return fmt::format("-{}, --{}", shortOpt, longOpt);
}

inline
bool Arg::hasDefaultValue() const noexcept
{
    return hasDefault;
}

inline
const std::string& Arg::getDefaultValue() const noexcept
{
    return defaultValue;
}

inline
void Arg::init_args(const std::string &shortArg, const std::string &longArg)
{
    const auto shortParsedArg = ParsedArg::parse_arg(shortArg);
    const auto longParsedArg = ParsedArg::parse_arg(longArg);

    if (shortParsedArg.argType != ArgTypeEnum::Short)
    {
        throw std::runtime_error{fmt::format("Invalid short option: {}", shortArg)};
    }

    if (longParsedArg.argType != ArgTypeEnum::Long)
    {
        throw std::runtime_error{fmt::format("Invalid long option: {}", longArg)};
    }

    // both can't be same parsed arguments
    if (shortParsedArg.parsedArg.size() >= longParsedArg.parsedArg.size())
    {
        throw std::runtime_error{fmt::format("Short option [{}] must be shorter than Long option [{}]", shortArg, longArg)};
    }
    shortOpt = shortParsedArg.parsedArg;
    longOpt = longParsedArg.parsedArg;
}

inline
std::string Arg::toVerboseString() const
{
    if (hasDefaultValue())
    {
        return fmt::format("{}\n\tdescription: {}, default: {}", toString(), helpMsg, getDefaultValue());
    }
    return fmt::format("{}\n\tdescription: {}", toString(), helpMsg);
}