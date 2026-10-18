#include "./world_cell_offset_data.h"
#include "../_common_cpp.h"
#include "../../files/tes_file_reading/file_loader.h"
#include "../../files/tes_file_writing/config.h"
#include "../../files/tes_file_writing/worldspace_cell_offset_state.h"
#include "../../form_stubs/helpers/for_each_child_form.h"
#include "../../form_stubs/helpers/get_worldspace_persistent_cell.h"
#include "../../form_stubs/helpers/needs_to_be_saved.h"

static bool _record_may_be_compressed(const dovah::tes_file_writing::record& record, const dovah::load_order_interfaces::form_save& intfc) {
   if (record.flags() & dovah::tes_file_record_header::flag::compressed)
      return true;
   if (intfc.get_save_config().record_compression == dovah::tes_file_writing::record_compression_policy::threshold)
      //
      // Record will be compressed if it's sufficiently large; we can't (easily) 
      // know this in advance. (I mean, we could have the caller check how much 
      // it's written into the record and manually compare that, plus the size 
      // of any potential OFST data, to the compression threshold; but I don't 
      // want to bother with that right now.)
      //
      return true;
   return false;
}

static bool _worldspace_has_cells_to_save(const dovah::form_stub& worldspace) {
   bool  any_forms_need_saving = false;
   auto* persistent_cell = dovah::form_stub_helpers::get_worldspace_persistent_cell(worldspace);
   dovah::form_stub_helpers::for_each_child_form(worldspace, [persistent_cell, &any_forms_need_saving](dovah::form_stub& child) -> bool {
      if (child.form_type != dovah::form_type::cell)
         return false;
      if (&child == persistent_cell)
         return false;
      if (!dovah::form_stub_helpers::needs_to_be_saved(child))
         return false;
      any_forms_need_saving = true;
      return true; // stop
   });
   return any_forms_need_saving;
}

namespace dovah::loaded_forms::structs {
   void world_cell_offset_data::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (subrecord.signature() == subrecord_signature) {
         const auto* file = intfc.current_file;
         auto&       data = this->offsets_by_file[file];

         const size_t size = subrecord.size() / sizeof(uint32_t);
         data.relative_offsets.resize(size);
         for (size_t i = 0; i < size; ++i)
            subrecord.unchecked_read(data.relative_offsets[i]);
         return;
      }
   }
   void world_cell_offset_data::save(tes_record_writer& record, load_order_interfaces::form_save& intfc, const world_bounds& bounds) {
      const auto* active_file = intfc.owner.get_active_file();
      assert(!!active_file);
      structs::world_cell_offset_data::offset_collection* data = &this->offsets_by_file[active_file];
      int32_t  min_gx = 0;
      int32_t  min_gy = 0;
      uint32_t grid_w = 0;
      uint32_t grid_h = 0;

      const bool saving_as_master = intfc.get_save_config().file_flags & tes_file_flag::master;

      data->bounds = bounds;
      data->relative_offsets.clear();
      {
         bool bounds_valid = data->bounds.valid();
         if (bounds_valid) {
            min_gx = (int32_t)data->bounds.min.x >> 0xC;
            min_gy = (int32_t)data->bounds.min.y >> 0xC;
            grid_w = ((int32_t)data->bounds.max.x >> 0xC) - min_gx;
            grid_h = ((int32_t)data->bounds.max.y >> 0xC) - min_gy;
            if (grid_w >= 1000 || grid_h >= 1000) { // CK-imposed maximums
               bounds_valid = false;
            }
         }

         if (
            // The OFST grid is not meaningful if there are no associated bounds.
            bounds_valid
            // The game doesn't load the OFST grid's bounds if the file is not a master.
         && saving_as_master
            // We have to write placeholders into the WRLD record, and then as we go and 
            // write the later CELL records, we have to jump back into the WRLD record to 
            // write the OFST entries. We can't do this if the WRLD record is compressed.
            //
            // The CK has the same limitation, but since Bethesda only compresses specific 
            // record types, they never encounter it.
         && !_record_may_be_compressed(record, intfc)
            // Only write OFST if the worldspace's (non-persistent) cells will actually 
            // be written into this file:
         && _worldspace_has_cells_to_save(*intfc.target_stub)
         ) {

            data->relative_offsets.resize(grid_w * grid_h, 0);
            //
            // The `file_writer` will have to fill in entries as it writes out each cell.
            //
         }
      }
      if (data->relative_offsets.empty()) {
         data = nullptr;
         this->offsets_by_file.erase(active_file);
         return;
      }
      auto& subrecord = record.open_next_subrecord('OFST');
      {
         //
         // Tell the `file_writer` where to find the cell offset state. The writer will 
         // take ownership of this pointer.
         //
         auto* state = new tes_file_writing::worldspace_cell_offset_state;
         state->offset_within_record = subrecord.header_pos();
         state->grid = {
            .min    = { min_gx, min_gy },
            .width  = grid_w,
            .height = grid_h,
         };
         intfc.worldspace_cell_offset_state = state;
      }
      subrecord.write(data->relative_offsets.data(), data->relative_offsets.size() * sizeof(uint32_t));
      subrecord.close();
   }
}