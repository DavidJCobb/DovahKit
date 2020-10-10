#include "Activator.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Activator::load(tes_record_reader& record) {
      Form::load(record);
      //
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord);
               break;
            case 'OBND':
               this->bounds.load(subrecord);
               break;
            case 'FULL':
               subrecord.to_string(this->name);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               this->model.load(subrecord);
               break;
            case 'DEST': // destruction stage header // details: https://en.uesp.net/wiki/Tes5Mod:Mod_File_Format/DEST_Field
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               this->destruction_data.load(subrecord);
               break;
            case 'KSIZ':
            case 'KWDA':
               this->keywords.load(subrecord);
               break;
            case 'PNAM':
               this->marker_color.load(subrecord);
               break;
            case 'SNAM':
               subrecord.read(this->looping_sound);
               break;
            case 'VNAM':
               subrecord.read(this->activation_sound);
               break;
            case 'WNAM':
               subrecord.read(this->water_type);
               break;
            case 'RNAM':
               subrecord.read(this->activation_verb);
               break;
            case 'FNAM':
               subrecord.read(this->activator_flags);
               break;
            case 'KNAM':
               subrecord.read(this->interact_keyword);
               break;
         }
      }
   }
   /*static*/ void Activator::generateUseInfo(tes_record_reader& record, form_stub* stub) {
      uint32_t keywordCount = 0;
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generateUseInfo(subrecord, stub);
               break;
            case 'SNAM': // looping sound (e.g. nirnroot bell)
            case 'VNAM': // activation sound
            case 'WNAM': // water type, for water activators
            case 'KNAM': // interaction keyword
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               components::model::generateUseInfo(subrecord, stub);
               break;
            case 'KSIZ':
            case 'KWDA':
               components::keyword_list::generateUseInfo(subrecord, stub);
               break;
            case 'OBND': // bounds
               components::object_bounds::generateUseInfo(subrecord, stub);
               break;
            case 'EDID': // editor ID
            case 'FULL': // displayed name
            case 'PNAM': // marker color
            case 'RNAM': // override activation prompt text
            case 'FNAM': // extra flags
               break;

         }
      }
   }
   bool Activator::_clone_impl(Form* out) const noexcept {
      auto copy = dynamic_cast<Activator*>(out);
      if (!copy)
         return false;
      copy->script_data.clone_from(this->script_data, *copy->stub);
      copy->bounds = this->bounds;
      copy->model.clone_from(this->model, *copy->stub);
      copy->destruction_data.clone_from(this->destruction_data, *copy->stub);
      copy->keywords.clone_from(this->keywords, *copy->stub);
      copy->name = this->name;
      copy->marker_color = this->marker_color;
      copy->looping_sound.set(copy->stub, this->looping_sound);
      copy->activation_sound.set(copy->stub, this->activation_sound);
      copy->water_type.set(copy->stub, this->water_type);
      copy->interact_keyword.set(copy->stub, this->interact_keyword);
      copy->activation_verb = this->activation_verb;
      copy->activator_flags = this->activator_flags;
      return true;
   }
   bool Activator::_save_impl(tes_record_writer& record) {
      this->script_data.save(record);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND);
      OBND.close();
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      this->model.save(record, 'MODL', 'MODT', 'MODS');
      this->destruction_data.save(record);
      this->keywords.save(record);
      auto& PNAM = record.open_next_subrecord('PNAM');
      this->marker_color.save(PNAM);
      PNAM.close();
      if (this->looping_sound)
         record.write_formID_subrecord('SNAM', this->looping_sound);
      if (this->activation_sound)
         record.write_formID_subrecord('VNAM', this->activation_sound);
      if (this->water_type)
         record.write_formID_subrecord('WNAM', this->water_type);
      if (!this->activation_verb.empty()) {
         auto& RNAM = record.open_next_subrecord('RNAM');
         RNAM.write(this->activation_verb);
         RNAM.close();
      }
      auto& FNAM = record.open_next_subrecord('FNAM');
      FNAM.write(this->activator_flags);
      FNAM.close();
      if (this->interact_keyword)
         record.write_formID_subrecord('KNAM', this->interact_keyword);
      return true;
   }
   void Activator::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this->stub);
      this->model.sever_outbound_references_to(other, *this->stub);
      this->destruction_data.sever_outbound_references_to(other, *this->stub);
      this->keywords.sever_outbound_references_to(other, *this->stub);
      //
      auto formID = other.formID;
      if (this->looping_sound == formID)
         this->looping_sound.set(this->stub, nullptr);
      if (this->activation_sound == formID)
         this->activation_sound.set(this->stub, nullptr);
      if (this->water_type == formID)
         this->water_type.set(this->stub, nullptr);
      if (this->interact_keyword == formID)
         this->interact_keyword.set(this->stub, nullptr);
   }
}