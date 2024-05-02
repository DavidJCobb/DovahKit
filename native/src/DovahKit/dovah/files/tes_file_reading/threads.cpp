#include "threads.h"
#include "file_loader.h"
#include "../../form_stub.h"
#include "../../logging.h"

#include "../../exceptions/file_load_failed.h"
#include "../../notices/file_load_errors/form_record_present_in_game_setting_group.h"
#include "../../notices/file_load_errors/unexpected_nested_group_in_simple_top_group.h"
#include "../../notices/file_load_warnings/game_setting_record_has_unrecognized_subrecord.h"
#include "../../notices/file_load_warnings/game_setting_record_is_misordered.h"
#include "../../notices/file_load_warnings/game_setting_value_has_extra_content.h"
#include "../../notices/file_load_warnings/game_setting_value_type_unknown.h"
#include "../../notices/file_load_warnings/game_setting_value_unreadable.h"
#include "../../notices/file_load_warnings/record_found_in_wrong_top_level_group.h"

namespace dovah::tes_file_reading::threads {
   #pragma region basic
   void basic::exec() {
      auto& lo = this->get_load_order();
      //
      auto size = this->queue.size();
      this->progress.maximum = size;
      for (uint32_t i = 0; i < size; i++) {
         this->progress.current = i;
         auto& desired = this->queue[i];
         this->set_position(desired.pos);
         this->reset_parse_state();
         assert(this->next_record_or_group() == object_type::group); // TODO: error instead
         object_type ot;
         uint32_t    lastGroupLabel = 0; // for debug logging
         uint32_t    lastSignature  = 0; // shortcut to reduce the number of form type lookups we need
         form_type   lastFormType   = dovah::form_type::none;
         while (ot = this->next_record_or_group(), ot != object_type::none) {
            {
               //
               // Stop if we've reached the end of the group we're meant to parse.
               //
               auto& first = this->_groups[0];
               if (!first || first.pos != desired.pos) {
                  //char sig_buffer[5];
                  //dovah::logging::print_line("[dovah::tes_file_reading::threads::basic] Thread %08X finished parse of group %s.", std::this_thread::get_id(), FMT_SIGNATURE(lastGroupLabel, sig_buffer));
                  break;
               }
            }
            if (ot == object_type::group) {
               lastGroupLabel = _byteswap_ulong(this->_groups[0].header.label);
               //
               if (this->_groups[1].exists()) {
                  auto error = std::make_unique<dovah::notices::file_load_errors::unexpected_nested_group_in_simple_top_group>();
                  auto ex    = dovah::exceptions::file_load_failed();
         
                  error->filename    = this->loader->get_filename();
                  error->file_offset = this->get_position();
         
                  ex.details.file_load_error = std::move(error);
                  throw ex;
               }
            }
            if (ot == object_type::record) {
               auto& record = this->get_current_record();
               auto& group  = this->get_current_group();
               if (record.signature() != lastSignature) {
                  lastSignature = record.signature();
                  lastFormType  = form_type_info::signature_to_form_type(lastSignature);
               }
               form_type formType = lastFormType;
               if (formType == dovah::form_type::none) // probably shouldn't happen; invalid signatures should cause errors
                  continue;
               //
               auto* stub = this->make_stub_for_record();
               if (stub->form_type == form_type::topic_info) {
                  uint32_t topicID = group.getRawIDOfParentTopic();
                  lo.local_formID_to_global_formID(this->loader, topicID);
                  this->set_stub_parent(stub, topicID);
                  if (!topicID)
                     dovah::logging::print_line("[dovah::tes_file_reading::threads::basic:%s] TopicInfo %08X is not in a topic?", this->loader->get_filename(), stub->formID);
               }
               if (!this->commit_stub(stub)) {
                  continue;
               }
               this->extract_high_value_subrecords_for_stub(*stub);
               //
               if (&group == &this->_groups[0]) { // is this a top-level group?
                  uint32_t group_signature = _byteswap_ulong(group.header.label);
                  if (record.signature() != group_signature) { // misplaced record?
                     form_type group_type = form_type_info::signature_to_form_type(group_signature);
                     
                     notices::file_load_warnings::record_found_in_wrong_top_level_group notice;
                     notice.source_file = this->loader->get_filename();
                     notice.top_level_group_label = group_signature;
                     notice.record = {
                        .local_id  = record.formID(),
                        .global_id = stub->formID,
                        .signature = record.signature(),
                     };
                     //
                     this->get_file_loader().get_load_interface(*this).log_warning(notice);
                  }
               }
               //
               continue;
            }
         }
      }
      //
      dovah::logging::print_line("[dovah::tes_file_reading::threads::basic] Thread %08X finished all of its work (%d queued entries).", std::this_thread::get_id(), this->queue.size());
   }
   void basic::add_group(uint32_t signature, uint32_t pos) {
      this->queue.emplace_back(signature, pos);
   }
   #pragma endregion
   
