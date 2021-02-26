#include "Quest.h"
#include "_common_cpp.h"
#include "../logging.h"
#include "../notice_code_list.h"

namespace {
   constexpr uint32_t _signature_for_alias_type(dovah::loaded_forms::Alias::alias_type t) {
      using namespace dovah::loaded_forms;
      switch (t) {
         case Alias::alias_type::location:
            return 'ALLS';
         case Alias::alias_type::reference:
            return 'ALST';
      }
      return 0;
   }
}

namespace dovah::loaded_forms {
   #pragma region Quest aliases
   void Alias::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      auto&    subrecord = record.get_current_subrecord();
      uint32_t required  = _signature_for_alias_type(this->type);
      if (required) {
         //
         // TODO: make this a load error, NOT an assert
         //
         assert(subrecord.signature() == required && "Alias::load should only be called just after the ALLS/ALST subrecord is opened.");
      }
      //
      subrecord.read(this->id);
      if (!record.next_subrecord())
         return;
      //
      alias_id_t external_alias = none_id; // ALEA
      alias_id_t internal_alias = none_id; // ALFA
      for (; subrecord.exists() && subrecord.signature() != 'ALED'; record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'ALID':
               subrecord.to_string(this->name);
               break;
            case 'FNAM':
               subrecord.read(this->flags);
               break;
            case 'BNAM':
               this->hidden_flags |= 1;
               break;
            case 'ONAM':
               this->hidden_flags |= 2;
               break;
            case 'ALFI':
               subrecord.read(this->force_into_alias_id);
               break;
               //
            case 'ALEA':
               subrecord.read(external_alias);
               break;
            case 'ALFA':
               subrecord.read(internal_alias);
               break;
            case 'ALEQ':
               subrecord.read(this->fill_from_alias.quest);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::quest, intfc.target_stub, this->fill_from_alias.quest)
               );
               this->fill_type = fill_type_t::other_alias_in_other_quest;
               break;
            case 'ALFE':
               subrecord.read_signature(this->fill_from_event.code);
               this->fill_type = fill_type_t::from_event;
               break;
            case 'ALFD':
               subrecord.read_signature(this->fill_from_event.member);
               if (this->fill_from_event.code == story_event_code::undefined)
                  this->fill_from_event.member = -1;
               else {
                  //
                  // TODO: The value undergoes further checks? See Skyrim Classic code from 0x0054E291.
                  //
               }
               break;
               //
            case 'CTDA':
               this->conditions.read_next(subrecord.get_containing_record(), intfc);
               break;
               //
            default:
               if (!this->_load_impl(subrecord, intfc)) {
                  //
                  // TODO: log unrecognized subrecord warning
                  //
               }
               break;
         }
      }
      if (this->fill_type == fill_type_t::other_alias_in_other_quest)
         this->fill_from_alias.alias = external_alias;
      else if (this->fill_type == fill_type_t::other_alias_in_same_quest)
         this->fill_from_alias.alias = internal_alias;
   }
   bool LocationAlias::_load_impl(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      switch (subrecord.signature()) {
         case 'ALFL':
            if (subrecord.read(this->fill_from_location))
               this->fill_type = fill_type_t::preset_location;
            return true;
         case 'KNAM': // ALFA+KNAM
            subrecord.read(this->fill_from_location_keyword);
            return true;
      }
      return false;
   }
   bool ReferenceAlias::_load_impl(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      form_reference_t formID;
      switch (subrecord.signature()) {
         case 'ALRT': // ALFA+ALRT
            subrecord.read(this->fill_loc_ref_type);
            intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
               detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::location_ref_type, intfc.target_stub, this->fill_loc_ref_type)
            );
            break;
         case 'ALFR':
            if (subrecord.read(this->fill_from_reference))
               this->fill_type = fill_type_t::preset_placed_reference;
            return true;
         case 'QNAM':
            this->hidden_flags |= 4;
            break;
            //
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
         case 'ALNA':
            this->fill_type = fill_type_t::find_matching_reference;
            subrecord.read(this->fill_near_alias);
            break;
         case 'ALNT':
            if (this->fill_type == fill_type_t::find_matching_reference)
               subrecord.read(this->fill_near_alias_type);
            break;
         case 'ALUA':
            this->fill_type = fill_type_t::preset_unique_actor;
            subrecord.read(this->fill_from_unique_actor_base);
            break;
            //
         case 'ALPC':
            if (subrecord.read(formID)) {
               this->packages.push_back(formID);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::package, intfc.target_stub, formID)
               );
            }
            break;
         case 'ALFC':
            if (subrecord.read(formID)) {
               this->factions.push_back(formID);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::faction, intfc.target_stub, formID)
               );
            }
            break;
         case 'ALSP':
            if (subrecord.read(formID)) {
               this->spells.push_back(formID);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::spell, intfc.target_stub, formID)
               );
            }
            break;
         case 'VTCK':
            subrecord.read(this->additional_voicetype);
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
      }
      return false;
   }

   void Alias::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto& start = record.open_next_subrecord(_signature_for_alias_type(this->type));
      start.write(this->id);
      start.close();
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
      bool fully_handled = true;
      switch (this->fill_type) {
         case fill_type_t::none:
            break;
         case fill_type_t::other_alias_in_same_quest:
            {
            }
            break;
         case fill_type_t::from_event:
            {
               auto& ALFE = record.open_next_subrecord('ALFE');
               ALFE.write_signature(this->fill_from_event.code);
               ALFE.close();
               auto& ALFD = record.open_next_subrecord('ALFD');
               ALFD.write_signature(this->fill_from_event.member);
               ALFD.close();
            }
            break;
         case fill_type_t::other_alias_in_other_quest:
            {
               record.write_formID_subrecord('ALEQ', this->fill_from_alias.quest);
               auto& ALEA = record.open_next_subrecord('ALEA');
               ALEA.write(this->fill_from_alias.alias);
               ALEA.close();
            }
            break;
         default:
            fully_handled = false;
            break;
      }
      if (!this->_save_fill_impl(record, intfc) && !fully_handled) {
         //
         // Unrecognized type.
         //
      }
      //
      for (auto& cnd : this->conditions)
         cnd.save(record, intfc);
      this->_save_body_impl(record, intfc);
      //
      record.open_next_subrecord('ALED').close(); // Alias end marker.
   }
   //
   bool LocationAlias::_save_fill_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      switch (this->fill_type) {
         case fill_type_t::preset_location:
            record.write_formID_subrecord('ALFL', this->fill_from_location);
            return true;
            //
         case fill_type_t::other_alias_in_same_quest:
            //
            // This is a special case: the base Alias class saved ALFA, but we need to also save KNAM.
            //
            record.write_formID_subrecord('KNAM', this->fill_from_location_keyword, true);
            return true;
      }
      return false;
   }
   void LocationAlias::_save_body_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
   }
   //
   bool ReferenceAlias::_save_fill_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      switch (this->fill_type) {
         case fill_type_t::create_object:
            {
               record.write_formID_subrecord('ALCO', this->create_object_of_type);
               auto& ALCA = record.open_next_subrecord('ALCA');
               ALCA.write(this->create_object_at_alias);
               ALCA.close();
               auto& ALCL = record.open_next_subrecord('ALCL');
               ALCL.write(this->create_object_of_level);
               ALCL.close();
            }
            return true;
         case fill_type_t::preset_placed_reference:
            record.write_formID_subrecord('ALFR', this->fill_from_reference);
            return true;
         case fill_type_t::find_matching_reference:
            {
               auto& ALNA = record.open_next_subrecord('ALNA');
               ALNA.write(this->fill_near_alias);
               ALNA.close();
               auto& ALNT = record.open_next_subrecord('ALNT');
               ALNT.write(this->fill_near_alias_type);
               ALNT.close();
            }
            return true;
         case fill_type_t::preset_unique_actor:
            record.write_formID_subrecord('ALUA', this->fill_from_unique_actor_base);
            return true;
            //
         case fill_type_t::other_alias_in_same_quest:
            //
            // This is a special case: the base Alias class saved ALFA, but we need to also save ALRT.
            //
            record.write_formID_subrecord('ALRT', this->fill_loc_ref_type);
            return true;
      }
      return false;
   }
   void ReferenceAlias::_save_body_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
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
      if (this->hidden_flags & 4)
         record.open_next_subrecord('QNAM').close();
   }

   void Alias::sever_outbound_references(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      this->fill_from_alias.quest.clear_if(my_owner, target);
      for (auto& cnd : this->conditions)
         cnd.sever_outbound_references_to(target, my_owner);
      this->script_data.sever_outbound_references_to(target, my_owner);
      //
      this->_sever_outbound_references_impl(target, my_owner);
   }
   void LocationAlias::_sever_outbound_references_impl(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      this->fill_from_location.clear_if(my_owner, target);
      this->fill_from_location_keyword.clear_if(my_owner, target);
   }
   void ReferenceAlias::_sever_outbound_references_impl(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      this->create_object_of_type.clear_if(my_owner, target);
      this->fill_from_reference.clear_if(my_owner, target);
      this->fill_from_unique_actor_base.clear_if(my_owner, target);
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

   Alias* Alias::clone(loaded_forms::Form& clone_owner) {
      Alias* copy = this->_clone_impl(clone_owner);
      if (!copy)
         return nullptr;
      //
      copy->id    = this->id;
      copy->name  = this->name;
      copy->flags = this->flags;
      copy->hidden_flags = this->hidden_flags;
      copy->force_into_alias_id = this->force_into_alias_id;
      copy->fill_from_alias.alias = this->fill_from_alias.alias;
      copy->fill_from_alias.quest.set(clone_owner, this->fill_from_alias.quest);
      copy->fill_from_event.code   = this->fill_from_event.code;
      copy->fill_from_event.member = this->fill_from_event.member;
      copy->fill_type = this->fill_type;
      //
      copy->conditions.append_all_of(clone_owner, this->conditions);
      copy->script_data.clone_from(this->script_data, clone_owner);
      //
      copy->_clone_impl(clone_owner);
      //
      return copy;
   }
   Alias* LocationAlias::_clone_impl(loaded_forms::Form& clone_owner) {
      auto* copy = new LocationAlias(this->owner);
      //
      copy->fill_from_location.set(clone_owner, this->fill_from_location);
      copy->fill_from_location_keyword.set(clone_owner, this->fill_from_location_keyword);
      //
      return copy;
   }
   Alias* ReferenceAlias::_clone_impl(loaded_forms::Form& clone_owner) {
      auto* copy = new ReferenceAlias(this->owner);
      //
      copy->keywords.clone_from(this->keywords, clone_owner);
      copy->inventory.clone_from(this->inventory, clone_owner);
      copy->additional_voicetype.set(clone_owner, this->additional_voicetype);
      copy->create_object_at_alias = this->create_object_at_alias;
      copy->create_object_of_level = this->create_object_of_level;
      copy->create_object_of_type.set(clone_owner, this->create_object_of_type);
      copy->display_name.set(clone_owner, this->display_name);
      copy_form_reference_list(clone_owner, copy->packages, this->packages);
      copy_form_reference_list(clone_owner, copy->factions, this->factions);
      copy_form_reference_list(clone_owner, copy->spells, this->spells);
      copy->package_override_lists.combat.set(clone_owner, this->package_override_lists.combat);
      copy->package_override_lists.guard_warn.set(clone_owner, this->package_override_lists.guard_warn);
      copy->package_override_lists.observe_corpse.set(clone_owner, this->package_override_lists.observe_corpse);
      copy->package_override_lists.spectator.set(clone_owner, this->package_override_lists.spectator);
      copy->fill_from_reference.set(clone_owner, this->fill_from_reference);
      copy->fill_from_unique_actor_base.set(clone_owner, this->fill_from_unique_actor_base);
      copy->fill_loc_ref_type.set(clone_owner, this->fill_loc_ref_type);
      copy->fill_near_alias = this->fill_near_alias;
      copy->fill_near_alias_type = this->fill_near_alias_type;
      //
      return copy;
   }

   void Alias::clear(loaded_forms::Form& my_owner) {
      this->id = -1;
      this->name.clear();
      this->flags = 0;
      this->hidden_flags = 0;
      this->force_into_alias_id = -1;
      this->fill_from_alias.alias = none_id;
      this->fill_from_alias.quest.set(my_owner, nullptr);
      this->fill_from_event.code   = story_event_code::undefined;
      this->fill_from_event.member = -1;
      this->conditions.clear(my_owner);
      this->script_data.clear(my_owner);
      //
      this->_clear_impl(my_owner);
   }
   void LocationAlias::_clear_impl(loaded_forms::Form& my_owner) {
      this->fill_from_location.set(my_owner, nullptr);
      this->fill_from_location_keyword.set(my_owner, nullptr);
   }
   void ReferenceAlias::_clear_impl(loaded_forms::Form& my_owner) {
      this->keywords.clear(my_owner);
      this->inventory.clear(my_owner);
      this->additional_voicetype.set(my_owner, nullptr);
      this->create_object_at_alias = this->create_object_at_alias;
      this->create_object_of_level = this->create_object_of_level;
      this->create_object_of_type.set(my_owner, this->create_object_of_type);
      this->display_name.set(my_owner, nullptr);
      clear_form_reference_list(this->packages, my_owner);
      clear_form_reference_list(this->factions, my_owner);
      clear_form_reference_list(this->spells, my_owner);
      this->package_override_lists.combat.set(my_owner, nullptr);
      this->package_override_lists.guard_warn.set(my_owner, nullptr);
      this->package_override_lists.observe_corpse.set(my_owner, nullptr);
      this->package_override_lists.spectator.set(my_owner, nullptr);
      this->fill_from_reference.set(my_owner, nullptr);
      this->fill_from_unique_actor_base.set(my_owner, nullptr);
      this->fill_loc_ref_type.set(my_owner, nullptr);
      this->fill_near_alias = -1;
      this->fill_near_alias_type = 0;
   }
   #pragma endregion

   #pragma region Quest components
      #pragma region Quest log entries
         #pragma region Quest fragments
         void Quest::LogEntry::script_fragment::load(tes_subrecord_reader& subrecord) {
            assert(subrecord.signature() == 'VMAD');
            subrecord.unchecked_read(this->stage_id);
            subrecord.unchecked_read(this->unknown02);
            subrecord.unchecked_read(this->entry_index);
            subrecord.unchecked_read(this->unknown08);
            if (!subrecord.read_length_prefixed_string<2>(this->filename))
               return;
            if (!subrecord.read_length_prefixed_string<2>(this->function))
               return;
         }
         bool Quest::LogEntry::script_fragment::save(tes_subrecord_writer& subrecord, uint16_t stage_id, uint32_t entry_index) {
            assert(subrecord.signature() == 'VMAD');
            subrecord.write(stage_id);
            subrecord.write(this->unknown02);
            subrecord.write(entry_index);
            subrecord.write(this->unknown08);
            subrecord.write_length_prefixed_string<2>(this->filename);
            subrecord.write_length_prefixed_string<2>(this->function);
            return true;
         }
         void Quest::LogEntry::script_fragment::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
            subrecord.skip_bytes(
               sizeof(stage_id) + 
               sizeof(unknown02) + 
               sizeof(entry_index) +
               sizeof(unknown08)
            );
            subrecord.skip_length_prefixed_string<2>();
            subrecord.skip_length_prefixed_string<2>();
         }
         void Quest::LogEntry::script_fragment::clear() {
            this->unknown02 = 0x0000;
            this->unknown08 = 0x01;
            this->stage_id    = 0;
            this->entry_index = 0;
            this->filename.clear();
            this->function.clear();
         }
         #pragma endregion

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
         void Quest::LogEntry::clone_from(const LogEntry& source, loaded_forms::Form& owner) {
            this->flags         = source.flags;
            this->journal_text  = source.journal_text;
            this->next_quest_id = source.next_quest_id;
            this->conditions.append_all_of(owner, source.conditions);
            this->fragment = source.fragment;
         }
         void Quest::LogEntry::clear(loaded_forms::Form& owner) {
            this->flags = 0;
            this->journal_text.reset();
            this->next_quest_id.set(owner, nullptr);
            this->conditions.clear(owner);
            this->fragment.clear();
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
      void Quest::Stage::clone_from(const Stage& copy, loaded_forms::Form& owner) {
         this->index   = copy.index;
         this->flags   = copy.flags;
         this->padding = copy.padding;
         //
         size_t size = copy.entries.size();
         this->entries.resize(size);
         for (size_t i = 0; i < size; ++i)
            this->entries[i].clone_from(copy.entries[i], owner);
      }
      void Quest::Stage::clear(loaded_forms::Form& owner) {
         this->index   = 0;
         this->flags   = 0;
         this->padding = 0;
         //
         for (auto& entry : this->entries)
            entry.clear(owner);
         this->entries.clear();
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
      void Quest::Target::clone_from(const Target& copy, loaded_forms::Form& owner) {
         this->aliasID = copy.aliasID;
         this->flags   = copy.flags;
         this->conditions.append_all_of(owner, copy.conditions);
      }
      void Quest::Target::clear(loaded_forms::Form& owner) {
         this->aliasID = -1;
         this->flags   = 0;
         this->conditions.clear(owner);
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
      void Quest::Objective::clone_from(const Objective& copy, loaded_forms::Form& owner) {
         this->index = copy.index;
         this->flags = copy.flags;
         this->text  = copy.text;
         //
         size_t size = copy.targets.size();
         this->targets.resize(size);
         for (size_t i = 0; i < size; ++i)
            this->targets[i].clone_from(copy.targets[i], owner);
      }
      void Quest::Objective::clear(loaded_forms::Form& owner) {
         this->index = 0;
         this->flags = 0;
         this->text.reset();
         //
         for (auto& t : this->targets)
            t.clear(owner);
         this->targets.clear();
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
      if (!intfc.is_winning_record)
         return;
      //
      struct _pending_alias_script_data {
         alias_id_t alias_id = -1;
         components::papyrus::script_data data;
      };
      //
      std::vector<_pending_alias_script_data> pending_alias_scripts;
      std::vector<LogEntry::script_fragment>  pending_log_entry_scripts;
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
               if (!subrecord.is_at_end()) {
                  uint16_t count;
                  //
                  subrecord.unchecked_read(this->script_fragment_root.unknown);
                  if (!subrecord.read(count)) // log entry fragment count
                     break;
                  subrecord.read_length_prefixed_string<2>(this->script_fragment_root.filename);
                  for (uint16_t i = 0; i < count; ++i) {
                     pending_log_entry_scripts.emplace_back().load(subrecord);
                  }
                  if (!subrecord.read(count)) // alias script data count
                     break;
                  for (uint16_t i = 0; i < count; ++i) {
                     components::papyrus::script_data::property_object_value owner;
                     owner.load(this->script_data.header, subrecord);
                     if (owner.form != &this->stub) {
                        detailed_notice warning;
                        warning.code = notice_code::alias_papyrus_data_specifies_wrong_quest;
                        warning.set_cause_form(this->stub);
                        warning.set_cause_subrecord(subrecord.signature());
                        if (owner.form)
                           warning.add_relevant_form(*owner.form.get_form_stub());
                        warning.extra_integers[0] = owner.aliasID;
                        intfc.log_load_warning(warning);
                        //
                        continue;
                     }
                     auto& entry = pending_alias_scripts.emplace_back();
                     entry.alias_id = owner.aliasID;
                     entry.data.load(subrecord, intfc);
                  }
               }
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
               subrecord.read_signature(this->event);
               break;
            case 'QTGL':
               {
                  form_reference_t id;
                  if (subrecord.read(id) && id) {
                     this->text_display_globals.push_back(id);
                     intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                        detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::global, this->stub, id)
                     );
                  }
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
               this->stages.emplace_back().load(subrecord, intfc);
               break;
            case 'QSDT':
               if (this->stages.size()) { // the game ignores QSDT that appear when there is no stage
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
                  auto& target = *objective.targets.rbegin();
                  target.load(subrecord, intfc);
               }
               hasLastLogEntry = false; // per TESV.exe TESQuest::LoadForm
               break;
            case 'CTDA':
               //
               // This is also how the game loads QUST/CTDA.
               //
               {
                  auto& record = subrecord.get_containing_record();
                  if (hasLastLogEntry) {
                     auto& entry = this->stages.back().entries.back();
                     entry.conditions.read_next(subrecord.get_containing_record(), intfc);
                     break;
                  }
                  if (!this->objectives.empty()) {
                     auto& objective = this->objectives.back();
                     if (!objective.targets.empty()) {
                        objective.targets.back().conditions.read_next(subrecord.get_containing_record(), intfc);
                        break;
                     }
                  }
                  auto& list = isInEventConditions ? this->conditions.event : this->conditions.dialogue;
                  list.read_next(subrecord.get_containing_record(), intfc);
               }
               break;
            case 'ALLS':
               {
                  auto alias = new LocationAlias(*this);
                  this->aliases.push_back(alias);
                  alias->load(record, intfc);
               }
               break;
            case 'ALST':
               {
                  auto alias = new ReferenceAlias(*this);
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
                  detailed_notice::warn_about_unrecognized_subrecord(subrecord.signature(), this->stub)
               );
               break;
         }
      }
      //
      // If we loaded any quest-specific VMAD data, this is the point where we need to distribute 
      // that data among its owning aliases and log entries.
      //
      if (!pending_alias_scripts.empty()) {
         for (auto& entry : pending_alias_scripts) {
            auto* alias = this->lookup_alias_by_id(entry.alias_id);
            if (!alias) {
               detailed_notice warning;
               warning.code = notice_code::alias_papyrus_data_belongs_to_missing_alias;
               warning.set_cause_form(this->stub);
               warning.set_cause_subrecord('VMAD');
               warning.extra_integers[0] = entry.alias_id;
               intfc.log_load_warning(warning);
               //
               continue;
            }
            assert(alias->script_data.empty() && "TODO: How do the game and CK handle multiple VMAD entries for a single alias?"); // TODO
            alias->script_data = entry.data;
         }
         pending_alias_scripts.clear();
      }
      if (!pending_log_entry_scripts.empty()) {
         for (auto& data : pending_log_entry_scripts) {
            auto* stage = this->lookup_stage_by_id(data.stage_id);
            if (stage) {
               auto& list = stage->entries;
               if (data.entry_index < list.size()) {
                  list[data.entry_index].fragment = data;
                  continue;
               }
            }
            static_assert(false, "If any VMAD-log-entry data specifies a bad stage or log entry, we should emit a load warning.");
            this->script_fragment_root.unowned_log_entry_data.push_back(data);
         }
         pending_log_entry_scripts.clear();
      }
      if (!this->script_fragment_root.unowned_log_entry_data.empty()) {
         detailed_notice warning;
         warning.code = notice_code::quest_has_phantom_script_data;
         warning.set_cause_form(this->stub);
         warning.set_cause_subrecord('VMAD');
         intfc.log_load_warning(warning);
      }
   }
   /*static*/ void Quest::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      //
      // Use info for quests is challenging because we need to exactly synch it with all of the data 
      // that we load *and retain.* We want to discard orphaned sub-form script data (e.g. VMAD data 
      // for a non-existent alias), which means that we can't just blindly collect that data's use 
      // info. We need to gather the data's use info and hold onto it, and only commit it when we 
      // know that the alias with the given ID exists.
      //
      // The game doesn't require that VMAD come before aliases, so in practice we have to commit 
      // any valid sub-form script data at the end.
      //
      struct _alias_papyrus_use_info {
         uint16_t alias_id;
         form_stub_use_info_builder* pending = nullptr;
         //
         _alias_papyrus_use_info(uint16_t i, form_stub_use_info_builder& owner) : alias_id(i), pending(owner.spawn_subordinate()) {}
         ~_alias_papyrus_use_info() {
            delete this->pending;
            this->pending = nullptr;
         }
      };
      std::vector<_alias_papyrus_use_info> alias_papyrus_use_info;
      std::vector<uint32_t> seen_aliases;
      //
      form_id_t formID;
      bool      is_in_alias = false;
      while (auto& subrecord = record.next_subrecord()) {
         if (is_in_alias) {
            switch (subrecord.signature()) {
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
               case 'COCT':
               case 'CNTO':
               case 'COED':
                  components::container_data::generate_use_info(subrecord, uib);
                  break;
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
               case 'QNAM': // alias hidden flag
                  break;
               case 'ALED': // alias end marker
                  is_in_alias = false;
                  break;
            }
            continue;
         }
         //
         switch (subrecord.signature()) {
            case 'VMAD':
               {
                  auto header = components::papyrus_attachment_data::generate_use_info(subrecord, uib);
                  if (!subrecord.is_at_end()) {
                     uint16_t count;
                     subrecord.skip_bytes(sizeof(script_fragment_root.unknown));
                     if (!subrecord.read(count)) // log entry fragment count
                        break;
                     subrecord.skip_length_prefixed_string<2>();
                     for (uint16_t i = 0; i < count; ++i)
                        LogEntry::script_fragment::generate_use_info(subrecord, uib);
                     if (!subrecord.read(count)) // alias script data count
                        break;
                     for (uint16_t i = 0; i < count; ++i) {
                        components::papyrus::script_data::property_object_value owner;
                        owner.load(header, subrecord);
                        if (owner.form != uib.stub()) {
                           components::papyrus::script_data::skip_use_info(subrecord);
                           continue;
                        }
                        auto& entry = alias_papyrus_use_info.emplace_back(owner.aliasID, uib);
                        components::papyrus::script_data::generate_use_info(subrecord, *entry.pending);
                     }
                  }
               }
               break;
            case 'QTGL': // text global (there can be multiple)
            case 'NAM0': // log entry next quest
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
            case 'CTDA':
               components::condition::generate_use_info(record, uib);
               break;
            case 'ALLS': // location alias start
               [[fallthrough]];
            case 'ALST': // reference alias start
               is_in_alias = true;
               {
                  uint32_t id;
                  if (subrecord.read(id))
                     seen_aliases.push_back(id);
               }
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
            case 'SCHR': // DEPRECATED: ObScript header
            case 'SCDA': // DEPRECATED: ObScript compiled code
            case 'SCTX': // DEPRECATED: ObScript source code
            case 'SCRO': // DEPRECATED: ObScript ObjectReference
            case 'SCRV': // DEPRECATED: ObScript ObjectReference variable
            case 'SLSD': // ObScript data? game doesn't load this
            case 'QNAM': // ObScript data? game doesn't load this (when it's outside of an alias)
            case 'DNAM': // quest form version?
               break;
            #endif
         }
      }
      for (auto& entry : alias_papyrus_use_info) {
         for (auto id : seen_aliases) {
            if (id == entry.alias_id) {
               entry.pending->commit();
               break;
            }
         }
      }
   }
   bool Quest::_clone_impl(Form* out) const noexcept {
      if (out->formType != form_type)
         return false;
      auto copy = (Quest*)out;
      //
      copy->script_data.clone_from(this->script_data, *copy);
      copy->name         = this->name;
      copy->flags        = this->flags;
      copy->priority     = this->priority;
      copy->form_version = this->form_version;
      copy->unknown      = this->unknown;
      copy->quest_type   = this->quest_type;
      //
      copy->event = this->event;
      //
      {
         size_t size = this->text_display_globals.size();
         copy->text_display_globals.resize(size);
         for (size_t i = 0; i < size; ++i)
            copy->text_display_globals[i].set(*copy, this->text_display_globals[i]);
      }
      //
      copy->editor_category = this->editor_category;
      //
      copy->conditions.dialogue.append_all_of(*copy, this->conditions.dialogue);
      copy->conditions.event.append_all_of(*copy, this->conditions.event);
      {
         size_t size = this->stages.size();
         copy->stages.resize(size);
         for (size_t i = 0; i < size; ++i)
            copy->stages[i].clone_from(this->stages[i], *copy);
      }
      {
         size_t size = this->objectives.size();
         copy->objectives.resize(size);
         for (size_t i = 0; i < size; ++i)
            copy->objectives[i].clone_from(this->objectives[i], *copy);
      }
      copy->next_alias_id = this->next_alias_id;
      {
         size_t size = this->aliases.size();
         copy->aliases.resize(size);
         for (size_t i = 0; i < size; ++i)
            copy->aliases[i] = this->aliases[i]->clone(*copy);
      }
      //
      return true;
   }
   bool Quest::_save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) {
      {
         //
         // Don't save unowned data, as we won't have corrected it e.g. to make sure that it doesn't refer 
         // to log entries or aliases created after load.
         //
         constexpr bool save_unowned_data = false;
         //
         size_t alias_count = 0;
         size_t log_count   = this->script_fragment_root.unowned_log_entry_data.size();
         if (!save_unowned_data && log_count) {
            static_assert(false, "Strongly consider throwing a save error if any unowned data exists, so we don't have to worry about \"owners\" being created under it (e.g. an unowned alias-VMAD loaded with an invalid alias ID, but we later create an alias with that ID) and things getting mangled at save time.");
            //
            // TODO: log (notice_code::quest_has_phantom_script_data) as a save error
            //
            return false;
         }
         for (auto& s : this->stages)
            for (auto& e : s.entries)
               if (!e.fragment.empty())
                  ++log_count;
         if (log_count > std::numeric_limits<uint16_t>::max()) {
            static_assert(false, "Log specific error");
            // TODO: log (notice_code::too_many_script_fragments_to_save)
            return false;
         }
         for (auto* a : this->aliases)
            if (!a->script_data.empty())
               ++alias_count;
         if (alias_count > std::numeric_limits<uint16_t>::max()) {
            static_assert(false, "Log specific error");
            // TODO: log (notice_code::too_many_aliases_with_scripts_to_save)
            return false;
         }
         if (alias_count || log_count || !this->script_data.empty()) {
            auto& VMAD = record.open_next_subrecord('VMAD');
            this->script_data.save(VMAD, intfc);
            //
            VMAD.write(this->script_fragment_root.unknown);
            VMAD.write(uint16_t(log_count));
            VMAD.write_length_prefixed_string<2>(this->script_fragment_root.filename);
            if (save_unowned_data) {
               for (auto& data : this->script_fragment_root.unowned_log_entry_data)
                  data.save(VMAD, data.stage_id, data.entry_index);
            }
            for (auto& s : this->stages) {
               auto& list = s.entries;
               auto  size = list.size();
               for (size_t i = 0; i < size; ++i) {
                  auto& e = list[i];
                  if (e.fragment.empty())
                     continue;
                  e.fragment.save(VMAD, s.index, i);
               }
            }
            VMAD.write(uint16_t(alias_count));
            for (auto* a : this->aliases) {
               if (a->script_data.empty())
                  continue;
               components::papyrus::script_data::property_object_value owner;
               owner.form.unmanaged_set(&this->stub);
               owner.aliasID = a->id;
               owner.save(this->script_data.header, VMAD);
               //
               a->script_data.save(VMAD, intfc);
            }
         }
      }
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
         ENAM.write_signature(this->event);
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
   void Quest::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->conditions.dialogue.clear(*this);
      this->conditions.event.clear(*this);
      for (auto& obj : this->stages)
         obj.clear(*this);
      this->stages.clear();
      for (auto& obj : this->objectives)
         obj.clear(*this);
      this->objectives.clear();
      for (auto* obj : this->aliases)
         obj->clear(*this);
      this->aliases.clear();
      clear_form_reference_list(this->text_display_globals, *this);
   }

   Alias* Quest::lookup_alias_by_id(uint32_t id) const noexcept {
      for (auto* alias : this->aliases)
         if (alias && alias->id == id)
            return alias;
      return nullptr;
   }
   void Quest::for_each_alias_of_type(Alias::alias_type t, std::function<bool(Alias*)> functor) {
      for (auto* alias : this->aliases) {
         if (t != Alias::alias_type::undifferentiated && alias->type != t)
            continue;
         if ((functor)(alias))
            break;
      }
   }

   Quest::Stage* Quest::lookup_stage_by_id(uint16_t id) noexcept {
      auto& list = this->stages;
      if (!list.empty())
         for (auto& stage : list)
            if (stage.index == id)
               return const_cast<Stage*>(&stage);
      return nullptr;
   }
   Quest::Stage* Quest::insert_stage(int id) noexcept {
      auto& list = this->stages;
      if (!list.empty())
         for (auto& stage : list)
            if (stage.index == id)
               return nullptr;
      auto& stage = list.emplace_back();
      stage.index = id;
      return &stage;
   }
   void Quest::remove_stage(int id) noexcept {
      auto& list = this->stages;
      auto  it   = list.begin();
      auto  end  = list.end();
      for (; it != end; ++it)
         if (it->index == id)
            break;
      if (it == end)
         return;
      list.erase(it);
   }

   void Quest::discard_invalid_script_data() {
      bool any_changes = false;
      //
      if (!this->script_fragment_root.unowned_log_entry_data.empty()) {
         any_changes = true;
         for (auto& data : this->script_fragment_root.unowned_log_entry_data)
            data.clear();
      }
      //
      if (any_changes && !this->is_working_copy)
         this->stub.set_edited(true);
   }
}