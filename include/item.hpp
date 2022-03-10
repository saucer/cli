#pragma once
#include <string>
#include <vector>
#include <functional>

namespace cli
{
    enum class error_t
    {
        insufficient_arguments,
        unknown_argument,
        bad_arguments,
        none
    };

    class item
    {
      private:
        std::string m_name;
        std::string m_desc;

      private:
        std::size_t m_param_count;
        std::vector<std::string> m_params;
        std::function<error_t(const std::vector<std::string> &)> m_callback;

      public:
        virtual ~item();
        template <typename Function> item(std::string name, std::string desc, Function &&callback, std::vector<std::string> params);

      public:
        std::string get_name() const;
        std::string get_desc() const;
        virtual std::size_t get_param_count() const;
        std::vector<std::string> get_params() const;

      public:
        error_t operator()(const std::vector<std::string> &) const;
    };
} // namespace cli

#include "item.inl"