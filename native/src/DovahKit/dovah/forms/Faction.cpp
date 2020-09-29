#include "Faction.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Faction::load(tes_record_reader& record) {
      Form::load(record);
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
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
               }
               break;
            case 'DATA':
               subrecord.read(this->faction_flags);
               break;
            case 'JAIL':
               subrecord.read(this->prison_marker);
               break;
            case 'WAIT':
               subrecord.read(this->follower_wait_marker);
               break;
            case 'STOL':
               subrecord.read(this->evidence_chest);
               break;
            case 'PLCN':
               subrecord.read(this->player_belongings_chest);
               break;
            case 'CRGR':
               subrecord.read(this->crime_group);
               break;
            case 'JOUT':
               subrecord.read(this->jail_outfit);
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
               break;
            case 'VENC':
               subrecord.read(this->vendor_chest);
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
               this->package_location_vendor.load(subrecord);
               break;
            case 'CITC':
               {
                  uint32_t count;
                  if (subrecord.read(count))
                     this->vendor_conditions.reserve(count);
               }
               break;
            case 'CTDA':
               {
                  auto& list = this->vendor_conditions;
                  list.emplace_back();
                  auto& cnd = *list.rbegin();
                  cnd.read(subrecord.get_containing_record());
               }
               break;
            case 'OBND':
               this->object_bounds.load(subrecord);
               break;
            case 'VMAD':
               this->script_data.load(subrecord);
               break;
         }
      }
   }
   /*static*/ void Faction::generateUseInfo(tes_record_reader& record, form_stub* stub) {
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
                  stub->add_outbound_reference(formID);
               break;
            case 'PLVD':
               components::package_location::generateUseInfo(subrecord, stub);
               break;
         }
      }
   }
   bool Faction::_save_impl(tes_record_writer& record) {
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
            auto& subrecord = record.open_next_subrecord('MNAM');
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
         condition.save(record);
      //
      return true;
   }
}