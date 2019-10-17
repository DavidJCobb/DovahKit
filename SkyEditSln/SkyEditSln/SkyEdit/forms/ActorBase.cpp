#include "ActorBase.h"
#include "../esp/TESPlugin.h"

void TESActorBase::load(TESPluginRecord& record) {
   while (TESPluginSubrecord subrecord = record.next_subrecord()) {
      switch (subrecord.signature()) {
         case 'EDID': // required; TODO: fail if this is not present
            subrecord.to_string(this->editorID);
            break;
         case 'FULL':
            subrecord.to_string(this->name);
            break;
      }
   }
}