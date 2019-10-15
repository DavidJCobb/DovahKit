#include "Quest.h"
#include "../esp/TESPlugin.h"

void TESQuest::load(TESPluginFile* file) {
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
         case 'VMAD':
            this->scriptData.load(file);
            break;
         case 'DNAM': // required; TODO: fail if this is not present
            file->read(this->flags);
            file->read(this->priority);
            file->read(this->formVersion);
            file->read(this->unknown);
            file->read(this->questType);
            break;
         case 'ENAM':
            //
            // TODO
            //
            break;
         case 'QTGL':
            //
            // TODO
            //
            break;
         case 'FLTR': // required; TODO: fail if this is not present
            file->readStringSubrecord(this->editorCategory);
            break;
         //
         // TODO: any incomplete subrecords above, CTDA, NEXT, CTDA, stages, objectives, ANAM, aliases
         //
      }
   }
}

const char* _questTypeNames[] = {
   "None",
   "Main Quest",
   "Mages Guild",
   "Thieves Guild",
   "Dark Brotherhood",
   "Companions",
   "Miscellaneous",
   "Daedric",
   "Sidequest",
   "Civil War",
   "Vampire (DLC 1)",
   "Dragonborn (DLC 2)",
};
/*static*/ const char* TESQuest::QuestTypeToString(QuestType qt) {
   if (qt >= std::extent<decltype(_questTypeNames)>::value)
      return nullptr;
   return _questTypeNames[qt];
}