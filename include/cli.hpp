#pragma once
#include <map>
#include <memory>
#include <string>
#include <optional>
#include "menu.hpp"

namespace cli
{
    class session
    {
        friend class menu;

      private:
        menu *m_current;
        std::unique_ptr<menu> m_root;
        std::vector<menu *> m_menu_history;

      protected:
        void set_menu(menu *);

      public:
        session(std::unique_ptr<menu> root);

      public:
        int start(int argc, char **argv);
        void print_usage(char **, item * = nullptr, const std::string & = "", error_t = error_t::none);
    };
} // namespace cli