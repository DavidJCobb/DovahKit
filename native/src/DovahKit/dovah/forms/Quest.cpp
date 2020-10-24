#include "Quest.h"
#include "_common_cpp.h"
#include "../logging.h"

namespace dovah::loaded_forms {
   void LocationAlias::load(tes_record_reader& record) {
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
                  list.emplace_back();
                  auto& cnd = *list.rbegin();
                  cnd.read(subrecord.get_containing_record());
               }
               break;
            case 'ALFD':
               subrecord.read(this->fill_from_event_data);
               if (this->fill_from_event == -1)
                  this->fill_from_event_data = -1;
               else {
                  //
                  // TODO: The value undergoes further checks? See Skyrim Classic code from 0x0054E291.
                  //
               }
               break;
            case 'ALFE':
               subrecord.read(this->fill_from_event);
               this->fill_type = fill_type_t::from_event;
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
               subrecord.read(this->fill_from_location);
               this->fill_type = fill_type_t::preset;
               break;
            case 'KNAM':
               subrecord.read(this->fill_from_location_keyword);
               break;
            case 'ALEQ':
               subrecord.read(this->fill_from_quest);
               this->fill_type = fill_type_t::other_alias_in_other_quest;
               break;
            case 'ONAM':
               this->hiddenFlags |= 2;
               break;
         }
      }
      if (this->fill_type == fill_type_t::other_alias_in_other_quest)
         this->fill_from_alias = externalAliasID;
      else if (this->fill_type == fill_type_t::other_alias_in_same_quest)
         this->fill_from_alias = internalAliasID;
   }
   void ReferenceAlias::load(tes_record_reader& record) {
      auto& subrecord = record.get_current_subrecord();
      assert(subrecord.signature() == 'ALST' && "ReferenceAlias::load should only be called just after the ALST subrecord is opened.");
      subrecord.read(this->id);
      //
      if (!record.next_subrecord())
         return;
      uint32_t perkListSize  = 0;
      uint32_t inventorySize = 0;
      form_reference_t formID;
      for (; subrecord.exists() && subrecord.signature() != 'ALED'; record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'ALID':
               subrecord.to_string(this->name);
               break;
            case 'FNAM':
               subrecord.read(this->flags);
               break;
            case 'ALFI':
               subrecord.read(this->forceIntoAliasID);
               break;
            case 'BNAM': // shared with BGSRefAlias
               this->hiddenFlags |= 1;
               break;
            case 'ONAM': // shared with BGSRefAlias
               this->hiddenFlags |= 2;
               break;
            case 'QNAM':
               this->hiddenFlags |= 4;
               break;
            case 'CTDA':
               {
                  auto& list = this->conditions;
                  list.emplace_back();
                  auto& cnd = *list.rbegin();
                  cnd.read(subrecord.get_containing_record());
               }
               break;
            case 'KSIZ':
            case 'KWDA':
               this->keywords.load(subrecord);
               break;
            case 'COCT':
            case 'CNTO':
            case 'COED':
               this->inventory.load(subrecord);
               break;
            case 'PRKZ':
               if (subrecord.read(perkListSize))
                  this->perks.reserve(perkListSize);
               break;
            case 'PRKR':
               if (subrecord.read(formID))
                  this->perks.push_back(formID);
               break;
            case 'SCOR':
               subrecord.read(this->spectatorOverridePackageListID);
               break;
            case 'OCOR':
               subrecord.read(this->observeCorpseOverridePackageListID);
               break;
            case 'GWOR':
               subrecord.read(this->guardWarnOverridePackageListID);
               break;
            case 'ECOR':
               subrecord.read(this->combatOverridePackageListID);
               break;
            case 'ALDN':
               subrecord.read(this->displayNameID);
               break;
            case 'ALCO':
               this->fillType = fill_type_t::create_object;
               subrecord.read(this->createObjectBaseID);
               break;
            case 'ALCA':
               if (this->fillType == fill_type_t::create_object)
                  subrecord.read(this->createObjectAt);
               break;
            case 'ALCL':
               if (this->fillType == fill_type_t::create_object)
                  subrecord.read(this->createObjectLevel);
               break;
            case 'ALEQ':
               subrecord.read(this->fillFromQuestID);
               this->fillType = fill_type_t::other_alias_in_other_quest;
               break;
            case 'ALEA':
               if (this->fillType == fill_type_t::other_alias_in_other_quest)
                  subrecord.read(this->fillFromAliasID);
               break;
            case 'ALFA':
               this->fillType = fill_type_t::other_alias_in_same_quest;
               subrecord.read(this->fillFromAliasID);
               break;
            case 'ALNA':
               this->fillType = fill_type_t::find_matching_reference;
               subrecord.read(this->fillNearAlias);
               break;
            case 'ALNT':
               if (this->fillType == fill_type_t::find_matching_reference)
                  subrecord.read(this->fillNearAliasType);
               break;
            case 'ALPC':
               if (subrecord.read(formID))
                  this->packages.push_back(formID);
               break;
            case 'ALFC':
               if (subrecord.read(formID))
                  this->factions.push_back(formID);
               break;
            case 'ALSP':
               if (subrecord.read(formID))
                  this->spells.push_back(formID);
               break;
            case 'ALUA':
               this->fillType = fill_type_t::preset_unique_actor;
               subrecord.read(this->fillFromUniqueActorBaseID);
               break;
            case 'ALFE':
               this->fillType = fill_type_t::from_event;
               subrecord.read(this->fill_from_event);
               break;
            case 'ALFD':
               subrecord.read(this->fill_from_event_data);
               if (this->fill_from_event == -1)
                  this->fill_from_event_data = -1;
               else {
                  //
                  // TODO: The value undergoes further checks? See Skyrim Classic code from 0x0054E291. 
                  // (That code is for loc aliases; the ref alias code is stranger-looking but probably 
                  // does the same stuff.)
                  //
               }
               break;
            case 'ALFR':
               this->fillType = fill_type_t::preset_placed_reference;
               subrecord.read(this->fillFromObjectReferenceID);
               break;
            case 'VTCK':
               subrecord.read(this->additionalVoiceTypeID);
               break;
            case 'ALRT':
               subrecord.read(this->fillLocRefTypeID);
               break;
         }
      }
   }

   void Quest::LogEntry::load(tes_record_reader& record) {
      auto& subrecord = record.get_current_subrecord();
      assert(subrecord.signature() == 'QSDT' && "Quest::LogEntry::load should only be called just after the QSDT subrecord is opened.");
      subrecord.read(this->flags);
      if (record.peek_next_subrecord_type() != 'NAM0')
         return;
      record.next_subrecord();
      subrecord.read(this->nextQuestID);
   }
   void Quest::LogEntry::loadText(tes_subrecord_reader& subrecord) {
      assert(subrecord.signature() == 'CNAM' && "Quest::LogEntry::loadText should only be called just after the CNAM subrecord is opened.");
      subrecord.to_string(this->journalText);
   }

   void Quest::Stage::load(tes_subrecord_reader& subrecord) {
      assert(subrecord.signature() == 'INDX' && "Quest::Stage::load should only be called just after the INDX subrecord is opened.");
      if (subrecord.is_in_bounds(4)) {
         subrecord.unchecked_read(this->index);
         subrecord.unchecked_read(this->flags);
         subrecord.unchecked_read(this->padding);
      }
   }

   void Quest::Target::load(tes_subrecord_reader& subrecord) {
      assert(subrecord.signature() == 'QSTA' && "Quest::Target::load should only be called just after the QSTA subrecord is opened.");
      subrecord.read(this->aliasID);
      subrecord.read(this->flags);
      subrecord.skip_bytes(3);
   }
   void Quest::Objective::load(tes_record_reader& record) {
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
   Quest::Target* Quest::getLastParsedQuestTarget() const noexcept {
      for (auto it = this->objectives.rbegin(); it != this->objectives.rend(); ++it) {
         auto jt = it->targets.rbegin();
         if (jt != it->targets.rend()) {
            auto&  target = const_cast<Quest::Target&>(*jt);
            return &target;
         }
      }
      return nullptr;
   }
   void Quest::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      bool     isInEventConditions = false;
      bool     hasLastLogEntry     = false;
      uint32_t lastLogEntryIndices[2];
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
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
                  form_reference_t id;
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
                  this->stages.emplace_back();
                  auto& stage = *this->stages.rbegin();
                  stage.load(subrecord);
               }
               break;
            case 'QSTD':
               if (this->stages.size()) { // the game ignores QSTD that appear when there is no stage
                  hasLastLogEntry = true;
                  lastLogEntryIndices[0] = this->stages.size() - 1;
                  auto& stage = this->stages[lastLogEntryIndices[0]];
                  lastLogEntryIndices[1] = stage.entries.size();
                  stage.entries.emplace_back();
                  auto& entry = stage.entries[lastLogEntryIndices[1]];
                  entry.load(record);
               }
               break;
            case 'CNAM':
               if (!hasLastLogEntry)
                  break;
               {
                  auto& entry = this->stages[lastLogEntryIndices[0]].entries[lastLogEntryIndices[1]];
                  entry.loadText(subrecord);
               }
               break;
            case 'SCHR':
               //
               // TODO: Add ObScript data to last-loaded entry
               //
               break;
            case 'QOBJ':
               {
                  this->objectives.emplace_back();
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
                  this->objectives.emplace_back();
               {
                  auto& objective = *this->objectives.rbegin();
                  objective.targets.emplace_back();
                  auto& target    = *objective.targets.rbegin();
                  target.load(subrecord);
               }
               hasLastLogEntry = false; // per TESV.exe TESQuest::LoadForm
               break;
            case 'CTDA':
               {
                  components::condition nc;
                  nc.read(subrecord.get_containing_record());
                  if (hasLastLogEntry) {
                     auto& entry = this->stages[lastLogEntryIndices[0]].entries[lastLogEntryIndices[1]];
                     entry.conditions.push_back(nc);
                     break;
                  }
                  if (auto target = this->getLastParsedQuestTarget()) {
                     target->conditions.push_back(nc);
                     break;
                  }
                  auto& list = this->dialogueConditions;
                  if (isInEventConditions)
                     list = this->eventConditions;
                  list.push_back(nc);
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
            case 'SCDA':
            case 'SCRV':
            case 'SLSD':
            case 'QNAM':
               //
               // TODO: The game passes all of these to TESQuest::LogEntry::Load, but it just ignores them; it 
               // returns instantly if it encounters any record other than QSDT and NAM0.
               //
               break;
            default:
               intfc.log_load_warning(
                  file_read_warning::warn_about_unrecognized_subrecord(subrecord.signature(), *this->stub)
               );
               break;
         }
      }
   }
   /*static*/ void Quest::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'QTGL': // text global (there can be multiple)
            case 'NAM0': // log entry next quest
            case 'PRKR': // alias perk
            case 'SCOR': // alias spectator override package list ID
            case 'OCOR': // alias override corpse override package list ID
            case 'GWOR': // alias guard warn override package list ID
            case 'ECOR': // alias combat override package list ID
            case 'ALDN': // alias display name form ID
            case 'ALCO': // alias create object base form ID
            case 'ALEQ': // alias fill-from-quest ID
            case 'ALPC': // alias package
            case 'ALFC': // alias faction
            case 'ALSP': // alias spell
            case 'ALUA': // alias fill from unique actor base ID
            case 'ALFR': // alias fill from preplaced ref ID
            case 'VTCK': // alias additional voicetype ID
            case 'ALRT': // alias fill from LocRefType ID
            case 'ALFL': // alias fill from location ID
            case 'KNAM': // alias fill from location keyword ID
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
            case 'KSIZ': // alias keywords
            case 'KWDA':
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case 'CTDA':
               components::condition::generate_use_info(record, uib);
               break;
            case 'COCT':
            case 'CNTO':
            case 'COED':
               components::container_data::generate_use_info(subrecord, uib);
               break;
            #ifdef _DEBUG
            case 'EDID': // editor ID
            case 'FULL': // name
            case 'ENAM': // event
            case 'FLTR': // Object Window categorization
            case 'NEXT': // separates dialogue and event conditions
            case 'ANAM': // next alias ID
            case 'CNAM': // text of last log entry
            case 'QOBJ': // objective
            case 'FNAM': // objective flags / alias flags
            case 'NNAM': // objective text
            case 'QSTA': // target
            case 'INDX': // stage
            case 'QSDT': // log entry
            case 'ALLS': // location alias start
            case 'ALST': // reference alias start
            case 'ALID': // alias ID
            case 'ALFI': // alias force-into-alias ID
            case 'BNAM': // alias hidden flag
            case 'ONAM': // alias hidden flag
            case 'PRKZ': // alias perk count
            case 'ALFA': // alias fill from internal alias ID
            case 'ALEA': // alias fill from external alias ID
            case 'ALFE': // alias fill from event
            case 'ALFD': // alias fill from event data
            case 'ALCA': // alias create object at
            case 'ALCL': // alias create object of level
            case 'ALNA': // alias find matching reference near alias
            case 'ALNT': // alias find matching reference near alias type
            case 'ALED': // alias end marker
            case 'SCHR': // DEPRECATED: ObScript header
            case 'SCDA': // DEPRECATED: ObScript compiled code
            case 'SCTX': // DEPRECATED: ObScript source code
            case 'SCRO': // DEPRECATED: ObScript ObjectReference
            case 'SCRV': // DEPRECATED: ObScript ObjectReference variable
            case 'SLSD': // ObScript data? game doesn't load this
            case 'QNAM': // ObScript data? game doesn't load this / alias hidden flag
            case 'DNAM': // quest form version?
               break;
            #endif
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