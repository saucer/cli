#pragma once
#include "item.hpp"
#include <tuple>

namespace cli
{
    namespace detail
    {
        template <typename T> struct remove_const_ref
        {
            using type = T;
        };
        template <typename T> struct remove_const_ref<const T &>
        {
            using type = T;
        };
        template <typename T> using remove_const_ref_t = typename remove_const_ref<T>::type;

        template <typename T> struct function_traits
        {
            using return_t = void;
            using args_t = std::tuple<>;
            static constexpr std::size_t args_c = 0;
        };
        template <typename Return, typename... Arguments> struct function_traits<std::function<Return(Arguments...)>>
        {
            using return_t = Return;
            static constexpr std::size_t args_c = sizeof...(Arguments);
            using args_t = std::tuple<remove_const_ref_t<Arguments>...>;
        };

        template <typename DesiredType> auto parse(const std::string &input)
        {
            if constexpr (std::is_same_v<DesiredType, bool>)
            {
                return input == "true" || input == "on" || input == "1";
            }
            else if constexpr (std::is_integral_v<DesiredType>)
            {
                return std::stoll(input);
            }
            else if constexpr (std::is_floating_point_v<DesiredType>)
            {
                return static_cast<DesiredType>(std::stod(input));
            }
            else
            {
                return input;
            }
        }

        template <typename Tuple, std::size_t I = 0> constexpr void to_tuple(Tuple &tuple, const std::vector<std::string> &params)
        {
            if constexpr (I < std::tuple_size_v<Tuple>)
            {
                std::get<I>(tuple) = parse<std::tuple_element_t<I, Tuple>>(params.at(I));
                to_tuple<Tuple, I + 1>(tuple, params);
            }
        }
    } // namespace detail

    template <typename Function>
    item::item(std::string name, std::string desc, Function &&callback, std::vector<std::string> params)
        : m_name(std::move(name)), m_desc(std::move(desc)), m_params(std::move(params))
    {
        using func_t = decltype(std::function(callback));
        using traits = detail::function_traits<func_t>;
        static_assert(std::is_void_v<typename traits::return_t> || std::is_same_v<typename traits::return_t, cli::error_t>, "Return type should be void or error_t");

        // NOLINTNEXTLINE
        m_param_count = traits::args_c;
        // NOLINTNEXTLINE
        m_callback = [callback](const std::vector<std::string> &params) {
            if (params.size() != traits::args_c)
            {
                return error_t::insufficient_arguments;
            }
            try
            {
                typename traits::args_t args{};
                detail::to_tuple(args, params);
                if constexpr (std::is_same_v<typename traits::return_t, bool>)
                {
                    return std::apply([callback](auto &&...args) { callback(args...); }, args);
                }
                else
                {
                    std::apply([callback](auto &&...args) { callback(args...); }, args);
                }
            }
            catch (...)
            {
                return error_t::bad_arguments;
            }
            return error_t::none;
        };
    }
} // namespace cli