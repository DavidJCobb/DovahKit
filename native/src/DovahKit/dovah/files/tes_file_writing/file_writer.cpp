#include "file_writer.h"
#include "../file_load_order.h"
#include "../tes_file_reading/file.h"
#include "../../core.h"
#include "../../form_stub.h"
#include "../../form_stub_helpers.h"
#include "../common.h"

namespace dovah::tes_file_writing {
   record& file_writer::_open_next_record(uint32_t signature, bare_form_id_t formID) {
      auto& record = this->get_current_record();
      auto& header = record.header;
      if (record.exists())
         record.close();
      //
      header.signature = signature;
      header.version   = this->source.header_record_version;
      header.formID    = formID;
      header.flags     = 0;
      header.version_control   = this->version_control;
      header.version_control_2 = this->version_control_2;
      //
      return record;
   }
   void file_writer::_write_header() {
      auto& record = this->_open_next_record('TES4', 0);
      auto& header = record.header;
      header.flags = this->source.flags;
      //
      {  // HEDR
         auto& subrecord = record.open_next_subrecord('HEDR');
         subrecord.reserve(0xC);
         subrecord.write(1.7F);
         uint32_t count = this->owner.active_file_form_count();
         subrecord.write(count);
         subrecord.write(this->source.recordCount);
         subrecord.close();
      }
      //
      // TODO: OFST? xEdit lists it in their definitions, but it doesn't appear in any Skyrim Classic masters.
      //
      // TODO: DELE? xEdit lists it in their definitions, but it doesn't appear in any Skyrim Classic masters.
      //
      if (!this->source.authorName[0]) { // CNAM
         auto& subrecord = record.open_next_subrecord('CNAM');
         subrecord.write(this->source.authorName);
         subrecord.close();
      }
      if (!this->source.description[0]) { // SNAM
         auto& subrecord = record.open_next_subrecord('SNAM');
         subrecord.write(this->source.description);
         subrecord.close();
      }
      this->owner.for_each_load_order_filename([&record](std::filesystem::path name) { // MAST, DATA
         auto  filename  = name.string();
         auto& subrecord = record.open_next_subrecord('MAST');
         subrecord.write(filename);
         subrecord.close();
         //
         auto& extra = record.open_next_subrecord('DATA');
         extra.write(uint64_t(0));
         extra.close();
         //
         return false;
      });
      {  // ONAM
         //
         // NOTE: ONAM doesn't use any obvious sort order. If I had to take a blind guess, I'd say that the CK 
         // probably generates it while looping over all cells in memory.
         //
         bool opened = false;
         for (auto& typeinfo : form_types) {
            bool is_cell_child = false;
            switch (typeinfo.formType) {
               case form_type::land:
               case form_type::navmesh:
                  is_cell_child = true;
                  break;
            }
            if (!is_cell_child) {
               is_cell_child = typeinfo.is_reference();
               if (!is_cell_child)
                  continue;
            }
            //
            this->owner.for_each_active_file_override_of_type(typeinfo.formType, [&opened, &record](const form_stub* stub) {
               if (!opened) {
                  record.open_next_subrecord('ONAM');
                  opened = true;
               }
               auto& subrecord = record.get_current_subrecord();
               subrecord.write(stub->formID);
               return false;
            });
         }
      }
      //
      // TODO: SCRN? xEdit lists it in their definitions, but it doesn't appear in any Skyrim Classic masters.
      //
      if (this->source.details & file_reader::detail_flag::has_intv) {
         auto& subrecord = record.open_next_subrecord('INTV');
         subrecord.write(uint32_t(this->source.subINTV));
         subrecord.close();
      }
      if (this->source.details & file_reader::detail_flag::has_incc) {
         auto& subrecord = record.open_next_subrecord('INCC');
         subrecord.write(uint32_t(this->source.subINCC));
         subrecord.close();
      }
      record.close();
   }
   bool file_writer::_write_form(form_stub* stub) {
      auto& read_write_interface = this->source.get_writer_interface(*this); // TODO: can we just use one instance, instead of creating a new one for each function call?
      bool  can_handwrite_form   = false; // can we save the form from its loaded data?

      //
      // TODO: if the form is loaded, call its (save) method. if that returns (true), write the 
      // data it provided; else, try to blind-copy data from the source file (and if that isn't 
      // possible, e.g. for a form that wasn't in the source file, then abort with an error).
      //

      if (!can_handwrite_form) {
         //
         // loaded_forms::Form::save(...) returned false, indicating that a write wasn't possible. 
         // Try to blind-copy data from the source file.
         //
         if (!stub->has_usable_source_file()) {
            //
            // This stub doesn't have a usable source file, e.g. because it was created during 
            // this session or it was a hardcoded form.
            //
            //
            // TODO: Fail with an error.
            //
            return false;
         }
         auto& write_info = this->stub_writes[stub->formID];
         write_info.offset = this->get_stream_position();
         //
         const void* source_data   = read_write_interface.data_at(stub->get_file_offset());
         const auto* source_header = (const tes_file_record_header*)source_data;
         uint32_t size = source_header->size;
         this->stream.write((const uint8_t*)source_data, size + sizeof(tes_file_record_header));
      } else {
         //
         // TODO: open a record
         //
         // TODO: write the form using its loaded data
         //
         // TODO: close the record
         //
      }
      if (stub->has_child_forms()) {
         //
         // Now, we need to write child groups and forms as appropriate:
         //
         switch (stub->formType) {
            case form_type::cell:
               this->_write_child_forms_for_cell(stub);
               break;
            case form_type::worldspace:
               this->_write_child_forms_for_worldspace(stub);
               break;
            case form_type::topic:
               this->_write_child_forms_for_topic(stub);
               break;
         }
      }
      return true;
   }
   void file_writer::_write_record() {
      auto& record = this->get_current_record();
      this->_write(record.header);
      this->stream.write(record.data.data(), record.header.size);
   }

