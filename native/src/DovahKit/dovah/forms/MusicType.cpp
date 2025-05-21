#include "MusicType.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void MusicType::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
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
               //
               // The loader checks for this and passes it to a virtual function on TESForm 
               // that's responsible for loading it. However, this form doesn't derive from 
               // TESBoundObject, so the TESForm implementation of that virtual function (a 
               // no-op) isn't overridden and therefore the data is not retained in memory.
               //
               break;
            case 'FNAM':
               subrecord.read(this->flags);
               break;
            case 'PNAM':
               subrecord.read(this->priority);
               subrecord.read(this->ducking_db);
               break;
            case 'WNAM':
               subrecord.read(this->fade_duration);
               break;
            case 'TNAM':
               if (subrecord.size() >= sizeof(uint32_t)) {
                  size_t count = subrecord.size() / sizeof(uint32_t);
                  this->tracks.reserve(this->tracks.size() + count);
                  for (size_t i = 0; i < count; ++i) {
                     subrecord.read(this->tracks.emplace_back());
                     intfc.warn_if_ref_is_wrong_type(this->tracks.back(), form_type::music_track, subrecord.signature());
                  }
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void MusicType::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'OBND':
               break;
            case 'DATA':
            case 'PNAM':
            case 'WNAM':
               break;
            case 'TNAM':
               if (subrecord.size() >= sizeof(uint32_t)) {
                  size_t count = subrecord.size() / sizeof(uint32_t);
                  for (size_t i = 0; i < count; ++i) {
                     form_id_t form_id;
                     if (subrecord.read(form_id) && form_id)
                        uib.add_outbound_reference(form_id);
                  }
               }
               break;
         }
      }
   }
   void MusicType::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (MusicType*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      //
      copy->flags = this->flags;
      copy->priority = this->priority;
      copy->ducking_db = this->ducking_db;
      copy->fade_duration = this->fade_duration;
      copy_form_reference_list(*copy, copy->tracks, this->tracks);
   }
   void MusicType::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('FNAM');
         subrecord.write(this->flags);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('PNAM');
         subrecord.write(this->priority);
         subrecord.write(this->ducking_db);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('WNAM');
         subrecord.write(this->fade_duration);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('TNAM');
         for (auto& item : this->tracks)
            subrecord.write(item);
         subrecord.close();
      }
   }
   void MusicType::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->flags  = 0;
      this->priority = 0;
      this->ducking_db = 0;
      this->fade_duration = 0;
      clear_form_reference_list(this->tracks, *this);
   }
   void MusicType::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      remove_form_from_reference_list(this->tracks, other, *this);
   }
}