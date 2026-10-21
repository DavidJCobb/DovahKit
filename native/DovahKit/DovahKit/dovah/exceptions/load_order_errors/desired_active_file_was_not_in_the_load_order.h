#pragma once
#include "../load_order_error.h"
#include <string>
#include <string_view>

namespace dovah::exceptions::load_order_errors {
   // This exception results from improper usage of the backend APIs. If it's 
   // ever thrown, you're doing something wrong.
   class desired_active_file_was_not_in_the_load_order : public load_order_error {
      public:
         desired_active_file_was_not_in_the_load_order(
            const std::filesystem::path& data_directory,
            game target_game,
            const std::string_view active_file_name
         ) :
            load_order_error(data_directory, target_game),
            active_file_name(active_file_name)
         {}

      public:
         std::string active_file_name;
   };
}