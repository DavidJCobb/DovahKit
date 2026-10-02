#pragma once
#include "../load_order_error.h"
#include <string>

namespace dovah::exceptions::load_order_errors {
   class load_order_would_have_too_many_files : public load_order_error {
      public:
         load_order_would_have_too_many_files(
            const std::filesystem::path& data_directory,
            game target_game
         ) :
            load_order_error(data_directory, target_game)
         {}

      public:
         struct {
            size_t light = 0;
            size_t heavy = 0;
            size_t total = 0;
         } file_counts;
         std::string overflowed_at_file;
   };
}