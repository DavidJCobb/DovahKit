#include "./loaded_game_setting.h"
#include "dovah/active_load_order.h"
#include "dovah/facts/game_settings/all_definitions.h"
#include "dovah/localization/string_collection.h"
#include "dovah/localization/use_node.h"
#include "dovah/tes/file.h"

namespace dovah {
   bare_form_id loaded_game_setting::_on_record_loaded(
      lu::passkey<game_setting_collection>,
      const tes::file&   source_file,
      uint32_t           record_offset,
      bare_form_id       form_id,
      const std::string& name,
      const std::string& name_lowercase,
      value_variant&&    value
   ) {
      using managed_l_string_type = managed_localized_string_use<localized_string_type::general>;
      bare_localized_string_id prior_string_id = 0;

      bare_form_id moved_from_form_id = 0;

      bool is_new_file = false;
      if (this->source_files.empty()) {
         this->name.loaded    = name;
         this->name.lowercase = name_lowercase;
         this->definition     = facts::game_settings::lookup_by_name(name);
         is_new_file = true;
      } else {
         auto& last_file_info = this->source_files.back();
         is_new_file   = last_file_info.source_file == &source_file;
         if (!is_new_file) {
            if (form_id) {
               moved_from_form_id = last_file_info.form_id;
               last_file_info.form_id = form_id;
            }
         }
      }

      this->_value.variant = std::move(value);
      //
      // Update l-string use info, if this is a string-type setting.
      //
      assert(!this->name.lowercase.empty());
      if (this->name.lowercase[0] == 's') {
         auto& alo = *source_file.get_active_load_order();
         auto& lsc = alo.get_localized_string_collection();
         assert(std::holds_alternative<managed_l_string_type>(this->_value.variant));
         const auto& value_s         = std::get<managed_l_string_type>(this->_value.variant);
         const auto  string_id_after = value_s.get_localized_id();
         if (auto* prior_use = this->_value.string_use_info) {
            bare_localized_string_id string_id_prior = prior_use->used.id;
            if (string_id_prior != string_id_after) {
               prior_use->users.game_settings.erase(this->name.lowercase);
               //
               // TODO: If string is now unused, tell LSC to destroy the use node.
               //
            }
         }
         if (string_id_after) {
            auto& use = lsc._get_or_create_use_node({}, source_file, localized_string_type::general, string_id_after);
            use.users.game_settings.insert(this->name.lowercase);
         }
      }

      if (is_new_file) {
         this->source_files.push_back({
            .source_file   = &source_file,
            .record_offset = 0,
            .form_id       = form_id,
         });
      }

      return moved_from_form_id;
   }
}