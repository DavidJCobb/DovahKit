#include "file_writer.h"
#include "../file_load_order.h"
#include "../tes_file_reading/file.h"
#include "../../core.h"
#include "../../form_stub.h"
#include "../../form_stub_helpers.h"
#include "../../forms/Form.h"
#include "../../notice_code_list.h"
#include "../common.h"
extern "C" {
   #include "../../../zlib/zlib.h" // interproject ref
}

namespace {
   static constexpr int record_compress_threshold = 0x280;
}

namespace dovah::tes_file_writing {
   file_writer::file_writer(file_load_order& owner, file_reader& source, const write_config& cfg) : owner(owner), source(source), _record(*this), _subrecord(*this) {
      this->config = cfg;
      //
      this->use_string_table = (this->source.header.flags & tes_file_flag::localized_string_table) != 0;
      if (!this->config.record_version)
         this->config.record_version = this->source.header.record_version;
   }
   file_writer::~file_writer() {
      this->stream.close();
   }

   record& file_writer::_open_next_record(uint32_t signature, bare_form_id_t formID) {
      auto& record = this->get_current_record();
      auto& header = record.header;
      if (record.exists())
         record._close();
      //
      header.signature = signature;
      header.version   = this->config.record_version;
      header.formID    = this->owner.remap_formID_for_save(formID);
      header.flags     = 0;
      header.version_control   = this->config.version_control;
      header.version_control_2 = this->config.version_control_2;
      //
      return record;
   }
   bool file_writer::_should_compress_current_record(form_stub* stub) const noexcept {
      switch (this->config.record_compression) {
         case record_compression_policy::never:
            return false;
         case record_compression_policy::bethesda:
            //
            // Inspection of Skyrim.esm indicates that Bethesda always compresses NPC_ and 
            // NAVM. CELL is compressed if it has a TVDT subrecord (and probably if it has 
            // similar large subrecords like MHDT), and LAND is compressed when inside of 
            // a compressed CELL.
            //
            // Bethesda doesn't seem to take record size into account at all when deciding 
            // whether to compress a record. I've seen ten-byte LANDs get compressed.
            //
            if (!stub)
               return false;
            switch (stub->formType) {
               case form_type::actor_base:
                  return true;
               case form_type::navmesh:
                  return true;
               case form_type::land:
                  return this->compress_state.containing_cell_is_compressed;
               case form_type::cell:
                  if (stub->form)
                     return stub->form->would_bethesda_compress();
                  break;
            }
            return false;
         case record_compression_policy::threshold:
            return this->_record.pos >= this->config.record_compress_threshold;
      }
      return false;
   }
   void file_writer::_write_header() {
      auto& record = this->_open_next_record('TES4', 0);
      auto& header = record.header;
      header.flags = this->config.file_flags;
      //
      {  // HEDR
         auto& subrecord = record.open_next_subrecord('HEDR');
         subrecord.reserve_more(0xC);
         subrecord.write(1.7F);
         //
         this->fixup_data.record_and_group_count.offset = this->get_output_position();
         subrecord.write(uint32_t(0));
         //
         subrecord.write(this->source.header.nextFormID);
         subrecord.close();
      }
      //
      // TODO: OFST? xEdit lists it in their definitions, but it doesn't appear in any Skyrim Classic masters.
      //
      // TODO: DELE? xEdit lists it in their definitions, but it doesn't appear in any Skyrim Classic masters.
      //
      if (!this->source.header.author.empty()) { // CNAM
         record.write_string_subrecord('CNAM', this->source.header.author);
      }
      if (!this->source.header.description.empty()) { // SNAM
         record.write_string_subrecord('SNAM', this->source.header.description);
      }
      this->owner.for_each_load_order_filename([&record](std::filesystem::path name, bool is_active_file) { // MAST, DATA
         if (is_active_file)
            return false;
         //
         auto  filename  = name.string();
         record.write_string_subrecord('MAST', filename);
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
      if (this->source.header.details & tes_file_header::detail_flag::has_intv) {
         auto& subrecord = record.open_next_subrecord('INTV');
         subrecord.write(uint32_t(this->source.header.subINTV));
         subrecord.close();
      }
      if (this->source.header.details & tes_file_header::detail_flag::has_incc) {
         auto& subrecord = record.open_next_subrecord('INCC');
         subrecord.write(uint32_t(this->source.header.subINCC));
         subrecord.close();
      }
      record._close();
      --this->fixup_data.record_and_group_count.value; // this should not include the file-header record
   }
   bool file_writer::_write_form(form_stub* stub) {
      auto loaded = stub->_load(true);
      if (loaded) {
         assert(stub->formType < form_types.size() && "Stub form type is out of bounds.");
         auto& record = this->_open_next_record(form_types[stub->formType].signature, stub->formID);
         record.header.flags = loaded->flags & ~tes_file_record_header::non_data_flags;
         cobb::edit_bit(record.header.flags, tes_file_record_header::flag::deleted, (stub->flags & form_stub::flag::flagged_as_deleted));
         //
         auto intfc = load_order_interfaces::form_save(this->owner);
         //
         if (loaded->save(record, intfc)) {
            auto& write_info = this->fixup_data.form_stubs[stub->formID];
            write_info.stub   = stub;
            write_info.offset = this->get_stream_position(); // we haven't closed the record yet, so this is still at the start of where we're about to write the record
            //
            if (this->_should_compress_current_record(stub))
               record.header.flags |= tes_file_record_header::flag::compressed;
            //
            if (stub->formType == form_type::cell) {
               this->compress_state.containing_cell_is_compressed = record.header.body_is_compressed();
            }
            //
            record._close();
            if (this->error.defined()) // any zlib errors?
               return false;
         } else {
            record._clear(); // abort this attempt at writing a record
            //
            // The question now is, what do we want to do as an alternative to trying to write 
            // the loaded form data?
            //
            // Here's a brave idea: what if we just copied the form's original data from its 
            // source file? I mean, if it's not a hardcoded form or a form that we've created 
            // at run-time, then it should have come from a file. The whole point of form stubs 
            // is to let us load form data on demand, and if it's good enough to load, then it 
            // should be good enough to save, too, right?
            //
            // Well, yes, but actually no. See, the source file could have a different list of 
            // masters than the new file that we're saving, which means that we need to fix up 
            // form IDs in the data that we're saving. We can't do that if we're just blindly 
            // copying data, so (outside of just resaving a file with the same masters it had 
            // to start with) we'd just end up writing a ton of garbage form IDs.
            //
            // The only suitable alternative to writing loaded form data, then, is failing with 
            // an error.
            //
            if (!this->error.defined()) // check this before setting the code in case whatever caused the write to fail also signalled an error on its own
               this->error.code = notice_code::unknown_form_type;
            this->error.formID      = stub->formID;
            this->error.form_type   = stub->formType;
            this->error.file_offset = this->get_stream_position();
            return false;
         }
      } else {
         this->error.code        = notice_code::unknown_form_type;
         this->error.formID      = stub->formID;
         this->error.form_type   = stub->formType;
         this->error.file_offset = this->get_stream_position();
         return false;
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
      if (stub->formType == form_type::cell) {
         this->compress_state.containing_cell_is_compressed = false;
      }
      return true;
   }
   void file_writer::_write_record(form_stub* stub) {
      auto& record = this->get_current_record();
      if (record.header.body_is_compressed()) {
         //
         // Write compressed record data.
         //
         auto& error  = this->error;
         uint32_t decompressed_size = record.header.size;
         uint32_t compressed_size   = compressBound(decompressed_size);
         auto  buffer = malloc(compressed_size);
         if (!buffer) {
            error.code        = notice_code::out_of_memory;
            error.formID      = record.header.formID;
            error.form_type   = stub ? stub->formType : form_type::none;
            error.file_offset = this->get_stream_position();
            return;
         }
         int result = compress2((Bytef*)buffer, (uLongf*)&compressed_size, (const Bytef*)record.data.data(), decompressed_size, Z_BEST_COMPRESSION);
         if (result != Z_OK) {
            error.formID      = record.header.formID;
            error.form_type   = stub ? stub->formType : form_type::none;
            error.file_offset = this->get_stream_position();
            switch (result) {
               case Z_MEM_ERROR:
                  error.code = notice_code::zlib_memory_error;
                  return;
               case Z_BUF_ERROR:
                  error.code = notice_code::zlib_buffer_error;
                  return;
            }
         }
         record.header.size = compressed_size + sizeof(decompressed_size);
         this->_write(record.header);
         this->_write(decompressed_size);
         this->stream.write((const uint8_t*)buffer, compressed_size);
         free(buffer);
      } else {
         //
         // Write uncompressed record data.
         //
         this->_write(record.header);
         this->stream.write(record.data.data(), record.header.size);
      }
      ++this->fixup_data.record_and_group_count.value;
   }

   void file_writer::_write_child_forms_for_cell(form_stub* stub) {
      if (stub->has_child_forms_of_group((int)tes_file_group_type::cell_persistent_children)) {
         bool group_opened = false;
         //
         form_stub_helpers::for_each_child_form(stub, [this, &group_opened](form_stub* child) {
            if ((tes_file_group_type)child->groupInfo.type != tes_file_group_type::cell_persistent_children)
               return false;
            if (!child->needs_save())
               return false;
            if (!group_opened) {
               this->open_group(tes_file_group_type::cell_persistent_children, child->groupInfo.parentFormID, tes_file_group_header::uninitialized_unknown);
               group_opened = true;
            }
            this->_write_form(child);
            return false;
         });
         //
         if (group_opened)
            this->close_current_group();
      }
      if (stub->has_child_forms_of_group((int)tes_file_group_type::cell_temporary_children)) {
         bool group_opened = false;
         //
         form_stub_helpers::for_each_child_form(stub, [this, &group_opened](form_stub* child) {
            if ((tes_file_group_type)child->groupInfo.type != tes_file_group_type::cell_temporary_children)
               return false;
            if (!child->needs_save())
               return false;
            if (!group_opened) {
               this->open_group(tes_file_group_type::cell_temporary_children, child->groupInfo.parentFormID, tes_file_group_header::uninitialized_unknown);
               group_opened = true;
            }
            this->_write_form(child);
            return false;
         });
         //
         if (group_opened)
            this->close_current_group();
      }
   }
   void file_writer::_write_child_forms_for_topic(form_stub* stub) {
      auto& group = this->open_group(tes_file_group_type::topic_children, stub->formID, tes_file_group_header::uninitialized_unknown);
      //
      form_stub_helpers::for_each_child_form(stub, [this](form_stub* child) {
         if (child->formType != form_type::topic_info)
            return false;
         if (!child->needs_save())
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
      bool group_opened         = false;
      auto open_group_if_needed = [this, stub, &group_opened]() {
         if (group_opened)
            return;
         this->open_group(tes_file_group_type::world_children, stub->formID, tes_file_group_header::uninitialized_unknown);
         group_opened = true;
      };
      //
      if (auto cell = form_stub_helpers::get_worldspace_persistent_cell(stub)) {
         if (cell->needs_save()) {
            (open_group_if_needed)();
            this->_write_form(cell);
         }
      }
      //
      std::map<uint32_t, _cell_block> blocks;
      form_stub_helpers::for_each_child_form(stub, [&blocks, &open_group_if_needed](form_stub* child) {
         if (child->formType != form_type::cell)
            return false;
         if (!child->needs_save())
            return false;
         (open_group_if_needed)();
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
      if (group_opened)
         this->close_current_group();
   }
   void file_writer::_write_interior_cells() {
      bool group_opened = false;
      //
      std::map<uint32_t, _cell_block> blocks;
      this->owner.for_each_active_file_form_of_type(form_type::cell, [this, &group_opened, &blocks](form_stub* stub) {
         if (stub->is_exterior_cell())
            return false;
         if (!group_opened) {
            this->open_group(tes_file_group_type::forms_of_type, _byteswap_ulong('CELL'), 0);
            group_opened = true;
         }
         auto b  = stub->get_cell_block();
         auto sb = stub->get_cell_sub_block();
         blocks[b].contents[sb].cells.push_back(stub);
         return false;
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
      //
      if (group_opened)
         this->close_current_group();
   }
   //
   void file_writer::_write_game_settings() {
      bool group_opened = false;
      //
      this->owner.for_each_active_file_game_setting([this, &group_opened](const dovah::loaded_game_setting& setting) {
         std::string name = setting.name;
         if (name.empty()) {
            if (!setting.definition)
               return false;
            name = setting.definition->name;
         }
         //
         if (!group_opened) { // only open a GMST GRUP if we have GMSTs to write
            group_opened = true;
            this->open_group(tes_file_group_type::forms_of_type, _byteswap_ulong('GMST'), 0);
         }
         //
         auto& record = this->_open_next_record(form_types[form_type::setting].signature, setting.formID);
         record.write_string_subrecord('EDID', name);
         //
         switch (setting.get_type()) {
            case dovah::game_setting_type::boolean:
               record.open_next_subrecord('DATA').write(uint32_t(setting.value.b));
               break;
            case dovah::game_setting_type::float32:
               record.open_next_subrecord('DATA').write(setting.value.f);
               break;
            case dovah::game_setting_type::integer:
               record.open_next_subrecord('DATA').write(setting.value.i);
               break;
            case dovah::game_setting_type::string:
               record.open_next_subrecord('DATA').write(setting.value.s);
               break;
         }
         record._close();
         //
         return false;
      });
      //
      if (group_opened)
         this->close_current_group();
   }

   void file_writer::_write_impl(const void* source, uint32_t size) {
      this->stream.write((const uint8_t*)source, size);
   }
   void file_writer::_write_impl(const tes_file_group_header& header) {
      this->_write(_byteswap_ulong(header.signature));
      this->_write(header.size);
      this->_write(header.label);
      this->_write(header.type);
      this->_write(header.version_control);
      this->_write(header.unknown);
   }
   void file_writer::_write_impl(const tes_file_record_header& header) {
      this->_write(_byteswap_ulong(header.signature));
      this->_write(header.size);
      this->_write(header.flags);
      this->_write(header.formID);
      this->_write(header.version_control);
      this->_write(header.version);
      this->_write(header.version_control_2);
   }

   bool file_writer::_can_serialize_form(const form_stub* stub) const noexcept {
      if (!stub)
         return true;
      if (stub->formType == form_type::none)
         return false;
      if (this->config.output_game != game::skyrim_special) {
         auto& info = form_type_info::lookup(stub->formType);
         if (info.flags & form_type_info::flag::is_skyrim_special)
            return false;
      }
      return true;
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
      group.header.label     = label;
      group.header.type      = group_type;
      group.header.unknown   = unknown;
      group.header.version_control = this->config.version_control;
      group.pos = this->get_stream_position();
      this->_write(group.header);
      return group;
   }
   void file_writer::close_current_group() {
      auto& record = this->get_current_record();
      if (record.exists())
         record._close();
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
      ++this->fixup_data.record_and_group_count.value;
   }

   uint32_t file_writer::get_stream_position() const noexcept {
      return this->stream.tellp();
   }
   void file_writer::set_stream_position(file_offset_t offset) noexcept {
      this->stream.seekp(offset);
   }
   uint32_t file_writer::get_output_position() const noexcept {
      auto position = this->get_stream_position();
      if (this->_record) {
         position += tes_file_record_header::struct_size;
         position += this->_record.pos;
         if (this->_subrecord) {
            position += tes_file_subrecord_header::struct_size;
            position += this->_subrecord.pos;
         }
      }
      return position;
   }

   void file_writer::open(std::filesystem::path path) {
      this->stream.open(path, std::ios_base::binary | std::ios_base::trunc);
   }
   bool file_writer::write() {
      this->_write_header();
      for (uint32_t signature : group_sequence_list) {
         auto form_type = form_type_info::signature_to_form_type(signature);
         if (form_type == form_type::setting) {
            this->_write_game_settings();
            continue;
         }
         if (!this->owner.active_file_has_forms_of_type(form_type))
            continue;
         //
         if (form_type_info::lookup(form_type).flags & form_type_info::flag::is_singleton) { // special case
            //
            // Singleton forms need special handling. We should only write out the "canonical" form stub, 
            // and none of the others.
            //
            auto* stub = this->owner.get_canonical_instance_of_singleton_form(form_type);
            if (!stub)
               continue;
            if (!this->owner.is_defined_or_overridden_in_active_file(*stub) && !stub->is_edited())
               continue;
            this->open_group(tes_file_group_type::forms_of_type, _byteswap_ulong(signature), 0);
            this->_write_form(stub);
            this->close_current_group();
            continue;
         }
         //
         if (form_type == form_type::cell) { // special case; requires a particular hierarchy of nested GRUPs
            this->_write_interior_cells();
            continue;
         }
         //
         this->open_group(tes_file_group_type::forms_of_type, _byteswap_ulong(signature), 0);
         this->owner.for_each_top_level_form_needing_save(form_type, [this](form_stub* stub) {
            return !this->_write_form(stub);
         });
         this->close_current_group();
      }
      //
      auto pos = this->get_stream_position();
      this->set_stream_position(this->fixup_data.record_and_group_count.offset);
      this->_write(uint32_t(this->fixup_data.record_and_group_count.value));
      this->set_stream_position(pos);
      //
      return !this->error.defined();
   }
   void file_writer::update_source_file_header() {
      auto& header = this->source.header;
      //
      header.flags = this->config.file_flags;
      cobb::edit_bit(header.flags, tes_file_flag::localized_string_table, this->use_string_table);
      //
      header.masters.clear();
      this->owner.for_each_load_order_filename([&header](std::filesystem::path name, bool is_active_file) { // MAST, DATA
         if (is_active_file)
            return false;
         //
         auto& entry  = header.masters.emplace_back();
         entry.master = name.string();
         entry.data   = 0;
         //
         return false;
      });
   }
   void file_writer::close() {
      this->stream.close();
   }
}