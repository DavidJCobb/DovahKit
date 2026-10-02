#pragma once
#include <filesystem>
#include <stdexcept>
namespace dovah {
   enum class game;
}

namespace dovah::exceptions {
   class load_order_error : public std::runtime_error {
      public:
         load_order_error(
            const std::filesystem::path& data_directory,
            game target_game
         ) :
            std::runtime_error("file load error"),
            data_directory(data_directory),
            target_game(target_game)
         {}

      public:
         std::filesystem::path data_directory;
         game                  target_game;
   };
}