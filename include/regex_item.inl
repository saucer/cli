#pragma once
#include "regex_item.hpp"

namespace cli
{
    template <typename Function>
    regex_item::regex_item(std::string name, std::regex regex, std::string desc, Function &&callback, std::vector<std::string> params)
        : item(std::move(name), std::move(desc), std::forward<Function>(callback), std::move(params)), m_regex(std::move(regex))
    {
    }
} // namespace cli