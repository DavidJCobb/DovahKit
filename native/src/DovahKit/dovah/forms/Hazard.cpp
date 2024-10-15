#include "Hazard.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Hazard::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               this->model.load(subrecord, intfc);
               break;
            case 'MNAM':
               if (auto& dst = this->imagespace_modifier; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::imagespace_modifier, subrecord.signature());
               break;
            case 'DATA':
               subrecord.read(this->limit);
               subrecord.read(this->radius);
               subrecord.read(this->lifetime);
               subrecord.read(this->imagespace_radius);
               subrecord.read(this->target_interval);
               subrecord.read(this->hazard_flags);
               if (auto& dst = this->spell; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, std::array{ form_type::spell, form_type::enchantment }, subrecord.signature());
               if (auto& dst = this->light; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::light, subrecord.signature());
               if (auto& dst = this->impact_data_set; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::impact_data_set, subrecord.signature());
               if (auto& dst = this->sound; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Hazard::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      
      form_id_t imagespace_modifier;
      form_id_t impact_data_set;
      form_id_t light;
      form_id_t sound;
      form_id_t spell;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'OBND':
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'MNAM':
               subrecord.read(imagespace_modifier);
               break;
            case 'DATA':
               subrecord.skip_bytes(
                  sizeof(limit) +
                  sizeof(radius) +
                  sizeof(lifetime) +
                  sizeof(imagespace_radius) +
                  sizeof(target_interval) +
                  sizeof(hazard_flags)
               );
               subrecord.read(spell);
               subrecord.read(light);
               subrecord.read(impact_data_set);
               subrecord.read(sound);
               break;
         }
      }
      uib.add_outbound_reference(imagespace_modifier);
      uib.add_outbound_reference(spell);
      uib.add_outbound_reference(light);
      uib.add_outbound_reference(impact_data_set);
      uib.add_outbound_reference(sound);
   }
   void Hazard::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Hazard*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->model.clone_from(this->model, *copy);
      copy->name = this->name;
      copy->imagespace_modifier.set(*copy, this->imagespace_modifier);
      copy->spell.set(*copy, this->spell);
      copy->light.set(*copy, this->light);
      copy->impact_data_set.set(*copy, this->impact_data_set);
      copy->sound.set(*copy, this->sound);
      copy->limit = this->limit;
      copy->radius = this->radius;
      copy->lifetime = this->lifetime;
      copy->imagespace_radius = this->imagespace_radius;
      copy->target_interval = this->target_interval;
      copy->hazard_flags = this->hazard_flags;
   }
   void Hazard::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      record.write_formID_subrecord('MNAM', this->imagespace_modifier, true);
      {
         auto& DATA = record.open_next_subrecord('DATA');
         DATA.write(this->limit);
         DATA.write(this->radius);
         DATA.write(this->lifetime);
         DATA.write(this->imagespace_radius);
         DATA.write(this->target_interval);
         DATA.write(this->hazard_flags);
         DATA.write(this->spell);
         DATA.write(this->light);
         DATA.write(this->impact_data_set);
         DATA.write(this->sound);
         DATA.close();
      }
   }
   void Hazard::_clear_impl() noexcept {
      this->bounds.clear();
      this->model.clear(*this);
      this->script_data.clear(*this);
      this->imagespace_modifier.set(*this, nullptr);
      this->limit = 0;
      this->radius = 0;
      this->lifetime = 0;
      this->imagespace_radius = 0;
      this->target_interval = 0;
      this->hazard_flags = 0;
      this->spell.set(*this, nullptr);
      this->light.set(*this, nullptr);
      this->impact_data_set.set(*this, nullptr);
      this->sound.set(*this, nullptr);
   }
   void Hazard::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      this->imagespace_modifier.clear_if(*this, other);
      this->spell.clear_if(*this, other);
      this->light.clear_if(*this, other);
      this->impact_data_set.clear_if(*this, other);
      this->sound.clear_if(*this, other);
   }
}