   void file_writer::_write_child_forms_for_cell(form_stub* stub) {
      if (stub->has_child_forms_of_group((int)tes_file_group_type::cell_persistent_children)) {
         auto& group = this->open_group(tes_file_group_type::cell_persistent_children, stub->formID, tes_file_group_header::uninitialized_unknown);
         //
         form_stub_helpers::for_each_child_form(stub, [this](form_stub* child) {
            if ((tes_file_group_type)child->groupInfo.type != tes_file_group_type::cell_persistent_children)
               return false;
            this->_write_form(child);
            return false;
         });
         //
         this->close_current_group();
      }
      if (stub->has_child_forms_of_group((int)tes_file_group_type::cell_temporary_children)) {
         auto& group = this->open_group(tes_file_group_type::cell_temporary_children, stub->formID, tes_file_group_header::uninitialized_unknown);
         //
         form_stub_helpers::for_each_child_form(stub, [this](form_stub* child) {
            if ((tes_file_group_type)child->groupInfo.type != tes_file_group_type::cell_temporary_children)
               return false;
            this->_write_form(child);
            return false;
         });
         //
         this->close_current_group();
      }
   }
   void file_writer::_write_child_forms_for_topic(form_stub* stub) {
      auto& group = this->open_group(tes_file_group_type::topic_children, stub->formID, tes_file_group_header::uninitialized_unknown);
      //
      form_stub_helpers::for_each_child_form(stub, [this](form_stub* child) {
         if (child->formType != form_type::topic_info)
            return false;
         this->_write_form(child);
         return false;
      });
      //
      this->close_current_group();
   }
   //
   namespace {
      struct _cell_sub_block {
         std::vector<form_stub*> cells;
      };
      struct _cell_block {
         std::map<uint32_t, _cell_sub_block> contents;
      };
   }
   void file_writer::_write_child_forms_for_worldspace(form_stub* stub) {
      auto& group_wc = this->open_group(tes_file_group_type::world_children, stub->formID, tes_file_group_header::uninitialized_unknown);
      //
      if (auto cell = form_stub_helpers::get_worldspace_persistent_cell(stub)) {
         this->_write_form(cell);
      }
      //
      std::map<uint32_t, _cell_block> blocks;
      form_stub_helpers::for_each_child_form(stub, [&blocks](form_stub* child) {
         if (child->formType != form_type::cell)
            return false;
         auto b  = child->get_cell_block();
         auto sb = child->get_cell_sub_block();
         blocks[b].contents[sb].cells.push_back(child);
         return false;
      });
      for (auto& pair : blocks) {
         auto& group_b = this->open_group(tes_file_group_type::exterior_cell_block, pair.first, tes_file_group_header::uninitialized_unknown);
         auto& list = pair.second.contents;
         for (auto& pair : list) {
            auto& group_s = this->open_group(tes_file_group_type::exterior_cell_sub_block, pair.first, tes_file_group_header::uninitialized_unknown);
            for (auto* cell : pair.second.cells) {
               this->_write_form(cell);
               assert(&group_s == &this->get_current_group() && "An exterior CELL record opened one or more child GRUPs for its REFRs, but forgot to close the GRUP(s) after writing all of the REFRs.");
            }
            this->close_current_group();
         }
         this->close_current_group();
      }
      //
      this->close_current_group();
   }
   void file_writer::_write_interior_cells() {
      std::map<uint32_t, _cell_block> blocks;
      this->owner.for_each_active_file_form_of_type(form_type::cell, [&blocks](form_stub* stub) {
         if (stub->is_exterior_cell())
            return false;
         auto b  = stub->get_cell_block();
         auto sb = stub->get_cell_sub_block();
         blocks[b].contents[sb].cells.push_back(stub);
      });
      for (auto& pair : blocks) {
         auto& group_b = this->open_group(tes_file_group_type::interior_cell_block, pair.first, tes_file_group_header::uninitialized_unknown);
         auto& list = pair.second.contents;
         for (auto& pair : list) {
            auto& group_s = this->open_group(tes_file_group_type::interior_cell_sub_block, pair.first, tes_file_group_header::uninitialized_unknown);
            for (auto* cell : pair.second.cells) {
               this->_write_form(cell);
               assert(&group_s == &this->get_current_group() && "An interior CELL record opened one or more child GRUPs for its REFRs, but forgot to close the GRUP(s) after writing all of the REFRs.");
            }
            this->close_current_group();
         }
         this->close_current_group();
      }
   }

