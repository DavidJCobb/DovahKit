#pragma once
#include "../load_order_error.h"
#include <set>
#include <string>
#include <vector>

namespace dovah::exceptions::load_order_errors {
   class cyclical_dependency_between_files : public load_order_error {
      public:
         cyclical_dependency_between_files(
            const std::filesystem::path& data_directory,
            game target_game
         ) :
            load_order_error(data_directory, target_game)
         {}
         
         cyclical_dependency_between_files(
            const std::filesystem::path& data_directory,
            game target_game,
            const std::set<std::string>& files_in_cycle,
            const std::string& cycle_detected_at
         ) :
            load_order_error(data_directory, target_game)
         {
            for (const auto& filename : files_in_cycle)
               this->files_seen.push_back(filename);
            this->files_seen.push_back(cycle_detected_at);
         }

      public:
         std::vector<std::string> files_seen;
   };
}