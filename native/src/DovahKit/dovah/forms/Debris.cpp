#include "Debris.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Debris::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'DATA':
               {
                  auto& v = this->variations.emplace_back();
                  subrecord.read(v.percentage);
                  {
                     //
                     // subrecord.read, when invoked on a std::string, doesn't do the sensible 
                     // thing and read a null-terminated string; instead, it reads the entire 
                     // subrecord into the std::string. I made that bad and dumb design choice 
                     // years ago and it's too late to change it now.
                     //
                     char c;
                     while (subrecord.read(c) && c) {
                        v.model.model_path += c;
                     }
                  }
                  subrecord.read(v.flags);
               }
               break;
            case 'MODT':
               if (!this->variations.empty()) {
                  auto& v = this->variations.back();
                  v.model.load(subrecord, intfc);
               }
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Debris::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
         }
      }
   }
   void Debris::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Debris*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->variations = this->variations;
   }
   void Debris::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      for (auto& v : this->variations) {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(v.percentage);
         subrecord.write(v.model.model_path); // subrecord.read handles std::string insensibly, but subrecord.write handles it sensibly.
         subrecord.write(v.flags);
         subrecord.close();

         auto& MODT = record.open_next_subrecord('MODT');
         v.model.save_precached_info(MODT, intfc);
         MODT.close();
      }
   }
   void Debris::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->variations.clear();
   }
   void Debris::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}