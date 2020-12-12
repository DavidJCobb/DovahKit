#include "threads.h"
#include "file_loader.h"
#include "../../form_stub.h"
#include "../../logging.h"
#include "../../notice_code_list.h"

namespace dovah::tes_file_reading::threads {
   #pragma region basic
   void basic::exec() {
      auto& lo = this->get_load_order();
      //
      auto size = this->queue.size();
      this->progress.maximum = size;
      for (uint32_t i = 0; i < size; i++) {
         if (this->owner.is_aborted()) {
            dovah::logging::print_line("[dovah::tes_file_reading::threads::basic] Thread %08X aborting as requested by owning file.", std::this_thread::get_id());
            break;
         }
         this->progress.current = i;
         auto& desired = this->queue[i];
         this->set_position(desired.pos);
         this->reset_parse_state();
         assert(this->next_record_or_group() == object_type::group); // TODO: error instead
         object_type ot;
         uint32_t    lastGroupLabel = 0; // for debug logging
         uint32_t    lastSignature  = 0; // shortcut to reduce the number of form type lookups we need
         form_type_t lastFormType   = 0;
         while (ot = this->next_record_or_group(), ot != object_type::none) {
            if (ot == object_type::group) {
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
               if (this->_groups[1].exists()) {
                  detailed_notice error;
                  error.code = notice_code::unexpected_nested_group_in_simple_top_group;
                  error.set_file_offset(this->get_position());
                  this->log_load_error(error);
                  this->owner.abort();
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
               form_type_t formType = lastFormType;
               if (!formType) // probably shouldn't happen; invalid signatures should cause errors
                  continue;
               //
               auto* stub = this->make_stub_for_record();
               stub->groupInfo.type = (int)group.header.type;
               if (stub->formType == form_type::topic_info) {
                  uint32_t topicID = group.getRawIDOfParentTopic();
                  if (topicID) {
                     lo.local_formID_to_global_formID(&this->owner, topicID);
                     stub->groupInfo.parentFormID = topicID;
                  } else
                     dovah::logging::print_line("[dovah::tes_file_reading::threads::basic:%s] TopicInfo %08X is not in a topic?", this->owner.get_filename(), stub->formID);
               }
               if (!this->commit_stub(*stub)) { // also normalizes (stub->formID)
                  delete stub;
                  continue;
               }
               this->extract_high_value_subrecords_for_stub(*stub);
               //
               if (&group == &this->_groups[0]) { // is this a top-level group?
                  uint32_t group_signature = _byteswap_ulong(group.header.label);
                  if (record.signature() != group_signature) { // misplaced record?
                     form_type_t group_type = form_type_info::signature_to_form_type(group_signature);
                     //
                     detailed_notice warning;
                     warning.code       = notice_code::record_found_in_wrong_top_level_group;
                     warning.cause_file = this->owner.get_filename();
                     warning.set_flag(detailed_notice::flag::has_cause_file);
                     warning.set_cause_form(*stub);
                     warning.set_cause_signature(group_signature);
                     warning.set_cause_form_type(group_type);
                     //
                     this->get_load_order().log_load_warning(warning);
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
         if (this->owner.is_aborted()) {
            dovah::logging::print_line("[dovah::tes_file_reading::threads::basic] Thread %08X aborting as requested by owning file.", std::this_thread::get_id());
            break;
         }
         this->progress.current = i;
         auto& desired = this->queue[i];
         this->set_position(desired.pos);
         this->reset_parse_state();
         assert(this->next_record_or_group() == object_type::group); // TODO: error instead
         object_type ot;
         uint32_t    lastGroupLabel = 0; // for debug logging
         uint32_t    lastSignature  = 0; // shortcut to reduce the number of form type lookups we need
         form_type_t lastFormType   = 0;
         while (ot = this->next_record_or_group(), ot != object_type::none) {
            if (ot == object_type::group) {
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
               form_type_t formType = lastFormType;
               if (!formType) // probably shouldn't happen; invalid signatures should cause errors
                  continue;
               //
               auto* stub = this->make_stub_for_record();
               stub->groupInfo.type = (int)group.header.type;
               if (stub->formType == form_type::topic_info) {
                  uint32_t topicID = group.getRawIDOfParentTopic();
                  if (topicID) {
                     lo.local_formID_to_global_formID(&this->owner, topicID);
                     stub->groupInfo.parentFormID = topicID;
                  } else
                     dovah::logging::print_line("[dovah::tes_file_reading::threads::basic:%s] TopicInfo %08X is not in a topic?", this->owner.get_filename(), stub->formID);
               }
               if (!this->commit_stub(*stub)) { // also normalizes (stub->formID)
                  delete stub;
                  continue;
               }
               this->extract_high_value_subrecords_for_stub(*stub);
               //
               if (&group == &this->_groups[0]) { // is this a top-level group?
                  uint32_t group_signature = _byteswap_ulong(group.header.label);
                  if (record.signature() != group_signature) { // misplaced record?
                     form_type_t group_type = form_type_info::signature_to_form_type(group_signature);
                     //
                     detailed_notice warning;
                     warning.code = notice_code::record_found_in_wrong_top_level_group;
                     warning.set_cause_form(*stub);
                     warning.set_cause_signature(group_signature);
                     warning.set_cause_form_type(group_type);
                     //
                     this->log_load_warning(warning);
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
         if (this->owner.is_aborted()) {
            dovah::logging::print_line("[dovah::tes_file_reading::threads::interior_cell] Thread %08X aborting as requested by owning file.", std::this_thread::get_id());
            break;
         }
         this->progress.current = i;
         auto& desired = this->queue[i];
         //dovah::logging::print_line("[dovah::tes_file_reading::threads::interior_cell] Thread %08X beginning with interior-cell-block %d at position %08X.", std::this_thread::get_id(), desired.blockNumber, desired.pos);
         this->set_position(desired.pos);
         this->reset_parse_state();
         assert(this->next_record_or_group() == object_type::group);
         object_type ot;
         uint32_t   lastBlockNumber = 0; // for debug logging
         uint32_t   lastSignature = 0; // shortcut to reduce the number of form type lookups we need
         form_type_t lastFormType  = 0;
         while (ot = this->next_record_or_group(), ot != object_type::none) {
            if (ot == object_type::group) {
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
               form_type_t formType = lastFormType;
               if (!formType)
                  continue;
               //
               auto* stub = this->make_stub_for_record();
               stub->groupInfo.type = (int)group.header.type;
               if (form_type_info::form_type_is_reference(stub->formType)) {
                  uint32_t cellID = group.getRawIDOfParentCell();
                  if (cellID) {
                     lo.local_formID_to_global_formID(&this->owner, cellID);
                     stub->groupInfo.parentFormID = cellID;
                  } else
                     dovah::logging::print_line("[dovah::tes_file_reading::threads::interior_cell:%s] Reference %08X is not in a cell?", this->owner.get_filename(), stub->formID);
               }
               if (!this->commit_stub(*stub)) { // also normalizes (stub->formID)
                  delete stub;
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
         if (this->owner.is_aborted()) {
            dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_sub_block] Thread %08X aborting as requested by owning file.", std::this_thread::get_id());
            break;
         }
         this->progress.current = i;
         auto& desired = this->queue[i];
         //dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_sub_block] Thread %08X beginning with [WRLD:%08X]/(%d, %d)/(%d, %d) at position %08X.", std::this_thread::get_id(), desired.worldspaceID, desired.blockX, desired.blockY, desired.subBlockX, desired.subBlockY, desired.pos);
         this->set_position(desired.pos);
         this->reset_parse_state();
         assert(this->next_record_or_group() == object_type::group);
         object_type ot;
         uint32_t   lastSignature = 0; // shortcut to reduce the number of form type lookups we need
         form_type_t lastFormType = 0;
         while (ot = this->next_record_or_group(), ot != object_type::none) {
            if (ot == object_type::group) {
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
               form_type_t formType = lastFormType;
               if (!formType)
                  continue;
               //
               auto* stub = this->make_stub_for_record();
               stub->groupInfo.type = (int)group.header.type;
               if (record.signature() == 'CELL') {
                  stub->groupInfo.parentFormID = desired.worldspaceID; // already normalized
               } else if (form_type_info::form_type_is_reference(stub->formType)) {
                  uint32_t cellID = group.getRawIDOfParentCell();
                  if (cellID) {
                     lo.local_formID_to_global_formID(&this->owner, cellID);
                     stub->groupInfo.parentFormID = cellID;
                  } else
                     dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_sub_block:%s] Reference %08X is not in a cell?", this->owner.get_filename(), stub->formID);
               }
               if (!this->commit_stub(*stub)) { // also normalizes (stub->formID)
                  delete stub;
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
         if (this->owner.is_aborted()) {
            dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_persistent_cell_children] Thread %08X aborting as requested by owning file.", std::this_thread::get_id());
            break;
         }
         this->progress.current = i;
         auto& desired = this->queue[i];
         //dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_persistent_cell_children] Thread %08X beginning with [WRLD:%08X]/(%d, %d)/(%d, %d) at position %08X.", std::this_thread::get_id(), desired.worldspaceID, desired.blockX, desired.blockY, desired.subBlockX, desired.subBlockY, desired.pos);
         this->set_position(desired.pos);
         this->reset_parse_state();
         assert(this->next_record_or_group() == object_type::group);
         object_type ot;
         uint32_t   lastSignature = 0; // shortcut to reduce the number of form type lookups we need
         form_type_t lastFormType = 0;
         while (ot = this->next_record_or_group(), ot != object_type::none) {
            if (ot == object_type::group) {
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
               form_type_t formType = lastFormType;
               if (!formType)
                  continue;
               //
               auto* stub = this->make_stub_for_record();
               stub->groupInfo.type = (int)group.header.type;
               if (form_type_info::form_type_is_reference(stub->formType)) {
                  uint32_t cellID = group.getRawIDOfParentCell();
                  if (cellID) {
                     lo.local_formID_to_global_formID(&this->owner, cellID);
                     stub->groupInfo.parentFormID = cellID;
                  } else
                     dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_persistent_cell_children:%s] Reference %08X is not in a cell?", this->owner.get_filename(), stub->formID);
               }
               if (!this->commit_stub(*stub)) { // also normalizes (stub->formID)
                  delete stub;
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
         if (this->owner.is_aborted()) {
            dovah::logging::print_line("[dovah::tes_file_reading::threads::game_setting] Thread %08X aborting as requested by owning file.", std::this_thread::get_id());
            break;
         }
         this->progress.current = i;
         auto& desired = this->queue[i];
         this->set_position(desired.pos);
         this->reset_parse_state();
         assert(this->next_record_or_group() == object_type::group); // TODO: error instead
         object_type ot;
         while (ot = this->next_record_or_group(), ot != object_type::none) {
            if (ot == object_type::group) {
               //
               // Stop if we've reached the end of the group we're meant to parse.
               //
               auto& first = this->_groups[0];
               if (!first.exists() || first.pos != desired.pos)
                  break;
               if (this->_groups[1].exists()) {
                  detailed_notice error;
                  error.code = notice_code::unexpected_nested_group_in_simple_top_group;
                  error.set_file_offset(this->get_position());
                  this->log_load_error(error); // also aborts the load
                  break;
               }
            }
            if (ot == object_type::record) {
               auto& record = this->get_current_record();
               auto& group  = this->get_current_group();
               if (record.signature() != 'GMST') { // misplaced record
                  detailed_notice error;
                  error.code = notice_code::record_found_in_wrong_top_level_group;
                  error.set_flag(detailed_notice::flag::has_cause_file);
                  error.cause_form.localID = record.formID();
                  error.cause_form.type    = form_type_info::signature_to_form_type(record.signature());
                  error.set_flag(detailed_notice::flag::has_cause_form);
                  error.set_cause_signature('GMST');
                  error.set_cause_form_type(form_type::setting);
                  error.set_file_offset(this->get_position());
                  this->log_load_error(error); // also aborts the load
                  break;
               }
               //
               bool found_data = false;
               loaded_game_setting working;
               auto& EDID = record.next_subrecord();
               if (EDID.signature() == 'EDID') {
                  std::string name;
                  EDID.to_string(name);
                  working.name = name;
                  working.definition = &game_setting_definition::lookup(name.c_str());
               } else {
                  found_data = EDID.signature() == 'DATA';
               }
               while (auto& subrecord = record.next_subrecord()) {
                  if (subrecord.signature() == 'EDID') {
                     detailed_notice warning;
                     warning.code               = notice_code::game_setting_record_is_misordered;
                     warning.cause_form.localID = record.formID();
                     warning.cause_form.fixedID = 0;
                     warning.cause_form.type    = form_type::setting;
                     warning.set_flag(detailed_notice::flag::has_cause_form);
                     warning.cause_file = this->owner.get_filename();
                     warning.set_flag(detailed_notice::flag::has_cause_file);
                     warning.set_cause_editor_id(working.name);
                     this->log_load_warning(warning);
                     continue;
                  }
                  if (subrecord.signature() != 'DATA') {
                     detailed_notice warning;
                     warning.code               = notice_code::unrecognized_subrecord;
                     warning.cause_form.localID = record.formID();
                     warning.cause_form.fixedID = 0;
                     warning.cause_form.type    = form_type::setting;
                     warning.cause_file         = this->owner.get_filename();
                     warning.set_flag(detailed_notice::flag::has_cause_form | detailed_notice::flag::has_cause_file);
                     warning.set_cause_subrecord(subrecord);
                     warning.set_cause_editor_id(working.name);
                     this->log_load_warning(warning);
                     //
                     continue;
                  }
                  found_data = true;
                  //
                  bool no_read_error = true;
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
                        no_read_error = subrecord.to_string(working.value.s);
                        break;
                     default:
                        {
                           detailed_notice warning;
                           warning.code               = notice_code::game_setting_record_has_bad_type;
                           warning.cause_form.localID = record.formID();
                           warning.cause_form.fixedID = 0;
                           warning.cause_form.type    = form_type::setting;
                           warning.cause_file         = this->owner.get_filename();
                           warning.set_flag(detailed_notice::flag::has_cause_form | detailed_notice::flag::has_cause_file);
                           warning.set_cause_editor_id(working.name);
                           this->log_load_warning(warning);
                        }
                        break;
                  }
                  if (!no_read_error) {
                     detailed_notice warning;
                     warning.code               = notice_code::game_setting_record_unreadable_data;
                     warning.cause_form.localID = record.formID();
                     warning.cause_form.fixedID = 0;
                     warning.cause_form.type    = form_type::setting;
                     warning.cause_file         = this->owner.get_filename();
                     warning.set_flag(detailed_notice::flag::has_cause_form | detailed_notice::flag::has_cause_file);
                     warning.set_cause_subrecord(subrecord.signature());
                     warning.set_cause_editor_id(working.name);
                     this->log_load_warning(warning);
                  }
                  if (!subrecord.is_at_end()) {
                     detailed_notice warning;
                     warning.code               = notice_code::subrecord_has_extra_content;
                     warning.cause_form.localID = record.formID();
                     warning.cause_form.fixedID = 0;
                     warning.cause_form.type    = form_type::setting;
                     warning.cause_file         = this->owner.get_filename();
                     warning.set_flag(detailed_notice::flag::has_cause_form | detailed_notice::flag::has_cause_file);
                     warning.set_cause_subrecord(subrecord.signature());
                     warning.set_cause_editor_id(working.name);
                     this->log_load_warning(warning);
                  }
                  break;
               }
               if (!found_data) {
                  detailed_notice warning;
                  warning.code               = notice_code::game_setting_record_has_no_data;
                  warning.cause_form.localID = record.formID();
                  warning.cause_form.fixedID = 0;
                  warning.cause_form.type    = form_type::setting;
                  warning.cause_file         = this->owner.get_filename();
                  warning.set_flag(detailed_notice::flag::has_cause_form | detailed_notice::flag::has_cause_file);
                  warning.set_cause_editor_id(working.name);
                  this->log_load_warning(warning);
               }
               lo.accept_game_setting(&this->owner, working, record.formID());
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