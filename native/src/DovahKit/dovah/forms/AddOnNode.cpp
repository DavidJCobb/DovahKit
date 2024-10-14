#include "AddOnNode.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void AddOnNode::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;
      
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
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               this->model.load(subrecord, intfc);
               break;
            case 'DATA':
               subrecord.read(this->unique_id);
               break;
            case 'DNAM':
               subrecord.read(this->master_particle_system_cap);
               {
                  uint8_t flags;
                  if (subrecord.read(flags)) {
                     this->addon_flags.is_valid_master_particle_system = (flags & 1);
                     this->addon_flags.always_loaded = (flags & 2);
                  }
               }
               subrecord.skip_bytes(1);
               break;
            case 'SNAM':
               if (auto& dst = this->sound; subrecord.read(dst)) {
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void AddOnNode::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      
      form_id_t sound;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               decltype(model)::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case 'OBND':
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'DATA':
               break;
            case 'SNAM':
               subrecord.read(sound);
               break;
         }
      }
      uib.add_outbound_reference(sound);
   }
   void AddOnNode::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (AddOnNode*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->model.clone_from(this->model, *copy);

      copy->unique_id   = this->unique_id;
      copy->master_particle_system_cap = this->master_particle_system_cap;
      copy->addon_flags = this->addon_flags;
      copy->sound.set(*copy, this->sound);
   }
   void AddOnNode::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      {
         auto& DATA = record.open_next_subrecord('DATA');
         DATA.write(this->unique_id);
         DATA.close();
      }
      record.write_formID_subrecord('SNAM', this->sound, true);
      {
         auto& DNAM = record.open_next_subrecord('DNAM');
         DNAM.write(this->master_particle_system_cap);
         {
            uint8_t flags = 0;
            flags |= (this->addon_flags.is_valid_master_particle_system);
            flags |= (this->addon_flags.always_loaded) << 1;
            DNAM.write(flags);
         }
         DNAM.skip_bytes(1);
         DNAM.close();
      }
   }
   void AddOnNode::_clear_impl() noexcept {
      this->bounds.clear();
      this->model.clear(*this);
      this->script_data.clear(*this);
      this->sound.set(*this, nullptr);
      this->unique_id = 0;
      this->master_particle_system_cap = 0;
      this->addon_flags = {};
   }
   void AddOnNode::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      this->sound.clear_if(*this, other);
   }
}