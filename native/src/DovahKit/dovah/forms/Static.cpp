#include "Static.h"
#include "_common_cpp.h"

#include "../notices/form_save_errors/unprefixed_string_is_too_long_to_serialize.h"

namespace dovah::loaded_forms {
   void Static::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'OBND':
               this->bounds.load(subrecord, intfc);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               this->model.load(subrecord, intfc);
               break;
            case 'DNAM':
               {
                  auto& dm = this->directional_material;
                  subrecord.read(dm.max_angle);
                  subrecord.read(dm.material_object);
                  intfc.warn_if_ref_is_wrong_type(dm.material_object, form_type::material_object, subrecord.signature());
                  if (subrecord.is_skyrim_special()) {
                     subrecord.read(dm.flags);
                  } else {
                     dm.flags = 0;
                  }
               }
               break;
            case 'MNAM':
               {
                  for (auto& item : this->distant_lod_paths) {
                     item.resize(max_lod_mesh_path_length);
                     subrecord.read(item.data(), max_lod_mesh_path_length);
                     //
                     auto i = item.find('\0');
                     if (i != std::string::npos)
                        item.resize(i);
                  }
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Static::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      form_id_t directional_material_object;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'DNAM': // directional material data
               subrecord.skip_bytes(sizeof(directional_material_data::max_angle));
               subrecord.read(directional_material_object);
               if (subrecord.is_skyrim_special()) {
                  subrecord.skip_bytes(sizeof(directional_material_data::flags));
               }
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               decltype(model)::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case 'OBND': // bounds
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'EDID': // editor ID
            case 'MNAM': // distant LOD paths
               break;

         }
      }
      uib.add_outbound_reference(directional_material_object);
   }
   void Static::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Static*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->model.clone_from(this->model, *copy);
      {
         auto& src = this->directional_material;
         auto& dst = copy->directional_material;
         dst.max_angle = src.max_angle;
         dst.material_object.set(*copy, src.material_object);
         dst.flags     = src.flags;
      }
      copy->distant_lod_paths = this->distant_lod_paths;
   }
   void Static::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      {
         auto& dm = this->directional_material;
         auto& DNAM = record.open_next_subrecord('DNAM');
         DNAM.write(dm.max_angle);
         DNAM.write(dm.material_object);
         if (record.is_skyrim_special()) {
            DNAM.write(dm.flags);
         }
         DNAM.close();
      }
      {
         auto& MNAM = record.open_next_subrecord('MNAM');
         for (auto& item : this->distant_lod_paths) {
            size_t size = item.size();
            if (size >= max_lod_mesh_path_length) {
               auto notice = notices::form_save_errors::unprefixed_string_is_too_long_to_serialize(
                  *intfc.target_stub,
                  size,
                  max_lod_mesh_path_length,
                  MNAM.signature()
               );
               intfc.throw_save_error(notice);
               return;
            }
            MNAM.write(item.data(), size);
            MNAM.skip_bytes(256 - size);
         }
         MNAM.close();
      }
   }
   void Static::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      this->directional_material.material_object.clear_if(*this, other);
   }
   void Static::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->bounds.clear();
      this->model.clear(*this);
      {
         auto& dm = this->directional_material;
         dm.material_object.set(*this, nullptr);
         dm = directional_material_data();
      }
      for (auto& item : this->distant_lod_paths)
         item.clear();
   }
}