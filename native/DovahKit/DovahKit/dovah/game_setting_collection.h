#pragma once
#include <mutex>
#include <string>
#include <unordered_map>
namespace dovah {
   class loaded_game_setting;
}

namespace dovah {
   class game_setting_collection {
      protected:
         std::unordered_map<std::string, loaded_game_setting> _settings_by_name; // keys are lowercase
         mutable std::mutex _lock; // for multi-threaded loads

   };
}