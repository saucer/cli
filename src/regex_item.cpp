#include "regex_item.hpp"

namespace cli
{
    regex_item::~regex_item() = default;

    bool regex_item::accept(const std::string &input) const
    {
        return std::regex_match(input, m_regex);
    }

    std::size_t regex_item::get_param_count() const
    {
        return item::get_param_count() - 1;
    }
} // namespace cli