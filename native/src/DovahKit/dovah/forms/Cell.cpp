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
               subrecord.skip_bytes(sizeof(group_stub::gridX)); // form stubs store this information
               subrecord.skip_bytes(sizeof(group_stub::gridY)); // form stubs store this information
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
   void Cell::setup() noexcept {
      cobb::edit_bit(this->cell_flags, cell_flag::interior, this->stub->groupInfo.parentFormID == 0);
   }
   bool Cell::would_bethesda_compress() const noexcept {
      if (this->exterior.occlusion_data.present)
         //
         // Skyrim.esm only seems to compress CELLs if they contain TVDT...
         //
         return true;
      if (this->exterior.max_height_data.present)
         //
         // ...but I've seen the Creation Kit compress cells that contain MHDT as well.
         //
         return true;
      return false;
   }
   bool Cell::_clone_impl(Form* out) const noexcept {
      auto copy = dynamic_cast<Cell*>(out);
      if (!copy)
         return false;
      copy->name = this->name;
      copy->cell_flags = this->cell_flags;
      copy->land_flags = this->land_flags;
      copy->extra_data.clone_from(this->extra_data, *copy->stub);
      copy->interior.lighting = this->interior.lighting;
      copy->interior.lighting_template_ID.set(copy->stub, this->interior.lighting_template_ID);
      copy->exterior.occlusion_data = this->exterior.occlusion_data;
      copy->exterior.max_height_data = this->exterior.max_height_data;
      copy->water.height = this->water.height;
      copy->water.noise_texture = this->water.noise_texture;
      copy->has_object_bounds = this->has_object_bounds;
      copy->object_bounds = this->object_bounds;
      copy->script_data.clone_from(this->script_data, *copy->stub);
      return true;
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
         auto* stub = this->stub;
         XCLC.write(stub->groupInfo.gridX);
         XCLC.write(stub->groupInfo.gridY);
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