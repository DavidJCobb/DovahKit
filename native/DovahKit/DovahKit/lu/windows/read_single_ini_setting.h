#pragma once
#include <filesystem>
#include <string>

namespace lu::windows {
   extern std::string read_single_ini_setting(const std::filesystem::path& ini_file, const std::string& category, const std::string& key);
   extern std::wstring read_single_ini_setting(const std::filesystem::path& ini_file, const std::wstring& category, const std::wstring& key);
}