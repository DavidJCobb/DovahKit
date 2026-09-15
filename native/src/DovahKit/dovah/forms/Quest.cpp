#include "Quest.h"
#include "_common_cpp.h"
#include "../form_stub_addenda.h"
#include "./components/legacy_script.h"

#include "../notices/form_load_warnings/by_form_type/quest/alias_papyrus_data_belongs_to_missing_alias.h"
#include "../notices/form_load_warnings/by_form_type/quest/alias_papyrus_data_specifies_wrong_quest.h"
#include "../notices/form_load_warnings/by_form_type/quest/papyrus_fragment_belongs_to_missing_log_entry.h"
#include "../notices/form_load_warnings/by_form_type/quest/unexpected_subrecord_in_objective.h"
#include "../notices/form_save_errors/by_form_type/quest/too_many_log_entry_papyrus_fragments.h"
#include "../notices/form_save_errors/by_form_type/quest/too_many_scripted_aliases.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::quest;
   }
   namespace specific_save_errors {
      using namespace dovah::notices::form_save_errors::by_type::quest;
   }
}

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
      assert(required && "Alias::load doesn't know what subrecord to check for. Did someone try to implement a new alias type without updating _signature_for_alias_type?");
      assert(subrecord.signature() == required && "Alias::load was asked to handle the wrong subrecord. How did this happen?");

      subrecord.read(this->id);
      if (!record.next_subrecord())
         return;

      for (; subrecord.exists() && subrecord.signature() != 'ALED'; record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'ALID':
               subrecord.read(this->name);
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

            case 'CTDA':
               this->conditions.read_next(subrecord.get_containing_record(), intfc);
               break;

            default:
               if (!this->_load_impl(subrecord, intfc)) {
                  //
                  // TODO: log unrecognized subrecord warning
                  //
               }
               break;
         }
      }
   }
   bool LocationAlias::_load_impl(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      switch (subrecord.signature()) {
         #pragma region Fill params
            #pragma region Specific Location
               case 'ALFL':
                  {
                     auto& data = this->fill_params.emplace<structs::alias_fill_params::loc::preassigned>();
                     subrecord.read(data.location);
                     intfc.warn_if_ref_is_wrong_type(data.location, form_type::location, subrecord.signature());
                  }
                  break;
            #pragma endregion
            #pragma region Reference Alias Location
               case 'ALFA':
                  {
                     auto& data = this->fill_params.emplace<structs::alias_fill_params::loc::at_reference_alias>();
                     subrecord.read(data.alias);
                  }
                  break;
               case 'KNAM':
                  if (auto* data = std::get_if<structs::alias_fill_params::loc::at_reference_alias>(&this->fill_params)) {
                     subrecord.read(data->keyword);
                     intfc.warn_if_ref_is_wrong_type(data->keyword, form_type::keyword, subrecord.signature());
                  }
                  break;
            #pragma endregion
            #pragma region External Alias Reference
               case 'ALEQ':
                  {
                     auto& data = this->fill_params.emplace<structs::alias_fill_params::copy_external_alias>();
                     subrecord.read(data.quest);
                     intfc.warn_if_ref_is_wrong_type(data.quest, form_type::quest, subrecord.signature());
                  }
                  break;
               case 'ALEA':
                  if (auto* data = std::get_if<structs::alias_fill_params::copy_external_alias>(&this->fill_params)) {
                     subrecord.read(data->alias);
                  }
                  break;
            #pragma endregion
            #pragma region Find Matching Location: From Event
               case 'ALFE':
                  {
                     auto& data = this->fill_params.emplace<structs::alias_fill_params::loc::find>();
                     auto& ev   = data.from_event.emplace();
                     subrecord.read_signature(ev.code);
                  }
                  break;
               case 'ALFD':
                  if (auto* data = std::get_if<structs::alias_fill_params::loc::find>(&this->fill_params)) {
                     auto& ev = data->from_event;
                     if (ev.has_value()) {
                        subrecord.read_signature((*ev).member);
                        ev->member = (ev->member << 16) | (ev->member >> 16);
                     }
                  }
                  break;
            #pragma endregion
         #pragma endregion
      }
      return false;
   }
   bool ReferenceAlias::_load_impl(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      form_reference_t formID;
      bool preassigned_present = false;
      switch (subrecord.signature()) {
         #pragma region Fill params
            #pragma region Specific Reference
               case 'ALFR':
                  {
                     preassigned_present = true;
                     auto& data = this->fill_params.emplace<structs::alias_fill_params::ref::preassigned>();
                     subrecord.read(data.ref);
                     //intfc.warn_if_ref_is_wrong_type(data.actor_base, form_type::actor_base, subrecord.signature());
                  }
                  break;
            #pragma endregion
            #pragma region Unique Actor
               case 'ALUA':
                  {
                     auto& data = this->fill_params.emplace<structs::alias_fill_params::ref::unique_actor>();
                     subrecord.read(data.actor_base);
                     intfc.warn_if_ref_is_wrong_type(data.actor_base, form_type::actor_base, subrecord.signature());
                  }
                  break;
            #pragma endregion
            #pragma region Location Alias Reference
               case 'ALFA':
                  {
                     auto& data = this->fill_params.emplace<structs::alias_fill_params::ref::at_location_alias>();
                     subrecord.read(data.alias);
                  }
                  break;
               case 'ALRT':
                  if (auto* data = std::get_if<structs::alias_fill_params::ref::at_location_alias>(&this->fill_params)) {
                     subrecord.read(data->loc_ref_type);
                     intfc.warn_if_ref_is_wrong_type(data->loc_ref_type, form_type::location_ref_type, subrecord.signature());
                  }
                  break;
            #pragma endregion
            #pragma region External Alias Reference
               case 'ALEQ':
                  {
                     auto& data = this->fill_params.emplace<structs::alias_fill_params::copy_external_alias>();
                     subrecord.read(data.quest);
                     intfc.warn_if_ref_is_wrong_type(data.quest, form_type::quest, subrecord.signature());
                  }
                  break;
               case 'ALEA':
                  if (auto* data = std::get_if<structs::alias_fill_params::copy_external_alias>(&this->fill_params)) {
                     subrecord.read(data->alias);
                  }
                  break;
            #pragma endregion
            #pragma region Create Reference to Object
               case 'ALCO':
                  {
                     auto& data = this->fill_params.emplace<structs::alias_fill_params::ref::create>();
                     subrecord.read(data.base_form);
                     //intfc.warn_if_ref_is_wrong_type(data.base_form, form_type::actor_base, subrecord.signature());
                  }
                  break;
               case 'ALCA':
                  if (auto* data = std::get_if<structs::alias_fill_params::ref::create>(&this->fill_params)) {
                     alias_id_t coalesced;
                     if (subrecord.read(coalesced) && coalesced != -1) {
                        data->at_reference.alias = coalesced & 0x7FFFFFFF;
                        data->at_reference.place_in_inventory = (coalesced >> 31) != 0;
                     } else {
                        data->at_reference.alias = -1;
                        data->at_reference.place_in_inventory = false;
                     }
                  }
                  break;
               case 'ALCL':
                  if (auto* data = std::get_if<structs::alias_fill_params::ref::create>(&this->fill_params)) {
                     subrecord.read(data->difficulty);
                  }
                  break;
            #pragma endregion
            #pragma region Find Matching Reference: Near Alias
               case 'ALNA':
                  {
                     auto& data = this->fill_params.emplace<structs::alias_fill_params::ref::find_near_alias>();
                     subrecord.read(data.alias);
                  }
                  break;
               case 'ALNT':
                  if (auto* data = std::get_if<structs::alias_fill_params::ref::find_near_alias>(&this->fill_params)) {
                     subrecord.read(data->near_type);
                  }
                  break;
            #pragma endregion
            #pragma region Find Matching Reference: From Event
               case 'ALFE':
                  {
                     auto& data = this->fill_params.emplace<structs::alias_fill_params::ref::find_from_event>();
                     subrecord.read_signature(data.code);
                  }
                  break;
               case 'ALFD':
                  if (auto* data = std::get_if<structs::alias_fill_params::ref::find_from_event>(&this->fill_params)) {
                     subrecord.read_signature(data->member);
                     data->member = (data->member << 16) | (data->member >> 16);
                  }
                  break;
            #pragma endregion
         #pragma endregion

         case 'QNAM':
            this->hidden_flags |= 4;
            break;
         case 'ALPC':
            if (subrecord.read(formID)) {
               this->packages.push_back(formID);
               intfc.warn_if_ref_is_wrong_type(formID, form_type::package, subrecord.signature());
            }
            break;
         case 'ALFC':
            if (subrecord.read(formID)) {
               this->factions.push_back(formID);
               intfc.warn_if_ref_is_wrong_type(formID, form_type::faction, subrecord.signature());
            }
            break;
         case 'ALSP':
            if (subrecord.read(formID)) {
               this->spells.push_back(formID);
               intfc.warn_if_ref_is_wrong_type(formID, form_type::spell, subrecord.signature());
            }
            break;
         case 'VTCK':
            if (subrecord.read(this->additional_voicetype)) {
               intfc.warn_if_ref_is_wrong_type(this->additional_voicetype, std::array{ form_type::actor_base, form_type::formlist }, subrecord.signature());
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
            if (subrecord.read(this->package_override_lists.spectator)) {
               intfc.warn_if_ref_is_wrong_type(this->package_override_lists.spectator, form_type::formlist, subrecord.signature());
            }
            break;
         case 'OCOR':
            if (subrecord.read(this->package_override_lists.observe_corpse)) {
               intfc.warn_if_ref_is_wrong_type(this->package_override_lists.observe_corpse, form_type::formlist, subrecord.signature());
            }
            break;
         case 'GWOR':
            if (subrecord.read(this->package_override_lists.guard_warn)) {
               intfc.warn_if_ref_is_wrong_type(this->package_override_lists.guard_warn, form_type::formlist, subrecord.signature());
            }
            break;
         case 'ECOR':
            if (subrecord.read(this->package_override_lists.combat)) {
               intfc.warn_if_ref_is_wrong_type(this->package_override_lists.combat, form_type::formlist, subrecord.signature());
            }
            break;
         case 'ALDN':
            subrecord.read(this->display_name);
            break;
         default:
            return false;
      }

      if (!preassigned_present && std::holds_alternative<structs::alias_fill_params::ref::preassigned>(this->fill_params)) {
         //
         // If no fill-type subrecords are loaded, then the alias defaults to Conditions if it has at 
         // least 1 condition, or to Preassigned (ref: None) otherwise.
         //
         if (!this->conditions.empty()) {
            if (this->flags & flag::limit_to_loaded_area) {
               auto& casted = this->fill_params.emplace<structs::alias_fill_params::ref::find_in_loaded_area>();
            } else {
               this->fill_params.emplace<structs::alias_fill_params::ref::find_anywhere>();
            }
         }
      }
      
      if (auto* casted = std::get_if<structs::alias_fill_params::ref::create>(&this->fill_params)) {
         casted->initially_disabled = this->flags & flag::initially_disabled;
      } else if (auto* casted = std::get_if<structs::alias_fill_params::ref::find_in_loaded_area>(&this->fill_params)) {
         casted->closest = this->flags & flag::use_closest;
      }

      return true;
   }

   /*static*/ alias_id_t Alias::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      auto& subrecord = record.get_current_subrecord();
      //
      alias_type type = alias_type::undifferentiated;
      switch (subrecord.signature()) {
         case 'ALLS':
            type = alias_type::location;
            break;
         case 'ALST':
            type = alias_type::reference;
            break;
      }
      if (type == alias_type::undifferentiated)
         return none_id;
      LocationAlias::_use_info_field_state  state_location;
      ReferenceAlias::_use_info_field_state state_reference;
      //
      alias_id_t id;
      if (!subrecord.read(id))
         return none_id;
      //
      form_id_t fill_from_alias_quest_id;
      for (; subrecord.exists() && subrecord.signature() != 'ALED'; record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'ALEQ': // quest which contains ALEA
               subrecord.read(fill_from_alias_quest_id);
               break;
            case 'CTDA':
               components::condition::generate_use_info(record, uib);
               break;
            case 'ALID': // alias name
            case 'FNAM': // flags
            case 'BNAM': // hidden flag
            case 'ONAM': // hidden flag
            case 'ALFI': // force-into-alias ID
            case 'ALEA': // fill from external alias
            case 'ALFA': // fill from internal alias
            case 'ALFE': // fill from event code
            case 'ALFD': // fill from event member
               break;
            default:
               if (type == alias_type::reference)
                  ReferenceAlias::generate_use_info_for_subrecord(state_reference, subrecord, uib);
               else if (type == alias_type::location)
                  LocationAlias::generate_use_info_for_subrecord(state_location, subrecord, uib);
               break;
         }
      }
      uib.add_outbound_reference(fill_from_alias_quest_id);
      if (type == alias_type::reference) {
         switch (state_reference.fill_params.which) {
            case 0:
               uib.add_outbound_reference(state_reference.fill_params.preassigned);
               break;
            case 1:
               uib.add_outbound_reference(state_reference.fill_params.unique_actor);
               break;
            case 2:
               uib.add_outbound_reference(state_reference.fill_params.loc_ref_type);
               break;
            case 3:
               uib.add_outbound_reference(state_reference.fill_params.external_quest);
               break;
            case 4:
               uib.add_outbound_reference(state_reference.fill_params.create_base_form);
               break;
         }
         uib.add_outbound_reference(state_reference.package_override_lists.spectator);
         uib.add_outbound_reference(state_reference.package_override_lists.observe_corpse);
         uib.add_outbound_reference(state_reference.package_override_lists.guard_warn);
         uib.add_outbound_reference(state_reference.package_override_lists.combat);
         uib.add_outbound_reference(state_reference.display_name);
         uib.add_outbound_reference(state_reference.additional_voicetype);
         uib.add_outbound_reference(state_reference.create_object_of_type);
      } else if (type == alias_type::location) {
         switch (state_location.fill_params.which) {
            case 0:
               uib.add_outbound_reference(state_location.fill_params.preassigned);
               break;
            case 1:
               uib.add_outbound_reference(state_location.fill_params.keyword);
               break;
            case 2:
               uib.add_outbound_reference(state_location.fill_params.external_quest);
               break;
         }
      }
      //
      return id;
   }
   /*static*/ void LocationAlias::generate_use_info_for_subrecord(_use_info_field_state& state, tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      switch (subrecord.signature()) {
         #pragma region Fill params
            #pragma region Specific Location
               case 'ALFL':
                  state.fill_params.which = 0;
                  subrecord.read(state.fill_params.preassigned);
                  break;
            #pragma endregion
            #pragma region Reference Alias Location
               case 'ALFA':
                  state.fill_params.which = 1;
                  break;
               case 'KNAM':
                  subrecord.read(state.fill_params.keyword);
                  break;
            #pragma endregion
            #pragma region External Alias Reference
               case 'ALEQ':
                  state.fill_params.which = 2;
                  subrecord.read(state.fill_params.external_quest);
                  break;
               case 'ALEA':
                  break;
            #pragma endregion
            #pragma region Find Matching Location: From Event
               case 'ALFE':
                  state.fill_params.which = 3;
                  break;
               case 'ALFD':
                  break;
            #pragma endregion
         #pragma endregion
      }
   }
   /*static*/ void ReferenceAlias::generate_use_info_for_subrecord(_use_info_field_state& state, tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      form_id_t formID;
      switch (subrecord.signature()) {
         #pragma region Fill params
            #pragma region Specific Reference
               case 'ALFR':
                  state.fill_params.which = 0;
                  subrecord.read(state.fill_params.preassigned);
                  break;
            #pragma endregion
            #pragma region Unique Actor
               case 'ALUA':
                  state.fill_params.which = 1;
                  subrecord.read(state.fill_params.unique_actor);
                  break;
            #pragma endregion
            #pragma region Location Alias Reference
               case 'ALFA':
                  state.fill_params.which = 2;
                  break;
               case 'ALRT':
                  subrecord.read(state.fill_params.loc_ref_type);
                  break;
            #pragma endregion
            #pragma region External Alias Reference
               case 'ALEQ':
                  state.fill_params.which = 3;
                  subrecord.read(state.fill_params.external_quest);
                  break;
               case 'ALEA':
                  break;
            #pragma endregion
            #pragma region Create Reference to Object
               case 'ALCO':
                  state.fill_params.which = 4;
                  subrecord.read(state.fill_params.create_base_form);
                  break;
               case 'ALCA':
                  break;
               case 'ALCL':
                  break;
            #pragma endregion
            #pragma region Find Matching Reference: Near Alias
               case 'ALNA':
                  state.fill_params.which = 5;
                  break;
               case 'ALNT':
            #pragma endregion
            #pragma region Find Matching Reference: From Event
               case 'ALFE':
                  state.fill_params.which = 6;
                  break;
               case 'ALFD':
                  break;
            #pragma endregion
         #pragma endregion

         case 'ALPC':
         case 'ALFC':
         case 'ALSP':
            if (subrecord.read(formID))
               uib.add_outbound_reference(formID);
            break;
         case 'VTCK':
            subrecord.read(state.additional_voicetype);
            break;
         case 'KSIZ':
         case 'KWDA':
            components::keyword_list::generate_use_info(subrecord, uib);
            break;
         case 'COCT':
         case 'CNTO':
         case 'COED':
            components::container_data::generate_use_info(subrecord, uib);
            break;
         case 'SCOR':
            subrecord.read(state.package_override_lists.spectator);
            break;
         case 'OCOR':
            subrecord.read(state.package_override_lists.observe_corpse);
            break;
         case 'GWOR':
            subrecord.read(state.package_override_lists.guard_warn);
            break;
         case 'ECOR':
            subrecord.read(state.package_override_lists.combat);
            break;
         case 'ALDN':
            subrecord.read(state.display_name);
            break;
      }
   }

   void Alias::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto& start = record.open_next_subrecord(_signature_for_alias_type(this->type));
      start.write(this->id);
      start.close();
      auto& ALID = record.open_next_subrecord('ALID');
      ALID.write(this->name);
      ALID.close();
      {
         uint32_t flags = this->flags;
         this->_adjust_flags_for_save(flags);

         auto& FNAM = record.open_next_subrecord('FNAM');
         FNAM.write(flags);
         FNAM.close();
      }
      if (this->hidden_flags) {
         if (this->hidden_flags & 1)
            record.open_next_subrecord('BNAM').close();
         if (this->hidden_flags & 2)
            record.open_next_subrecord('ONAM').close();
      }
      if (this->force_into_alias_id != -1) {
         auto& ALFI = record.open_next_subrecord('ALFI');
         ALFI.write(this->force_into_alias_id);
         ALFI.close();
      }
      if (!this->_save_fill_impl(record, intfc)) {
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
   void LocationAlias::_adjust_flags_for_save(uint32_t& flags) {
   }
   bool LocationAlias::_save_fill_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      if (const auto* data = std::get_if<structs::alias_fill_params::loc::preassigned>(&this->fill_params)) {
         record.write_formID_subrecord('ALFL', data->location, true);
      } else if (const auto* data = std::get_if<structs::alias_fill_params::loc::at_reference_alias>(&this->fill_params)) {
         {
            auto& ALFA = record.open_next_subrecord('ALFA');
            ALFA.write(data->alias);
            ALFA.close();
         }
         record.write_formID_subrecord('KNAM', data->keyword);
      } else if (const auto* data = std::get_if<structs::alias_fill_params::copy_external_alias>(&this->fill_params)) {
         record.write_formID_subrecord('ALEQ', data->quest);
         {
            auto& ALFA = record.open_next_subrecord('ALEA');
            ALFA.write(data->alias);
            ALFA.close();
         }
      } else if (const auto* data = std::get_if<structs::alias_fill_params::loc::find>(&this->fill_params)) {
         if (data->from_event.has_value()) {
            auto& ev = *data->from_event;
            auto& ALFE = record.open_next_subrecord('ALFE');
            ALFE.write_signature(ev.code);
            ALFE.close();

            uint32_t alfd_data = (ev.member >> 16) | (ev.member << 16);
            auto& ALFD = record.open_next_subrecord('ALFD');
            ALFD.write_signature(alfd_data);
            ALFD.close();
         } else {
            //
            // The CK doesn't write anything in this case.
            //
         }
      } else {
         return false;
      }
      return true;
   }
   void LocationAlias::_save_body_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
   }
   //
   void ReferenceAlias::_adjust_flags_for_save(uint32_t& flags) {
      if (const auto* data = std::get_if<structs::alias_fill_params::ref::find_in_loaded_area>(&this->fill_params)) {
         flags |= flag::limit_to_loaded_area;
         if (data->closest) {
            flags |= flag::use_closest;
         } else {
            flags &= ~flag::use_closest;
         }
      } else {
         flags &= ~flag::limit_to_loaded_area;
         flags &= ~flag::use_closest;
      }
      if (const auto* data = std::get_if<structs::alias_fill_params::ref::create>(&this->fill_params)) {
         if (data->initially_disabled)
            flags |= flag::initially_disabled;
         else
            flags &= ~flag::initially_disabled;
      } else {
         flags &= ~flag::initially_disabled;
      }
   }
   bool ReferenceAlias::_save_fill_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      if (const auto* data = std::get_if<structs::alias_fill_params::ref::preassigned>(&this->fill_params)) {
         record.write_formID_subrecord('ALFR', data->ref);
      } else if (const auto* data = std::get_if<structs::alias_fill_params::ref::unique_actor>(&this->fill_params)) {
         record.write_formID_subrecord('ALUA', data->actor_base);
      } else if (const auto* data = std::get_if<structs::alias_fill_params::ref::at_location_alias>(&this->fill_params)) {
         {
            auto& ALFA = record.open_next_subrecord('ALFA');
            ALFA.write(data->alias);
            ALFA.close();
         }
         record.write_formID_subrecord('ALRT', data->loc_ref_type);
      } else if (const auto* data = std::get_if<structs::alias_fill_params::copy_external_alias>(&this->fill_params)) {
         record.write_formID_subrecord('ALEQ', data->quest);
         {
            auto& ALFA = record.open_next_subrecord('ALEA');
            ALFA.write(data->alias);
            ALFA.close();
         }
      } else if (const auto* data = std::get_if<structs::alias_fill_params::ref::find_anywhere>(&this->fill_params)) {
         //
         // As long as we have at least one condition, and we clear certain flags on save (which 
         // we do), this is the default.
         //
      } else if (const auto* data = std::get_if<structs::alias_fill_params::ref::find_in_loaded_area>(&this->fill_params)) {
         //
         // Handled by adjusting flags on save.
         //
      } else if (const auto* data = std::get_if<structs::alias_fill_params::ref::create>(&this->fill_params)) {
         record.write_formID_subrecord('ALCO', data->base_form);
         {
            uint32_t coalesced = data->at_reference.alias;
            coalesced &= 0x7FFFFFFF;
            if (data->at_reference.place_in_inventory)
               coalesced |= (1 << 31);

            auto& ALCA = record.open_next_subrecord('ALCA');
            ALCA.write(coalesced);
            ALCA.close();
         }
         {
            auto& ALCL = record.open_next_subrecord('ALCL');
            ALCL.write(data->difficulty);
            ALCL.close();
         }
      } else if (const auto* data = std::get_if<structs::alias_fill_params::ref::find_from_event>(&this->fill_params)) {
         auto& ev = *data;
         auto& ALFE = record.open_next_subrecord('ALFE');
         ALFE.write_signature(ev.code);
         ALFE.close();
         auto& ALFD = record.open_next_subrecord('ALFD');
         ALFD.write_signature(ev.member);
         ALFD.close();
      } else if (const auto* data = std::get_if<structs::alias_fill_params::ref::find_near_alias>(&this->fill_params)) {
         auto& ALNA = record.open_next_subrecord('ALNA');
         ALNA.write_signature(data->alias);
         ALNA.close();
         auto& ALNT = record.open_next_subrecord('ALNT');
         ALNT.write_signature(data->near_type);
         ALNT.close();
      } else {
         return false;
      }
      return true;
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
         record.write_formID_subrecord('ALSP', id, true);
      for (auto& id : this->factions)
         record.write_formID_subrecord('ALFC', id, true);
      for (auto& id : this->packages)
         record.write_formID_subrecord('ALPC', id, true);
      record.write_formID_subrecord('VTCK', this->additional_voicetype, false);
      //
      if (this->hidden_flags & 4)
         record.open_next_subrecord('QNAM').close();
   }

   void Alias::sever_outbound_references(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      for (auto& cnd : this->conditions)
         cnd.sever_outbound_references_to(target, my_owner);
      this->script_data.sever_outbound_references_to(target, my_owner);
      //
      this->_sever_outbound_references_impl(target, my_owner);
   }
   void LocationAlias::_sever_outbound_references_impl(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      if (auto* data = std::get_if<structs::alias_fill_params::loc::preassigned>(&this->fill_params)) {
         data->location.clear_if(my_owner, target);
      } else if (auto* data = std::get_if<structs::alias_fill_params::loc::at_reference_alias>(&this->fill_params)) {
         data->keyword.clear_if(my_owner, target);
      } else if (auto* data = std::get_if<structs::alias_fill_params::copy_external_alias>(&this->fill_params)) {
         data->quest.clear_if(my_owner, target);
         if (!data->quest) {
            data->alias = -1;
         }
      } else if (const auto* data = std::get_if<structs::alias_fill_params::loc::find>(&this->fill_params)) {
         ;
      }
   }
   void ReferenceAlias::_sever_outbound_references_impl(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      //
      // Fill params:
      //
      if (auto* data = std::get_if<structs::alias_fill_params::ref::preassigned>(&this->fill_params)) {
         data->ref.clear_if(my_owner, target);
      } else if (auto* data = std::get_if<structs::alias_fill_params::ref::unique_actor>(&this->fill_params)) {
         data->actor_base.clear_if(my_owner, target);
      } else if (auto* data = std::get_if<structs::alias_fill_params::ref::at_location_alias>(&this->fill_params)) {
         data->loc_ref_type.clear_if(my_owner, target);
         if (!data->loc_ref_type) {
            data->alias = -1;
         }
      } else if (auto* data = std::get_if<structs::alias_fill_params::copy_external_alias>(&this->fill_params)) {
         data->quest.clear_if(my_owner, target);
         if (!data->quest) {
            data->alias = -1;
         }
      } else if (auto* data = std::get_if<structs::alias_fill_params::ref::find_in_loaded_area>(&this->fill_params)) {
         ;
      } else if (auto* data = std::get_if<structs::alias_fill_params::ref::create>(&this->fill_params)) {
         data->base_form.clear_if(my_owner, target);
      } else if (auto* data = std::get_if<structs::alias_fill_params::ref::find_from_event>(&this->fill_params)) {
         ;
      } else if (auto* data = std::get_if<structs::alias_fill_params::ref::find_near_alias>(&this->fill_params)) {
         ;
      }
      //
      // Other data:
      //
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
      //
      copy->conditions.append_all_of(clone_owner, this->conditions);
      copy->script_data.clone_from(this->script_data, clone_owner);
      //
      return copy;
   }
   Alias* LocationAlias::_clone_impl(loaded_forms::Form& clone_owner) {
      auto* copy = new LocationAlias(this->owner);
      //
      // Fill params:
      //
      if (const auto* src = std::get_if<structs::alias_fill_params::loc::preassigned>(&this->fill_params)) {
         auto& src_data = *src;
         auto& dst_data = copy->fill_params.emplace<std::decay_t<decltype(src_data)>>();
         dst_data.location.set(clone_owner, src_data.location);
      } else if (const auto* src = std::get_if<structs::alias_fill_params::loc::at_reference_alias>(&this->fill_params)) {
         auto& src_data = *src;
         auto& dst_data = copy->fill_params.emplace<std::decay_t<decltype(src_data)>>();
         dst_data.alias = src_data.alias;
         dst_data.keyword.set(clone_owner, src_data.keyword);
      } else if (const auto* src = std::get_if<structs::alias_fill_params::copy_external_alias>(&this->fill_params)) {
         auto& src_data = *src;
         auto& dst_data = copy->fill_params.emplace<std::decay_t<decltype(src_data)>>();
         dst_data.quest.set(clone_owner, src_data.quest);
         dst_data.alias = src_data.alias;
      } else if (const auto* src = std::get_if<structs::alias_fill_params::loc::find>(&this->fill_params)) {
         auto& src_data = *src;
         auto& dst_data = copy->fill_params.emplace<std::decay_t<decltype(src_data)>>();
         dst_data.from_event = src_data.from_event;
      }
      //
      // Done.
      //
      return copy;
   }
   Alias* ReferenceAlias::_clone_impl(loaded_forms::Form& clone_owner) {
      auto* copy = new ReferenceAlias(this->owner);
      //
      // Fill params:
      //
      if (const auto* src = std::get_if<structs::alias_fill_params::ref::preassigned>(&this->fill_params)) {
         auto& src_data = *src;
         auto& dst_data = copy->fill_params.emplace<std::decay_t<decltype(src_data)>>();
         dst_data.ref.set(clone_owner, src_data.ref);
      } else if (const auto* src = std::get_if<structs::alias_fill_params::ref::unique_actor>(&this->fill_params)) {
         auto& src_data = *src;
         auto& dst_data = copy->fill_params.emplace<std::decay_t<decltype(src_data)>>();
         dst_data.actor_base.set(clone_owner, src_data.actor_base);
      } else if (const auto* src = std::get_if<structs::alias_fill_params::ref::at_location_alias>(&this->fill_params)) {
         auto& src_data = *src;
         auto& dst_data = copy->fill_params.emplace<std::decay_t<decltype(src_data)>>();
         dst_data.alias = src_data.alias;
         dst_data.loc_ref_type.set(clone_owner, src_data.loc_ref_type);
      } else if (const auto* src = std::get_if<structs::alias_fill_params::copy_external_alias>(&this->fill_params)) {
         auto& src_data = *src;
         auto& dst_data = copy->fill_params.emplace<std::decay_t<decltype(src_data)>>();
         dst_data.quest.set(clone_owner, src_data.quest);
         dst_data.alias = src_data.alias;
      } else if (const auto* src = std::get_if<structs::alias_fill_params::ref::find_in_loaded_area>(&this->fill_params)) {
         auto& src_data = *src;
         auto& dst_data = copy->fill_params.emplace<std::decay_t<decltype(src_data)>>();
         ;
      } else if (const auto* src = std::get_if<structs::alias_fill_params::ref::create>(&this->fill_params)) {
         auto& src_data = *src;
         auto& dst_data = copy->fill_params.emplace<std::decay_t<decltype(src_data)>>();
         dst_data.base_form.set(clone_owner, src_data.base_form);
         dst_data.at_reference = src_data.at_reference;
         dst_data.difficulty   = src_data.difficulty;
      } else if (const auto* src = std::get_if<structs::alias_fill_params::ref::find_from_event>(&this->fill_params)) {
         auto& src_data = *src;
         auto& dst_data = copy->fill_params.emplace<std::decay_t<decltype(src_data)>>();
         dst_data = src_data;
      } else if (const auto* src = std::get_if<structs::alias_fill_params::ref::find_near_alias>(&this->fill_params)) {
         auto& src_data = *src;
         auto& dst_data = copy->fill_params.emplace<std::decay_t<decltype(src_data)>>();
         dst_data.alias     = src_data.alias;
         dst_data.near_type = src_data.near_type;
      }
      //
      // Other data:
      //
      copy->keywords.clone_from(this->keywords, clone_owner);
      copy->inventory.clone_from(this->inventory, clone_owner);
      copy->additional_voicetype.set(clone_owner, this->additional_voicetype);
      copy->display_name.set(clone_owner, this->display_name);
      copy_form_reference_list(clone_owner, copy->packages, this->packages);
      copy_form_reference_list(clone_owner, copy->factions, this->factions);
      copy_form_reference_list(clone_owner, copy->spells, this->spells);
      copy->package_override_lists.combat.set(clone_owner, this->package_override_lists.combat);
      copy->package_override_lists.guard_warn.set(clone_owner, this->package_override_lists.guard_warn);
      copy->package_override_lists.observe_corpse.set(clone_owner, this->package_override_lists.observe_corpse);
      copy->package_override_lists.spectator.set(clone_owner, this->package_override_lists.spectator);
      //
      // Done.
      //
      return copy;
   }

   void Alias::clear(loaded_forms::Form& my_owner) {
      this->id = -1;
      this->name.clear();
      this->flags = 0;
      this->hidden_flags = 0;
      this->force_into_alias_id = -1;
      this->conditions.clear(my_owner);
      this->script_data.clear(my_owner);
      //
      this->_clear_impl(my_owner);
   }
   void Alias::clear_fill_params(loaded_forms::Form& my_owner) {
      this->_clear_fill_params_impl(my_owner);
      this->_adjust_flags_for_save(this->flags);
   }

   void LocationAlias::_clear_impl(loaded_forms::Form& my_owner) {
      this->_clear_fill_params_impl(my_owner);
   }
   void LocationAlias::_clear_fill_params_impl(loaded_forms::Form& my_owner) {
      if (auto* data = std::get_if<structs::alias_fill_params::loc::preassigned>(&this->fill_params)) {
         data->location.set(my_owner, nullptr);
      } else if (auto* data = std::get_if<structs::alias_fill_params::loc::at_reference_alias>(&this->fill_params)) {
         data->keyword.set(my_owner, nullptr);
      } else if (auto* data = std::get_if<structs::alias_fill_params::copy_external_alias>(&this->fill_params)) {
         data->quest.set(my_owner, nullptr);
      } else if (const auto* data = std::get_if<structs::alias_fill_params::loc::find>(&this->fill_params)) {
         ;
      }
      this->fill_params.emplace<structs::alias_fill_params::loc::preassigned>();
   }
   void ReferenceAlias::_clear_impl(loaded_forms::Form& my_owner) {
      //
      // Fill params:
      //
      this->_clear_fill_params_impl(my_owner);
      //
      // Other data:
      //
      this->keywords.clear(my_owner);
      this->inventory.clear(my_owner);
      this->additional_voicetype.set(my_owner, nullptr);
      this->display_name.set(my_owner, nullptr);
      clear_form_reference_list(this->packages, my_owner);
      clear_form_reference_list(this->factions, my_owner);
      clear_form_reference_list(this->spells, my_owner);
      this->package_override_lists.combat.set(my_owner, nullptr);
      this->package_override_lists.guard_warn.set(my_owner, nullptr);
      this->package_override_lists.observe_corpse.set(my_owner, nullptr);
      this->package_override_lists.spectator.set(my_owner, nullptr);
   }
   void ReferenceAlias::_clear_fill_params_impl(loaded_forms::Form& my_owner) {
      if (auto* data = std::get_if<structs::alias_fill_params::ref::preassigned>(&this->fill_params)) {
         data->ref.set(my_owner, nullptr);
      } else if (auto* data = std::get_if<structs::alias_fill_params::ref::unique_actor>(&this->fill_params)) {
         data->actor_base.set(my_owner, nullptr);
      } else if (auto* data = std::get_if<structs::alias_fill_params::ref::at_location_alias>(&this->fill_params)) {
         data->loc_ref_type.set(my_owner, nullptr);
      } else if (auto* data = std::get_if<structs::alias_fill_params::copy_external_alias>(&this->fill_params)) {
         data->quest.set(my_owner, nullptr);
      } else if (auto* data = std::get_if<structs::alias_fill_params::ref::find_in_loaded_area>(&this->fill_params)) {
         ;
      } else if (auto* data = std::get_if<structs::alias_fill_params::ref::create>(&this->fill_params)) {
         data->base_form.set(my_owner, nullptr);
      } else if (auto* data = std::get_if<structs::alias_fill_params::ref::find_from_event>(&this->fill_params)) {
         ;
      } else if (auto* data = std::get_if<structs::alias_fill_params::ref::find_near_alias>(&this->fill_params)) {
         ;
      }
      this->fill_params.emplace<structs::alias_fill_params::ref::preassigned>();
   }
   #pragma endregion

   #pragma region Quest components
      #pragma region Quest log entries
         #pragma region Quest fragments
         void Quest::LogEntry::script_fragment::load(tes_subrecord_reader& subrecord) {
            assert(subrecord.signature() == 'VMAD');
            if (!subrecord.is_in_bounds(
               sizeof(stage_id) +
               sizeof(entry_index) +
               sizeof(unknown08)
            )) {
               return;
            }
            subrecord.unchecked_read(this->stage_id);
            subrecord.unchecked_read(this->entry_index);
            subrecord.unchecked_read(this->unknown08);
            if (!subrecord.read_length_prefixed_string<2>(this->filename))
               return;
            if (!subrecord.read_length_prefixed_string<2>(this->function))
               return;
         }
         void Quest::LogEntry::script_fragment::save(tes_subrecord_writer& subrecord, uint16_t stage_id, uint32_t entry_index) {
            assert(subrecord.signature() == 'VMAD');
            subrecord.write((uint32_t)stage_id);
            subrecord.write(entry_index);
            subrecord.write(this->unknown08);
            subrecord.write_length_prefixed_string<2>(this->filename);
            subrecord.write_length_prefixed_string<2>(this->function);
         }
         void Quest::LogEntry::script_fragment::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
            subrecord.skip_bytes(
               sizeof(stage_id) + 
               sizeof(entry_index) +
               sizeof(unknown08)
            );
            subrecord.skip_length_prefixed_string<2>();
            subrecord.skip_length_prefixed_string<2>();
         }
         void Quest::LogEntry::script_fragment::clear() {
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
            while (true) {
               switch (record.peek_next_subrecord_type()) {
                  case components::legacy_script::subrecord_signature_header:
                  case components::legacy_script::subrecord_signature_compiled_data:
                  case components::legacy_script::subrecord_signature_source_code:
                  case components::legacy_script::subrecord_signature_quest:
                  case components::legacy_script::subrecord_signature_ref_objects:
                  case components::legacy_script::subrecord_signature_ref_variables:
                     record.next_subrecord();
                     continue;
                  default:
                     break;
               }
               break;
            }
            if (record.peek_next_subrecord_type() != 'NAM0')
               return;
            record.next_subrecord();
            subrecord.read(this->next_quest_id);
         }
         void Quest::LogEntry::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load&) {
            assert(subrecord.signature() == 'CNAM' && "Quest::LogEntry::loadText should only be called just after the CNAM subrecord is opened.");
            subrecord.read(this->journal_text);
         }
         void Quest::LogEntry::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
            auto& QSDT = record.open_next_subrecord('QSDT');
            QSDT.write(this->flags);
            QSDT.close();
            for (auto& cnd : this->conditions)
               cnd.save(record, intfc);
            auto& CNAM = record.open_next_subrecord('CNAM');
            CNAM.write(this->journal_text);
            CNAM.close();
            record.write_formID_subrecord('NAM0', this->next_quest_id, true);
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
      void Quest::Stage::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
         auto& INDX = record.open_next_subrecord('INDX');
         INDX.write(this->index);
         INDX.write(this->flags);
         INDX.write(this->padding);
         INDX.close();
         for (auto& entry : this->entries)
            entry.save(record, intfc);
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
      void Quest::Target::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
         auto& QSTA = record.open_next_subrecord('QSTA');
         QSTA.write(this->aliasID);
         QSTA.write(this->flags);
         QSTA.skip_bytes(3);
         QSTA.close();
         for (auto& cnd : this->conditions)
            cnd.save(record, intfc);
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
                  subrecord.read(this->text);
                  return;
               default:
                  {
                     specific_load_warnings::unexpected_subrecord_in_objective notice(
                        intfc.target_stub,
                        subrecord.signature()
                     );
                     intfc.log_load_warning(notice);
                  }
                  break;
            }
         }
      }
      void Quest::Objective::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
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
         components::papyrus::attachment_data data;
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
               subrecord.read(this->name);
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               if (!subrecord.is_at_end()) {
                  uint16_t count;
                  //
                  // Load unknown field that is only present when log entries are present, but that is 
                  // highly consequential to whether alias script data is even loaded:
                  //
                  subrecord.read(this->script_fragment_root.unknown);
                  //
                  // Load log entry fragment data:
                  //
                  if (!subrecord.read(count)) // log entry fragment count
                     break;
                  subrecord.read_length_prefixed_string<2>(this->script_fragment_root.filename);
                  for (uint16_t i = 0; i < count; ++i) {
                     pending_log_entry_scripts.emplace_back().load(subrecord);
                  }
                  if (!subrecord.read(count)) // alias script data count
                     break;
                  for (uint16_t i = 0; i < count; ++i) {
                     components::papyrus::property_object_value owner;
                     owner.load(this->script_data.header, subrecord);
                     if (owner.form != &this->stub) {
                        //
                        // A quest actually can attach scripts to aliases on other quests, by specifying 
                        // those quests instead. I have no idea why the hell Bethesda implemented this, 
                        // and their own tools never make use of it. This is an unfortunate case where 
                        // while the data is technically ill-formed, the game actually has no problem 
                        // loading it -- but we do. We load forms completely independently of one another, 
                        // so we can't properly handle this. At least, not without adding some major 
                        // special-case code for this deep into the loader...
                        //
                        specific_load_warnings::alias_papyrus_data_specifies_wrong_quest notice(
                           this->stub,
                           *owner.form.get_form_stub(),
                           owner.alias_id
                        );
                        intfc.log_load_warning(notice);
                        //
                        components::papyrus::attachment_data::skip_use_info(subrecord);
                        continue;
                     }
                     //
                     // Load alias script data:
                     //
                     bool already_present = false;
                     for (auto& prior : pending_alias_scripts) {
                        if (prior.alias_id == owner.alias_id) {
                           prior.data.load(subrecord, intfc);
                           already_present = true;
                           break;
                        }
                     }
                     if (already_present)
                        break;
                     auto& entry = pending_alias_scripts.emplace_back();
                     entry.alias_id = owner.alias_id;
                     entry.data.load(subrecord, intfc);
                  }
               }
               break;
            case 'DNAM': // required; TODO: fail if this is not present; fail if it is too short
               if (subrecord.is_in_bounds(12)) {
                  subrecord.unchecked_read(this->flags);
                  subrecord.unchecked_read(this->priority);
                  subrecord.skip_bytes(1);
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
                     intfc.warn_if_ref_is_wrong_type(id, form_type::global, subrecord.signature());
                  }
               }
               break;
            case 'FLTR':
               subrecord.read(this->filter);
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
               intfc.warn_on_unrecognized_subrecord(subrecord);
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
               specific_load_warnings::alias_papyrus_data_belongs_to_missing_alias notice(
                  this->stub,
                  entry.alias_id
               );
               intfc.log_load_warning(notice);
               //
               continue;
            }
            assert(alias->script_data.empty() && "We should've prevented duplicate entries from appearing in this list, coalescing them as they were loaded!");
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
                  //
                  // Log entries can only have a single fragment. If multiple fragments are provided, 
                  // the last loaded one "wins." This is basically the same behavior as overriding.
                  //
                  list[data.entry_index].fragment = data;
                  continue;
               }
            }
            specific_load_warnings::papyrus_fragment_belongs_to_missing_log_entry notice(
               this->stub,
               data.stage_id,
               data.entry_index
            );
            intfc.log_load_warning(notice);
         }
         pending_log_entry_scripts.clear();
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
         _alias_papyrus_use_info(_alias_papyrus_use_info&& other) {
            this->alias_id = other.alias_id;
            this->pending  = other.pending;
            other.pending  = nullptr;
         }
         _alias_papyrus_use_info& operator=(_alias_papyrus_use_info&& other) noexcept {
            this->alias_id = other.alias_id;
            this->pending  = other.pending;
            other.pending  = nullptr;
            return *this;
         }
      };
      std::vector<_alias_papyrus_use_info> alias_papyrus_use_info;
      std::vector<uint32_t> seen_aliases;
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
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
                        components::papyrus::property_object_value owner;
                        owner.load(header, subrecord);
                        if (owner.form != uib.stub()) {
                           components::papyrus::attachment_data::skip_use_info(subrecord);
                           continue;
                        }
                        auto& entry = alias_papyrus_use_info.emplace_back(owner.alias_id, uib);
                        components::papyrus::attachment_data::generate_use_info(subrecord, *entry.pending);
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
               Alias::generate_use_info(record, uib);
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
         auto& seen = seen_aliases;
         for (auto it = seen.begin(); it != seen.end(); ++it) {
            auto id = *it;
            if (id == entry.alias_id) {
               entry.pending->commit();
               //
               // Remove the ID from the "seen" list, so that if there are multiple VMAD entries for 
               // the same alias, we use only the first one seen (refer to Quest::load for info):
               //
               seen.erase(it);
               //
               break;
            }
         }
      }
   }
   void Quest::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Quest*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->name       = this->name;
      copy->flags      = this->flags;
      copy->priority   = this->priority;
      copy->unknown    = this->unknown;
      copy->quest_type = this->quest_type;
      copy->filter     = this->filter;
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
   }
   void Quest::_save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) {
      {
         size_t alias_count = 0;
         size_t log_count   = 0;
         for (auto& s : this->stages)
            for (auto& e : s.entries)
               if (!e.fragment.empty())
                  ++log_count;
         if (log_count > std::numeric_limits<uint16_t>::max()) {
            auto notice = specific_save_errors::too_many_log_entry_papyrus_fragments(
               *intfc.target_stub,
               log_count
            );
            intfc.throw_save_error(notice);
         }
         for (auto* a : this->aliases)
            if (!a->script_data.empty())
               ++alias_count;
         if (alias_count > std::numeric_limits<uint16_t>::max()) {
            auto notice = specific_save_errors::too_many_scripted_aliases(
               *intfc.target_stub,
               alias_count
            );
            intfc.throw_save_error(notice);
         }
         if (alias_count || log_count || !this->script_data.empty()) {
            auto& VMAD = record.open_next_subrecord('VMAD');
            this->script_data.save(VMAD, intfc);
            if (alias_count || log_count) {
               VMAD.write(this->script_fragment_root.unknown);
               VMAD.write(uint16_t(log_count));
               VMAD.write_length_prefixed_string<2>(this->script_fragment_root.filename);
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
                  components::papyrus::property_object_value owner;
                  owner.form.unmanaged_set(&this->stub);
                  owner.alias_id = a->id;
                  owner.save(this->script_data.header, VMAD);
                  //
                  a->script_data.save(VMAD, intfc);
               }
            }
            VMAD.close();
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
      DNAM.skip_bytes(1);
      DNAM.write(this->unknown);
      DNAM.write((uint32_t)this->quest_type);
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
      if (!this->filter.empty())
         record.write_string_subrecord('FLTR', this->filter);
      for (auto& cnd : this->conditions.dialogue) {
         cnd.save(record, intfc);
      }
      record.open_next_subrecord('NEXT').close();
      for (auto& cnd : this->conditions.event) {
         cnd.save(record, intfc);
      }
      for (auto& obj : this->stages)
         obj.save(record, intfc);
      for (auto& obj : this->objectives)
         obj.save(record, intfc);
      auto& ANAM = record.open_next_subrecord('ANAM');
      ANAM.write(this->next_alias_id);
      ANAM.close();
      for (auto* alias : this->aliases)
         alias->save(record, intfc);
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
      if (id == Alias::none_id)
         return nullptr;
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
      if (id < 0 || id > std::numeric_limits<decltype(Stage::index)>::max())
         return nullptr;
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
      it->clear(*this);
      list.erase(it);
   }
   void Quest::remove_log_entry(int stage_id, int log_entry) noexcept {
      if (log_entry < 0)
         return;
      auto* stage = this->lookup_stage_by_id(stage_id);
      if (!stage)
         return;
      auto& list = stage->entries;
      if (log_entry >= list.size())
         return;
      list[log_entry].clear(*this);
      list.erase(list.begin() + log_entry);
   }
   void Quest::remove_objective(int id) noexcept {
      auto& list = this->objectives;
      auto  it   = list.begin();
      auto  end  = list.end();
      for (; it != end; ++it)
         if (it->index == id)
            break;
      if (it == end)
         return;
      it->clear(*this);
      list.erase(it);
   }
}