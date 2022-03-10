#pragma once
#include <string>
#include <cstdint>
#include <string_view>

namespace ansi
{
    template <std::size_t ID> struct color
    {
        static inline const auto m_val = "\x1B[38;5;" + std::to_string(ID) + "m";
        friend std::ostream &operator<<(std::ostream &stream, const color<ID> &color)
        {
            stream << color.m_val;
            return stream;
        }
    };

    static const auto error = color<1>();
    static const auto success = color<2>();
    static const auto grey = color<242>();
    static const auto primary = color<63>();
    static const auto dark_grey = color<240>();

    static constexpr auto bold = "\x1B[1m";
    static constexpr auto reset = "\x1B[0m";
    static constexpr auto italic = "\x1B[3m";

    static const auto error_indicator = dark_grey.m_val + "[" + error.m_val + "X" + dark_grey.m_val + "] " + reset;
    static const auto success_indicator = dark_grey.m_val + "[" + success.m_val + ">" + dark_grey.m_val + "] " + reset;
} // namespace ansi