#pragma once
#include "./loaded_game_setting.h"

namespace dovah {
   constexpr const facts::game_settings::definition* loaded_game_setting::definition() const noexcept {
      return this->_definition;
   }
   constexpr const std::string_view loaded_game_setting::name() const noexcept {
      return this->_name.loaded;
   }
   constexpr const std::string_view loaded_game_setting::name_in_lowercase() const noexcept {
      return this->_name.lowercase;
   }
   constexpr const std::vector<loaded_game_setting::source_file_info>& loaded_game_setting::source_file_list() const noexcept {
      return this->_source_files;
   }

   constexpr const loaded_game_setting::value_variant& loaded_game_setting::get_value_variant() const noexcept {
      return this->_value.variant;
   }
}