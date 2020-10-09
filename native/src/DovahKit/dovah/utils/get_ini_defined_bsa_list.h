#pragma once
#include <filesystem>
#include <vector>

namespace dovah::utils {
   extern std::vector<std::filesystem::path> get_ini_defined_bsa_list();
}