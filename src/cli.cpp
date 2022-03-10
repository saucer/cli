#include "cli.hpp"
#include "info.hpp"
#include "ansi.hpp"
#include "regex_item.hpp"

#include <ostream>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace cli
{
    session::session(std::unique_ptr<menu> root) : m_root(std::move(root))
    {
        m_root->set_parent(this);
        m_current = m_root.get();

#ifdef _WIN32
        DWORD lMode = 0;
        HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleMode(hStdout, &lMode);
        SetConsoleMode(hStdout, lMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);
#endif
    }

    void session::set_menu(menu *menu)
    {
        m_menu_history.emplace_back(m_current);
        m_current = menu;
    }

    void session::print_usage(char **argv, item *current, const std::string &current_arg, error_t type)
    {
        using namespace ansi; // NOLINT

        switch (type)
        {
        case error_t::bad_arguments:
            std::cerr << error_indicator << "Bad arguments for \"" << current->get_name() << "\"" << std::endl;
            break;
        case error_t::unknown_argument:
            std::cerr << error_indicator << "Unknown argument \"" << current_arg << "\"" << std::endl;
            break;
        case error_t::insufficient_arguments:
            std::cerr << error_indicator << "Insufficient arguments for \"" << current->get_name() << "\"" << std::endl;
            break;
        case error_t::none:
            break;
        }

        std::cout << std::endl;
        std::cout << primary << bold << "VERSION" << std::endl;
        std::cout << reset << "  saucer-cli/" << VERSION << " " << ARCH << " " << COMPILER << std::endl;

        std::cout << std::endl;
        std::cout << primary << bold << "USAGE" << std::endl;

        std::cout << reset << "  $ " << argv[0] << " ";
        if (current)
        {
            std::cout << current->get_name() << " ";
            for (const auto &param : current->get_params())
            {
                std::cout << "<" << param << "> ";
            }
            std::cout << std::endl;
        }
        else
        {
            std::cout << (m_current && m_current != m_root.get() ? m_current->get_name() + " " : "") << "[COMMAND] <OPTIONS...>" << std::endl << std::endl;
            std::cout << primary << bold << (current ? "OPTIONS" : "COMMANDS") << reset << std::endl;

            const auto &items = m_current->m_items;
            std::size_t longest{0};

            for (const auto &item : items)
            {
                std::size_t param_size{0};
                for (const auto &param : item->get_params())
                {
                    param_size += param.size();
                }
                longest = std::max(longest, item->get_name().size() + param_size + 5);
            }

            for (const auto &item : items)
            {
                std::cout << "  " << item->get_name() << " ";
                std::size_t spaces{item->get_name().size() + 1};
                for (const auto &param : item->get_params())
                {
                    spaces += param.size() + 3;
                    std::cout << "<" << param << "> ";
                }
                std::cout << std::string(longest - spaces, ' ') << italic << grey << item->get_desc() << reset << std::endl;
            }
        }
    }

    int session::start(int argc, char **argv)
    {
        if (argc > 1)
        {
            for (auto i = 1; argc > i; i++)
            {
                item *current{};
                std::string current_arg(argv[i]);

                if (current_arg == "--" && !m_menu_history.empty())
                {
                    set_menu(m_menu_history.back());
                    m_menu_history.pop_back();
                    continue;
                }

                if (current_arg == "help")
                {
                    print_usage(argv, current, current_arg, error_t::none);
                    continue;
                }

                for (const auto &item : m_current->m_items)
                {
                    if (auto *re_item = dynamic_cast<regex_item *>(item.get()); re_item && re_item->accept(current_arg))
                    {
                        current = item.get();
                        break;
                    }
                    if (!current_arg.empty() && item->get_name() == current_arg)
                    {
                        current = item.get();
                        break;
                    }
                }

                if (current)
                {
                    std::vector<std::string> params;
                    for (auto j = 0u; current->get_param_count() > j; j++)
                    {
                        if (j + i + 1 < static_cast<std::size_t>(argc))
                        {
                            params.emplace_back(argv[i + j + 1]);
                        }
                        else
                        {
                            print_usage(argv, current, current_arg, error_t::insufficient_arguments);
                            return 1;
                        }
                    }

                    i += static_cast<int>(current->get_param_count());
                    if (dynamic_cast<regex_item *>(current))
                    {
                        params.insert(params.begin(), current_arg);
                    }

                    if (auto error = (*current)(params); error != error_t::none)
                    {
                        print_usage(argv, current, current_arg, error);
                        return 1;
                    }
                    continue;
                }

                print_usage(argv, current, current_arg, error_t::unknown_argument);
                return 1;
            }

            return 0;
        }

        print_usage(argv);
        return 1;
    }
} // namespace cli