#pragma once
#include <filesystem>
#include <string>

namespace cobb {
   extern std::string read_single_ini_string_setting(const std::filesystem::path& file, const std::string& category, const std::string& key);
   extern std::wstring read_single_ini_string_setting(const std::filesystem::path& file, const std::wstring& category, const std::wstring& key);
}