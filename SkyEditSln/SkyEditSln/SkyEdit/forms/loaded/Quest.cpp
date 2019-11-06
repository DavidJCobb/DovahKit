#include "Quest.h"
#include "../../esp/TESPlugin.h"

namespace LoadedForms {
   void Quest::load(TESPluginRecord& record) {
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'EDID': // required; TODO: fail if this is not present
               subrecord.to_string(this->editorID);
               break;
            case 'FULL':
               subrecord.to_string(this->name);
               break;
            case 'VMAD':
               this->scriptData.load(subrecord);
               break;
            case 'DNAM': // required; TODO: fail if this is not present
               subrecord.read(this->flags);
               subrecord.read(this->priority);
               subrecord.read(this->formVersion);
               subrecord.read(this->unknown);
               subrecord.read(this->questType);
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
               subrecord.to_string(this->editorCategory);
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
   /*static*/ const char* Quest::QuestTypeToString(QuestType qt) {
      if (qt >= std::extent<decltype(_questTypeNames)>::value)
         return nullptr;
      return _questTypeNames[qt];
   }
}