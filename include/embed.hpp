#pragma once
#include <string>
#include <vector>
#include <optional>
#include <filesystem>

namespace cli::embed
{
    namespace fs = std::filesystem;

    class file
    {
        friend bool write_files(const std::vector<file> &, const fs::path &);

      private:
        std::string m_file_name;
        std::string m_formatted_name;

        std::string m_mime;
        std::vector<unsigned char> m_buffer;

      public:
        bool write(const fs::path &path) const;

      public:
        static std::optional<file> from(const fs::path &, const fs::path &);
    };

    bool write_files(const std::vector<file> &files, const fs::path &out_path);
} // namespace cli::embed