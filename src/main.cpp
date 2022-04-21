#include "cli.hpp"
#include "ansi.hpp"
#include "item.hpp"
#include "embed.hpp"
#include "regex_item.hpp"

#include <fstream>
#include <iostream>
#include <filesystem>

#include "main.template.hpp"
#include "CmakeLists.template.hpp"

namespace fs = std::filesystem;

int main(int argc, char **argv)
{
    auto root = std::make_unique<cli::menu>("saucer");
    root->add(std::make_unique<cli::item>(
        "init", "Initialize a saucer project in a new directory <name>",
        [](const std::string &name) {
            try
            {
                fs::create_directory(name);

                std::ofstream cmakelists(name + "/CMakeLists.txt");
                cmakelists << std::regex_replace(cmakelists_template, std::regex("<NAME>"), "\"" + name + "\"");
                cmakelists.close();

                fs::create_directory(name + "/src");
                std::ofstream main(name + "/src/main.cpp");
                main << main_template;
                main.close();

                std::cout << ansi::success_indicator << "Created project: " << name << std::endl;
            }
            catch (const std::exception &ex)
            {
                std::cerr << ansi::error_indicator << "Failed to initialize project: " << ex.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << ansi::error_indicator << "Failed to initialize project: <Unknown Error>" << std::endl;
            }
        },
        std::vector<std::string>{"name"}));

    auto embed_menu = std::make_unique<cli::menu>("embed", "Embed files and folders");
    fs::path output_path{fs::current_path() / "embedding"};
    std::vector<cli::embed::file> files;

    embed_menu->add(std::make_unique<cli::item>(
        "output", "Set the output directory to <path>", [&](const std::filesystem::path &path) { output_path = path; }, std::vector<std::string>{"path"}));

    embed_menu->add(std::make_unique<cli::regex_item>(
        "[File/Folder]...", std::regex(".*"), "Generate the embedding headers for the specified <files/folders>",
        [&](const fs::path &path) {
            if (fs::is_directory(path))
            {
                for (const auto &file : fs::recursive_directory_iterator(path))
                {
                    if (file.is_regular_file())
                    {
                        auto embedded_file = cli::embed::file::from(file);
                        if (embedded_file)
                        {
                            files.emplace_back(*embedded_file);
                        }
                        else
                        {
                            return cli::error_t::bad_arguments;
                        }
                    }
                }

                if (!files.empty())
                {
                    return cli::error_t::none;
                }
            }

            if (fs::is_regular_file(path))
            {
                auto embedded_file = cli::embed::file::from(path);
                if (embedded_file)
                {
                    files.emplace_back(*embedded_file);
                    return cli::error_t::none;
                }

                return cli::error_t::bad_arguments;
            }

            return cli::error_t::bad_arguments;
        },
        std::vector<std::string>{}));

    root->add(std::move(embed_menu));

    cli::session session(std::move(root));
    auto error = session.start(argc, argv);

    if (!error && !files.empty())
    {
        cli::embed::write_files(files, output_path);
    }

    return error;
}