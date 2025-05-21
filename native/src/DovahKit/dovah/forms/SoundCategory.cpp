#include "SoundCategory.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/sound_category/is_own_parent.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::sound_category;
   }
}

namespace dovah::loaded_forms {
   void SoundCategory::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'FNAM':
               subrecord.read(this->flags);
               break;
            case 'PNAM':
               if (subrecord.read(this->parent)) {
                  intfc.warn_if_ref_is_wrong_type(this->parent, form_type::sound_category, subrecord.signature());
                  if (this->parent == &this->stub) {
                     specific_load_warnings::is_own_parent notice(
                        this->stub
                     );
                     intfc.log_load_warning(notice);
                  }
               }
               break;
            case 'VNAM':
               subrecord.read(this->static_volume_mult);
               break;
            case 'UNAM':
               subrecord.read(this->default_menu_value);
               break;
            case 'SNAM':
               {
                  float v;
                  if (subrecord.read(v)) {
                     v = v * 65535.0F;
                     if (v >= 65535.0F) {
                        this->static_volume_mult = 0xFFFF;
                     } else {
                        this->static_volume_mult = v;
                     }
                  }
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void SoundCategory::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      
      form_id_t parent;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'OBND':
            case 'FULL':
            case 'FNAM':
            case 'UNAM':
            case 'VNAM':
            case 'SNAM':
               break;
            case 'PNAM':
               subrecord.read(parent);
               break;
         }
      }
      uib.add_outbound_reference(parent);
   }
   void SoundCategory::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (SoundCategory*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->name = this->name;
      copy->flags = this->flags;
      copy->parent.set(*copy, this->parent);
      copy->static_volume_mult = this->static_volume_mult;
      copy->default_menu_value = this->default_menu_value;
   }
   void SoundCategory::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      auto& FNAM = record.open_next_subrecord('FNAM');
      FNAM.write(this->flags);
      FNAM.close();
      record.write_formID_subrecord('PNAM', this->parent, true);
      auto& VNAM = record.open_next_subrecord('VNAM');
      VNAM.write(this->static_volume_mult);
      VNAM.close();
      if (this->flags & sound_category_flag::show_in_audio_menu) {
         auto& UNAM = record.open_next_subrecord('UNAM');
         UNAM.write(this->default_menu_value);
         UNAM.close();
      }
   }
   void SoundCategory::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->name.reset();
      this->parent.set(*this, nullptr);
      this->flags = 0;
      this->static_volume_mult = 0xFFFF;
      this->default_menu_value = 0xFFFF;
   }
   void SoundCategory::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->parent.clear_if(*this, other);
   }
}