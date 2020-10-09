#pragma once
#include <filesystem>
#include <string>

namespace DovahKitDebug {
   extern void enumerate_bsa_contents(const std::filesystem::path& bsa, const std::string& target, const std::filesystem::path& extract_to);
}