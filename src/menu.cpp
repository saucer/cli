#include "cli.hpp"
#include "menu.hpp"

namespace cli
{
    menu::~menu() = default;
    menu::menu(std::string name, std::string desc) : item(std::move(name), std::move(desc), [this]() { m_parent->set_menu(this); }, {}) {}

    void menu::set_parent(session *session) // NOLINT
    {
        m_parent = session;

        for (const auto &item : m_items)
        {
            if (auto *menu_item = dynamic_cast<menu *>(item.get()))
            {
                menu_item->set_parent(session);
            }
        }
    }

    void menu::add(std::unique_ptr<item> item)
    {
        // m_items.emplace_back(std::move(item));
        m_items.push_back(std::move(item));
    }
} // namespace cli