   void file_writer::_write_impl(const void* source, uint32_t size) {
      this->stream.write((const uint8_t*)source, size);
   }
   void file_writer::_write_impl(const tes_file_group_header& header) {
      this->_write(header.signature);
      this->_write(header.size);
      this->_write(header.label);
      this->_write(header.type);
      this->_write(header.version_control);
      this->_write(header.unknown);
   }
   void file_writer::_write_impl(const tes_file_record_header& header) {
      this->_write(header.signature);
      this->_write(header.size);
      this->_write(header.flags);
      this->_write(header.formID);
      this->_write(header.version_control);
      this->_write(header.version);
      this->_write(header.version_control_2);
   }

   group& file_writer::open_group(tes_file_group_type group_type, uint32_t label, uint32_t unknown) {
      int32_t parent = -1;
      for (uint32_t i = 0; i < this->_groups.size(); i++) {
         auto& group = this->_groups[i];
         if (!group)
            break;
         parent = i;
      }
      assert(parent + 1 < this->_groups.size());
      auto& group = this->_groups[parent + 1];
      group.header.signature = 'GRUP';
      group.header.type      = group_type;
      group.header.unknown   = unknown;
      group.header.version_control = this->version_control;
      group.pos = this->get_stream_position();
      this->_write(group.header);
      return group;
   }
   void file_writer::close_current_group() {
      auto& record = this->get_current_record();
      if (record.exists())
         record.close();
      //
      auto& group = this->get_current_group();
      auto  pos   = this->get_stream_position();
      if (!group)
         assert(false && "file_writer: tried to close a group when there are no groups!");
      this->set_stream_position(group.pos + tes_file_group_header::offset_of_size);
      this->_write(uint32_t(pos - group.pos));
      this->set_stream_position(pos);
      //
      group.header = tes_file_group_header();
      group.pos    = 0;
   }

   uint32_t file_writer::get_stream_position() const noexcept {
      return this->stream.tellp();
   }
   void file_writer::set_stream_position(file_offset_t offset) noexcept {
      this->stream.seekp(offset);
   }

   void file_writer::write() {
      this->_write_header();
      for (uint32_t signature : group_sequence_list) {
         auto form_type = form_type_info::signature_to_form_type(signature);
         if (!this->owner.active_file_has_forms_of_type(form_type))
            continue;
         //
         if (form_type == form_type::cell) { // special case; requires a particular hierarchy of nested GRUPs
            this->_write_interior_cells();
            continue;
         }
         //
         this->open_group(tes_file_group_type::forms_of_type, signature, 0);
         this->owner.for_each_active_file_form_of_type(form_type, [this](form_stub* stub) {
            return !this->_write_form(stub);
         });
         this->close_current_group();
      }
   }
}