#include "ActorBase.h"
#include "../esp/TESPlugin.h"

void TESActorBase::load(TESPluginFile* file) {
   while (file->nextSubrecord()) {
      uint32_t signature = file->getSubrecordType();
      uint32_t size      = file->getSubrecordSize();
      switch (signature) {
         case 'EDID': // required; TODO: fail if this is not present
            file->readStringSubrecord(this->editorID);
            break;
         case 'FULL':
            file->readStringSubrecord(this->name);
            break;
      }
   }
}