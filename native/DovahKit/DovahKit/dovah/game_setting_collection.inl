#pragma once
#include "./game_setting_collection.h"

namespace dovah {
   template<typename Callable>
      requires std::is_invocable_v<Callable, const loaded_game_setting&>
   void game_setting_collection::for_each_active_file_game_setting(Callable&& callable) const {
      const auto* active_file = this->_get_active_file();
      for (auto& pair : this->_settings_by_name) {
         auto& gmst         = pair.second;
         auto& source_files = gmst.source_file_list();
         if (source_files.empty())
            continue;
         if (source_files.back().source_file != active_file)
            continue;
         if constexpr (std::is_invocable_r_v<lu::foreach_lambda_result, Callable, const loaded_game_setting&>) {
            auto result = callable(pair.second);
            if (result == lu::foreach_lambda_result::stop)
               break;
         } else {
            callable(pair.second);
         }
      }
   }
   
   template<typename Callable>
      requires std::is_invocable_v<Callable, loaded_game_setting&>
   void game_setting_collection::for_each_active_file_game_setting(Callable&& callable) {
      const auto* active_file = this->_get_active_file();
      for (auto& pair : this->_settings_by_name) {
         auto& gmst         = pair.second;
         auto& source_files = gmst.source_file_list();
         if (source_files.empty())
            continue;
         if (source_files.back().source_file != active_file)
            continue;
         if constexpr (std::is_invocable_r_v<lu::foreach_lambda_result, Callable, loaded_game_setting&>) {
            auto result = callable(pair.second);
            if (result == lu::foreach_lambda_result::stop)
               break;
         } else {
            callable(pair.second);
         }
      }
   }

   template<typename Callable>
      requires std::is_invocable_v<Callable, const loaded_game_setting&>
   void game_setting_collection::for_each_loaded_game_setting(Callable&& callable) const {
      for (auto& pair : this->_settings_by_name) {
         if constexpr (std::is_invocable_r_v<lu::foreach_lambda_result, Callable, const loaded_game_setting&>) {
            auto result = callable(pair.second);
            if (result == lu::foreach_lambda_result::stop)
               break;
         } else {
            callable(pair.second);
         }
      }
   }

   template<typename Callable>
      requires std::is_invocable_v<Callable, loaded_game_setting&>
   void game_setting_collection::for_each_loaded_game_setting(Callable&& callable) {
      for (auto& pair : this->_settings_by_name) {
         if constexpr (std::is_invocable_r_v<lu::foreach_lambda_result, Callable, loaded_game_setting&>) {
            auto result = callable(pair.second);
            if (result == lu::foreach_lambda_result::stop)
               break;
         } else {
            callable(pair.second);
         }
      }
   }
}