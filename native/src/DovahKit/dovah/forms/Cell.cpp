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
            case 'XEZN':
               subrecord.read(this->encounter_zone_ID);
               break;
            case 'IMGS':
               subrecord.read(this->imagespace_ID);
               break;
            case 'XLCN':
               subrecord.read(this->location_ID);
               break;
            case 'XCMO':
               subrecord.read(this->music_type_ID);
               break;
            case 'XCCM':
               subrecord.read(this->sky_region_ID);
               break;
            case 'XCLL':
               this->interior.lighting.load(subrecord);
               break;
            case 'XCAS':
               subrecord.read(this->interior.acoustic_space_ID);
               break;
            case 'LTMP':
               subrecord.read(this->interior.lighting_template_ID);
               break;
            case 'XILL':
               subrecord.read(this->interior.lock_list_ID);
               break;
            case 'XOWN':
               subrecord.read(this->interior.owner_ID);
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
            case 'XCLR':
               while (subrecord.is_in_bounds(4)) {
                  if (subrecord.read(formID))
                     this->exterior.containing_region_IDs.push_back(formID);
               }
               break;
            case 'XCLW':
               subrecord.read(this->water.height);
               break;
            case 'XCWT':
               subrecord.read(this->water.type);
               break;
            case 'XWEM':
               subrecord.to_string(this->water.environment_map);
               break;
            case 'XNAM':
               subrecord.to_string(this->water.noise_texture);
               break;
            case 'XWCU':
               this->water.velocity.present = true;
               subrecord.read(this->water.velocity.linear.x);
               subrecord.read(this->water.velocity.linear.y);
               subrecord.read(this->water.velocity.linear.z);
               subrecord.read(this->water.velocity.unk0C);
               subrecord.read(this->water.velocity.angular.x);
               subrecord.read(this->water.velocity.angular.y);
               subrecord.read(this->water.velocity.angular.z);
               break;
            case 'XWCN':
               //
               // TODO
               //
               break;
            case 'XWCS':
               //
               // TODO
               //
               break;
            //
            // Miscellaneous extra-data:
            //
            case 'XGLB':
               subrecord.read(this->misc_extra.global_ID);
               break;
            case 'XTNM':
               subrecord.read(this->misc_extra.teleport_message_ID);
               break;
         }
      }
   }
   /*static*/ void Cell::generateUseInfo(tes_record_reader& record, form_stub* stub) {
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'XEZN':
            case 'IMGS':
            case 'XLCN':
            case 'XCMO':
            case 'XCCM':
            case 'XCAS':
            case 'LTMP':
            case 'XILL':
            case 'XOWN':
            case 'XCWT':
            case 'XGLB': // miscellaneous extra data...
            case 'XTNM':
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               break;
            case 'XCLR':
               while (subrecord.is_in_bounds(4))
                  if (subrecord.read(formID))
                     stub->add_outbound_reference(formID);
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
      //
      // TODO: TVDT
      //
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
      //
      // TODO: LNAM
      //
      auto& XCLW = record.open_next_subrecord('XCLW');
      XCLW.write(this->water.height);
      XCLW.close();
      //
      // TODO: XNAM
      // TODO: XCLR
      //
      // --- Forms below here can appear in any order ---
      //
      if (this->location_ID)
         record.write_formID_subrecord('XLCN', this->location_ID);
      //
      // TODO: XWCS
      // TODO: XWCN
      //
      if (this->water.velocity.present) {
         auto& data = this->water.velocity;
         auto& XWCU = record.open_next_subrecord('XWCU');
         XWCU.write(this->water.velocity.linear.x);
         XWCU.write(this->water.velocity.linear.y);
         XWCU.write(this->water.velocity.linear.z);
         XWCU.write(this->water.velocity.unk0C);
         XWCU.write(this->water.velocity.angular.x);
         XWCU.write(this->water.velocity.angular.y);
         XWCU.write(this->water.velocity.angular.z);
         XWCU.close();
      }
      if (this->water.type)
         record.write_formID_subrecord('XCWT', this->water.type);
      if (this->interior.owner_ID)
         record.write_formID_subrecord('XOWN', this->interior.owner_ID);
      if (this->interior.lock_list_ID)
         record.write_formID_subrecord('XILL', this->interior.lock_list_ID);
      //
      // TODO: XWEM
      //
      if (this->sky_region_ID)
         record.write_formID_subrecord('XCCM', this->sky_region_ID);
      if (this->interior.acoustic_space_ID)
         record.write_formID_subrecord('XCAS', this->interior.acoustic_space_ID);
      if (this->encounter_zone_ID)
         record.write_formID_subrecord('XEZN', this->encounter_zone_ID);
      if (this->music_type_ID)
         record.write_formID_subrecord('XCMO', this->music_type_ID);
      if (this->imagespace_ID)
         record.write_formID_subrecord('XCIM', this->imagespace_ID);
      return true;
   }
}