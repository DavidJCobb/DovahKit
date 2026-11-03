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
      bare_form_id       form_id,
      const std::string& name,
      value_variant&&    value
   ) {
      using managed_l_string_type = managed_localized_string_use<localized_string_type::general>;
      bare_localized_string_id prior_string_id = 0;

      bare_form_id moved_from_form_id = 0;

      bool is_new_file = false;
      if (this->data.empty()) {
         this->name       = name;
         this->definition = facts::game_settings::lookup_by_name(name);
         is_new_file = true;
      } else {
         auto& last_file_info = this->data.back();
         is_new_file   = last_file_info.source_file == &source_file;
         if (!is_new_file) {
            if (form_id) {
               moved_from_form_id = last_file_info.last_form_id;
               last_file_info.last_form_id = form_id;
            }
            if (std::holds_alternative<managed_l_string_type>(last_file_info.value)) {
               auto& info = std::get<managed_l_string_type>(last_file_info.value);
               prior_string_id = info.get_localized_id();
            }
            last_file_info.value = std::move(value);
         }
      }
      if (std::holds_alternative<managed_l_string_type>(value)) {
         //
         // Update l-string use info.
         //
         auto& alo       = *source_file.get_active_load_order();
         auto& lsc       = alo.get_localized_string_collection();
         auto  string_id = std::get<managed_l_string_type>(value).get_localized_id();
         if (string_id != prior_string_id) {
            std::string key;
            {
               key.resize(this->name.size());
               for (size_t i = 0; i < this->name.size(); ++i) {
                  auto c = this->name[i];
                  if (c >= 'A' && c <= 'Z')
                     c |= 0x20;
                  key[i] = c;
               }
            }
            if (prior_string_id) {
               assert(!this->_string_use_info.empty());
               auto& use  = *this->_string_use_info.back();
               auto& used = use.used;
               assert(used.id   == prior_string_id);
               assert(used.type == localized_string_type::general);
               if (used.tes_file == &source_file) {
                  --use.users.game_settings[key];
                  this->_string_use_info.pop_back();
               }
               // TODO: can we tell the LSC to delete this use node if all its use counts are zero?
            }
            if (string_id) {
               bool found = false;
               for (auto* use : this->_string_use_info) {
                  auto& used = use->used;
                  if (used.id == string_id && used.type == localized_string_type::general && used.tes_file == &source_file) {
                     ++use->users.game_settings[key];
                     found = true;
                     break;
                  }
               }
               if (!found) {
                  auto& use = lsc._get_or_create_use_node({}, source_file, localized_string_type::general, string_id);
                  ++use.users.game_settings[key];
                  this->_string_use_info.push_back(&use);
               }
            }
         }
      }
      if (is_new_file) {
         this->data.push_back({
            .source_file  = &source_file,
            .last_form_id = form_id,
            .value        = std::move(value)
         });
      }
      return moved_from_form_id;
   }
}