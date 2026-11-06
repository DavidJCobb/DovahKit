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
      this->_prepare_to_edit_value();
      std::get<decltype(v)>(this->_value.variant) = v;
   }
   void loaded_game_setting::set_value(float v) {
      #if _DEBUG
      {
         auto n = this->name_in_lowercase();
         assert(!n.empty() && n[0] == 'f' && "Writing the incorrect value type to a GMST.");
      }
      #endif
      this->_prepare_to_edit_value();
      std::get<decltype(v)>(this->_value.variant) = v;
   }
   void loaded_game_setting::set_value(int32_t v) {
      #if _DEBUG
      {
         auto n = this->name_in_lowercase();
         assert(!n.empty() && n[0] == 'i' && "Writing the incorrect value type to a GMST.");
      }
      #endif
      this->_prepare_to_edit_value();
      std::get<decltype(v)>(this->_value.variant) = v;
   }
   void loaded_game_setting::set_value(string_id_type id) {
      #if _DEBUG
      {
         auto n = this->name_in_lowercase();
         assert(!n.empty() && n[0] == 's' && "Writing the incorrect value type to a GMST.");
      }
      #endif
      this->_prepare_to_edit_value();
      auto& dst         = std::get<string_value_type>(this->_value.variant);
      auto& alo         = this->_owner->owner;
      auto* active_file = alo.get_active_file();
      assert(!!active_file);
      assert(active_file->header().flags.has_all_of<tes::file_header_flag::localized_strings>() && "Cannot set a string GMST to a localized string ID if the active file is non-localized.");
      dst = id;
      //
      // Update l-string use info:
      //
      this->_sever_string_use();
      if (id) {
         auto& lsc = alo.get_localized_string_collection();
         auto& use = lsc._get_or_create_use_node({}, *active_file, string_id_type::category, (bare_localized_string_id)id);
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
      this->_prepare_to_edit_value();
      auto& dst         = std::get<string_value_type>(this->_value.variant);
      auto* active_file = this->_owner->owner.get_active_file();
      assert(!!active_file);
      assert(!active_file->header().flags.has_all_of<tes::file_header_flag::localized_strings>() && "Cannot set a string GMST to a non-localized string if the active file is localized.");
      dst = std::move(text);
      //
      // Update l-string use info:
      //
      this->_sever_string_use();
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
         assert(std::holds_alternative<string_value_type>(this->_value.variant));
         const auto& value_s = std::get<string_value_type>(this->_value.variant);

         string_id_type string_id_after;
         if (std::holds_alternative<string_id_type>(value_s)) {
            string_id_after = std::get<string_id_type>(value_s);
         }

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
            auto& use = lsc._get_or_create_use_node({}, source_file, string_id_type::category, (bare_localized_string_id)string_id_after);
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

   void loaded_game_setting::_sever_string_use() {
      if (auto* pu = this->_value.string_use_info) {
         pu->users.game_settings.erase(this->_name.lowercase);
         this->_value.string_use_info = nullptr;
      }
   }
   void loaded_game_setting::_prepare_to_edit_value() {
      //
      // There are two things we need to do before we can edit the GMST's value.
      // 
      //  - For values of any type, we need to ensure that we have a valid form ID. If we 
      //    don't, then we need to create a new form stub with a valid form ID, so we can 
      //    accept that form ID as our own.
      // 
      //  - If our source file list doesn't contain the active file, then we need to add 
      //    the active file to the list. This is both for the above (since form IDs are 
      //    tracked per source file) and so that we can properly track localized string 
      //    use info.
      // 
      //    You see, ihree criteria are needed to identify a localized string ID:
      // 
      //     - The ID itself.
      //     - The string category (`localization::category`).
      //     - The TES file with which the string files are associated.
      // 
      //    We store the ID as part of our value. The category is the same for all game 
      //    settings, so that's just hardcoded. The TES file is the last source file in 
      //    our source file list.
      //
      // This function handles both of the above tasks, and should be called before any 
      // attempt is made to modify the setting's current value.
      //
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