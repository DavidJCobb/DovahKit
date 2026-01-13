#include "./file_writer.h"
#include "../file_load_order.h"
#include "../tes_file_reading/file_loader.h"
#include "../../data/game/max_file_version.h"
#include "../../data/game/min_file_version.h"
#include "../../load_order_interfaces/form_save.h"
#include "../../form_stub.h"
#include "../../form_stub_addenda.h"
#include "../../form_stubs/helpers/for_each_child_form.h"
#include "../../form_stubs/helpers/for_each_persistent_ref_in_world.h"
#include "../../form_stubs/helpers/get_worldspace_persistent_cell.h"
#include "../../form_stubs/helpers/is_persistent.h"
#include "../../forms/Form.h"
#include "../../forms/ObjectReference.h"
#include "../common.h"
extern "C" {
   #include "../../../zlib/zlib.h" // interproject ref
}
#include "../../exceptions/file_save_failed.h"

namespace {
   // If this is set to `true`, then when we save a non-edited form, we flag its record 
   // as "partial" and save only what data would be appropriate for partial records. This 
   // is an attempt to take advantage of the fact that the Rule of One is waived for a 
   // partial record; however, I've seen reports of bugs (e.g. CELLs' refs failing to 
   // load) when this is done, so I want to test it extensively before I make use of it.
   constexpr const bool enable_partial_flagged_containers = false;
}

namespace {
   using exception  = dovah::exceptions::file_save_failed;
   using error_code = exception::error_code;

   constexpr const auto reference_form_types = []() {
      constexpr const size_t count = []() {
         size_t n = 0;
         for (const auto& item : dovah::form_types)
            if (dovah::form_type_is_reference(item.form_type))
               ++n;
         return n;
      }();

      std::array<dovah::form_type, count> out = {};
      size_t n = 0;
      for (const auto& item : dovah::form_types)
         if (dovah::form_type_is_reference(item.form_type))
            out[n++] = item.form_type;
      return out;
   }();
}

