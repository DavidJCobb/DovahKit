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

#include "dovah/exceptions/file_parse_errors/record_fatally_misplaced.h"
#include "dovah/exceptions/file_parse_errors/unexpected_nested_group.h"

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
               // We've reached the end of the desired group.
               //
               break;
            }
            if (ot == tes::file_view::chunk_type::group) {
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
               if (record.signature() != form_type_info::lookup(form_type::setting).signature) {
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
      loaded_game_setting working;

      record.next_subrecord();
      if (subrecord.signature() == 'EDID') {
         std::string name;
         subrecord.read_all_to_string(name);
         working.name       = name;
         working.definition = facts::game_settings::lookup_by_name(name.c_str());
      } else {
         found_data = subrecord.signature() == 'DATA';
         if (found_data) {
            logged_misordered = true;
            //
            notices::file_load_warnings::game_setting_record_is_misordered notice;
            notice.setting_name    = working.name;
            notice.record.local_id = record.formID();
            this->log_load_warning(notice);
         }
      }
      while (record.next_subrecord()) {
         if (subrecord.signature() == 'EDID') {
            if (!logged_misordered) {
               notices::file_load_warnings::game_setting_record_is_misordered notice;
               notice.setting_name    = working.name;
               notice.record.local_id = record.formID();
               this->log_load_warning(notice);
            }
            continue;
         }
         if (subrecord.signature() != 'DATA') {
            notices::file_load_warnings::game_setting_record_has_unrecognized_subrecord notice;
            notice.setting_name        = working.name;
            notice.form_ids.local      = record.formID();
            notice.subrecord_signature = subrecord.signature();
            this->log_load_warning(notice);
            //
            continue;
         }
         found_data = true;
         //
         bool no_read_error = true;
         bool type_is_known = true;
         switch (working.get_type()) {
            case game_setting_type::boolean:
               {
                  uint32_t dummy;
                  no_read_error = subrecord.read(dummy);
                  working.value.b = dummy != 0;
               }
               break;
            case game_setting_type::float32:
               no_read_error = subrecord.read(working.value.f);
               break;
            case game_setting_type::integer:
               no_read_error = subrecord.read(working.value.i);
               break;
            case game_setting_type::string:
               no_read_error = subrecord.read(working.value.s);
               break;
            default:
               type_is_known = false;
               if (!working.name.empty()) {
                  notices::file_load_warnings::game_setting_value_type_unknown notice;
                  notice.setting_name = working.name;
                  notice.record = {
                     .local_id = record.formID(),
                  };
                  //
                  this->log_load_warning(notice);
               }
               break;
         }
         if (!no_read_error) {
            notices::file_load_warnings::game_setting_value_unreadable notice;
            notice.setting_name = working.name;
            notice.record = {
               .local_id = record.formID(),
            };
            notice.subrecord_is_present = true;
            //
            this->log_load_warning(notice);
         }
         if (type_is_known && !subrecord.is_at_end()) {
            notices::file_load_warnings::game_setting_value_has_extra_content notice;
            notice.setting_name  = working.name;
            notice.data_size     = subrecord.size();
            notice.expected_size = subrecord.offset();
            notice.record = {
               .local_id = record.formID(),
            };
            //
            this->log_load_warning(notice);
         }
         break;
      }
      if (!found_data) {
         notices::file_load_warnings::game_setting_value_unreadable notice;
         notice.setting_name = working.name;
         notice.record = {
            .local_id = record.formID(),
         };
         notice.subrecord_is_present = false;
         //
         this->log_load_warning(notice);
      }

      // Can't quite separate this into the GSC only: the `form_stub_collection` needs to store 
      // dummy stubs marking form IDs occupied by game settings!
      alo.get_game_setting_collection().accept_game_setting(source_file, working, record.formID());
   }
}