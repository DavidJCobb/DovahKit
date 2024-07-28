#include "Class.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Class::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      bool content_loaded = false;
      form_reference_t form_id;
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
               //
               // The loader checks for this and passes it to a virtual function on TESForm 
               // that's responsible for loading it. However, this form doesn't derive from 
               // TESBoundObject, so the TESForm implementation of that virtual function (a 
               // no-op) isn't overridden and therefore the data is not retained in memory.
               //
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'DESC':
               subrecord.read(this->description);
               break;
            case 'ICON':
               subrecord.read(this->icon);
               break;
            case 'DATA':
               {
                  int8_t skill;

                  subrecord.read(this->unk30);
                  if (subrecord.read(skill))
                     this->training.skill = (dovah::skill)skill;
                  subrecord.read(this->training.max_level);
                  subrecord.read(this->skill_weights);
                  subrecord.read(this->bleedout_default);
                  subrecord.read(this->voice_points);
                  subrecord.read(this->attribute_weights.health);
                  subrecord.read(this->attribute_weights.magicka);
                  subrecord.read(this->attribute_weights.stamina);
                  subrecord.read(this->attribute_weights.unknown);
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Class::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'FULL':
               break;
            case 'ICON':
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'DATA':
               break;
         }
      }
   }
   void Class::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Class*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->name         = this->name;
      copy->description  = this->description;
      copy->icon         = this->icon;
      copy->training     = this->training;
      copy->skill_weights     = this->skill_weights;
      copy->attribute_weights = this->attribute_weights;
      copy->bleedout_default  = this->bleedout_default;
      copy->voice_points      = this->voice_points;
      copy->unk30 = this->unk30;
   }
   void Class::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      auto& DESC = record.open_next_subrecord('DESC');
      DESC.write(this->description);
      DESC.close();
      if (!this->icon.empty())
         record.write_string_subrecord('ICON', this->icon);
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->unk30);
      DATA.write((int8_t)this->training.skill);
      DATA.write(this->training.max_level);
      DATA.write(this->skill_weights);
      DATA.write(this->bleedout_default);
      DATA.write(this->voice_points);
      DATA.write(this->attribute_weights.health);
      DATA.write(this->attribute_weights.magicka);
      DATA.write(this->attribute_weights.stamina);
      DATA.write(this->attribute_weights.unknown);
      DATA.close();
   }
   void Class::_clear_impl() noexcept {
      this->name.reset();
      this->description.reset();
      this->icon.clear();
      this->script_data.clear(*this);

      this->training = {};
      this->skill_weights = { 0 };
      this->attribute_weights = {};
      this->unk30 = 0;
      this->bleedout_default = 0;
      this->voice_points = 0;
   }
   void Class::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}