namespace dovah::tes_file_writing {
   file_writer::file_writer(file_load_order& owner, file_loader& source, const write_config& cfg)
      :
      owner(owner),
      source(source),
      _record(*this),
      _subrecord(*this),
      ref_persistence_checker(owner)
   {
      this->config = cfg;
      
      this->use_string_table = (this->source.header.flags & tes_file_flag::localized_string_table) != 0;
      if (!this->config.record_version)
         this->config.record_version = this->source.header.record_version;

      this->ref_persistence_checker._set_is_during_save({});
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
            switch (stub->form_type) {
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
         if (this->config.use_file_version.has_value()) {
            subrecord.write(this->config.use_file_version.value());
         } else {
            subrecord.write(
               std::min(
                  game_feature_support::max_file_version(this->config.output_game),
                  std::max(
                     game_feature_support::min_file_version(this->config.output_game), this->source.header.file_version
                  )
               )
            );
         }
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
            switch (typeinfo.form_type) {
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
            this->owner.for_each_active_file_override_of_type(typeinfo.form_type, [&opened, &record](const form_stub* stub) {
               if (stub->test_record_flags(tes_file_record_header::flag::persistent))
                  return false;
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
      {  // INCC
         uint32_t count = 0;
         this->owner.for_each_active_file_form_of_type(form_type::cell, [&count](dovah::form_stub* stub) {
            if (!stub->is_exterior_cell())
               ++count;
            return false;
         });
         this->source.header.interior_cell_count = count;
         //
         auto& subrecord = record.open_next_subrecord('INCC');
         subrecord.write(count);
         subrecord.close();
      }
      record._close();
      --this->fixup_data.record_and_group_count.value; // this should not include the file-header record
   }
   bool file_writer::_write_form(form_stub* stub, std::optional<form_stub*> previous_child) {
      this->_current_target = stub;

      auto loaded = stub->load_even_if_unsafe({});
      if (!loaded) {
         auto ex = exception(error_code::unimplemented_form_type);
         ex.details.unimplemented_form = stub;
         throw ex;
      }

      assert(is_valid_form_type(stub->form_type) && "Stub form type is out of bounds.");
      auto& record = this->_open_next_record(form_type_info::lookup(stub->form_type).signature, stub->formID);
      record.header.flags = stub->get_record_flags() & ~tes_file_record_header::non_data_flags;
      if (stub->is_edited()) {
         record.header.flags &= ~tes_file_record_header::flag::partial; // if the stub was previously a partial record in the active file but is now edited, clear the "partial" flag
      } else {
         if constexpr (enable_partial_flagged_containers) {
            if (!stub->file_list_includes(&this->source))
               record.header.flags |= tes_file_record_header::flag::partial; // if the stub is not in the active file, thne we must be saving it because one of its new child forms is, so set the "partial" flag
         }
      }
      //
      auto intfc = load_order_interfaces::form_save(this->owner, *this);
      intfc.previous_child = previous_child;
      intfc.target_stub    = stub;
      //
      loaded->save(record, intfc);
      this->_current_target = nullptr;
      {
         auto& write_info = this->fixup_data.form_stubs[stub->formID];
         write_info.stub    = stub;
         write_info.offset  = this->get_stream_position(); // we haven't closed the record yet, so this is still at the start of where we're about to write the record
         write_info.partial = (record.header.flags & tes_file_record_header::flag::partial);
         for (auto& pair : stub->outbound) {
            auto  id    = pair.first;
            auto& entry = pair.second;
            if (!this->_can_serialize_form(entry.other))
               write_info.sever_references_to.push_back(id);
         }
         //
         if (this->_should_compress_current_record(stub))
            record.header.flags |= tes_file_record_header::flag::compressed;
         //
         if (stub->form_type == form_type::cell) {
            this->compress_state.containing_cell_is_compressed = record.header.body_is_compressed();
         }
         //
         record._close();
      }

      if (stub->has_child_forms()) {
         //
         // Now, we need to write child groups and forms as appropriate:
         //
         switch (stub->form_type) {
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
      if (stub->form_type == form_type::cell) {
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
         uint32_t decompressed_size = record.header.size;
         uint32_t compressed_size   = compressBound(decompressed_size);
         auto  buffer = malloc(compressed_size);
         if (!buffer) {
            auto ex = exception(error_code::out_of_memory);
            ex.details.unimplemented_form = stub; // TODO: rename this field
            throw ex;
         }
         int result = compress2((Bytef*)buffer, (uLongf*)&compressed_size, (const Bytef*)record.data.data(), decompressed_size, Z_BEST_COMPRESSION);
         if (result != Z_OK) {
            switch (result) {
               case Z_MEM_ERROR:
                  {
                     auto ex = exception(error_code::zlib_memory_error);
                     ex.details.unimplemented_form = stub; // TODO: rename this field
                     throw ex;
                  }
               case Z_BUF_ERROR:
                  {
                     auto ex = exception(error_code::zlib_buffer_error);
                     ex.details.unimplemented_form = stub; // TODO: rename this field
                     throw ex;
                  }
            }
            auto ex = exception(error_code::zlib_unknown_error);
            ex.details.unimplemented_form = stub; // TODO: rename this field
            throw ex;
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
      assert(stub);
      assert(stub->form_type == dovah::form_type::cell);
      //
      form_stub* worldspace      = stub->get_parent_form();
      form_stub* persistent_cell = nullptr;
      if (worldspace) {
         if (worldspace->form_type == dovah::form_type::worldspace) {
            persistent_cell = form_stub_helpers::get_worldspace_persistent_cell(*worldspace);
         } else {
            worldspace = nullptr;
         }
      }
      //
      std::vector<form_stub*> persistent;
      std::vector<form_stub*> temporary;
      if (stub == persistent_cell) {
         assert(worldspace);
         form_stub_helpers::for_each_persistent_ref_in_world(*worldspace, [&persistent](form_stub& child) {
            if (!child.needs_save())
               return false;
            persistent.push_back(&child);
            return false;
         });
      } else {
         //
         // Exterior cells generally don't store their own persistent REFRs; rather, 
         // those are sorted under the worldspace's persistent cell. Interior cells, 
         // however, do store their own persistent REFRs.
         // 
         // Moreover, if we're resaving a malformed file (or the frontend has made 
         // some sort of mistake) such that a worldspace contains persistent refs 
         // but no persistent cell, we should ensure that we don't lose those refs, 
         // even if that means putting them in the wrong GRUP. We may not have the 
         // option to simply create a persistent cell if e.g. the file has no free 
         // form IDs remaining or the frontend isn't ready for a form to be created 
         // during the save process.
         //
         bool gather_persistent = !persistent_cell || !stub->is_exterior_cell();
         form_stub_helpers::for_each_child_form(*stub, [stub, gather_persistent, &persistent, &temporary](form_stub& child) {
            if (!child.needs_save())
               return false;
            if (form_stub_helpers::is_persistent(child)) {
               if (!gather_persistent)
                  return false;
               persistent.push_back(&child);
            } else {
               temporary.push_back(&child);
            }
            return false;
         });
      }
      //
      if (!persistent.empty() || !temporary.empty()) {
         this->open_group(tes_file_group_type::cell_children, stub->formID, 0);
         if (!persistent.empty()) {
            this->open_group(tes_file_group_type::cell_persistent_children, stub->formID, tes_file_group_header::uninitialized_unknown);
            for (auto* child : persistent) {
               if (!this->_write_form(child))
                  break;
            }
            this->close_current_group();
         }
         if (!temporary.empty()) {
            this->open_group(tes_file_group_type::cell_temporary_children, stub->formID, tes_file_group_header::uninitialized_unknown);
            for (auto* child : temporary) {
               if (!this->_write_form(child))
                  break;
            }
            this->close_current_group();
         }
         this->close_current_group();
      }
   }
   void file_writer::_write_child_forms_for_topic(form_stub* stub) {
      if (!stub->addenda)
         return;

      auto& list_a = stub->addenda->ordered_children.active_file;
      auto& list_d = stub->addenda->ordered_children.dependencies;
      if (list_a.empty())
         return;
      
      auto& group = this->open_group(tes_file_group_type::topic_children, stub->formID, tes_file_group_header::uninitialized_unknown);
      
      bool is_new_form = stub->get_owning_load_order().is_defined_in_active_file(*stub);
      if (is_new_form) {
         //
         // If this DIAL is new -- defined in the active file -- then we don't have 
         // to worry about the pre-active-file ordering of its INFOs.
         //
         for (dovah::form_stub* child : list_a) {
            if (child->form_type != form_type::topic_info)
               continue;
            if (!child->needs_save())
               continue;
            this->_write_form(child);
         }
      } else {
         //
         // We're overriding this DIAL and, potentially, its INFOs. We need to compare 
         // the order of its children from the master files to the order we wish to 
         // serialize them in now, and write INFO/PNAM as appropriate.
         //
         size_t ia = 0;
         size_t id = 0;
         for (; ia < list_a.size(); ++ia) {
            auto* child = list_a[ia];
            if (child->form_type != form_type::topic_info)
               continue;

            dovah::form_stub* previous = nullptr;
            if (ia > 0) {
               previous = list_a[ia - 1];
            }

            bool child_is_new = child->get_owning_load_order().is_defined_in_active_file(*child);
            if (!child_is_new) {
               //
               // This INFO was originally defined in one of the master files. Its 
               // order may have changed.
               //
               auto it = std::find(list_d.begin(), list_d.end(), child);
               if (it == list_d.end()) {
                  //
                  // The INFO isn't newly-defined in the active file, but has been 
                  // transplanted into this DIAL by the active file.
                  //
                  child_is_new = true;
               }
            }
            //
            // Now we need to decide whether to serialize INFO/PNAM based on the info 
            // above. Notably, if you reorder INFOs in the Creation Kit from ABCD to 
            // ACBD, the CK actually serializes overrides for A, B, and C, even though 
            // in theory only B and C should need changes to their PNAMs. We're not 
            // gonna try and do that because frankly, uh, it's easier not to bother.
            //
            if (child_is_new) {
               this->_write_form(child, previous);
            } else {
               if (child == list_d[id]) {
                  if (child->needs_save())
                     this->_write_form(child);
               } else {
                  child->set_edited(true); // because we have to serialize a potentially different INFO/PNAM
                  this->_write_form(child, previous);
               }
               ++id;
            }
         }
      }
      
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
      if (auto cell = form_stub_helpers::get_worldspace_persistent_cell(*stub)) {
         if (cell->needs_save()) {
            //
            // NOTE: You may be aware that persistent refs need to be handled differently from 
            //       normal refs. On load, we reparent them from the worldspace persistent cell 
            //       into whatever cell their coordinates would place them in, so on save, we 
            //       need to be sure to serialize them into the persistent cell rather than 
            //       into what DovahKit views as their parent form.
            // 
            //       Don't worry about it. We deal with that in `_write_child_forms_for_cell`. 
            //       The persistent cell reaches into its owning worldspace and looks up all of 
            //       the persistent refs therein; the non-persistent cells skip persistent refs 
            //       when serializing their own children; and it all works out.
            //
            (open_group_if_needed)();
            this->_write_form(cell);
         }
      }
      //
      std::map<uint32_t, _cell_block> blocks;
      form_stub_helpers::for_each_child_form(*stub, [&blocks, &open_group_if_needed](form_stub& child) {
         if (child.form_type != form_type::cell)
            return false;
         if (!child.needs_save())
            return false;
         (open_group_if_needed)();
         auto b  = child.get_cell_block();
         auto sb = child.get_cell_sub_block();
         blocks[b].contents[sb].cells.push_back(&child);
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
         auto& record = this->_open_next_record(form_type_info::lookup(form_type::setting).signature, setting.formID);
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
      if (stub->is_none_stub())
         return false;
      if (this->config.output_game != game::skyrim_special) {
         auto& info = form_type_info::lookup(stub->form_type);
         if (info.flags & form_type_info::flag::is_skyrim_special)
            return false;
      }
      return true;
   }

   void file_writer::_update_ref_persistence_pre_save() {
      //
      // It's tempting to do this as we save the REFRs (and other REFR subclasses e.g. ACHR), 
      // since we load the REFRs in full at that time. However, REFRs in an exterior space 
      // are only saved when we save the cell that they should be serialized into... and we 
      // only know which cell that is based on the REFR's persistent flag. So we have to set 
      // the flag in advance.
      // 
      // In practice, it's simplest to just update persistence flags before we even save any 
      // data at all.
      //
      bool do_flag   = this->config.persistent_refs.add_flag_when_needed;
      bool do_unflag = this->config.persistent_refs.remove_flag_when_unneeded;
      if (!do_flag && !do_unflag)
         return;
      
      for (const auto ft : reference_form_types) {
         this->owner.for_each_active_file_form_of_type(ft, [this, do_flag, do_unflag](dovah::form_stub* refr) -> bool {
            bool should_check = do_flag && do_unflag;
            if (!should_check) {
               bool already = refr->test_record_flags(dovah::loaded_forms::ObjectReference::form_flag::persistent);
               if (do_flag && !already)
                  should_check = true;
               else if (do_unflag && already)
                  should_check = true;
               //
               if (!should_check)
                  return false;
            }

            bool needs_persistence = this->ref_persistence_checker.check_ref(*refr);
            if (do_flag && needs_persistence) {
               refr->edit_record_flags(dovah::loaded_forms::ObjectReference::form_flag::persistent, true);
            } else if (do_unflag && !needs_persistence) {
               refr->edit_record_flags(dovah::loaded_forms::ObjectReference::form_flag::persistent, false);
            }

            return false;
         });
      }
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

   void file_writer::open() {
      this->stream.open(this->path, std::ios_base::binary | std::ios_base::trunc);
   }
   void file_writer::write() {
      this->_update_ref_persistence_pre_save();
      this->_write_header();
      for (uint32_t signature : group_sequence_list) {
         auto form_type = form_type_info::signature_to_form_type(signature);
         if (form_type == form_type::setting) {
            this->_write_game_settings();
            continue;
         }
         const auto& type_info = form_type_info::lookup(form_type);
         if (this->config.output_game != game::skyrim_special) {
            if (type_info.flags & form_type_info::flag::is_skyrim_special) {
               continue;
            }
         }
         if (type_info.flags & form_type_info::flag::is_singleton) { // special case
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
         
         bool group_opened = false;
         this->owner.for_each_top_level_form_needing_save(form_type, [this, signature, &group_opened](form_stub* stub) {
            if (!group_opened) {
               group_opened = true;
               this->open_group(tes_file_group_type::forms_of_type, _byteswap_ulong(signature), 0);
            }
            return !this->_write_form(stub);
         });
         if (group_opened) {
            this->close_current_group();
         }
      }
      //
      auto pos = this->get_stream_position();
      this->set_stream_position(this->fixup_data.record_and_group_count.offset);
      this->_write(uint32_t(this->fixup_data.record_and_group_count.value));
      this->set_stream_position(pos);
   }
   void file_writer::update_source_file_header() {
      auto& header = this->source.header;
      //
      header.flags = this->config.file_flags;
      cobb::edit_bit(header.flags, tes_file_flag::localized_string_table, this->use_string_table);
      if (this->config.use_file_version.has_value())
         header.file_version = this->config.use_file_version.value();
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
   bool file_writer::post_save_rename(const std::filesystem::path& desired) {
      std::error_code code;
      std::filesystem::rename(this->path, desired, code);
      if (!code) {
         this->path = desired;
      }
      this->source.get_save_interface(*this).update_path(desired, this->path); // if the rename fails, then we want to set the saved file's path to the file path used when saving
      return !code;
   }

   const file_writer::form_stub_write_info* file_writer::get_write_info_for_stub(const form_stub& stub) const noexcept {
      auto& list = this->fixup_data.form_stubs;
      for (auto& pair : list) {
         auto& entry = pair.second;
         if (entry.stub == &stub)
            return &entry;
      }
      return nullptr;
   }
}