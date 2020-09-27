#include "Worldspace.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Worldspace::load(tes_record_reader& record) {
      Form::load(record);
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'DATA':
               subrecord.read(this->world_flags);
               break;
            case 'MHDT':
               {
                  auto& mhdt = this->max_height_data;
                  mhdt.present = true;
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
               break;
            case 'OBND':
               this->object_bounds.load(subrecord);
               break;
            case 'FULL':
               subrecord.to_string(this->name);
               break;
            case 'WCTR':
               subrecord.read(this->center_cell_coordinates.x);
               subrecord.read(this->center_cell_coordinates.y);
               break;
            case 'CNAM':
               subrecord.read(this->climate);
               break;
            case 'LTMP':
               subrecord.read(this->lighting_template);
               break;
            case 'XEZN':
               subrecord.read(this->encounter_zone);
               break;
            case 'XLCN':
               subrecord.read(this->location);
               break;
            case 'NAM2':
               subrecord.read(this->water_type);
               break;
            case 'NAM3':
               subrecord.read(this->water_type_lod);
               break;
            case 'NAM4':
               subrecord.read(this->lod_water_height);
               break;
            case 'ICON':
               subrecord.to_string(this->map_icon);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               this->cloud_model.load(subrecord);
               break;
            case 'MNAM':
               subrecord.read(this->map_data.usable_dimensions.x);
               subrecord.read(this->map_data.usable_dimensions.y);
               subrecord.read(this->map_data.coordinates.northwest.x);
               subrecord.read(this->map_data.coordinates.northwest.y);
               subrecord.read(this->map_data.coordinates.southeast.x);
               subrecord.read(this->map_data.coordinates.southeast.y);
               subrecord.read(this->map_data.camera.height_min);
               subrecord.read(this->map_data.camera.height_max);
               subrecord.read(this->map_data.camera.initial_pitch);
               break;
            case 'ONAM':
               subrecord.read(this->map_offset_data.scale);
               subrecord.read(this->map_offset_data.offset.x);
               subrecord.read(this->map_offset_data.offset.y);
               subrecord.read(this->map_offset_data.offset.z);
               break;
            case 'NAMA':
               subrecord.read(this->distant_lod_multiplier);
               break;
            case 'NAM0':
               subrecord.read(this->bounds.min.x);
               subrecord.read(this->bounds.min.y);
               break;
            case 'NAM9':
               subrecord.read(this->bounds.max.x);
               subrecord.read(this->bounds.max.y);
               break;
            case 'ZNAM':
               subrecord.read(this->music);
               break;
            case 'NNAM':
               subrecord.to_string(this->tree_canopy_shadow);
               break;
            case 'XNAM':
               subrecord.to_string(this->water_environment_map);
               break;
            case 'TNAM':
               subrecord.to_string(this->hd_lod_diffuse_texture);
               break;
            case 'UNAM':
               subrecord.to_string(this->hd_lod_normal_texture);
               break;
            case 'XWEM':
               subrecord.to_string(this->water_environment_map);
               break;
            case 'OFST':
               //
               // TODO
               //
               break;
         }
      }
   }
   /*static*/ void Worldspace::generateUseInfo(tes_record_reader& record, form_stub* stub) {
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'CNAM': // climate
            case 'LTMP': // lighting template
            case 'XEZN': // encounter zone
            case 'XLCN': // location
            case 'NAM2': // water type
            case 'NAM3': // water type (LOD)
            case 'WNAM': // parent worldspace
            case 'ZNAM': // music type
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               break;
         }
      }
   }
   bool Worldspace::_save_impl(tes_record_writer& record) {
      bool is_fixed_dimensions = this->world_flags & world_flag::fixed_dimensions;
      //
      // TODO: RNAM
      //
      if (this->max_height_data.present) {
         auto& MHDT = record.open_next_subrecord('MHDT');
         auto& data = this->max_height_data;
         MHDT.write(data.min.x);
         MHDT.write(data.min.y);
         MHDT.write(data.max.x);
         MHDT.write(data.max.y);
         for (auto& entry : data.cells) {
            MHDT.write(entry.sw);
            MHDT.write(entry.se);
            MHDT.write(entry.nw);
            MHDT.write(entry.ne);
         }
         MHDT.close();
      }
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      if (is_fixed_dimensions) { // TODO: test with the CK to see whether it strips this for non-fixed-dimensions worldspaces
         auto& WCTR = record.open_next_subrecord('WCTR');
         WCTR.write(this->center_cell_coordinates.x);
         WCTR.write(this->center_cell_coordinates.y);
         WCTR.close();
      }
      if (this->lighting_template)
         record.write_formID_subrecord('LTMP', this->lighting_template);
      if (this->encounter_zone)
         record.write_formID_subrecord('XEZN', this->encounter_zone);
      if (this->location)
         record.write_formID_subrecord('XLCN', this->location);
      if (this->parent.form) {
         record.write_formID_subrecord('WNAM', this->parent.form);
         auto& PNAM = record.open_next_subrecord('PNAM');
         PNAM.write(this->parent.flags);
         PNAM.write(this->parent.pad05);
         PNAM.close();
      }
      if (this->climate)
         record.write_formID_subrecord('CNAM', this->climate);
      if (this->water_type)
         record.write_formID_subrecord('NAM2', this->water_type);
      if (this->water_type_lod)
         record.write_formID_subrecord('NAM3', this->water_type_lod);
      auto& NAM4 = record.open_next_subrecord('NAM4'); // TODO: this is only written if NAM3 is also written?
      NAM4.write(this->lod_water_height);
      NAM4.close();
      auto& DNAM = record.open_next_subrecord('DNAM'); // TODO: under what circumstances is this NOT written? when both floats are zero?
      DNAM.write(this->land_data.default_land_height);
      DNAM.write(this->land_data.default_water_height);
      DNAM.close();
      if (!this->map_icon.empty()) {
         auto& ICON = record.open_next_subrecord('ICON');
         ICON.write(this->map_icon);
         ICON.close();
      }
      this->cloud_model.save(record, 'MODL', 'MODT', 'MODS');
      //
      auto& MNAM = record.open_next_subrecord('MNAM'); // TODO: under what circumstances is this NOT written? when both floats are zero?
      MNAM.write(this->map_data.usable_dimensions.x);
      MNAM.write(this->map_data.usable_dimensions.y);
      MNAM.write(this->map_data.coordinates.northwest.x);
      MNAM.write(this->map_data.coordinates.northwest.y);
      MNAM.write(this->map_data.coordinates.southeast.x);
      MNAM.write(this->map_data.coordinates.southeast.y);
      MNAM.write(this->map_data.camera.height_min);
      MNAM.write(this->map_data.camera.height_max);
      MNAM.write(this->map_data.camera.initial_pitch);
      MNAM.close();
      //
      auto& ONAM = record.open_next_subrecord('ONAM');
      ONAM.write(this->map_offset_data.scale);
      ONAM.write(this->map_offset_data.offset.x);
      ONAM.write(this->map_offset_data.offset.y);
      ONAM.write(this->map_offset_data.offset.z);
      ONAM.close();
      //
      auto& NAMA = record.open_next_subrecord('NAMA');
      NAMA.write(this->distant_lod_multiplier);
      NAMA.close();
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->world_flags);
      DATA.close();
      auto& NAM0 = record.open_next_subrecord('NAM0');
      NAM0.write(this->bounds.min.x);
      NAM0.write(this->bounds.min.y);
      NAM0.close();
      auto& NAM9 = record.open_next_subrecord('NAM9');
      NAM9.write(this->bounds.max.x);
      NAM9.write(this->bounds.max.y);
      NAM9.close();
      if (this->music)
         record.write_formID_subrecord('ZNAM', this->music);
      if (!this->tree_canopy_shadow.empty()) {
         auto& subrecord = record.open_next_subrecord('NNAM');
         subrecord.write(this->tree_canopy_shadow);
         subrecord.close();
      }
      if (!this->water_noise_texture.empty()) {
         auto& subrecord = record.open_next_subrecord('XNAM');
         subrecord.write(this->water_noise_texture);
         subrecord.close();
      }
      if (!this->hd_lod_diffuse_texture.empty()) {
         auto& subrecord = record.open_next_subrecord('TNAM');
         subrecord.write(this->hd_lod_diffuse_texture);
         subrecord.close();
      }
      if (!this->hd_lod_normal_texture.empty()) {
         auto& subrecord = record.open_next_subrecord('UNAM');
         subrecord.write(this->hd_lod_normal_texture);
         subrecord.close();
      }
      if (!this->water_environment_map.empty()) {
         auto& subrecord = record.open_next_subrecord('XWEM');
         subrecord.write(this->water_environment_map);
         subrecord.close();
      }
      //
      // And make a point of NOT saving OFST.
      //

      //
      return true;
   }
}