   #pragma region dialogue
   void dialogue::exec() {
      auto& lo = this->get_load_order();
      //
      auto size = this->queue.size();
      this->progress.maximum = size;
      for (uint32_t i = 0; i < size; i++) {
         this->progress.current = i;
         auto& desired = this->queue[i];
         this->set_position(desired.pos);
         this->reset_parse_state();
         assert(this->next_record_or_group() == object_type::group); // TODO: error instead
         object_type ot;
         uint32_t    lastGroupLabel = 0; // for debug logging
         uint32_t    lastSignature  = 0; // shortcut to reduce the number of form type lookups we need
         form_type   lastFormType   = dovah::form_type::none;
         while (ot = this->next_record_or_group(), ot != object_type::none) {
            {
               //
               // Stop if we've reached the end of the group we're meant to parse.
               //
               auto& first = this->_groups[0];
               if (first.exists() && first.pos == desired.pos) {
                  lastGroupLabel = _byteswap_ulong(first.header.label);
               } else {
                  //char sig_buffer[5];
                  //dovah::logging::print_line("[dovah::tes_file_reading::threads::basic] Thread %08X finished parse of group %s.", std::this_thread::get_id(), FMT_SIGNATURE(lastGroupLabel, sig_buffer));
                  break;
               }
            }
            if (ot == object_type::record) {
               auto& record = this->get_current_record();
               auto& group  = this->get_current_group();
               if (record.signature() != lastSignature) {
                  lastSignature = record.signature();
                  lastFormType  = form_type_info::signature_to_form_type(lastSignature);
               }
               form_type formType = lastFormType;
               if (formType == dovah::form_type::none) // probably shouldn't happen; invalid signatures should cause errors
                  continue;
               //
               auto* stub = this->make_stub_for_record();
               if (stub->form_type == form_type::topic_info) {
                  uint32_t topicID = group.getRawIDOfParentTopic();
                  lo.local_formID_to_global_formID(this->loader, topicID);
                  this->set_stub_parent(stub, topicID);
                  if (!topicID)
                     dovah::logging::print_line("[dovah::tes_file_reading::threads::basic:%s] TopicInfo %08X is not in a topic?", this->loader->get_filename(), stub->formID);
               }
               if (!this->commit_stub(stub)) {
                  continue;
               }
               this->extract_high_value_subrecords_for_stub(*stub);
               //
               if (&group == &this->_groups[0]) { // is this a top-level group?
                  uint32_t group_signature = _byteswap_ulong(group.header.label);
                  if (record.signature() != group_signature) { // misplaced record?
                     form_type group_type = form_type_info::signature_to_form_type(group_signature);

                     notices::file_load_warnings::record_found_in_wrong_top_level_group notice;
                     notice.top_level_group_label = group_signature;
                     notice.record = {
                        .local_id  = record.formID(),
                        .global_id = stub->formID,
                        .signature = record.signature(),
                     };
                     //
                     this->get_file_loader().get_load_interface(*this).log_warning(notice);
                  }
               }
               //
               continue;
            }
         }
      }
      //
      dovah::logging::print_line("[dovah::tes_file_reading::threads::basic] Thread %08X finished all of its work (%d queued entries).", std::this_thread::get_id(), this->queue.size());
   }
   void dialogue::add_group(uint32_t signature, uint32_t pos) {
      this->queue.emplace_back(signature, pos);
   }
   #pragma endregion

