#include "Quest.h"
#include "../../esp/TESPlugin.h"

namespace LoadedForms {
   void LocationAlias::load(TESPluginRecord& record) {
      auto& subrecord = record.get_current_subrecord();
      assert(subrecord.signature() == 'ALLS' && "LocationAlias::load should only be called just after the ALLS subrecord is opened.");
      subrecord.read(this->id);
      //
      if (!record.next_subrecord())
         return;
      uint32_t externalAliasID = 0xFFFFFFFF;
      uint32_t internalAliasID = 0xFFFFFFFF;
      for (; subrecord.exists() && subrecord.signature() != 'ALED'; record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'ALFA':
               subrecord.read(internalAliasID);
               break;
            case 'ALEA':
               subrecord.read(externalAliasID);
               break;
            case 'CTDA':
               {
                  auto& list = this->conditions;
                  list.push_back(Condition());
                  auto& cnd = *list.rbegin();
                  cnd.read(subrecord.get_containing_record());
               }
               break;
            case 'ALFD':
               subrecord.read(this->fillFromEventData);
               if (this->fillFromEvent == -1)
                  this->fillFromEventData = -1;
               else {
                  //
                  // TODO: The value undergoes further checks? See Skyrim Classic code from 0x0054E291.
                  //
               }
               break;
            case 'ALFE':
               subrecord.read(this->fillFromEvent);
               this->fillType = fill_type::from_event;
               break;
            case 'ALID':
               subrecord.to_string(this->name);
               break;
            case 'ALFI':
               subrecord.read(this->forceIntoAliasID);
               break;
            case 'FNAM':
               subrecord.read(this->flags);
               break;
            case 'BNAM':
               this->hiddenFlags |= 1;
               break;
            case 'ALFL':
               subrecord.read(this->fillFromLocationID);
               this->fillType = fill_type::preset;
               break;
            case 'KNAM':
               subrecord.read(this->fillFromLocationKeywordID);
               break;
            case 'ALEQ':
               subrecord.read(this->fillFromQuestID);
               this->fillType = fill_type::other_alias_in_other_quest;
               break;
            case 'ONAM':
               this->hiddenFlags |= 2;
               break;
         }
      }
      if (this->fillType == fill_type::other_alias_in_other_quest)
         this->fillFromAliasID = externalAliasID;
      else if (this->fillType == fill_type::other_alias_in_same_quest)
         this->fillFromAliasID = internalAliasID;
   }

   void Quest::LogEntry::load(TESPluginRecord& record) {
      auto& subrecord = record.get_current_subrecord();
      assert(subrecord.signature() == 'QSDT' && "Quest::LogEntry::load should only be called just after the QSDT subrecord is opened.");
      subrecord.read(this->flags);
      while (true) {
         auto next = record.peek_next_subrecord_type();
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
   void Quest::Stage::load(TESPluginSubrecord& subrecord) {
      assert(subrecord.signature() == 'INDX' && "Quest::Stage::load should only be called just after the INDX subrecord is opened.");
      if (subrecord.is_in_bounds(4)) {
         subrecord.unchecked_read(this->index);
         subrecord.unchecked_read(this->flags);
         subrecord.unchecked_read(this->padding);
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
   void Quest::Target::load(TESPluginSubrecord& subrecord) {
      assert(subrecord.signature() == 'QSTA' && "Quest::Target::load should only be called just after the QSTA subrecord is opened.");
      subrecord.read(this->aliasID);
      subrecord.read(this->flags);
      subrecord.skip_bytes(3);
   }
   void Quest::Objective::load(TESPluginRecord& record) {
      auto& subrecord = record.get_current_subrecord();
      assert(subrecord.signature() == 'QOBJ' && "Quest::Objective::load should only be called just after the QOBJ subrecord is opened.");
      subrecord.read(this->index);
      if (subrecord.size() == 4) // the game checks for this and conditionally loads the index into a uint32_t temporary before shearing off the high bytes and keeping a 16-bit value
         subrecord.skip_bytes(2);
      #if LOAD_NAIVELY_WHEN_THE_GAME_DOES == 1
         //
         // Once Skyrim sees a QOBJ, it just blindly reads subrecords either until it finds an 'NNAM' 
         // subrecord or until it hits the end of the containing record.
         //
         // See: Classic 0x00551C70 == void TESQuest::Objective::Load(BGSLoadFormBuffer*);
         //
         this->error_isMissingFlags = true;
         this->error_isMissingText  = true;
         while (record.next_subrecord().exists()) { // TESPluginRecord::next_subrecord alters (subrecord) and returns it
            switch (subrecord.signature()) {
               case 'FNAM':
                  this->error_isMissingFlags = false;
                  subrecord.read(this->flags);
                  break;
               case 'NNAM':
                  this->error_isMissingText  = false;
                  subrecord.to_string(this->text);
                  return;
            }
         }
      #else
         //
         // Only consume FNAM and NNAM if they directly follow QOBJ; if we encounter any other 
         // subrecord types first, then assume there are no matching FNAM and NNAM and abort. This 
         // is good for catching potential errors in the file, but is not future-compatible (i.e. 
         // if Bethesda wanted to extend QOBJ in a backward-compatible way, they'd want to do so 
         // by adding new subrecords that precede FNAM and NNAM).
         //
         bool foundFNAM = false;
         bool foundNNAM = false;
         uint32_t next;
         while (next = record.peek_next_subrecord_type()) {
            if (next == 'FNAM' || next == 'NNAM')
               record.next_subrecord();
            else
               break;
            if (next == 'FNAM') {
               foundFNAM = true;
               subrecord.read(this->flags);
            } else {
               foundNNAM = true;
               subrecord.to_string(this->text);
            }
         }
         this->error_isMissingFlags = !foundFNAM;
         this->error_isMissingText  = !foundNNAM;
      #endif
   }

   Quest::~Quest() {
      for (auto it = this->aliases.begin(); it != this->aliases.end(); ++it)
         delete (*it);
      this->aliases.clear();
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
                  uint32_t type;
                  subrecord.unchecked_read(type); // stored as a uint32_t, but only the low byte is retained in memory
                  this->questType = (QuestType)type;
               }
               break;
            case 'ENAM':
               subrecord.read(this->event);
               break;
            case 'QTGL':
               {
                  uint32_t id;
                  if (subrecord.read(id) && id)
                     this->textDisplayGlobalIDs.push_back(id);
               }
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
                  stage.load(subrecord);
               }
               break;
            case 'QSTD':
               if (this->stages.size()) {
                  auto& stage = *this->stages.rbegin();
                  stage.entries.push_back(LogEntry());
                  auto& entry = *stage.entries.rbegin();
                  entry.load(record);
               }
               break;
            case 'QOBJ':
               {
                  this->objectives.push_back(Objective());
                  auto& objective = *this->objectives.rbegin();
                  objective.load(subrecord.get_containing_record());
               }
               break;
            case 'QSTA':
               if (!this->objectives.size())
                  //
                  // Skyrim keeps track of the last QOBJ loaded. If there is none when it 
                  // encounters a QSTA, it creates a dummy objective and stuffs the target 
                  // into that.
                  //
                  this->objectives.push_back(Objective());
               {
                  auto& objective = *this->objectives.rbegin();
                  objective.targets.push_back(Target());
                  auto& target    = *objective.targets.rbegin();
                  target.load(subrecord);
               }
               break;
            case 'CTDA':
               //
               // TODO:
               //  - if there's a relevant log entry, add it to that
               //  - else if there's a relevant target, add it to that
               //  - else if we're in event conditions, add it to those
               //  - else add it to dialogue conditions
               //
               {
                  auto& list = this->dialogueConditions;
                  if (isInEventConditions)
                     list = this->eventConditions;
                  list.push_back(Condition());
                  auto& cnd = *list.rbegin();
                  cnd.read(subrecord.get_containing_record());
               }
               break;
            case 'ALLS':
               {
                  auto alias = new LocationAlias;
                  this->aliases.push_back(alias);
                  alias->load(record);
               }
               break;
            case 'ALST':
               {
                  auto alias = new ReferenceAlias;
                  this->aliases.push_back(alias);
                  alias->load(record);
               }
               break;
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