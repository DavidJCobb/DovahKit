#pragma once
#include <filesystem>
#include <string>

namespace DovahKitDebug {
   extern void extract_bsa_file(const std::filesystem::path& bsa, const std::string& target, const std::filesystem::path& extract_to);
}