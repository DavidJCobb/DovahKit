#pragma once
#include <filesystem>
namespace dovah {
   enum class game;
}

namespace editor_helpers {
   extern std::filesystem::path get_autodetected_game_path(dovah::game);
}