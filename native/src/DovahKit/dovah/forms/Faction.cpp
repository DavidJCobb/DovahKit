#include "Faction.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Faction::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      #pragma region TESFaction members that get reset with each override
      //
      // Data known to be cleared upon encountering an override (not including 
      // run-time-exclusive data):
      //
      //  - TESForm::ClearAllComponentData
      //     - TESFaction::TESFullName
      //     - TESFaction::TESReactionForm
      //  - TESFaction::ClearData
      //     - Faction::ranks                   | TESFaction::ranks
      //     - Faction::package_location_vendor | TESFaction::VendorData::packageLocation
      //     - Faction::vendor_conditions       | TESFaction::VendorData::unk10
      //  - TESFaction::InitializeData
      //     - Faction::prison_marker           // All by way of a memset call.
      //     - Faction::follower_wait_marker    // 
      //     - Faction::evidence_chest          // 
      //     - Faction::player_belongings_chest // 
      //     - Faction::crime_group             // 
      //     - Faction::jail_outfit             // 
      //     - Faction::faction_flags
      //     - Faction::crime_values
      //     - Faction::vendor_list
      //     - Faction::vendor_chest
      //     - Faction::vendor_data
      //     - Faction::package_location_vendor // Pointer abandoned but not freed. Would have already been freed by ClearData.
      //
      // In practice, it looks like absolutely all of a Faction's data is wiped with 
      // each override, such that only the winning record's content is ever loaded.
      //
      if (!intfc.is_winning_record)
         return;
      #pragma endregion
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'FULL':
               subrecord.to_string(this->name);
               break;
            case 'XNAM':
               {
                  auto& entry = this->relationships.emplace_back();
                  subrecord.read(entry.other);
                  subrecord.read(entry.mod);
                  subrecord.read(entry.combat);
                  intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::faction, this->stub, entry.other)
                  );
               }
               break;
            case 'DATA':
               subrecord.read(this->faction_flags);
               break;
            case 'JAIL':
               subrecord.read(this->prison_marker);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::reference, this->stub, this->prison_marker)
               );
               break;
            case 'WAIT':
               subrecord.read(this->follower_wait_marker);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::reference, this->stub, this->follower_wait_marker)
               );
               break;
            case 'STOL':
               subrecord.read(this->evidence_chest);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::reference, this->stub, this->evidence_chest)
               );
               break;
            case 'PLCN':
               subrecord.read(this->player_belongings_chest);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::reference, this->stub, this->player_belongings_chest)
               );
               break;
            case 'CRGR':
               subrecord.read(this->crime_group);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::formlist, this->stub, this->crime_group)
               );
               break;
            case 'JOUT':
               subrecord.read(this->jail_outfit);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::outfit, this->stub, this->jail_outfit)
               );
               break;
            case 'CRVA':
               subrecord.read(this->crime_values.arrest);
               subrecord.read(this->crime_values.attack_on_sight);
               subrecord.read(this->crime_values.murder);
               subrecord.read(this->crime_values.assault);
               subrecord.read(this->crime_values.trespass);
               subrecord.read(this->crime_values.pickpocket);
               subrecord.read(this->crime_values.unused);
               subrecord.read(this->crime_values.theft_multiplier);
               subrecord.read(this->crime_values.jail_escape);
               subrecord.read(this->crime_values.werewolf_transformation);
               break;
            case 'RNAM':
               {
                  auto& entry = this->ranks.emplace_back();
                  subrecord.read(entry.id);
               }
               break;
            case 'FNAM':
               if (!this->ranks.empty())
                  subrecord.to_string(this->ranks.back().title_fem);
               break;
            case 'MNAM':
               if (!this->ranks.empty())
                  subrecord.to_string(this->ranks.back().title_masc);
               break;
            case 'VEND':
               subrecord.read(this->vendor_list);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::formlist, this->stub, this->vendor_list)
               );
               break;
            case 'VENC':
               subrecord.read(this->vendor_chest);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::reference, this->stub, this->vendor_chest)
               );
               break;
            case 'VENV':
               subrecord.read(this->vendor_data.start_hour);
               subrecord.read(this->vendor_data.end_hour);
               subrecord.read(this->vendor_data.radius);
               subrecord.read(this->vendor_data.unused06);
               subrecord.read(this->vendor_data.buys_stolen);
               subrecord.read(this->vendor_data.vendor_list_is_blacklist);
               subrecord.read(this->vendor_data.unused0A);
               break;
            case 'PLVD':
               this->package_location_vendor.load(subrecord, intfc);
               break;
            case 'CITC':
               {
                  uint32_t count;
                  if (subrecord.read(count))
                     this->vendor_conditions.reserve(count);
               }
               break;
            case 'CTDA':
               components::condition::append_to_condition_list(this->stub, this->vendor_conditions, subrecord.get_containing_record(), intfc);
               break;
            case 'OBND':
               this->has_object_bounds = true;
               this->object_bounds.load(subrecord, intfc);
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            default:
               intfc.log_load_warning(
                  detailed_notice::warn_about_unrecognized_subrecord(subrecord.signature(), this->stub)
               );
               break;
         }
      }
   }
   /*static*/ void Faction::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'JAIL':
            case 'WAIT':
            case 'STOL':
            case 'PLCN':
            case 'CRGR':
            case 'JOUT':
            case 'VEND':
            case 'VENC':
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
            case 'PLVD':
               components::package_location::generate_use_info(subrecord, uib);
               break;
         }
      }
   }
   bool Faction::_clone_impl(Form* out) const noexcept {
      if (out->formType != form_type)
         return false;
      auto copy = (Faction*)out;
      //
      copy->name = this->name;
      {
         size_t size = this->relationships.size();
         copy->relationships.reserve(size);
         for (size_t i = 0; i < size; ++i) {
            auto& entry = copy->relationships[i];
            auto& from  = this->relationships[i];
            entry.other.set(*copy, from.other);
            entry.combat = from.combat;
            entry.mod    = from.mod;
         }
      }
      copy->faction_flags = this->faction_flags;
      copy->prison_marker.set(*copy, this->prison_marker);
      copy->follower_wait_marker.set(*copy, this->follower_wait_marker);
      copy->evidence_chest.set(*copy, this->evidence_chest);
      copy->player_belongings_chest.set(*copy, this->player_belongings_chest);
      copy->crime_group.set(*copy, this->crime_group);
      copy->jail_outfit.set(*copy, this->jail_outfit);
      copy->crime_values = this->crime_values;
      {
         size_t size = this->ranks.size();
         copy->ranks.resize(size);
         for (size_t i = 0; i < size; ++i)
            copy->ranks[i] = this->ranks[i];
      }
      copy->vendor_list.set(*copy, this->vendor_list);
      copy->vendor_chest.set(*copy, this->vendor_chest);
      copy->vendor_data = this->vendor_data;
      copy->package_location_vendor.clone_from(this->package_location_vendor, *copy);
      components::condition::clone_condition_list(copy->stub, copy->vendor_conditions, this->vendor_conditions);
      copy->has_object_bounds = this->has_object_bounds;
      copy->object_bounds = this->object_bounds;
      copy->script_data.clone_from(this->script_data, *copy);
      return true;
   }
   bool Faction::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      for (auto& rel : this->relationships) {
         auto& subrecord = record.open_next_subrecord('XNAM');
         subrecord.write(rel.other);
         subrecord.write(rel.mod);
         subrecord.write(rel.combat);
         subrecord.close();
      }
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->faction_flags);
      DATA.close();
      if (this->jail_outfit)
         record.write_formID_subrecord('JAIL', this->jail_outfit);
      if (this->follower_wait_marker)
         record.write_formID_subrecord('WAIT', this->follower_wait_marker);
      if (this->evidence_chest)
         record.write_formID_subrecord('STOL', this->evidence_chest);
      if (this->player_belongings_chest)
         record.write_formID_subrecord('PLCN', this->player_belongings_chest);
      if (this->crime_group)
         record.write_formID_subrecord('CRGR', this->crime_group);
      if (this->jail_outfit)
         record.write_formID_subrecord('JOUT', this->jail_outfit);
      auto& CRVA = record.open_next_subrecord('CRVA');
      CRVA.write(this->crime_values.arrest);
      CRVA.write(this->crime_values.attack_on_sight);
      CRVA.write(this->crime_values.murder);
      CRVA.write(this->crime_values.assault);
      CRVA.write(this->crime_values.trespass);
      CRVA.write(this->crime_values.pickpocket);
      CRVA.write(this->crime_values.unused);
      CRVA.write(this->crime_values.theft_multiplier);
      CRVA.write(this->crime_values.jail_escape);
      CRVA.write(this->crime_values.werewolf_transformation);
      CRVA.close();
      for (auto& r : this->ranks) {
         auto& RNAM = record.open_next_subrecord('RNAM');
         RNAM.write(r.id);
         RNAM.close();
         if (!r.title_masc.empty()) {
            auto& subrecord = record.open_next_subrecord('MNAM');
            subrecord.write(r.title_masc);
            subrecord.close();
         }
         if (!r.title_fem.empty()) {
            auto& subrecord = record.open_next_subrecord('FNAM');
            subrecord.write(r.title_fem);
            subrecord.close();
         }
      }
      if (this->vendor_list)
         record.write_formID_subrecord('VEND', this->vendor_list);
      if (this->vendor_chest)
         record.write_formID_subrecord('VENC', this->vendor_chest);
      auto& VENV = record.open_next_subrecord('VENV');
      VENV.write(this->vendor_data.start_hour);
      VENV.write(this->vendor_data.end_hour);
      VENV.write(this->vendor_data.radius);
      VENV.write(this->vendor_data.unused06);
      VENV.write(this->vendor_data.buys_stolen);
      VENV.write(this->vendor_data.vendor_list_is_blacklist);
      VENV.write(this->vendor_data.unused0A);
      VENV.close();
      auto& PLVD = record.open_next_subrecord('PLVD');
      this->package_location_vendor.save(PLVD);
      PLVD.close();
      auto& CITC = record.open_next_subrecord('CITC');
      CITC.write(uint32_t(this->vendor_conditions.size()));
      CITC.close();
      for (auto& condition : this->vendor_conditions)
         condition.save(record, intfc);
      //
      if (this->has_object_bounds) {
         auto& subrecord = record.open_next_subrecord('OBND');
         this->object_bounds.save(subrecord, intfc);
         subrecord.close();
      }
      this->script_data.save(record, intfc); // VMAD (won't write anything if no scripts are attached)
      //
      return true;
   }
   void Faction::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->package_location_vendor.sever_outbound_references_to(other, *this);
      //
      bool removals = false;
      for (auto& entry : this->relationships) {
         if (entry.other == &other) {
            entry.other.set(*this, nullptr);
            removals = true;
         }
      }
      if (removals) {
         auto& list = this->relationships;
         list.erase(
            std::remove_if(
               list.begin(),
               list.end(),
               [](relationship& entry) {
                  return entry.other == nullptr;
               }
            ),
            list.end()
         );
      }
      //
      for (auto& cnd : this->vendor_conditions)
         cnd.sever_outbound_references_to(other);
      //
      this->prison_marker.clear_if(*this, other);
      this->follower_wait_marker.clear_if(*this, other);
      this->evidence_chest.clear_if(*this, other);
      this->player_belongings_chest.clear_if(*this, other);
      this->crime_group.clear_if(*this, other);
      this->jail_outfit.clear_if(*this, other);
      this->vendor_list.clear_if(*this, other);
      this->vendor_chest.clear_if(*this, other);
   }
   void Faction::_clear_impl() noexcept {
      this->name.reset();
      //
      for (auto& entry : this->relationships)
         entry.other.set(*this, nullptr);
      this->relationships.clear();
      //
      this->faction_flags = 0;
      this->prison_marker.set(*this, nullptr);
      this->follower_wait_marker.set(*this, nullptr);
      this->evidence_chest.set(*this, nullptr);
      this->player_belongings_chest.set(*this, nullptr);
      this->crime_group.set(*this, nullptr);
      this->jail_outfit.set(*this, nullptr);
      this->ranks.clear();
      this->vendor_list.set(*this, nullptr);
      this->vendor_chest.set(*this, nullptr);
      this->package_location_vendor.clear(*this);
      for (auto& cnd : this->vendor_conditions)
         cnd.clear();
      this->vendor_conditions.clear();
      //
      this->has_object_bounds = false;
      this->object_bounds.clear();
      this->script_data.clear(*this);
   }
}