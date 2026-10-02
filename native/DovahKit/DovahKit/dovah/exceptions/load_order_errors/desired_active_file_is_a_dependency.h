#pragma once
#include "../load_order_error.h"
#include <string>

namespace dovah::exceptions::load_order_errors {
   class desired_active_file_is_a_dependency : public load_order_error {
      public:
         desired_active_file_is_a_dependency(
            const std::filesystem::path& data_directory,
            game target_game,
            const std::string& active_file_name,
            const std::string& dependent_file_name
         ) :
            load_order_error(data_directory, target_game),
            active_file_name(active_file_name),
            dependent_file_name(dependent_file_name)
         {}

      public:
         std::string active_file_name;
         std::string dependent_file_name;
   };
}