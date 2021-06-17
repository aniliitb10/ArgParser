#pragma once

#include <algorithm>
#include <string>

enum class ArgTypeEnum
{
    Short,
    Long,
    Invalid
};

struct ParsedArg
{
    ArgTypeEnum argType;
    std::string parsedArg;

    static ParsedArg parse_arg(const std::string& arg);
};

inline
ParsedArg ParsedArg::parse_arg(const std::string& arg)
{
    const auto pos = arg.find_first_not_of('-');
    if (pos == 0 || pos == std::string::npos)
    {
        return ParsedArg{ArgTypeEnum::Invalid, arg};
    }

    if (!std::all_of(std::next(arg.cbegin(), pos) , arg.cend(), [](unsigned char c) { return std::isalnum(c); }))
    {
        return ParsedArg{ArgTypeEnum::Invalid, arg};
    }

    if (pos == 1)
    {
        return ParsedArg{ArgTypeEnum::Short, arg.substr(pos)};
    }

    if (pos == 2)
    {
        return ParsedArg{ArgTypeEnum::Long, arg.substr(pos)};
    }

    return ParsedArg{ArgTypeEnum::Invalid, arg};
}
