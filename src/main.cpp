#include "cli.hpp"
#include "ansi.hpp"
#include "regex_item.hpp"

#include <fstream>
#include <iostream>
#include <filesystem>

#include "mimes.hpp"
#include "main.template.hpp"
#include "CmakeLists.template.hpp"

namespace fs = std::filesystem;

struct file
{
    fs::path path;
    std::string mime;
    std::size_t size;
    std::string original_name;
    std::string formatted_name;
};

void embed_file(const fs::path &path, const fs::path &out_path, std::vector<file> &files)
{
    auto file_name = path.filename();

    std::cout << ansi::success_indicator << "Embedding " << ansi::bold << path << ansi::reset << " to " << ansi::bold << (out_path / (file_name.string() + ".hpp")) << ansi::reset
              << std::endl;

    try
    {
        auto extension = file_name.extension().string();
        if (!mimes.count(extension))
        {
            std::cerr << ansi::error_indicator << "Could not deduce mime-type for: " << ansi::bold << file_name << ansi::reset << std::endl;
            return;
        }

        std::ifstream original_file(path, std::ios::binary);
        std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(original_file), {});
        original_file.close();

        auto final_path = out_path / (file_name.string() + ".hpp");
        std::ofstream out(final_path);

        auto formatted_name = file_name.string();
        std::replace(formatted_name.begin(), formatted_name.end(), '.', '_');
        std::replace(formatted_name.begin(), formatted_name.end(), '-', '_');
        std::replace(formatted_name.begin(), formatted_name.end(), ' ', '_');

        out << "#pragma once" << std::endl;
        out << "constexpr unsigned char embedded_" << formatted_name << "[" + std::to_string(buffer.size()) + "] = {";

        for (auto byte_it = buffer.begin(); byte_it != buffer.end(); byte_it++)
        {
            out << "0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(*byte_it);
            if (std::distance(byte_it, buffer.end()) > 1)
            {
                out << ",";
            }
        }
        out << "};" << std::endl;
        out.close();

        files.emplace_back(file{final_path, mimes.at(extension), buffer.size(), file_name.string(), formatted_name});
        std::cout << ansi::success_indicator << "Embedded " << ansi::bold << (out_path / (file_name.string() + ".hpp")) << ansi::reset << " with mime-type " << ansi::bold
                  << files.back().mime << ansi::reset << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Failed to embed file " << ansi::bold << file_name << ansi::reset << ": " << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Failed to embed file " << ansi::bold << file_name << ansi::reset << ": <Unknown Error>" << std::endl;
    }
}

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

    auto embed = std::make_unique<cli::menu>("embed", "Embed files and folders");
    fs::path output_path{fs::current_path() / "embedding"};
    std::vector<file> files;

    embed->add(std::make_unique<cli::item>(
        "output", "Sets the output directory for the generated files",
        [&](const fs::path &path) {
            if (fs::is_directory(path) || !fs::exists(path))
            {
                if (!fs::exists(path))
                {
                    fs::create_directory(path);
                }

                files.clear();
                output_path = path;
                std::cout << ansi::success_indicator << "Set path: " << ansi::bold << output_path << ansi::reset << std::endl;
            }
            else
            {
                return cli::error_t::bad_arguments;
            }
            return cli::error_t::none;
        },
        std::vector<std::string>{"path"}));

    embed->add(std::make_unique<cli::regex_item>(
        "[File/Folder]...", std::regex(".*"), "Generate the embedding headers for the specified <files/folders>",
        [&](const std::filesystem::path &path) {
            if (fs::is_directory(path))
            {
                for (const auto &file : fs::recursive_directory_iterator(path))
                {
                    if (file.is_regular_file())
                    {
                        embed_file(file, output_path, files);
                    }
                }
            }
            if (fs::is_regular_file(path))
            {
                embed_file(path, output_path, files);
            }

            if (files.empty())
            {
                return cli::error_t::bad_arguments;
            }

            std::ofstream all(output_path / "all.hpp");
            all << "#pragma once" << std::endl;
            all << "#include <map>" << std::endl;
            all << "#include <tuple>" << std::endl;
            all << "#include <string>" << std::endl;

            for (const auto &file : files)
            {
                all << "#include " << file.path.filename() << std::endl;
            }

            all << std::endl;
            all << "namespace embedded {" << std::endl;
            all << "\tinline auto get_all_files() {" << std::endl;
            all << "\t\tstd::map<const std::string, std::tuple<std::string, std::size_t, const std::uint8_t *>> rtn;" << std::endl;

            for (const auto &file : files)
            {
                all << "\t\trtn[\"" << file.original_name << "\"] = std::make_tuple(\"" << file.mime << "\", " << file.size << ", embedded_" << file.formatted_name << ");"
                    << std::endl;
            }

            all << "\t\treturn rtn;" << std::endl;
            all << "\t}" << std::endl << "} // namespace embedded";
            all.close();

            std::cout << ansi::success_indicator << "Embedded " << ansi::bold << files.size() << ansi::reset << " file(s)" << std::endl;
            return cli::error_t::none;
        },
        std::vector<std::string>{}));

    root->add(std::move(embed));

    cli::session session(std::move(root));
    return session.start(argc, argv);
}