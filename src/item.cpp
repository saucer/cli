#include "item.hpp"

namespace cli
{
    item::~item() = default;

    std::string item::get_name() const
    {
        return m_name;
    }

    std::string item::get_desc() const
    {
        return m_desc;
    }

    std::size_t item::get_param_count() const
    {
        return m_param_count;
    }

    std::vector<std::string> item::get_params() const
    {
        return m_params;
    }

    error_t item::operator()(const std::vector<std::string> &input) const
    {
        return m_callback(input);
    }
} // namespace cli