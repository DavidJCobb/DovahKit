#include "Worldspace.h"
#include "_common_cpp.h"
#include "factories/hardcoded.h"

namespace {
   //
   // xEdit discards this data when saving, since it should only appear in masters.
   //
   inline constexpr bool KEEP_WORLDSPACE_LARGE_REFERENCES = false;
}

namespace dovah::loaded_forms {
   void Worldspace::large_reference_t::clone_from(const large_reference_t& original, form_stub& my_owner) noexcept {
      if (!this->entries.empty()) {
         for (auto& entry : this->entries) {
            for (auto& ref : entry.refs)
               ref.form.set(my_owner, nullptr);
         }
         this->entries.clear();
      }
      //
      size_t size = original.entries.size();
      this->entries.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& entry = this->entries[i];
         auto& from  = original.entries[i];
         entry.y = from.y;
         entry.x = from.x;
         //
         size_t rs = from.refs.size();
         entry.refs.resize(rs);
         for (size_t j = 0; j < rs; ++j) {
            auto& ref   = entry.refs[j];
            auto& other = from.refs[j];
            ref.form.set(my_owner, other.form);
            ref.y = other.y;
            ref.x = other.x;
         }
      }
   }
   void Worldspace::large_reference_t::sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept {
      bool removals_pending = false;
      for (auto& entry : this->entries) {
         bool edited = false;
         for (auto& ref : entry.refs) {
            if (ref.form == &target) {
               ref.form.set(my_owner, nullptr);
               edited = true;
            }
         }
         if (edited) {
            auto& list = entry.refs;
            list.erase(
               std::remove_if(
                  list.begin(),
                  list.end(),
                  [](ref& e) {
                     return e.form == nullptr;
                  }
               ),
               list.end()
            );
            if (list.empty())
               removals_pending = true;
         }
      }
      if (removals_pending) {
         auto& list = this->entries;
         list.erase(
            std::remove_if(
               list.begin(),
               list.end(),
               [](entry& e) {
                  return e.refs.empty();
               }
            ),
            list.end()
         );
      }
   }

   void Worldspace::load(tes_record_reader& record) {
      Form::load(record);
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'RNAM':
               {  // SSE-only, but we'll still load it if we see it in a Classic file.
                  auto& data  = this->large_references;
                  auto& entry = data.entries.emplace_back();
                  subrecord.read(entry.y);
                  subrecord.read(entry.x);
                  while (subrecord.is_in_bounds(8)) {
                     auto& ref = entry.refs.emplace_back();
                     subrecord.unchecked_read(ref.form);
                     subrecord.unchecked_read(ref.y);
                     subrecord.unchecked_read(ref.x);
                  }
               }
               break;
            case 'DATA':
               if (subrecord.size() == 4) { // this is how the game does it
                  uint32_t temporary;
                  subrecord.unchecked_read(temporary);
                  this->world_flags = temporary;
               } else {
                  subrecord.read(this->world_flags);
               }
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
               this->has_object_bounds = true;
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
            case 'DNAM':
               subrecord.read(this->land_data.default_land_height);
               subrecord.read(this->land_data.default_water_height);
               break;
            case 'WNAM':
               subrecord.read(this->parent.form);
               break;
            case 'PNAM':
               subrecord.read(this->parent.flags);
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
               // TODO: Loader code.
               //
               break;
            case 'VMAD':
               this->script_data.load(subrecord);
               break;
         }
      }
   }
   /*static*/ void Worldspace::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'OBND': // bounds
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'CNAM': // climate
            case 'LTMP': // lighting template
            case 'XEZN': // encounter zone
            case 'XLCN': // location
            case 'NAM2': // water type
            case 'NAM3': // water type (LOD)
            case 'WNAM': // parent worldspace
            case 'ZNAM': // music type
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
            case 'RNAM': // large references // SSE-only, but we'll still load it if we see it in a Classic file.
               if (!subrecord.is_skyrim_special())
                  break;
               subrecord.skip_bytes(4);
               while (subrecord.is_in_bounds(8)) {
                  if (subrecord.read(formID))
                     uib.add_outbound_reference(formID);
                  subrecord.skip_bytes(4);
               }
               break;
         }
      }
   }
   void Worldspace::setup(const file_load_order& load_order) noexcept {
      auto* default_water = load_order.get_form(hardcoded_form_ids::DefaultWater);
      //
      this->water_type.set(*this->stub, default_water);
      this->water_type_lod.set(*this->stub, default_water);
   }
   bool Worldspace::_clone_impl(Form* out) const noexcept {
      auto* copy = dynamic_cast<Worldspace*>(out);
      if (!copy)
         return false;
      auto& clone = *copy;
      clone.large_references.clone_from(this->large_references, *clone.stub);
      clone.name = this->name;
      clone.max_height_data = this->max_height_data;
      clone.center_cell_coordinates = this->center_cell_coordinates;
      clone.climate.set(*clone.stub, this->climate);
      clone.lighting_template.set(*clone.stub, this->lighting_template);
      clone.encounter_zone.set(*clone.stub, this->encounter_zone);
      clone.location.set(*clone.stub, this->location);
      clone.water_type.set(*clone.stub, this->water_type);
      clone.water_type_lod.set(*clone.stub, this->water_type_lod);
      clone.lod_water_height = this->lod_water_height;
      clone.land_data = this->land_data;
      clone.parent.form.set(*clone.stub, this->parent.form);
      clone.parent.flags = this->parent.flags;
      clone.map_icon = this->map_icon;
      clone.cloud_model.clone_from(this->cloud_model, *clone.stub);
      clone.map_data = this->map_data;
      clone.map_offset_data = this->map_offset_data;
      clone.distant_lod_multiplier = this->distant_lod_multiplier;
      clone.world_flags = this->world_flags;
      clone.bounds = this->bounds;
      clone.music.set(*clone.stub, this->music);
      clone.tree_canopy_shadow = this->tree_canopy_shadow;
      clone.water_noise_texture = this->water_noise_texture;
      clone.hd_lod_diffuse_texture = this->hd_lod_diffuse_texture;
      clone.hd_lod_normal_texture = this->hd_lod_normal_texture;
      clone.offset_data = this->offset_data;
      clone.has_object_bounds = this->has_object_bounds;
      clone.object_bounds = this->object_bounds;
      clone.script_data.clone_from(this->script_data, *clone.stub);
      return true;
   }
   bool Worldspace::_save_impl(tes_record_writer& record) {
      bool is_fixed_dimensions = this->world_flags & world_flag::fixed_dimensions;
      if (KEEP_WORLDSPACE_LARGE_REFERENCES) {
         for (auto& entry : this->large_references.entries) {
            auto& subrecord = record.open_next_subrecord('RNAM');
            subrecord.write(entry.y);
            subrecord.write(entry.x);
            for (auto& ref : entry.refs) {
               subrecord.write(ref.form);
               subrecord.write(ref.y);
               subrecord.write(ref.x);
            }
            subrecord.close();
         }
      }
      if (this->max_height_data.present) { // under what conditions is this generated?
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
      if (is_fixed_dimensions) {
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
         PNAM.close();
      }
      if (!this->parent.form || !(this->parent.flags & parent_flag::use_parent_climate)) {
         if (this->climate)
            record.write_formID_subrecord('CNAM', this->climate);
      }
      if (!this->parent.form || !(this->parent.flags & parent_flag::use_parent_water)) {
         if (this->water_type)
            record.write_formID_subrecord('NAM2', this->water_type);
      }
      if (!this->parent.form || !(this->parent.flags & parent_flag::use_parent_lod)) {
         if (this->water_type_lod)
            record.write_formID_subrecord('NAM3', this->water_type_lod);
         auto& NAM4 = record.open_next_subrecord('NAM4');
         NAM4.write(this->lod_water_height);
         NAM4.close();
      }
      if (!this->parent.form || !(this->parent.flags & parent_flag::use_parent_land)) {
         auto& DNAM = record.open_next_subrecord('DNAM');
         DNAM.write(this->land_data.default_land_height);
         DNAM.write(this->land_data.default_water_height);
         DNAM.close();
      }
      if (!this->map_icon.empty()) {
         auto& ICON = record.open_next_subrecord('ICON');
         ICON.write(this->map_icon);
         ICON.close();
      }
      this->cloud_model.save(record, 'MODL', 'MODT', 'MODS');
      //
      if (!this->parent.form || !(this->parent.flags & parent_flag::use_parent_map)) {
         auto& MNAM = record.open_next_subrecord('MNAM');
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
      }
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
      if (this->offset_data.present) {
         //
         // And make a point of NOT saving OFST.
         //
      }
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
   void Worldspace::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this->stub);
      //
      this->climate.clear_if(*this->stub, other);
      this->lighting_template.clear_if(*this->stub, other);
      this->encounter_zone.clear_if(*this->stub, other);
      this->location.clear_if(*this->stub, other);
      this->water_type.clear_if(*this->stub, other);
      this->water_type_lod.clear_if(*this->stub, other);
      this->parent.form.clear_if(*this->stub, other);
      this->music.clear_if(*this->stub, other);
      this->climate.clear_if(*this->stub, other);
      this->climate.clear_if(*this->stub, other);
   }
}