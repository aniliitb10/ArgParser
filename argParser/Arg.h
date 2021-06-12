#pragma once

#include <functional>
#include <utility>
#include <string>
#include <algorithm>
#include <cctype>
#include <ParsedArg.h>
#include <fmt/format.h>

class Arg
{
public:

    Arg(const std::string &shortArg, const std::string &longArg, std::string helpMsg) :
            helpMsg(std::move(helpMsg)),
            hasDefault(false)
    {
        init_args(shortArg, longArg);
    }

    Arg(const std::string &shortArg, const std::string &longArg, std::string helpMsg, std::string defaultValue) :
            helpMsg(std::move(helpMsg)),
            defaultValue(std::move(defaultValue)),
            hasDefault(true)
    {
        init_args(shortArg, longArg);
    }

    bool operator==(const Arg &rhs) const
    {
        return shortOpt == rhs.shortOpt && longOpt == rhs.longOpt;
    }

    bool operator!=(const Arg &rhs) const
    {
        return !(*this == rhs);
    }

    [[nodiscard]] const std::string &getShortOpt() const
    {
        return shortOpt;
    }

    [[nodiscard]] const std::string &getLongOpt() const
    {
        return longOpt;
    }

    [[nodiscard]] bool match(const ParsedArg &arg) const
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

    [[nodiscard]] bool match(const std::string &arg) const noexcept
    {
        return arg == shortOpt || arg == longOpt;
    }

    [[nodiscard]] std::string to_string() const
    {
        if (!shortOpt.empty() && !longOpt.empty())
        {
            return fmt::format("-{} --{}", shortOpt, longOpt);
        }

        if (!shortOpt.empty())
        {
            return fmt::format("-{}", shortOpt);
        }

        if (!longOpt.empty())
        {
            return fmt::format("--{}", longOpt);
        }

        throw std::runtime_error("Empty argument");
    }

    [[nodiscard]] bool hasDefaultValue() const noexcept
    {
        return hasDefault;
    }

    [[nodiscard]] const std::string& getDefaultValue() const noexcept
    {
        return defaultValue;
    }

private:

    void init_args(const std::string &shortArg, const std::string &longArg)
    {
        const auto shortParsedArg = ParsedArg::parse_arg(shortArg);
        const auto longParsedArg = ParsedArg::parse_arg(longArg);
        if (shortParsedArg.argType == longParsedArg.argType)
        {
            throw std::runtime_error{fmt::format("Invalid arguments: {} and {}", shortArg, longArg)};
        }
        set_args(shortParsedArg);
        set_args(longParsedArg);
    }

    void set_args(const ParsedArg &parsedArg)
    {
        switch (parsedArg.argType)
        {
            case ArgTypeEnum::Long:
                longOpt = parsedArg.parsedArg;
                break;
            case ArgTypeEnum::Short:
                shortOpt = parsedArg.parsedArg;
                break;
            case ArgTypeEnum::Invalid:
                throw std::runtime_error{fmt::format("Invalid arg: {}", parsedArg.parsedArg)};
        }
    }

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
