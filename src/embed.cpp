#include "ansi.hpp"
#include "embed.hpp"
#include "mimes.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <cstring>
#include <fstream>

namespace cli::embed
{
    std::optional<file> file::from(const fs::path &path)
    {
        file rtn;
        rtn.m_file_name = path.filename().string();

        try
        {
            auto extension = path.filename().extension().string();

            if (!mimes.count(extension))
            {
                std::cerr << ansi::error_indicator << "Failed to deduce mime-type for: " << ansi::bold << path << ansi::reset << std::endl;
                return std::nullopt;
            }

            rtn.m_mime = mimes.at(extension);

            std::ifstream original_file(path, std::ios::binary);
            rtn.m_buffer = {std::istreambuf_iterator<char>(original_file), {}};
            original_file.close();

            rtn.m_formatted_name = path.filename().string();
            std::replace(rtn.m_formatted_name.begin(), rtn.m_formatted_name.end(), '.', '_');
            std::replace(rtn.m_formatted_name.begin(), rtn.m_formatted_name.end(), '-', '_');
            std::replace(rtn.m_formatted_name.begin(), rtn.m_formatted_name.end(), ' ', '_');

            return rtn;
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Failed to embed file " << ansi::bold << path << ansi::reset << ": " << ex.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << "Failed to embed file " << ansi::bold << path << ansi::reset << ": <Unknown Error>" << std::endl;
        }

        return std::nullopt;
    }

    bool file::write(const fs::path &path) const
    {
        try
        {
            auto final_path = path / (m_file_name + ".hpp");
            std::cout << ansi::success_indicator << "Embedding " << ansi::bold << m_file_name << ansi::reset << " to " << ansi::bold << final_path << ansi::reset << " with mime "
                      << ansi::bold << m_mime << std::endl;

            std::ofstream out(final_path);
            out.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            out << "#pragma once" << std::endl;
            out << "inline constexpr unsigned char embedded_" << m_formatted_name << "[" + std::to_string(m_buffer.size()) + "] = {";

            for (auto byte_it = m_buffer.begin(); byte_it != m_buffer.end(); byte_it++)
            {
                out << "0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(*byte_it);

                if (std::distance(byte_it, m_buffer.end()) > 1)
                {
                    out << ",";
                }
            }

            out << "};" << std::endl;
            out.close();

            return true;
        }
        catch (const std::ios_base::failure &)
        {
            // NOLINTNEXTLINE
            std::cerr << ansi::error_indicator << "Failed to embed file " << ansi::bold << path << ansi::reset << ": " << strerror(errno) << std::endl;
        }
        catch (...)
        {
            std::cerr << ansi::error_indicator << "Failed to embed file " << ansi::bold << path << ansi::reset << ": <Unknown Error>" << std::endl;
        }

        return false;
    }

    bool write_files(const std::vector<file> &files, const fs::path &out_path)
    {
        if (!fs::exists(out_path))
        {
            fs::create_directory(out_path);
        }

        for (const auto &file : files)
        {
            auto success = file.write(out_path);
            if (!success)
            {
                return false;
            }
        }

        try
        {
            std::ofstream all(out_path / "all.hpp");
            all.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            all << "#pragma once" << std::endl;
            all << "#include <map>" << std::endl;
            all << "#include <tuple>" << std::endl;
            all << "#include <string>" << std::endl;
            all << "#include <saucer/webview.hpp>" << std::endl << std::endl;

            for (const auto &file : files)
            {
                all << "#include \"" << file.m_file_name << ".hpp\"" << std::endl;
            }

            all << std::endl;
            all << "namespace embedded {" << std::endl;
            all << "\tinline auto get_all_files() {" << std::endl;
            all << "\t\tstd::map<const std::string, const saucer::embedded_file> rtn;" << std::endl;

            for (const auto &file : files)
            {
                all << "\t\trtn.emplace(\"" << file.m_file_name << "\", saucer::embedded_file{\"" << file.m_mime << "\", " << file.m_buffer.size() << ", embedded_"
                    << file.m_formatted_name << "});" << std::endl;
            }

            all << "\t\treturn rtn;" << std::endl;
            all << "\t}" << std::endl << "} // namespace embedded";
            all.close();

            std::cout << ansi::success_indicator << "Embedded " << ansi::bold << files.size() << ansi::reset << " file(s)" << std::endl;
            return true;
        }
        catch (const std::ios_base::failure &)
        {
            // NOLINTNEXTLINE
            std::cerr << ansi::error_indicator << "Failed to write embedding files: " << strerror(errno) << std::endl;
        }
        catch (...)
        {
            std::cerr << ansi::error_indicator << "Failed to write embedding files: <Unknown Error>" << std::endl;
        }

        return false;
    }
} // namespace cli::embed