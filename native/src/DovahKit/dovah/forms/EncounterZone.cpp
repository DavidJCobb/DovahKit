#include "EncounterZone.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void EncounterZone::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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

            case 'DATA':
               if (auto& form = this->owner; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, std::array{ form_type::actor_base, form_type::faction }, subrecord.signature());
               if (auto& form = this->location; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::location, subrecord.signature());
               subrecord.read(this->data.rank);
               subrecord.read(this->data.min_level);
               subrecord.read(this->data.flags);
               subrecord.read(this->data.max_level);
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void EncounterZone::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;

      form_id_t owner;
      form_id_t location;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;

            case 'DATA':
               subrecord.read(owner);
               subrecord.read(location);
               break;
         }
      }
      uib.add_outbound_reference(owner);
      uib.add_outbound_reference(location);
   }
   void EncounterZone::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (EncounterZone*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->owner.set(*copy, this->owner);
      copy->location.set(*copy, this->location);
      copy->data = this->data;
   }
   void EncounterZone::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->owner);
         subrecord.write(this->location);
         subrecord.write(this->data.rank);
         subrecord.write(this->data.min_level);
         subrecord.write(this->data.flags);
         subrecord.write(this->data.max_level);
         subrecord.close();
      }
   }
   void EncounterZone::_clear_impl() noexcept {
      this->script_data.clear(*this);

      this->owner.set(*this, nullptr);
      this->location.set(*this, nullptr);
      this->data = {};
   }
   void EncounterZone::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);

      this->owner.clear_if(*this, other);
      this->location.clear_if(*this, other);
   }
}