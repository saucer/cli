#pragma once
#include <regex>
#include "item.hpp"

namespace cli
{
    class regex_item : public item
    {
      private:
        std::regex m_regex;

      public:
        ~regex_item() override;
        template <typename Function> regex_item(std::string name, std::regex regex, std::string desc, Function &&callback, std::vector<std::string> params);

      public:
        bool accept(const std::string &) const;
        std::size_t get_param_count() const override;
    };
} // namespace cli

#include "regex_item.inl"