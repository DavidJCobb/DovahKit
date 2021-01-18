#include "Quest.h"
#include "_common_cpp.h"
#include "../logging.h"
#include "../notice_code_list.h"

namespace dovah::loaded_forms {
   #pragma region Quest aliases
   void LocationAlias::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
                  cnd.read(subrecord.get_containing_record(), intfc);
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
               subrecord.read(this->force_into_alias_id);
               break;
            case 'FNAM':
               subrecord.read(this->flags);
               break;
            case 'BNAM':
               this->hidden_flags |= 1;
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
               this->hidden_flags |= 2;
               break;
         }
      }
      if (this->fill_type == fill_type_t::other_alias_in_other_quest)
         this->fill_from_alias = externalAliasID;
      else if (this->fill_type == fill_type_t::other_alias_in_same_quest)
         this->fill_from_alias = internalAliasID;
   }
   void LocationAlias::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto& ALLS = record.open_next_subrecord('ALLS');
      ALLS.write(this->id);
      ALLS.close();
      auto& ALID = record.open_next_subrecord('ALID');
      ALID.write(this->name);
      ALID.close();
      auto& FNAM = record.open_next_subrecord('FNAM');
      FNAM.write(this->flags);
      FNAM.close();
      if (this->hidden_flags) {
         if (this->hidden_flags & 1)
            record.open_next_subrecord('BNAM').close();
         if (this->hidden_flags & 2)
            record.open_next_subrecord('ONAM').close();
      }
      if (this->force_into_alias_id) {
         auto& ALFI = record.open_next_subrecord('ALFI');
         ALFI.write(this->force_into_alias_id);
         ALFI.close();
      }
      if (this->fill_type == fill_type_t::other_alias_in_other_quest) {
         record.write_formID_subrecord('ALEQ', this->fill_from_quest);
         auto& ALEA = record.open_next_subrecord('ALEA');
         ALEA.write(this->fill_from_alias);
         ALEA.close();
      } else if (this->fill_type == fill_type_t::other_alias_in_same_quest) {
         auto& ALFA = record.open_next_subrecord('ALFA');
         ALFA.write(this->fill_from_alias);
         ALFA.close();
         record.write_formID_subrecord('KNAM', this->fill_from_location_keyword, true);
      } else if (this->fill_type == fill_type_t::from_event) {
         auto& ALFE = record.open_next_subrecord('ALFE');
         ALFE.write(this->fill_from_event);
         ALFE.close();
         auto& ALFD = record.open_next_subrecord('ALFD');
         ALFD.write(this->fill_from_event_data);
         ALFD.close();
      } else if (this->fill_type == fill_type_t::preset) {
         record.write_formID_subrecord('ALFL', this->fill_from_location);
      }
      //
      for (auto& cnd : this->conditions)
         cnd.save(record, intfc);
      //
      record.open_next_subrecord('ALED').close(); // Alias end marker.
   }
   void LocationAlias::sever_outbound_references(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      this->fill_from_location.clear_if(my_owner, target);
      this->fill_from_location_keyword.clear_if(my_owner, target);
      this->fill_from_quest.clear_if(my_owner, target);
      for (auto& cnd : this->conditions)
         cnd.sever_outbound_references_to(target, my_owner);
   }

   void ReferenceAlias::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
               subrecord.read(this->force_into_alias_id);
               break;
            case 'BNAM': // shared with BGSRefAlias
               this->hidden_flags |= 1;
               break;
            case 'ONAM': // shared with BGSRefAlias
               this->hidden_flags |= 2;
               break;
            case 'QNAM':
               this->hidden_flags |= 4;
               break;
            case 'CTDA':
               {
                  auto& list = this->conditions;
                  list.emplace_back();
                  auto& cnd = *list.rbegin();
                  cnd.read(subrecord.get_containing_record(), intfc);
               }
               break;
            case 'KSIZ':
            case 'KWDA':
               this->keywords.load(subrecord, intfc);
               break;
            case 'COCT':
            case 'CNTO':
            case 'COED':
               this->inventory.load(subrecord, intfc);
               break;
            case 'SCOR':
               subrecord.read(this->package_override_lists.spectator);
               break;
            case 'OCOR':
               subrecord.read(this->package_override_lists.observe_corpse);
               break;
            case 'GWOR':
               subrecord.read(this->package_override_lists.guard_warn);
               break;
            case 'ECOR':
               subrecord.read(this->package_override_lists.combat);
               break;
            case 'ALDN':
               subrecord.read(this->display_name);
               break;
            case 'ALCO':
               this->fill_type = fill_type_t::create_object;
               subrecord.read(this->create_object_of_type);
               break;
            case 'ALCA':
               if (this->fill_type == fill_type_t::create_object)
                  subrecord.read(this->create_object_at_alias);
               break;
            case 'ALCL':
               if (this->fill_type == fill_type_t::create_object)
                  subrecord.read(this->create_object_of_level);
               break;
            case 'ALEQ':
               subrecord.read(this->fill_from_quest);
               this->fill_type = fill_type_t::other_alias_in_other_quest;
               break;
            case 'ALEA':
               if (this->fill_type == fill_type_t::other_alias_in_other_quest)
                  subrecord.read(this->fill_from_alias);
               break;
            case 'ALFA':
               this->fill_type = fill_type_t::other_alias_in_same_quest;
               subrecord.read(this->fill_from_alias);
               break;
            case 'ALNA':
               this->fill_type = fill_type_t::find_matching_reference;
               subrecord.read(this->fill_near_alias);
               break;
            case 'ALNT':
               if (this->fill_type == fill_type_t::find_matching_reference)
                  subrecord.read(this->fill_near_alias_type);
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
               this->fill_type = fill_type_t::preset_unique_actor;
               subrecord.read(this->fill_from_unique_actor_base);
               break;
            case 'ALFE':
               this->fill_type = fill_type_t::from_event;
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
               this->fill_type = fill_type_t::preset_placed_reference;
               subrecord.read(this->fill_from_reference);
               break;
            case 'VTCK':
               subrecord.read(this->additional_voicetype);
               break;
            case 'ALRT':
               subrecord.read(this->fill_loc_ref_type);
               break;
         }
      }
   }
   void ReferenceAlias::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto& ALST = record.open_next_subrecord('ALST');
      ALST.write(this->id);
      ALST.close();
      auto& ALID = record.open_next_subrecord('ALID');
      ALID.write(this->name);
      ALID.close();
      auto& FNAM = record.open_next_subrecord('FNAM');
      FNAM.write(this->flags);
      FNAM.close();
      if (this->hidden_flags) {
         if (this->hidden_flags & 1)
            record.open_next_subrecord('BNAM').close();
         if (this->hidden_flags & 2)
            record.open_next_subrecord('ONAM').close();
         if (this->hidden_flags & 4)
            record.open_next_subrecord('QNAM').close();
      }
      if (this->force_into_alias_id) {
         auto& ALFI = record.open_next_subrecord('ALFI');
         ALFI.write(this->force_into_alias_id);
         ALFI.close();
      }
      if (this->fill_type == fill_type_t::create_object) {
         record.write_formID_subrecord('ALCO', this->create_object_of_type);
         auto& ALCA = record.open_next_subrecord('ALCA');
         ALCA.write(this->create_object_at_alias);
         ALCA.close();
         auto& ALCL = record.open_next_subrecord('ALCL');
         ALCL.write(this->create_object_of_level);
         ALCL.close();
      } else if (this->fill_type == fill_type_t::other_alias_in_other_quest) {
         record.write_formID_subrecord('ALEQ', this->fill_from_quest);
         auto& ALEA = record.open_next_subrecord('ALEA');
         ALEA.write(this->fill_from_alias);
         ALEA.close();
      } else if (this->fill_type == fill_type_t::other_alias_in_same_quest) {
         auto& ALFA = record.open_next_subrecord('ALFA');
         ALFA.write(this->fill_from_alias);
         ALFA.close();
         record.write_formID_subrecord('ALRT', this->fill_loc_ref_type);
      } else if (this->fill_type == fill_type_t::from_event) {
         auto& ALFE = record.open_next_subrecord('ALFE');
         ALFE.write(this->fill_from_event);
         ALFE.close();
         auto& ALFD = record.open_next_subrecord('ALFD');
         ALFD.write(this->fill_from_event_data);
         ALFD.close();
      } else if (this->fill_type == fill_type_t::preset_placed_reference) {
         record.write_formID_subrecord('ALFR', this->fill_from_reference);
      } else if (this->fill_type == fill_type_t::find_matching_reference) {
         auto& ALNA = record.open_next_subrecord('ALNA');
         ALNA.write(this->fill_near_alias);
         ALNA.close();
         auto& ALNT = record.open_next_subrecord('ALNT');
         ALNT.write(this->fill_near_alias_type);
         ALNT.close();
      } else if (this->fill_type == fill_type_t::preset_unique_actor) {
         record.write_formID_subrecord('ALUA', this->fill_from_unique_actor_base);
      }
      //
      for (auto& cnd : this->conditions)
         cnd.save(record, intfc);
      this->keywords.save(record, intfc);
      this->inventory.save(record, intfc);
      record.write_formID_subrecord('SPOR', this->package_override_lists.spectator, true);
      record.write_formID_subrecord('OCOR', this->package_override_lists.observe_corpse, true);
      record.write_formID_subrecord('GWOR', this->package_override_lists.guard_warn, true);
      record.write_formID_subrecord('ECOR', this->package_override_lists.combat, true);
      record.write_formID_subrecord('ALDN', this->display_name, true);
      for (auto& id : this->spells)
         record.write_formID_subrecord('ALSP', id);
      for (auto& id : this->factions)
         record.write_formID_subrecord('ALFC', id);
      for (auto& id : this->packages)
         record.write_formID_subrecord('ALPC', id);
      record.write_formID_subrecord('VTCK', this->additional_voicetype, true);
      //
      record.open_next_subrecord('ALED').close(); // Alias end marker.
   }
   void ReferenceAlias::sever_outbound_references(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      this->create_object_of_type.clear_if(my_owner, target);
      this->fill_from_quest.clear_if(my_owner, target);
      this->fill_from_reference.clear_if(my_owner, target);
      this->fill_from_unique_actor_base.clear_if(my_owner, target);
      for (auto& cnd : this->conditions)
         cnd.sever_outbound_references_to(target, my_owner);
      this->keywords.sever_outbound_references_to(target, my_owner);
      this->inventory.sever_outbound_references_to(target, my_owner);
      this->package_override_lists.spectator.clear_if(my_owner, target);
      this->package_override_lists.observe_corpse.clear_if(my_owner, target);
      this->package_override_lists.guard_warn.clear_if(my_owner, target);
      this->package_override_lists.combat.clear_if(my_owner, target);
      remove_form_from_reference_list(this->spells, target, my_owner);
      remove_form_from_reference_list(this->factions, target, my_owner);
      remove_form_from_reference_list(this->packages, target, my_owner);
      this->additional_voicetype.clear_if(my_owner, target);
   }
   #pragma endregion

   #pragma region Quest components
      #pragma region Quest log entries
      void Quest::LogEntry::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
         auto& subrecord = record.get_current_subrecord();
         assert(subrecord.signature() == 'QSDT' && "Quest::LogEntry::load should only be called just after the QSDT subrecord is opened.");
         subrecord.read(this->flags);
         if (record.peek_next_subrecord_type() != 'NAM0')
            return;
         record.next_subrecord();
         subrecord.read(this->next_quest_id);
      }
      void Quest::LogEntry::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load&) {
         assert(subrecord.signature() == 'CNAM' && "Quest::LogEntry::loadText should only be called just after the CNAM subrecord is opened.");
         subrecord.to_string(this->journal_text);
      }
      bool Quest::LogEntry::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
         auto& QSDT = record.open_next_subrecord('QSDT');
         QSDT.write(this->flags);
         QSDT.close();
         for (auto& cnd : this->conditions)
            cnd.save(record, intfc);
         auto& CNAM = record.open_next_subrecord('CNAM');
         CNAM.write(this->journal_text);
         CNAM.close();
         record.write_formID_subrecord('NAM0', this->next_quest_id, true);
         return true;
      }
      void Quest::LogEntry::sever_outbound_references(form_stub& other, loaded_forms::Form& my_owner) noexcept {
         for (auto& cnd : this->conditions)
            cnd.sever_outbound_references_to(other, my_owner);
         this->next_quest_id.clear_if(my_owner, other);
      }
      #pragma endregion

      #pragma region Quest stages
      void Quest::Stage::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
         assert(subrecord.signature() == 'INDX' && "Quest::Stage::load should only be called just after the INDX subrecord is opened.");
         if (subrecord.is_in_bounds(4)) {
            subrecord.unchecked_read(this->index);
            subrecord.unchecked_read(this->flags);
            subrecord.unchecked_read(this->padding);
         }
      }
      bool Quest::Stage::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
         auto& INDX = record.open_next_subrecord('INDX');
         INDX.write(this->index);
         INDX.write(this->flags);
         INDX.write(this->padding);
         INDX.close();
         for (auto& entry : this->entries)
            entry.save(record, intfc);
         return true;
      }
      void Quest::Stage::sever_outbound_references(form_stub& other, loaded_forms::Form& my_owner) noexcept {
         for (auto& entry : this->entries)
            entry.sever_outbound_references(other, my_owner);
      }
      #pragma endregion

      #pragma region Quest targets
      void Quest::Target::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
         assert(subrecord.signature() == 'QSTA' && "Quest::Target::load should only be called just after the QSTA subrecord is opened.");
         subrecord.read(this->aliasID);
         subrecord.read(this->flags);
         subrecord.skip_bytes(3);
         //
         // Conditions aren't loaded here; the main QUST loader handles that.
         //
      }
      bool Quest::Target::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
         auto& QSTA = record.open_next_subrecord('QSTA');
         QSTA.write(this->aliasID);
         QSTA.write(this->flags);
         QSTA.skip_bytes(3);
         QSTA.close();
         for (auto& cnd : this->conditions)
            cnd.save(record, intfc);
         return true;
      }
      void Quest::Target::sever_outbound_references(form_stub& other, loaded_forms::Form& my_owner) noexcept {
         for (auto& cnd : this->conditions)
            cnd.sever_outbound_references_to(other, my_owner);
      }
      #pragma endregion

      #pragma region Quest objectives
      void Quest::Objective::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
         auto& subrecord = record.get_current_subrecord();
         assert(subrecord.signature() == 'QOBJ' && "Quest::Objective::load should only be called just after the QOBJ subrecord is opened.");
         if (subrecord.size() == 4) {
            uint32_t temp;
            subrecord.read(temp);
            this->index = temp;
         } else {
            subrecord.read(this->index);
         }
         //
         // Once Skyrim sees a QOBJ, it just blindly reads subrecords either until it finds an 'NNAM' 
         // subrecord or until it hits the end of the containing record.
         //
         // See: Classic 0x00551C70 == void TESQuest::Objective::Load(TESFile*);
         //
         while (record.next_subrecord().exists()) { // TESPluginRecord::next_subrecord alters (subrecord) and returns it
            switch (subrecord.signature()) {
               case 'FNAM':
                  subrecord.read(this->flags);
                  break;
               case 'NNAM':
                  subrecord.to_string(this->text);
                  return;
               default:
                  {
                     detailed_notice warning;
                     warning.code = notice_code::quest_objective_unexpected_subrecord;
                     warning.set_cause_form(intfc.target_stub);
                     warning.set_cause_subrecord(subrecord.signature());
                     //
                     intfc.log_load_warning(warning);
                  }
                  break;
            }
         }
      }
      bool Quest::Objective::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
         auto& QOBJ = record.open_next_subrecord('QOBJ');
         QOBJ.write(this->index);
         QOBJ.close();
         auto& FNAM = record.open_next_subrecord('FNAM');
         FNAM.write(this->flags);
         FNAM.close();
         auto& NNAM = record.open_next_subrecord('NNAM');
         NNAM.write(this->text);
         NNAM.close();
         for (auto& t : this->targets)
            t.save(record, intfc);
         return true;
      }
      void Quest::Objective::sever_outbound_references(form_stub& other, loaded_forms::Form& my_owner) noexcept {
         for (auto& t : this->targets)
            t.sever_outbound_references(other, my_owner);
      }
      #pragma endregion
   #pragma endregion

   Quest::~Quest() {
      for (auto it = this->aliases.begin(); it != this->aliases.end(); ++it)
         delete (*it);
      this->aliases.clear();
   }
   void Quest::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      bool isInEventConditions = false;
      bool hasLastLogEntry     = false;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'FULL':
               subrecord.to_string(this->name);
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'DNAM': // required; TODO: fail if this is not present; fail if it is too short
               if (subrecord.is_in_bounds(12)) {
                  subrecord.unchecked_read(this->flags);
                  subrecord.unchecked_read(this->priority);
                  subrecord.unchecked_read(this->form_version);
                  subrecord.unchecked_read(this->unknown);
                  uint32_t type;
                  subrecord.unchecked_read(type); // stored as a uint32_t, but only the low byte is retained in memory
                  this->quest_type = type;
               }
               break;
            case 'ENAM':
               subrecord.read(this->event);
               break;
            case 'QTGL':
               {
                  form_reference_t id;
                  if (subrecord.read(id) && id)
                     this->text_display_globals.push_back(id);
               }
               break;
            case 'FLTR': // required; TODO: fail if this is not present
               subrecord.to_string(this->editor_category);
               break;
            case 'NEXT':
               isInEventConditions = true;
               break;
            case 'ANAM':
               subrecord.read(this->next_alias_id);
               break;
            case 'INDX': // also handles QSDT
               {
                  this->stages.emplace_back();
                  auto& stage = *this->stages.rbegin();
                  stage.load(subrecord, intfc);
               }
               break;
            case 'QSTD':
               if (this->stages.size()) { // the game ignores QSTD that appear when there is no stage
                  hasLastLogEntry = true;
                  auto& stage = this->stages.back();
                  auto& entry = stage.entries.emplace_back();
                  entry.load(record, intfc);
               }
               break;
            case 'CNAM':
               if (!hasLastLogEntry)
                  break;
               this->stages.back().entries.back().load(subrecord, intfc);
               break;
            case 'SCHR':
               if (!hasLastLogEntry)
                  break;
               //
               // TODO: Add ObScript data to last-loaded entry
               //
               break;
            case 'QOBJ':
               {
                  this->objectives.emplace_back();
                  auto& objective = *this->objectives.rbegin();
                  objective.load(subrecord.get_containing_record(), intfc);
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
                  target.load(subrecord, intfc);
               }
               hasLastLogEntry = false; // per TESV.exe TESQuest::LoadForm
               break;
            case 'CTDA':
               //
               // This is also how the game loads QUST/CTDA.
               //
               {
                  components::condition nc;
                  nc.read(subrecord.get_containing_record(), intfc);
                  if (hasLastLogEntry) {
                     auto& entry = this->stages.back().entries.back();
                     entry.conditions.push_back(nc);
                     break;
                  }
                  if (!this->objectives.empty()) {
                     auto& objective = this->objectives.back();
                     if (!objective.targets.empty()) {
                        objective.targets.back().conditions.push_back(nc);
                     }
                  }
                  auto& list = this->conditions.dialogue;
                  if (isInEventConditions)
                     list = this->conditions.event;
                  list.push_back(nc);
               }
               break;
            case 'ALLS':
               {
                  auto alias = new LocationAlias;
                  this->aliases.push_back(alias);
                  alias->load(record, intfc);
               }
               break;
            case 'ALST':
               {
                  auto alias = new ReferenceAlias;
                  this->aliases.push_back(alias);
                  alias->load(record, intfc);
               }
               break;
            case 'SCDA':
            case 'SCRV':
            case 'SLSD':
            case 'QNAM':
               //
               // The game passes all of these to TESQuest::LogEntry::Load, but it just ignores them; it 
               // returns instantly if it encounters any record other than QSDT and NAM0.
               //
               break;
            default:
               intfc.log_load_warning(
                  detailed_notice::warn_about_unrecognized_subrecord(subrecord.signature(), *this->stub)
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
   bool Quest::_save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc); // VMAD (won't write anything if no scripts are attached)
      //
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      //
      auto& DNAM = record.open_next_subrecord('DNAM');
      DNAM.write(this->flags);
      DNAM.write(this->priority);
      DNAM.write(this->form_version);
      DNAM.write(this->unknown);
      DNAM.write(this->quest_type);
      DNAM.close();
      //
      if (this->event) {
         auto& ENAM = record.open_next_subrecord('ENAM');
         ENAM.write(this->event);
         ENAM.close();
      }
      for (auto& id : this->text_display_globals) {
         auto& QTGL = record.open_next_subrecord('QTGL');
         QTGL.write(id);
         QTGL.close();
      }
      auto& FLTR = record.open_next_subrecord('FLTR');
      FLTR.write(this->editor_category);
      FLTR.close();
      for (auto& cnd : this->conditions.dialogue) {
         cnd.save(record, intfc);
      }
      record.open_next_subrecord('NEXT').close();
      for (auto& cnd : this->conditions.event) {
         cnd.save(record, intfc);
      }
      for (auto& obj : this->stages)
         if (!obj.save(record, intfc))
            return false;
      for (auto& obj : this->objectives)
         if (!obj.save(record, intfc))
            return false;
      auto& ANAM = record.open_next_subrecord('ANAM');
      ANAM.write(this->next_alias_id);
      ANAM.close();
      for (auto* alias : this->aliases)
         alias->save(record, intfc);
      return true;
   }
   void Quest::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      for (auto& cnd : this->conditions.dialogue)
         cnd.sever_outbound_references_to(other, *this);
      for (auto& cnd : this->conditions.event)
         cnd.sever_outbound_references_to(other, *this);
      for (auto& obj : this->stages)
         obj.sever_outbound_references(other, *this);
      for (auto& obj : this->objectives)
         obj.sever_outbound_references(other, *this);
      for (auto* obj : this->aliases)
         obj->sever_outbound_references(other, *this);
      remove_form_from_reference_list(this->text_display_globals, other, *this);
   }
}