   #pragma region interior_cell
   void interior_cell::exec() {
      auto& lo = this->get_load_order();
      //
      auto size = this->queue.size();
      this->progress.maximum = size;
      for (uint32_t i = 0; i < size; i++) {
         this->progress.current = i;
         auto& desired = this->queue[i];
         //dovah::logging::print_line("[dovah::tes_file_reading::threads::interior_cell] Thread %08X beginning with interior-cell-block %d at position %08X.", std::this_thread::get_id(), desired.blockNumber, desired.pos);
         this->set_position(desired.pos);
         this->reset_parse_state();
         assert(this->next_record_or_group() == object_type::group);
         object_type ot;
         uint32_t    lastBlockNumber = 0; // for debug logging
         uint32_t    lastSignature   = 0; // shortcut to reduce the number of form type lookups we need
         form_type   lastFormType    = dovah::form_type::none;
         while (ot = this->next_record_or_group(), ot != object_type::none) {
            {
               //
               // Stop if we've reached the end of the group we're meant to parse.
               //
               auto& first = this->_groups[0];
               if (first && first.pos == desired.pos) {
                  lastBlockNumber = first.header.label;
               } else {
                  //dovah::logging::print_line("[dovah::tes_file_reading::threads::interior_cell] Thread %08X finished parse of interior-cell block %d.", std::this_thread::get_id(), lastBlockNumber);
                  break;
               }
            }
            if (ot == object_type::record) {
               auto& record = this->get_current_record();
               auto& group  = this->get_current_group();
               if (record.signature() != lastSignature) {
                  lastSignature = record.signature();
                  lastFormType  = form_type_info::signature_to_form_type(lastSignature);
               }
               form_type formType = lastFormType;
               if (formType == dovah::form_type::none)
                  continue;
               //
               auto* stub = this->make_stub_for_record();
               if (form_type_is_reference(stub->form_type)) {
                  uint32_t cellID = group.getRawIDOfParentCell();
                  lo.local_formID_to_global_formID(this->loader, cellID);
                  this->set_stub_parent(stub, cellID);
                  if (!cellID)
                     dovah::logging::print_line("[dovah::tes_file_reading::threads::interior_cell:%s] Reference %08X is not in a cell?", this->loader->get_filename(), stub->formID);
               }
               if (!this->commit_stub(stub)) {
                  continue;
               }
               this->extract_high_value_subrecords_for_stub(*stub);
               continue;
            }
         }
      }
      //
      dovah::logging::print_line("[dovah::tes_file_reading::threads::interior_cell] Thread %08X finished all of its work (%d queued entries).", std::this_thread::get_id(), this->queue.size());
   }
   void interior_cell::add_group(uint32_t blockNumber, uint32_t pos) {
      this->queue.emplace_back(blockNumber, pos);
   }
   #pragma endregion

