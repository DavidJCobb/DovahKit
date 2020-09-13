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
               this->papyrus.load(subrecord);
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
               subrecord.read(this->marker_color.r);
               subrecord.read(this->marker_color.g);
               subrecord.read(this->marker_color.b);
               subrecord.read(this->marker_color.alpha);
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
}