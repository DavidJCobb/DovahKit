#include "Location.h"
#include "_common_cpp.h"
#include "../../helpers/vector.h"

namespace dovah::loaded_forms {
   void Location::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      bool is_active_file = intfc.is_active_file();
      if (!intfc.is_winning_record) {
         this->name.reset();
      }
      //
      this->subrecordFlags = 0;
      form_reference_t formID;
      uint32_t  keywordCount = 0;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            #pragma region Special refs
            case 'ACSR':
               [[fallthrough]];
            case 'LCSR':
               while (subrecord.is_in_bounds(16)) {
                  auto& stat = this->special_refs.emplace_back();
                  subrecord.unchecked_read(stat.ref_type);
                  subrecord.unchecked_read(stat.reference);
                  subrecord.unchecked_read(stat.cell_or_world);
                  subrecord.unchecked_read(stat.grid_y);
                  subrecord.unchecked_read(stat.grid_x);
               }
               break;
            case 'RCSR':
               if (subrecord.read(formID)) {
                  auto& list = this->special_refs;
                  for (auto it = list.begin(); it != list.end(); ++it) {
                     auto& entry = *it;
                     if (entry.reference == formID) {
                        if (is_active_file)
                           entry.removed_by_active_file = true;
                        else
                           list.erase(it);
                        break;
                     }
                  }
               }
               break;
            #pragma endregion
            #pragma region Unique refs
            case 'ACUN':
               [[fallthrough]];
            case 'LCUN':
               while (subrecord.is_in_bounds(12)) {
                  auto& unique = this->uniques.emplace_back();
                  subrecord.unchecked_read(unique.actor_base);
                  subrecord.unchecked_read(unique.actor);
                  subrecord.unchecked_read(unique.editor_location);
               }
               break;
            case 'RCUN':
               if (subrecord.read(formID)) {
                  auto& list = this->uniques;
                  for (auto it = list.begin(); it != list.end(); ++it) {
                     auto& entry = *it;
                     if (entry.actor == formID) {
                        if (is_active_file)
                           entry.removed_by_active_file = true;
                        else
                           list.erase(it);
                        break;
                     }
                  }
               }
               break;
            #pragma endregion
            static_assert(false, "identify *CPR and fix load and use info code for it");
            static_assert(false, "identify *CEC and fix load and use info code for it");
            static_assert(false, "identify *CEP and fix load and use info code for it");
            static_assert(false, "identify *CID and fix load and use info code for it");
            case 'ACPR':
               this->subrecordFlags |= subrecord_flag::population_is_a;
               // and fall through
            case 'LCPR':
               while (subrecord.is_in_bounds(12)) {
                  auto& pop = this->population.emplace_back();
                  subrecord.unchecked_read(pop.actor);
                  subrecord.unchecked_read(pop.cell_or_world);
                  subrecord.unchecked_read(pop.grid_y);
                  subrecord.unchecked_read(pop.grid_x);
               }
               break;
            case 'RCPR':
               while (subrecord.is_in_bounds(4))
                  if (subrecord.read(formID))
                     this->populationActors.push_back(formID);
               break;
            case 'ACEC':
               this->subrecordFlags |= subrecord_flag::encounter_is_a;
               // and fall through
            case 'LCEC':
               {
                  auto enc = this->encounters.emplace_back();
                  subrecord.read(enc.worldID);
                  enc.ints.reserve((subrecord.size() - 4) / 4);
                  while (subrecord.is_in_bounds(4)) {
                     uint32_t i;
                     subrecord.unchecked_read(i);
                     enc.ints.push_back(i);
                  }
               }
               break;
            case 'ACEP':
               this->subrecordFlags |= subrecord_flag::enable_is_a;
               // and fall through
            case 'LCEP':
               while (subrecord.is_in_bounds(12)) {
                  auto& ep = this->enablePoints.emplace_back();
                  subrecord.unchecked_read(ep.actor);
                  subrecord.unchecked_read(ep.reference);
                  subrecord.unchecked_read(ep.grid_y);
                  subrecord.unchecked_read(ep.grid_x);
               }
               break;
            case 'ACID':
               this->subrecordFlags |= subrecord_flag::cid_is_a;
               // and fall through
            case 'LCID':
               while (subrecord.is_in_bounds(4))
                  if (subrecord.read(formID))
                     this->populationActors.push_back(formID);
               break;
            case 'KSIZ':
            case 'KWDA':
               if (!intfc.is_winning_record)
                  break;
               this->keywords.load(subrecord, intfc);
               break;
            case 'PNAM':
               if (!intfc.is_winning_record)
                  break;
               subrecord.read(this->parent_location);
               break;
            case 'NAM1':
               if (!intfc.is_winning_record)
                  break;
               subrecord.read(this->music);
               break;
            case 'FNAM':
               if (!intfc.is_winning_record)
                  break;
               subrecord.read(this->unreported_crime_faction);
               break;
            case 'MNAM':
               if (!intfc.is_winning_record)
                  break;
               subrecord.read(this->marker);
               break;
            case 'RNAM':
               if (!intfc.is_winning_record)
                  break;
               subrecord.read(this->radius);
               break;
            case 'NAM0':
               if (!intfc.is_winning_record)
                  break;
               subrecord.read(this->horse_marker);
               break;
            case 'CNAM':
               if (!intfc.is_winning_record)
                  break;
               subrecord.read(this->color.r);
               subrecord.read(this->color.g);
               subrecord.read(this->color.b);
               subrecord.read(this->color.alpha);
               break;
            default:
               intfc.log_load_warning(
                  detailed_notice::warn_about_unrecognized_subrecord(subrecord.signature(), this->stub)
               );
               break;
         }
      }
   }
   /*static*/ void Location::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      struct _sr {
         form_id_t ref_type;
         form_id_t reference;
         form_id_t cell_or_world;
      };
      struct _un { // ACUN and LCUN add; RCUN removes
         form_id_t actor_base;
         form_id_t actor;
         form_id_t editor_location; // usually self
      };
      //
      form_id_t crime_faction;
      form_id_t marker;
      form_id_t horse_marker;
      form_id_t music;
      form_id_t parent;
      std::vector<_sr> special_refs;
      std::vector<_un> unique_refs;
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            #pragma region Special refs
            case 'ACSR':
               [[fallthrough]];
            case 'LCSR':
               while (subrecord.is_in_bounds(16)) {
                  auto& stat = special_refs.emplace_back();
                  subrecord.unchecked_read(stat.ref_type);
                  subrecord.unchecked_read(stat.reference);
                  subrecord.unchecked_read(stat.cell_or_world);
                  subrecord.skip_bytes(4);
               }
               break;
            case 'RCSR': // Remove entries
               if (uib.is_active_file())
                  break;
               while (subrecord.is_in_bounds(4)) {
                  if (!subrecord.read(formID))
                     continue;
                  cobb::unordered_erase(special_refs, [formID](const _sr& entry) {
                     return formID == entry.reference;
                  });
               }
               break;
            #pragma endregion
            #pragma region Unique refs
            case 'ACUN':
               [[fallthrough]];
            case 'LCUN':
               while (subrecord.is_in_bounds(12)) {
                  auto& unique = unique_refs.emplace_back();
                  subrecord.unchecked_read(unique.actor_base);
                  subrecord.unchecked_read(unique.actor);
                  subrecord.unchecked_read(unique.editor_location);
               }
               break;
            case 'RCUN': // Remove entries
               if (uib.is_active_file())
                  break;
               while (subrecord.is_in_bounds(4)) {
                  if (!subrecord.read(formID))
                     continue;
                  cobb::unordered_erase(unique_refs, [formID](const _sr& entry) {
                     return formID == entry.reference;
                  });
               }
               break;
            #pragma endregion
               //
            case 'PNAM': // parent location
               subrecord.read(parent);
               break;
            case 'NAM1': // music
               subrecord.read(music);
               break;
            case 'FNAM': // unreported crime faction
               subrecord.read(crime_faction);
               break;
            case 'MNAM': // marker
               subrecord.read(marker);
               break;
            case 'NAM0': // horse marker
               subrecord.read(horse_marker);
               break;
            case 'KSIZ':
            case 'KWDA':
               if (!uib.is_final_file())
                  break;
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case 'ACLR':
            case 'LCPR':
               //
               // TODO: Is this coalesced across multiple files?
               //
               while (subrecord.is_in_bounds(12)) {
                  subrecord.unchecked_read(formID);
                  uib.add_outbound_reference(formID);
                  subrecord.unchecked_read(formID);
                  uib.add_outbound_reference(formID);
                  subrecord.skip_bytes(4);
               }
               break;
            case 'RCPR':
            case 'ACID':
            case 'LCID':
               //
               // TODO: Is this coalesced across multiple files?
               //
               while (subrecord.is_in_bounds(4))
                  if (subrecord.read(formID))
                     uib.add_outbound_reference(formID);
               break;
            case 'ACEP':
            case 'LCEP':
               //
               // TODO: Is this coalesced across multiple files?
               //
               while (subrecord.is_in_bounds(12)) {
                  subrecord.unchecked_read(formID);
                  uib.add_outbound_reference(formID);
                  subrecord.unchecked_read(formID);
                  uib.add_outbound_reference(formID);
                  subrecord.skip_bytes(4);
               }
               break;
            case 'ACEC':
            case 'LCEC':
               //
               // TODO: Is this coalesced across multiple files?
               //
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               // we can ignore the rest
               break;
            case 'EDID': // editor ID
            case 'FULL': // name
            case 'RNAM': // radius
            case 'CNAM': // color
               break;
         }
      }
      if (uib.is_final_file()) { // These fields are not coalesced across files.
         uib.add_outbound_reference(crime_faction);
         uib.add_outbound_reference(marker);
         uib.add_outbound_reference(horse_marker);
         uib.add_outbound_reference(music);
         uib.add_outbound_reference(parent);
      }
      for (auto& entry : special_refs) {
         uib.add_outbound_reference(entry.ref_type);
         uib.add_outbound_reference(entry.reference);
         uib.add_outbound_reference(entry.cell_or_world);
      }
      for (auto& entry : unique_refs) {
         uib.add_outbound_reference(entry.actor_base);
         uib.add_outbound_reference(entry.actor);
         uib.add_outbound_reference(entry.editor_location);
      }
   }
}