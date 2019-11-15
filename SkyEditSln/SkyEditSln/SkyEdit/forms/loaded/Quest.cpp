#include "Quest.h"
#include "../../esp/TESPlugin.h"

namespace LoadedForms {
   void Quest::LogEntry::load(TESPluginRecord& record) {
      auto& subrecord = record.get_current_subrecord();
      assert(subrecord.signature() == 'QSDT' && "Quest::LogEntry::load should only be called just after the QSDT subrecord is opened.");
      subrecord.read(this->flags);
      auto next = record.peek_next_subrecord_type();
      while (next == 'CTDA') {
         auto& subrecord = record.next_subrecord();
         this->conditions.push_back(Condition());
         auto& cnd = *this->conditions.rbegin();
         cnd.read(record);
         next = record.peek_next_subrecord_type();
      }
      while (true) {
         next = record.peek_next_subrecord_type();
         switch (next) {
            case 'CNAM':
               {
                  auto& subrecord = record.next_subrecord();
                  subrecord.to_string(this->journalText);
               }
               continue;
            case 'NAM0':
               {
                  auto& subrecord = record.next_subrecord();
                  subrecord.read(this->nextQuestID);
               }
               continue;
            case 'SCHR':
               assert(false && "Not implemented!");
               continue;
         }
         break;
      }
   }
   void Quest::Stage::load(TESPluginRecord& record) {
      auto& subrecord = record.get_current_subrecord();
      assert(subrecord.signature() == 'INDX' && "Quest::Stage::load should only be called just after the INDX subrecord is opened.");
      if (subrecord.is_in_bounds(4)) {
         subrecord.unchecked_read(this->index);
         subrecord.unchecked_read(this->flags);
         subrecord.unchecked_read(this->padding);
      }
      while (record.peek_next_subrecord_type() == 'QSTD') {
         this->entries.push_back(LoadedForms::Quest::LogEntry());
         auto& entry = *this->entries.rbegin();
         record.next_subrecord();
         entry.load(record);
      }
   }

   void Quest::Target::load(TESPluginRecord& record) {
      auto& subrecord = record.get_current_subrecord();
      assert(subrecord.signature() == 'QSTA' && "Quest::Target::load should only be called just after the QSTA subrecord is opened.");
      subrecord.read(this->aliasID);
      subrecord.read(this->flags);
      auto next = record.peek_next_subrecord_type();
      while (next == 'CTDA') {
         auto& subrecord = record.next_subrecord();
         this->conditions.push_back(Condition());
         auto& cnd = *this->conditions.rbegin();
         cnd.read(record);
         next = record.peek_next_subrecord_type();
      }

   }
   void Quest::Objective::load(TESPluginRecord& record) {
      auto& subrecord = record.get_current_subrecord();
      assert(subrecord.signature() == 'QOBJ' && "Quest::Objective::load should only be called just after the QOBJ subrecord is opened.");
      subrecord.read(this->index);
      while (auto next = record.peek_next_subrecord_type()) {
         switch (next) {
            case 'FNAM':
               {
                  auto& subrecord = record.next_subrecord();
                  subrecord.read(this->flags);
               }
               continue;
            case 'NNAM':
               {
                  auto& subrecord = record.next_subrecord();
                  subrecord.to_string(this->text);
               }
               continue;
            case 'QSTA':
               {
                  auto& subrecord = record.next_subrecord();
                  this->targets.push_back(Target());
                  auto& t = *this->targets.rbegin();
                  t.load(record);
               }
               continue;
         }
         break;
      }
   }

   void Quest::load(TESPluginRecord& record) {
      bool isInEventConditions = false;
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
            case 'DNAM': // required; TODO: fail if this is not present; fail if it is too short
               if (subrecord.is_in_bounds(12)) {
                  subrecord.unchecked_read(this->flags);
                  subrecord.unchecked_read(this->priority);
                  subrecord.unchecked_read(this->formVersion);
                  subrecord.unchecked_read(this->unknown);
                  subrecord.unchecked_read(this->questType);
               }
               break;
            case 'ENAM':
               subrecord.read(this->event);
               break;
            case 'QTGL':
               //
               // TODO
               //
               break;
            case 'FLTR': // required; TODO: fail if this is not present
               subrecord.to_string(this->editorCategory);
               break;
            case 'NEXT':
               isInEventConditions = true;
               break;
            case 'ANAM':
               subrecord.read(this->nextAliasID);
               break;
            case 'INDX': // also handles QSDT
               {
                  this->stages.push_back(Stage());
                  auto& stage = *this->stages.rbegin();
                  stage.load(subrecord.get_containing_record());
               }
               break;
            case 'QOBJ':
               {
                  this->objectives.push_back(Objective());
                  auto& objective = *this->objectives.rbegin();
                  objective.load(subrecord.get_containing_record());
               }
               break;
            case 'CTDA':
               {
                  auto& list = this->dialogueConditions;
                  if (isInEventConditions)
                     list = this->eventConditions;
                  list.push_back(Condition());
                  auto& cnd = *list.rbegin();
                  cnd.read(subrecord.get_containing_record());
               }
               break;
            //
            // TODO: any incomplete subrecords above, aliases
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