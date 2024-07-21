#include "Relationship.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Relationship::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      form_reference_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
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
            case 'DATA':
               {
                  if (subrecord.read(this->referrer)) {
                     intfc.warn_if_ref_is_wrong_type(this->referrer, form_type::actor_base, subrecord.signature());
                  }
                  if (subrecord.read(this->referent)) {
                     intfc.warn_if_ref_is_wrong_type(this->referent, form_type::actor_base, subrecord.signature());
                  }
                  subrecord.read(this->rank);
                  subrecord.read(this->flags);
                  if (subrecord.read(this->association_type)) {
                     intfc.warn_if_ref_is_wrong_type(this->association_type, form_type::association_type, subrecord.signature());
                  }
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Relationship::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      //
      form_id_t referrer;
      form_id_t referent;
      form_id_t association_type;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'DATA':
               subrecord.read(referrer);
               subrecord.read(referent);
               subrecord.skip_bytes(sizeof(rank));
               subrecord.skip_bytes(sizeof(flags));
               subrecord.read(association_type);
               break;
         }
      }
      uib.add_outbound_reference(referrer);
      uib.add_outbound_reference(referent);
      uib.add_outbound_reference(association_type);
   }
   void Relationship::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Relationship*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      //
      copy->referrer.set(*copy, this->referrer);
      copy->referent.set(*copy, this->referent);
      copy->flags = this->flags;
      copy->rank  = this->rank;
      copy->association_type.set(*copy, this->association_type);
   }
   void Relationship::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->referrer);
      DATA.write(this->referent);
      DATA.write(this->rank);
      DATA.write(this->flags);
      DATA.write(this->association_type);
      DATA.close();
   }
   void Relationship::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->referrer.set(*this, nullptr);
      this->referent.set(*this, nullptr);
      this->rank  = rank::acquaintance;
      this->flags = 0;
      this->association_type.set(*this, nullptr);
   }
   void Relationship::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      //
      this->referrer.clear_if(*this, other);
      this->referent.clear_if(*this, other);
      this->association_type.clear_if(*this, other);
   }
}