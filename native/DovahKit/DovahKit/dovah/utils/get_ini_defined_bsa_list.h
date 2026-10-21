#pragma once
#include <filesystem>
#include <vector>
#include "dovah/game.h"

namespace dovah::utils {
   extern std::vector<std::filesystem::path> get_ini_defined_bsa_list(game);
}
