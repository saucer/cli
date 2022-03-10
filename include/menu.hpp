#pragma once
#include <map>
#include <memory>
#include "item.hpp"

namespace cli
{
    class session;
    class menu : public item
    {
        friend class session;

      private:
        session *m_parent;
        std::vector<std::unique_ptr<item>> m_items;

      protected:
        void set_parent(session *);

      public:
        ~menu() override;
        menu(std::string name, std::string desc = "");

      public:
        void add(std::unique_ptr<item>);
    };
} // namespace cli