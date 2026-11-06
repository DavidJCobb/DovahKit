#include "./loaded_game_setting.h"
#include <cassert>
#include "dovah/active_load_order.h"
#include "dovah/facts/game_settings/all_definitions.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_collection.h"
#include "dovah/game_setting_collection.h"
#include "dovah/localization/string_collection.h"
#include "dovah/localization/use_node.h"
#include "dovah/tes/file.h"
#include "dovah/types/localized_string_id.h"

namespace dovah {
   bare_form_id loaded_game_setting::form_id() const noexcept {
      for (auto it = this->_source_files.rbegin(); it != this->_source_files.rend(); ++it) {
         auto& info = *it;
         if (info.form_id)
            return info.form_id;
      }
      return 0;
   }

   void loaded_game_setting::set_value(bool v) {
      #if _DEBUG
      {
         auto n = this->name_in_lowercase();
         assert(!n.empty() && n[0] == 'b' && "Writing the incorrect value type to a GMST.");
      }
      #endif
      this->_on_value_edited();
      std::get<decltype(v)>(this->_value.variant) = v;
   }
   void loaded_game_setting::set_value(float v) {
      #if _DEBUG
      {
         auto n = this->name_in_lowercase();
         assert(!n.empty() && n[0] == 'f' && "Writing the incorrect value type to a GMST.");
      }
      #endif
      this->_on_value_edited();
      std::get<decltype(v)>(this->_value.variant) = v;
   }
   void loaded_game_setting::set_value(int32_t v) {
      #if _DEBUG
      {
         auto n = this->name_in_lowercase();
         assert(!n.empty() && n[0] == 'i' && "Writing the incorrect value type to a GMST.");
      }
      #endif
      this->_on_value_edited();
      std::get<decltype(v)>(this->_value.variant) = v;
   }
   void loaded_game_setting::set_value(localized_string_id<localization::category::general> id) {
      #if _DEBUG
      {
         auto n = this->name_in_lowercase();
         assert(!n.empty() && n[0] == 's' && "Writing the incorrect value type to a GMST.");
      }
      #endif
      this->_on_value_edited();
      auto& dst         = std::get<facts::game_settings::value_types::key_of<'s'>>(this->_value.variant);
      auto& alo         = this->_owner->owner;
      auto* active_file = alo.get_active_file();
      assert(!!active_file);
      assert(active_file->header().flags.has_all_of<tes::file_header_flag::localized_strings>() && "Cannot set a string GMST to a localized string ID if the active file is non-localized.");
      dst.set_localized_id(*active_file, (bare_localized_string_id)id);
      //
      // Update l-string use info:
      //
      if (auto* pu = this->_value.string_use_info) {
         pu->users.game_settings.erase(this->_name.lowercase);
         this->_value.string_use_info = nullptr;
      }
      if (id) {
         auto& lsc = alo.get_localized_string_collection();
         auto& use = lsc._get_or_create_use_node({}, *active_file, localization::category::general, (bare_localized_string_id)id);
         use.users.game_settings.insert(this->_name.lowercase);
         this->_value.string_use_info = &use;
      }
   }
   void loaded_game_setting::set_value(std::string&& text) {
      #if _DEBUG
      {
         auto n = this->name_in_lowercase();
         assert(!n.empty() && n[0] == 's' && "Writing the incorrect value type to a GMST.");
      }
      #endif
      this->_on_value_edited();
      auto& dst         = std::get<facts::game_settings::value_types::key_of<'s'>>(this->_value.variant);
      auto* active_file = this->_owner->owner.get_active_file();
      assert(!!active_file);
      assert(!active_file->header().flags.has_all_of<tes::file_header_flag::localized_strings>() && "Cannot set a string GMST to a non-localized string if the active file is localized.");
      dst.set_non_localized_string(std::move(text));
      //
      // Update l-string use info:
      //
      if (auto* pu = this->_value.string_use_info) {
         pu->users.game_settings.erase(this->_name.lowercase);
         this->_value.string_use_info = nullptr;
      }
   }

   bare_form_id loaded_game_setting::_on_record_loaded(
      lu::passkey<game_setting_collection>,
      const tes::file&   source_file,
      uint32_t           record_offset,
      bare_form_id       form_id,
      const std::string& name,
      const std::string& name_lowercase,
      value_variant&&    value
   ) {
      using managed_l_string_type = facts::game_settings::value_types::key_of<'s'>;
      bare_localized_string_id prior_string_id = 0;

      bare_form_id moved_from_form_id = 0;

      bool is_new_file = false;
      if (this->_source_files.empty()) {
         this->_name.loaded    = name;
         this->_name.lowercase = name_lowercase;
         this->_definition     = facts::game_settings::lookup_by_name(name);
         is_new_file = true;
      } else {
         auto& last_file_info = this->_source_files.back();
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
      assert(!this->_name.lowercase.empty());
      if (this->_name.lowercase[0] == 's') {
         auto& alo = *source_file.get_active_load_order();
         auto& lsc = alo.get_localized_string_collection();
         assert(std::holds_alternative<facts::game_settings::value_types::key_of<'s'>>(this->_value.variant));
         const auto& value_s         = std::get<facts::game_settings::value_types::key_of<'s'>>(this->_value.variant);
         const auto  string_id_after = value_s.get_localized_id();
         if (auto* prior_use = this->_value.string_use_info) {
            bare_localized_string_id string_id_prior = prior_use->used.id;
            if (string_id_prior != string_id_after || prior_use->used.tes_file != &source_file) {
               prior_use->users.game_settings.erase(this->_name.lowercase);
               //
               // TODO: If string is now unused, tell LSC to destroy the use node.
               //
            }
         }
         if (string_id_after) {
            auto& use = lsc._get_or_create_use_node({}, source_file, localization::category::general, string_id_after);
            use.users.game_settings.insert(this->_name.lowercase);
         }
      }

      if (is_new_file) {
         this->_source_files.push_back({
            .source_file   = &source_file,
            .record_offset = 0,
            .form_id       = form_id,
         });
      }

      return moved_from_form_id;
   }

   void loaded_game_setting::_on_value_edited() {
      auto& alo         = this->_owner->owner;
      auto* active_file = alo.get_active_file();
      assert(!!active_file);

      bare_form_id form_id = this->form_id();
      if (!this->_source_files.empty()) {
         auto& info = this->_source_files.back();
         if (info.source_file == active_file) {
            if (!form_id) {
               auto& stub = alo.get_form_stub_collection()._make_stub_for_game_setting({}, this->_name.loaded);
               info.form_id = stub.form_id;
            }
            return;
         }
      }
      auto& info = this->_source_files.emplace_back();
      info.source_file = active_file;
      if (!form_id) {
         auto& stub = alo.get_form_stub_collection()._make_stub_for_game_setting({}, this->_name.loaded);
         form_id = stub.form_id;
      }
      info.form_id = form_id;
   }
}