   #pragma region worldspace_sub_block
   void worldspace_sub_block::exec() {
      auto& lo = this->get_load_order();
      //
      auto size = this->queue.size();
      this->progress.maximum = size;
      for (uint32_t i = 0; i < size; i++) {
         this->progress.current = i;
         auto& desired = this->queue[i];
         //dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_sub_block] Thread %08X beginning with [WRLD:%08X]/(%d, %d)/(%d, %d) at position %08X.", std::this_thread::get_id(), desired.worldspaceID, desired.blockX, desired.blockY, desired.subBlockX, desired.subBlockY, desired.pos);
         this->set_position(desired.pos);
         this->reset_parse_state();
         assert(this->next_record_or_group() == object_type::group);
         object_type ot;
         uint32_t    lastSignature = 0; // shortcut to reduce the number of form type lookups we need
         form_type   lastFormType  = dovah::form_type::none;
         while (ot = this->next_record_or_group(), ot != object_type::none) {
            {
               //
               // Stop if we've reached the end of the group we're meant to parse.
               //
               auto& first = this->_groups[0];
               if (!first || first.pos != desired.pos) {
                  //dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_sub_block] Thread %08X finished parse of [WRLD:%08X]/(%d, %d)/(%d, %d) at position %08X.", std::this_thread::get_id(), desired.worldspaceID, desired.blockX, desired.blockY, desired.subBlockX, desired.subBlockY, desired.pos);
                  break;
               }
            }
            if (ot == object_type::record) {
               auto& record = this->get_current_record();
               auto& group  = this->get_current_group();
               if (record.signature() != lastSignature) {
                  lastSignature = record.signature();
                  lastFormType  = form_type_info::signature_to_form_type(lastSignature);
               }
               form_type formType = lastFormType;
               if (formType == dovah::form_type::none)
                  continue;
               //
               auto* stub = this->make_stub_for_record();
               if (record.signature() == 'CELL') {
                  this->set_stub_parent(stub, desired.worldspaceID);
               }
               switch (group.header.type) {
                  case tes_file_group_type::cell_children:
                  case tes_file_group_type::cell_persistent_children:
                  case tes_file_group_type::cell_temporary_children:
                     {
                        uint32_t cellID = group.getRawIDOfParentCell();
                        lo.local_formID_to_global_formID(this->loader, cellID);
                        this->set_stub_parent(stub, cellID);
                        if (!cellID)
                           dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_sub_block:%s] Form %08X is not in a cell?", this->loader->get_filename(), stub->formID);
                     }
                     break;
               }
               if (!this->commit_stub(stub)) {
                  continue;
               }
               this->extract_high_value_subrecords_for_stub(*stub);
               continue;
            }
         }
      }
      //
      dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_sub_block] Thread %08X finished all of its work (%d queued entries).", std::this_thread::get_id(), this->queue.size());
   }
   void worldspace_sub_block::add_group(uint32_t worldID, int16_t bx, int16_t by, int16_t sbx, int16_t sby, uint32_t pos) {
      this->queue.emplace_back(worldID, bx, by, sbx, sby, pos);
   }
   #pragma endregion

   #pragma region worldspace_persistent_cell_children
   void worldspace_persistent_cell_children::exec() {
      auto& lo = this->get_load_order();
      //
      auto size = this->queue.size();
      this->progress.maximum = size;
      for (uint32_t i = 0; i < size; i++) {
         this->progress.current = i;
         auto& desired = this->queue[i];
         //dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_persistent_cell_children] Thread %08X beginning with [WRLD:%08X]/(%d, %d)/(%d, %d) at position %08X.", std::this_thread::get_id(), desired.worldspaceID, desired.blockX, desired.blockY, desired.subBlockX, desired.subBlockY, desired.pos);
         this->set_position(desired.pos);
         this->reset_parse_state();
         assert(this->next_record_or_group() == object_type::group);
         object_type ot;
         uint32_t    lastSignature = 0; // shortcut to reduce the number of form type lookups we need
         form_type   lastFormType  = dovah::form_type::none;
         while (ot = this->next_record_or_group(), ot != object_type::none) {
            {
               //
               // Stop if we've reached the end of the group we're meant to parse.
               //
               auto& first = this->_groups[0];
               if (!first || first.pos != desired.pos) {
                  //dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_persistent_cell_children] Thread %08X finished parse of [WRLD:%08X]/(%d, %d)/(%d, %d) at position %08X.", std::this_thread::get_id(), desired.worldspaceID, desired.blockX, desired.blockY, desired.subBlockX, desired.subBlockY, desired.pos);
                  break;
               }
            }
            if (ot == object_type::record) {
               auto& record = this->get_current_record();
               auto& group  = this->get_current_group();
               if (record.signature() != lastSignature) {
                  lastSignature = record.signature();
                  lastFormType  = form_type_info::signature_to_form_type(lastSignature);
               }
               form_type formType = lastFormType;
               if (formType == dovah::form_type::none)
                  continue;
               //
               auto* stub = this->make_stub_for_record();
               if (form_type_is_reference(stub->form_type)) {
                  uint32_t cellID = group.getRawIDOfParentCell();
                  lo.local_formID_to_global_formID(this->loader, cellID);
                  this->set_stub_parent(stub, cellID);
                  if (!cellID)
                     dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_persistent_cell_children:%s] Reference %08X is not in a cell?", this->loader->get_filename(), stub->formID);
               }
               if (!this->commit_stub(stub)) {
                  continue;
               }
               this->extract_high_value_subrecords_for_stub(*stub);
               continue;
            }
         }
      }
      //
      dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_persistent_cell_children] Thread %08X finished all of its work (%d queued entries).", std::this_thread::get_id(), this->queue.size());
   }
   void worldspace_persistent_cell_children::add_group(uint32_t cellID, uint32_t pos) {
      this->queue.emplace_back(cellID, pos);
   }
   #pragma endregion
         
   #pragma region game_setting
   void game_setting::exec() {
      auto& lo = this->get_load_order();
      //
      auto size = this->queue.size();
      this->progress.maximum = size;
      for (uint32_t i = 0; i < size; i++) {
         this->progress.current = i;
         auto& desired = this->queue[i];
         this->set_position(desired.pos);
         this->reset_parse_state();
         assert(this->next_record_or_group() == object_type::group); // TODO: error instead
         object_type ot;
         while (ot = this->next_record_or_group(), ot != object_type::none) {
            {
               //
               // Stop if we've reached the end of the group we're meant to parse.
               //
               auto& first = this->_groups[0];
               if (!first.exists() || first.pos != desired.pos)
                  break;
               if (this->_groups[1].exists()) {
                  auto error = std::make_unique<dovah::notices::file_load_errors::unexpected_nested_group_in_simple_top_group>();
                  auto ex    = dovah::exceptions::file_load_failed();
         
                  error->filename    = this->loader->get_filename();
                  error->file_offset = this->get_position();
         
                  ex.details.file_load_error = std::move(error);
                  throw ex;
               }
            }
            if (ot == object_type::record) {
               auto& record = this->get_current_record();
               auto& group  = this->get_current_group();
               if (record.signature() != 'GMST') { // misplaced record
                  auto error = std::make_unique<dovah::notices::file_load_errors::form_record_present_in_game_setting_group>();
                  auto ex    = dovah::exceptions::file_load_failed();
         
                  error->filename    = this->loader->get_filename();
                  error->file_offset = this->get_position();
                  error->record.local_form_id = record.formID();
                  error->record.signature     = record.signature();
         
                  ex.details.file_load_error = std::move(error);
                  throw ex;
               }
               //
               bool found_data        = false;
               bool logged_misordered = false;
               loaded_game_setting working;
               auto& EDID = record.next_subrecord();
               if (EDID.signature() == 'EDID') {
                  std::string name;
                  EDID.read(name);
                  working.name = name;
                  working.definition = game_setting_definition::lookup(name.c_str());
               } else {
                  found_data = EDID.signature() == 'DATA';
                  if (found_data) {
                     logged_misordered = true;
                     //
                     notices::file_load_warnings::game_setting_record_is_misordered notice;
                     notice.setting_name    = working.name;
                     notice.record.local_id = record.formID();
                     this->log_load_warning(notice);
                  }
               }
               while (auto& subrecord = record.next_subrecord()) {
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
               lo.accept_game_setting(this->loader, working, record.formID());
               continue;
            }
         }
      }
      //
      dovah::logging::print_line("[dovah::tes_file_reading::threads::game_setting] Thread %08X finished all of its work (%d queued entries).", std::this_thread::get_id(), this->queue.size());
   }
   void game_setting::add_group(uint32_t pos) {
      this->queue.emplace_back(pos);
   }
   #pragma endregion
}