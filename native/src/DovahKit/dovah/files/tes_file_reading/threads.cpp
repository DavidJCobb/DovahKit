#include "threads.h"
#include "file.h"
#include "../../../helpers/strings.h"
#include "../../form_stub.h"
#include "../../logging.h"
#include "localized_string_file.h"

namespace dovah {
   namespace tes_file_reading {
      namespace threads {
         #pragma region basic
         void basic::_thread_handler(basic* instance) {
            instance->_load();
         }
         void basic::_load() {
            this->file = this->owner->file;
            auto& lo = this->owner->load_order;
            //
            auto size = this->queue.size();
            this->progress.maximum = size;
            for (uint32_t i = 0; i < size; i++) {
               if (this->owner->aborted) {
                  dovah::logging::print_line("[dovah::tes_file_reading::threads::basic] Thread %08X aborting as requested by owning file.", std::this_thread::get_id());
                  break;
               }
               this->progress.current = i;
               auto& desired = this->queue[i];
               this->setPos(desired.pos);
               this->resetParseState();
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
                     if (!this->allow_nested_groups && this->_groups[1].exists()) {
                        auto& error = this->owner->error;
                        //
                        error.code       = file_read_error::error_code::malformed_file;
                        error.file       = this->as_file()->get_filename();
                        error.fileOffset = this->getPos();
                        char sig[5];
                        cobb::sprintf(error.message, "Unexpected nested group within \"simple\" top-group %s.", dovah::logging::format_signature(desired.signature, sig));
                        //
                        this->owner->abort();
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
                     auto  stub = this->make_stub_for_record(*this->owner);
                     stub->groupInfo.type = (int)group.header.type;
                     if (stub->formType == form_type::topic_info) {
                        uint32_t topicID = group.getRawIDOfParentTopic();
                        if (topicID) {
                           lo.local_formID_to_global_formID(this->owner->as_file(), topicID);
                           stub->groupInfo.parentFormID = topicID;
                        } else
                           dovah::logging::print_line("[dovah::tes_file_reading::threads::basic:%s] TopicInfo %08X is not in a topic?", this->owner->as_file()->get_filename(), stub->formID);
                     }
                     if (!this->owner->_insert_form(stub->formID, stub)) { // also normalizes (stub->formID)
                        delete stub;
                        continue;
                     }
                     this->extract_high_value_subrecords_for_stub(stub);
                     continue;
                  }
               }
            }
            this->file = nullptr;
            //
            dovah::logging::print_line("[dovah::tes_file_reading::threads::basic] Thread %08X finished all of its work (%d queued entries).", std::this_thread::get_id(), this->queue.size());
         }
         void basic::add_group(uint32_t signature, uint32_t pos) {
            this->queue.emplace_back(signature, pos);
         }
         void basic::start() {
            this->thread = std::thread(basic::_thread_handler, this);
         }
         void basic::wait_for() {
            this->thread.join();
         }
         #pragma endregion

