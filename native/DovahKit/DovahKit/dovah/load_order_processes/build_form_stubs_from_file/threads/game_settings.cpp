#include "./game_settings.h"
#include "dovah/active_load_order.h"
#include "dovah/callback_sets/data_load.h"
#include "dovah/facts/game_settings/all_definitions.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_collection.h"
#include "dovah/form_type.h"
#include "dovah/form_type_info.h"
#include "dovah/game_setting_collection.h"
#include "dovah/tes/file.h"
#include "dovah/tes/reading/file_view.h"
#include "dovah/tes/reading/record_view.h"
#include "dovah/tes/reading/subrecord_view.h"
#include "dovah/loaded_game_setting.h"
#include "../process.h"

#include "dovah/exceptions/file_parse_errors/record_fatally_misplaced.h"
#include "dovah/exceptions/file_parse_errors/unexpected_nested_group.h"
#include "dovah/notices/file_parse_warnings/game_setting_record_has_unrecognized_subrecord.h"
#include "dovah/notices/file_parse_warnings/game_setting_record_is_misordered.h"
#include "dovah/notices/file_parse_warnings/game_setting_value_has_extra_content.h"
#include "dovah/notices/file_parse_warnings/game_setting_value_type_unknown.h"
#include "dovah/notices/file_parse_warnings/game_setting_value_unreadable.h"

namespace {
   constexpr const auto expected_record_signature = dovah::form_type_info::lookup(dovah::form_type::setting).signature;
}

namespace dovah::load_order_processes::build_form_stubs_from_file::threads {
   void game_settings::execute_on_thread() {
      auto& alo         = this->_config.active_load_order;
      auto& callbacks   = this->_config.callbacks;
      auto& source_file = this->_config.source_file;

      tes::file_view view;
      view.set_target_file(&source_file);

      const size_t size = this->queue.size();
      this->_progress.maximum = (uint32_t)size;
      for (uint32_t i = 0; i < size; i++) {
         this->_progress.current = i;
         auto& desired = this->queue[i];
         view.set_position(desired.pos);
         assert(view.next_record_or_group() == tes::file_view::chunk_type::group);

         const auto& group_of_interest = view.groups[0];

         tes::file_view::chunk_type ot;
         while (ot = view.next_record_or_group(), ot != tes::file_view::chunk_type::none) {
            if (!group_of_interest.signature) {
               //
               // We've reached the end of this queued-for-parse group.
               //
               break;
            }
            if (ot == tes::file_view::chunk_type::group) {
               if (view.get_current_group() == &group_of_interest) [[likely]] {
                  //
                  // We've reached the end of this queued-for-parse group.
                  //
                  break;
               }
               throw exceptions::file_parse_errors::unexpected_nested_group(
                  source_file,
                  group_of_interest.position,
                  group_of_interest,
                  view.groups[1].position,
                  view.groups[1]
               );
            }
            if (ot == tes::file_view::chunk_type::record) {
               auto& record = view.subviews.record;
               if (record.signature() != expected_record_signature) [[unlikely]] {
                  throw exceptions::file_parse_errors::record_fatally_misplaced(
                     source_file,
                     record.file_offset_of_header(),
                     record.header()
                  );
               }
               this->_parse_record(record);
            }
         }
      }
   }

