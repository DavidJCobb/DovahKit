#include "TopicInfo.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   /*static*/ void TopicInfo::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'PNAM': // previous-sibling topicinfo
            case 'TCLT': // follow-up topics
            case 'DNAM': // sharedinfo to inherit from
            case 'SNAM': // response speaker idle anim
            case 'LNAM': // response listener idle anim
            case 'ANAM': // speaker
            case 'TWAT': // walk away topic
            case 'ONAM': // audio output override
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
            case 'CTDA':
               components::condition::generate_use_info(record, uib);
               break;
            case 'DATA': // metadata (old)
            case 'ENAM': // metadata (new)
            case 'CNAM': // favor level
            case 'TRDT': // response header
            case 'NAM1': // response text
            case 'NAM2': // response script notes
            case 'NAM3': // response edits
            case 'SCHR': // DEPRECATED: ObScript header
            case 'SCDA': // DEPRECATED: ObScript compiled code
            case 'SCTX': // DEPRECATED: ObScript source code
            case 'SCRO': // DEPRECATED: ObScript ObjectReference
            case 'SCRV': // DEPRECATED: ObScript ObjectReference variable
            case 'QNAM': // DEPRECATED: ObScript
            case 'NEXT': // ObScript separator
            case 'RNAM': // override topic text
               break;
         }
      }
   }
}