         #pragma region interior_cell
         void interior_cell::_thread_handler(interior_cell* instance) {
            instance->_load();
         }
         void interior_cell::_load() {
            this->file = this->owner->file;
            auto& lo = this->owner->load_order;
            //
            auto size = this->queue.size();
            this->progress.maximum = size;
            for (uint32_t i = 0; i < size; i++) {
               if (this->owner->aborted) {
                  dovah::logging::print_line("[dovah::tes_file_reading::threads::interior_cell] Thread %08X aborting as requested by owning file.", std::this_thread::get_id());
                  break;
               }
               this->progress.current = i;
               auto& desired = this->queue[i];
               //dovah::logging::print_line("[dovah::tes_file_reading::threads::interior_cell] Thread %08X beginning with interior-cell-block %d at position %08X.", std::this_thread::get_id(), desired.blockNumber, desired.pos);
               this->setPos(desired.pos);
               this->resetParseState();
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
                     auto  stub = this->make_stub_for_record(*this->owner);
                     stub->groupInfo.type = (int)group.header.type;
                     if (form_type_info::form_type_is_reference(stub->formType)) {
                        uint32_t cellID = group.getRawIDOfParentCell();
                        if (cellID) {
                           lo.local_formID_to_global_formID(this->owner->as_file(), cellID);
                           stub->groupInfo.parentFormID = cellID;
                        } else
                           dovah::logging::print_line("[dovah::tes_file_reading::threads::interior_cell:%s] Reference %08X is not in a cell?", this->owner->as_file()->get_filename(), stub->formID);
                     }
                     if (!this->owner->_insert_form(stub->formID, stub)) { // also normalizes (stub->formID)
                        delete stub;
                        continue;
                     }
                     this->extract_high_value_subrecords_for_stub(stub);
                     continue;
                  }
               }
            }
            this->file = nullptr;
            //
            dovah::logging::print_line("[dovah::tes_file_reading::threads::interior_cell] Thread %08X finished all of its work (%d queued entries).", std::this_thread::get_id(), this->queue.size());
         }
         void interior_cell::add_group(uint32_t blockNumber, uint32_t pos) {
            this->queue.emplace_back(blockNumber, pos);
         }
         void interior_cell::start() {
            this->thread = std::thread(interior_cell::_thread_handler, this);
         }
         void interior_cell::wait_for() {
            this->thread.join();
         }
         #pragma endregion

         #pragma region worldspace_sub_block
         void worldspace_sub_block::_thread_handler(worldspace_sub_block* instance) {
            instance->_load();
         }
         void worldspace_sub_block::_load() {
            this->file = this->owner->file;
            auto& lo = this->owner->load_order;
            //
            auto size = this->queue.size();
            this->progress.maximum = size;
            for (uint32_t i = 0; i < size; i++) {
               if (this->owner->aborted) {
                  dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_sub_block] Thread %08X aborting as requested by owning file.", std::this_thread::get_id());
                  break;
               }
               this->progress.current = i;
               auto& desired = this->queue[i];
               //dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_sub_block] Thread %08X beginning with [WRLD:%08X]/(%d, %d)/(%d, %d) at position %08X.", std::this_thread::get_id(), desired.worldspaceID, desired.blockX, desired.blockY, desired.subBlockX, desired.subBlockY, desired.pos);
               this->setPos(desired.pos);
               this->resetParseState();
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
                     auto  stub = this->make_stub_for_record(*this->owner);
                     stub->groupInfo.type = (int)group.header.type;
                     if (record.signature() == 'CELL') {
                        stub->groupInfo.parentFormID = desired.worldspaceID; // already normalized
                     } else if (form_type_info::form_type_is_reference(stub->formType)) {
                        uint32_t cellID = group.getRawIDOfParentCell();
                        if (cellID) {
                           lo.local_formID_to_global_formID(this->owner->as_file(), cellID);
                           stub->groupInfo.parentFormID = cellID;
                        } else
                           dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_sub_block:%s] Reference %08X is not in a cell?", this->owner->as_file()->get_filename(), stub->formID);
                     }
                     if (!this->owner->_insert_form(stub->formID, stub)) { // also normalizes (stub->formID)
                        delete stub;
                        continue;
                     }
                     this->extract_high_value_subrecords_for_stub(stub);
                     continue;
                  }
               }
            }
            this->file = nullptr;
            //
            dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_sub_block] Thread %08X finished all of its work (%d queued entries).", std::this_thread::get_id(), this->queue.size());
         }
         void worldspace_sub_block::add_group(uint32_t worldID, int16_t bx, int16_t by, int16_t sbx, int16_t sby, uint32_t pos) {
            this->queue.emplace_back(worldID, bx, by, sbx, sby, pos);
         }
         void worldspace_sub_block::start() {
            this->thread = std::thread(worldspace_sub_block::_thread_handler, this);
         }
         void worldspace_sub_block::wait_for() {
            this->thread.join();
         }
         #pragma endregion

         #pragma region worldspace_persistent_cell_children
         void worldspace_persistent_cell_children::_thread_handler(worldspace_persistent_cell_children* instance) {
            instance->_load();
         }
         void worldspace_persistent_cell_children::_load() {
            this->file = this->owner->file;
            auto& lo = this->owner->load_order;
            //
            auto size = this->queue.size();
            this->progress.maximum = size;
            for (uint32_t i = 0; i < size; i++) {
               if (this->owner->aborted) {
                  dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_persistent_cell_children] Thread %08X aborting as requested by owning file.", std::this_thread::get_id());
                  break;
               }
               this->progress.current = i;
               auto& desired = this->queue[i];
               //dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_persistent_cell_children] Thread %08X beginning with [WRLD:%08X]/(%d, %d)/(%d, %d) at position %08X.", std::this_thread::get_id(), desired.worldspaceID, desired.blockX, desired.blockY, desired.subBlockX, desired.subBlockY, desired.pos);
               this->setPos(desired.pos);
               this->resetParseState();
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
                     auto stub = this->make_stub_for_record(*this->owner);
                     stub->groupInfo.type = (int)group.header.type;
                     if (form_type_info::form_type_is_reference(stub->formType)) {
                        uint32_t cellID = group.getRawIDOfParentCell();
                        if (cellID) {
                           lo.local_formID_to_global_formID(this->owner->as_file(), cellID);
                           stub->groupInfo.parentFormID = cellID;
                        } else
                           dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_persistent_cell_children:%s] Reference %08X is not in a cell?", this->owner->as_file()->get_filename(), stub->formID);
                     }
                     if (!this->owner->_insert_form(stub->formID, stub)) { // also normalizes (stub->formID)
                        delete stub;
                        continue;
                     }
                     this->extract_high_value_subrecords_for_stub(stub);
                     continue;
                  }
               }
            }
            this->file = nullptr;
            //
            dovah::logging::print_line("[dovah::tes_file_reading::threads::worldspace_persistent_cell_children] Thread %08X finished all of its work (%d queued entries).", std::this_thread::get_id(), this->queue.size());
         }
         void worldspace_persistent_cell_children::add_group(uint32_t cellID, uint32_t pos) {
            this->queue.emplace_back(cellID, pos);
         }
         void worldspace_persistent_cell_children::start() {
            this->thread = std::thread(worldspace_persistent_cell_children::_thread_handler, this);
         }
         void worldspace_persistent_cell_children::wait_for() {
            this->thread.join();
         }
         #pragma endregion
         
         #pragma region game_setting
         void game_setting::_thread_handler(game_setting* instance) {
            instance->_load();
         }
         void game_setting::_load() {
            this->file = this->owner->file;
            auto& lo = this->owner->load_order;
            //
            auto size = this->queue.size();
            this->progress.maximum = size;
            for (uint32_t i = 0; i < size; i++) {
               if (this->owner->aborted) {
                  dovah::logging::print_line("[dovah::tes_file_reading::threads::game_setting] Thread %08X aborting as requested by owning file.", std::this_thread::get_id());
                  break;
               }
               this->progress.current = i;
               auto& desired = this->queue[i];
               this->setPos(desired.pos);
               this->resetParseState();
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
                        auto& error = this->owner->error;
                        //
                        error.code       = file_read_error::error_code::malformed_file;
                        error.file       = this->as_file()->get_filename();
                        error.fileOffset = this->getPos();
                        cobb::sprintf(error.message, "Unexpected nested group within \"simple\" top-group GMST.");
                        //
                        this->owner->abort();
                        break;
                     }
                  }
                  if (ot == object_type::record) {
                     auto& record = this->get_current_record();
                     auto& group  = this->get_current_group();
                     if (record.signature() != 'GMST')
                        continue;
                     //
                     loaded_game_setting working;
                     auto& EDID = record.next_subrecord();
                     if (EDID.signature() != 'EDID')
                        continue;
                     std::string name;
                     EDID.to_string(name);
                     working.name       = name;
                     working.definition = &game_setting_definition::lookup(name.c_str());
                     while (auto& subrecord = record.next_subrecord()) {
                        if (subrecord.signature() != 'DATA')
                           continue;
                        switch (working.definition->type) {
                           case game_setting_type::float32:
                              subrecord.read(working.value.f);
                              break;
                           case game_setting_type::integer:
                              subrecord.read(working.value.i);
                              break;
                           case game_setting_type::string:
                              subrecord.to_string(working.value.s);
                              break;
                        }
                        break;
                     }
                     //
                     // TODO: EDID after DATA won't load
                     //
                     lo.accept_game_setting(this->owner, working, record.formID());
                     continue;
                  }
               }
            }
            this->file = nullptr;
            //
            dovah::logging::print_line("[dovah::tes_file_reading::threads::game_setting] Thread %08X finished all of its work (%d queued entries).", std::this_thread::get_id(), this->queue.size());
         }
         void game_setting::add_group(uint32_t pos) {
            this->queue.emplace_back(pos);
         }
         void game_setting::start() {
            this->thread = std::thread(game_setting::_thread_handler, this);
         }
         void game_setting::wait_for() {
            this->thread.join();
         }
         #pragma endregion
      }
   }
}