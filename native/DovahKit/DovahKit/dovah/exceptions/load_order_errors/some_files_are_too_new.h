#pragma once
#include "../load_order_error.h"
#include <string>
#include <vector>

namespace dovah::exceptions::load_order_errors {
   class some_files_are_too_new : public load_order_error {
      public:
         some_files_are_too_new(
            const std::filesystem::path& data_directory,
            game target_game,
            std::vector<std::string>&& filenames
         ) :
            load_order_error(data_directory, target_game),
            filenames(std::move(filenames))
         {}

      public:
         std::vector<std::string> filenames;
   };
}