#include "LandTexture.h"
#include "_common_cpp.h"
#include "../notice_code_list.h"

namespace dovah::loaded_forms {
   void LandTexture::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      form_reference_t form_id;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'OBND':
               this->bounds.load(subrecord, intfc);
               break;
            case 'TNAM':
               if (subrecord.read(this->texture_set)) {
                  intfc.warn_if_ref_is_wrong_type(this->texture_set, form_type::texture_set, subrecord.signature());
               }
               break;
            case 'MNAM':
               if (subrecord.read(this->havok.material)) {
                  intfc.warn_if_ref_is_wrong_type(this->havok.material, form_type::material_type, subrecord.signature());
               }
               break;
            case 'HNAM':
               subrecord.read(this->havok.friction);
               subrecord.read(this->havok.restitution);
               break;
            case 'SNAM':
               subrecord.read(this->specular_exponent);
               break;
            case 'GNAM':
               if (subrecord.read(form_id) && form_id) {
                  this->grasses.push_back(form_id);
                  intfc.warn_if_ref_is_wrong_type(form_id, form_type::grass, subrecord.signature());
               }
               break;
            case 'INAM':
               if (subrecord.is_skyrim_special()) {
                  subrecord.read(this->remaster_flags);
                  break;
               }
               [[fallthrough]];
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void LandTexture::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      form_id_t id;
      std::vector<form_id_t> grasses;
      form_id_t havok_material;
      form_id_t texture_set;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'OBND':
               break;
            case 'TNAM':
               subrecord.read(texture_set);
               break;
            case 'MNAM':
               subrecord.read(havok_material);
               break;
            case 'HNAM':
               break;
            case 'SNAM':
               break;
            case 'GNAM':
               {
                  subrecord.read(id);
                  grasses.push_back(id);
               }
               break;
            case 'INAM':
               if (subrecord.is_skyrim_special()) {
                  break;
               }
               [[fallthrough]];
            default:
               break;
         }
      }
      uib.add_outbound_reference(havok_material);
      uib.add_outbound_reference(texture_set);
      for (const auto id : grasses)
         uib.add_outbound_reference(id);
   }
   bool LandTexture::_clone_impl(Form* out) const noexcept {
      if (out->type != form_type)
         return false;
      auto* copy = (LandTexture*)out;
      //
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->texture_set.set(*copy, this->texture_set);
      copy->havok.material.set(*copy, this->havok.material);
      copy->havok.friction    = this->havok.friction;
      copy->havok.restitution = this->havok.restitution;
      copy->specular_exponent = this->specular_exponent;
      copy->remaster_flags    = this->remaster_flags;
      copy_form_reference_list(*copy, copy->grasses, this->grasses);
      //
      return true;
   }
   bool LandTexture::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      if (!this->bounds.is_zero()) {
         auto& OBND = record.open_next_subrecord('OBND');
         this->bounds.save(OBND, intfc);
         OBND.close();
      }
      record.write_formID_subrecord('TNAM', this->texture_set,    true);
      record.write_formID_subrecord('MNAM', this->havok.material, true);
      //
      auto& HNAM = record.open_next_subrecord('HNAM');
      HNAM.write(this->havok.friction);
      HNAM.write(this->havok.restitution);
      HNAM.close();
      //
      auto& SNAM = record.open_next_subrecord('SNAM');
      SNAM.write(this->specular_exponent);
      SNAM.close();
      //
      for (auto& ref : this->grasses) {
         record.write_formID_subrecord('GNAM', ref, true);
      }
      //
      if (record.is_skyrim_special()) {
         auto& INAM = record.open_next_subrecord('INAM');
         INAM.write(this->remaster_flags);
         INAM.close();
      }
      return true;
   }
   void LandTexture::_clear_impl() noexcept {
      this->texture_set.set(*this, nullptr);
      this->havok.material.set(*this, nullptr);
      clear_form_reference_list(this->grasses, *this);
      this->remaster_flags = 0;
      //
      this->script_data.clear(*this);
   }
   void LandTexture::_sever_outbound_references_impl(form_stub& other) noexcept {
      remove_form_from_reference_list(this->grasses, other, *this);
      this->texture_set.clear_if(*this, other);
      this->havok.material.clear_if(*this, other);
      //
      this->script_data.sever_outbound_references_to(other, *this);
   }
}