   void game_settings::_parse_record(tes::record_view& record) {
      auto& alo         = this->_config.active_load_order;
      auto& callbacks   = this->_config.callbacks;
      auto& source_file = this->_config.source_file;

      auto& subrecord = record.get_current_subrecord();

      bool found_data        = false;
      bool logged_misordered = false;
      std::string                                 setting_name;
      types::game_settings::managed_value_variant setting_value;

      record.next_subrecord();
      if (subrecord.signature() == 'EDID') {
         subrecord.read_all_to_string(setting_name);
      } else {
         found_data = subrecord.signature() == 'DATA';
         if (found_data) {
            logged_misordered = true;
            if (callbacks.on_warning) {
               (callbacks.on_warning)(
                  notices::file_parse_warnings::game_setting_record_is_misordered(
                     source_file.path().filename().string(),
                     std::string_view(setting_name),
                     record.formID()
                  )
               );
            }
         }
      }
      while (record.next_subrecord()) {
         if (subrecord.signature() == 'EDID') {
            if (!logged_misordered) {
               logged_misordered = true;
               if (callbacks.on_warning) {
                  (callbacks.on_warning)(
                     notices::file_parse_warnings::game_setting_record_is_misordered(
                        source_file.path().filename().string(),
                        std::string_view(setting_name),
                        record.formID()
                     )
                  );
               }
            }
            continue;
         }
         if (subrecord.signature() != 'DATA') {
            if (callbacks.on_warning) {
               (callbacks.on_warning)(
                  notices::file_parse_warnings::game_setting_record_has_unrecognized_subrecord(
                     source_file.path().filename().string(),
                     std::string_view(setting_name),
                     record.formID(),
                     subrecord.signature()
                  )
               );
            }
            continue;
         }
         found_data = true;
         
         bool no_read_error = true;
         bool type_is_known = false;
         facts::game_setting_types<true>::for_each_pair([&subrecord, &no_read_error, &type_is_known, &setting_name, &setting_value]<typename K, char V>() {
            if (setting_name.empty())
               return lu::foreach_lambda_result::stop;
            if (setting_name[0] == V || setting_name[0] == (V - 0x20)) {
               type_is_known = true;
               if constexpr (std::is_same_v<K, bool>) {
                  uint32_t dword = 0;
                  no_read_error = subrecord.read(dword);
                  setting_value.emplace<K>() = dword;
               } else {
                  no_read_error = subrecord.read(setting_value.emplace<K>());
               }
               return lu::foreach_lambda_result::stop;
            }
            return lu::foreach_lambda_result::keep_going;
         });
         if (callbacks.on_warning) {
            if (!no_read_error) {
               (callbacks.on_warning)(
                  notices::file_parse_warnings::game_setting_value_unreadable(
                     source_file.path().filename().string(),
                     std::string_view(setting_name),
                     record.formID(),
                     true
                  )
               );
            } else if (type_is_known && subrecord.is_in_bounds()) {
               (callbacks.on_warning)(
                  notices::file_parse_warnings::game_setting_value_has_extra_content(
                     source_file.path().filename().string(),
                     std::string_view(setting_name),
                     record.formID(),
                     subrecord.position(),
                     subrecord.size()
                  )
               );
            } else if (!type_is_known) {
               if (!setting_name.empty()) { // if name is empty, we already logged a "misordered" warning
                  (callbacks.on_warning)(
                     notices::file_parse_warnings::game_setting_value_type_unknown(
                        source_file.path().filename().string(),
                        std::string_view(setting_name),
                        record.formID()
                     )
                  );
               }
            }
         }
         break;
      }
      if (!found_data) {
         if (callbacks.on_warning) {
            (callbacks.on_warning)(
               notices::file_parse_warnings::game_setting_value_unreadable(
                  source_file.path().filename().string(),
                  std::string_view(setting_name),
                  record.formID(),
                  false
               )
            );
         }
         //
         // Ensure that we have a dummy value to use, since in Debug, `_store_game_setting` 
         // validates that the setting name and type match.
         //
         if (!setting_name.empty()) {
            facts::game_setting_types<true>::for_each_pair([&setting_name, &setting_value]<typename K, char V>() {
               if (setting_name.empty())
                  return lu::foreach_lambda_result::stop;
               if (setting_name[0] == V || setting_name[0] == (V - 0x20)) {
                  setting_value.emplace<K>();
                  return lu::foreach_lambda_result::stop;
               }
               return lu::foreach_lambda_result::keep_going;
            });
         }
      }

      // Can't quite separate this into the GSC only: the `form_stub_collection` needs to store 
      // dummy stubs marking form IDs occupied by game settings!
      this->_config.process._commit_game_setting(
         {},
         record.file_offset_of_header(),
         record.header(),
         std::move(setting_name),
         std::move(setting_value)
      );
   }
}