#include "TopicInfo.h"
#include "../../esp/TESPlugin.h"
#include "../conditions.h"

namespace LoadedForms {
   /*static*/ void TopicInfo::generateUseInfo(TESPluginRecord& record, FormStub* stub) {
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               PapyrusScriptData::generateUseInfo(subrecord, stub);
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
                  stub->add_outbound_reference(formID);
               break;
            case 'CTDA':
               Condition::generateUseInfo(record, stub);
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