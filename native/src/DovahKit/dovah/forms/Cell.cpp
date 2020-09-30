#include "Cell.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Cell::load(tes_record_reader& record) {
      Form::load(record);
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'DATA':
               subrecord.read(this->cell_flags);
               break;
            case 'XCLC':
               subrecord.read(this->grid_coords.x);
               subrecord.read(this->grid_coords.y);
               subrecord.read(this->land_flags);
               break;
            case 'XCLL':
               this->interior.lighting.load(subrecord);
               break;
            case 'LTMP':
               subrecord.read(this->interior.lighting_template_ID);
               break;
            case 'TVDT':
               this->exterior.occlusion_data.present = true;
               this->exterior.occlusion_data.bytes.resize(subrecord.size());
               subrecord.read(this->exterior.occlusion_data.bytes.data(), subrecord.size());
               break;
            case 'MHDT':
               /*// whoops, this is for WRLD
               {
                  auto& mhdt = this->exterior.max_height_data;
                  subrecord.read(mhdt.min.x);
                  subrecord.read(mhdt.min.y);
                  subrecord.read(mhdt.max.x);
                  subrecord.read(mhdt.max.y);
                  while (subrecord.is_in_bounds(4)) {
                     auto& entry = mhdt.cells.emplace_back();
                     subrecord.unchecked_read(entry.sw);
                     subrecord.unchecked_read(entry.se);
                     subrecord.unchecked_read(entry.nw);
                     subrecord.unchecked_read(entry.ne);
                  }
               }
               //*/
               if (subrecord.is_in_bounds(sizeof(float) + 32 * 32)) {
                  auto& mhdt = this->exterior.max_height_data;
                  subrecord.unchecked_read(mhdt.offset);
                  for (auto& row : mhdt.grid)
                     for (auto& col : row)
                        subrecord.unchecked_read(col);
                  mhdt.present = true;
               }
               break;
            case 'LNAM':
               subrecord.read(this->interior.lighting.inherit_flags);
               break;
            case 'XCLW':
               subrecord.read(this->water.height);
               break;
            case 'XNAM':
               subrecord.to_string(this->water.noise_texture);
               break;
            case 'OBND':
               this->has_object_bounds = true;
               this->object_bounds.load(subrecord);
               break;
            case 'VMAD':
               this->script_data.load(subrecord);
               break;
            default:
               if (this->extra_data.load(record) == components::extra_data_load_result::unrecognized) {
                  //
                  // Subrecord is not extra-data.
                  //
               }
               break;
         }
      }
   }
   /*static*/ void Cell::generateUseInfo(tes_record_reader& record, form_stub* stub) {
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'LTMP':
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               break;
            default:
               if (components::extra_data_list::generate_use_info(record, stub) == components::extra_data_load_result::unrecognized) {
                  //
                  // Subrecord is not extra-data.
                  //
               }
               break;
         }
      }
   }
   bool Cell::_save_impl(tes_record_writer& record) {
      bool is_exterior = this->stub->is_exterior_cell();
      //
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->cell_flags);
      DATA.close();
      if (is_exterior) {
         auto& XCLC = record.open_next_subrecord('XCLC');
         XCLC.write(this->grid_coords.x);
         XCLC.write(this->grid_coords.y);
         XCLC.write(this->land_flags);
         XCLC.close();
      } else {
         auto& XCLL = record.open_next_subrecord('XCLL');
         this->interior.lighting.save(XCLL);
         XCLL.close();
      }
      if (this->exterior.occlusion_data.present) {
         auto& data = this->exterior.occlusion_data;
         auto& TVDT = record.open_next_subrecord('TVDT');
         TVDT.write(data.bytes.data(), data.bytes.size());
         TVDT.close();
      }
      if (this->exterior.max_height_data.present) {
         auto& data = this->exterior.max_height_data;
         auto& MHDT = record.open_next_subrecord('MHDT');
         MHDT.write(data.offset);
         for (auto& row : data.grid)
            for (auto& col : row)
               MHDT.write(col);
         MHDT.close();
      }
      record.write_formID_subrecord('LTMP', this->interior.lighting_template_ID);
      auto& XCLW = record.open_next_subrecord('XCLW');
      XCLW.write(this->water.height);
      XCLW.close();
      auto& XNAM = record.open_next_subrecord('XNAM');
      XNAM.write(this->water.noise_texture);
      XNAM.close();
      //
      this->extra_data.save(record);
      //
      if (this->has_object_bounds) {
         auto& subrecord = record.open_next_subrecord('OBND');
         this->object_bounds.save(subrecord);
         subrecord.close();
      }
      this->script_data.save(record); // VMAD (won't write anything if no scripts are attached)
      //
      return true;
   }
}