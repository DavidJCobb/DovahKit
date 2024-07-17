#include "Location.h"
#include "_common_cpp.h"
#include "../../helpers/vector.h"

#include "../../../incomplete_code_warnings.h"
static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_forms, "The backend for Location is incomplete (missing everything except the loader).");

namespace dovah::loaded_forms {
   void Location::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      bool is_active_file = intfc.is_active_file();
      //
      form_reference_t formID;
      uint32_t  keywordCount = 0;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'OBND':
               //
               // The loader checks for this and passes it to a virtual function on TESForm 
               // that's responsible for loading it. However, this form doesn't derive from 
               // TESBoundObject, so the TESForm implementation of that virtual function (a 
               // no-op) isn't overridden and therefore the data is not retained in memory.
               //
               break;
            #pragma region Enable points
            case 'ACEP':
               [[fallthrough]];
            case 'LCEP':
               while (subrecord.is_in_bounds(12)) {
                  auto& entry = this->enable_points.emplace_back();
                  subrecord.read(entry.actor);
                  subrecord.read(entry.enable_parent);
                  subrecord.read(entry.grid.y);
                  subrecord.read(entry.grid.x);
               }
               break;
            case 'RCEP':
               if (subrecord.read(formID)) {
                  auto& list = this->enable_points;
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
            #pragma region Exterior cells
            case 'ACEC':
               [[fallthrough]];
            case 'LCEC':
               if (!subrecord.is_in_bounds(4))
                  break;
               {
                  auto& entry = this->exterior_cells.emplace_back();
                  subrecord.unchecked_read(entry.worldspace);
                  while (subrecord.is_in_bounds(4)) {
                     auto& gc = entry.cells.emplace_back();
                     subrecord.read(gc.y);
                     subrecord.read(gc.x);
                  }
               }
               break;
            case 'RCEC':
               if (subrecord.read(formID)) {
                  auto& list = this->exterior_cells;
                  for (auto it = list.begin(); it != list.end(); ++it) {
                     auto& entry = *it;
                     if (entry.worldspace == formID) {
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
            #pragma region Markers
            case 'ACID':
               [[fallthrough]];
            case 'LCID':
               if (subrecord.read(formID))
                  markers.push_back(formID);
               break;
            #pragma endregion
            #pragma region Persistent refs
            case 'ACPR':
               [[fallthrough]];
            case 'LCPR':
               while (subrecord.is_in_bounds(12)) {
                  auto& entry = this->persistent_refs.emplace_back();
                  subrecord.unchecked_read(entry.actor);
                  subrecord.unchecked_read(entry.cell_or_world);
                  subrecord.unchecked_read(entry.grid.y);
                  subrecord.unchecked_read(entry.grid.x);
               }
               break;
            case 'RCPR':
               if (subrecord.read(formID)) {
                  auto& list = this->persistent_refs;
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
            #pragma region Special refs
            case 'ACSR':
               [[fallthrough]];
            case 'LCSR':
               while (subrecord.is_in_bounds(16)) {
                  auto& stat = this->special_refs.emplace_back();
                  subrecord.unchecked_read(stat.ref_type);
                  subrecord.unchecked_read(stat.reference);
                  subrecord.unchecked_read(stat.cell_or_world);
                  subrecord.unchecked_read(stat.grid.y);
                  subrecord.unchecked_read(stat.grid.x);
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
               //
            case 'FULL':
               if (!intfc.is_winning_record)
                  break;
               subrecord.read(this->name);
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
               intfc.warn_if_ref_is_wrong_type(this->parent_location, form_type::location, subrecord.signature());
               break;
            case 'NAM1':
               if (!intfc.is_winning_record)
                  break;
               subrecord.read(this->music);
               intfc.warn_if_ref_is_wrong_type(this->music, form_type::music_type, subrecord.signature());
               break;
            case 'FNAM':
               if (!intfc.is_winning_record)
                  break;
               subrecord.read(this->unreported_crime_faction);
               intfc.warn_if_ref_is_wrong_type(this->unreported_crime_faction, form_type::faction, subrecord.signature());
               break;
            case 'MNAM':
               if (!intfc.is_winning_record)
                  break;
               subrecord.read(this->marker);
               intfc.warn_if_ref_is_wrong_type(this->marker, form_type::reference, subrecord.signature());
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
               intfc.warn_if_ref_is_wrong_type(this->horse_marker, form_type::reference, subrecord.signature());
               break;
            case 'CNAM':
               if (!intfc.is_winning_record)
                  break;
               this->color.load(subrecord);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Location::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      struct _ep {
         form_id_t actor;
         form_id_t enable_point;
      };
      struct _pr {
         form_id_t actor;
         form_id_t cell_or_world;
      };
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
      std::vector<form_id_t> markers;
      std::vector<form_id_t> worldspaces;
      std::vector<_ep> enable_points;
      std::vector<_pr> persistent_refs;
      std::vector<_sr> special_refs;
      std::vector<_un> unique_refs;
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            #pragma region Enable points
            case 'ACEP':
               [[fallthrough]];
            case 'LCEP':
               while (subrecord.is_in_bounds(12)) {
                  auto& entry = enable_points.emplace_back();
                  subrecord.unchecked_read(entry.actor);
                  subrecord.unchecked_read(entry.enable_point);
                  subrecord.skip_bytes(4);
               }
               break;
            case 'RCEP': // Remove entries
               if (uib.is_active_file())
                  break;
               while (subrecord.is_in_bounds(4)) {
                  if (!subrecord.read(formID))
                     continue;
                  cobb::unordered_erase(enable_points, [formID](const _ep& entry) {
                     return formID == entry.actor;
                  });
               }
               break;
            #pragma endregion
            #pragma region Exterior cells
            case 'ACEC':
               [[fallthrough]];
            case 'LCEC':
               if (subrecord.read(formID))
                  worldspaces.push_back(formID);
               break;
            case 'RCEC':
               if (uib.is_active_file())
                  break;
               if (subrecord.read(formID)) {
                  cobb::unordered_erase(worldspaces, [formID](const form_id_t entry) {
                     return formID == entry;
                  });
               }
               break;
            #pragma endregion
            #pragma region Markers
            case 'ACID':
               [[fallthrough]];
            case 'LCID':
               if (subrecord.read(formID))
                  markers.push_back(formID);
               break;
            #pragma endregion
            #pragma region Persistent refs
            case 'ACPR':
               [[fallthrough]];
            case 'LCPR':
               while (subrecord.is_in_bounds(16)) {
                  auto& stat = persistent_refs.emplace_back();
                  subrecord.unchecked_read(stat.actor);
                  subrecord.unchecked_read(stat.cell_or_world);
                  subrecord.skip_bytes(4);
               }
               break;
            case 'RCPR': // Remove entries
               if (uib.is_active_file())
                  break;
               while (subrecord.is_in_bounds(4)) {
                  if (!subrecord.read(formID))
                     continue;
                  cobb::unordered_erase(persistent_refs, [formID](const _pr& entry) {
                     return formID == entry.actor;
                  });
               }
               break;
            #pragma endregion
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
                  cobb::unordered_erase(unique_refs, [formID](const _un& entry) {
                     //
                     // TODO: Is this what RCUN looks for, or does it want the actor-base?
                     //
                     return formID == entry.actor;
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
      for (auto id : markers)
         uib.add_outbound_reference(id);
      for (auto id : worldspaces)
         uib.add_outbound_reference(id);
      for (auto& entry : enable_points) {
         uib.add_outbound_reference(entry.actor);
         uib.add_outbound_reference(entry.enable_point);
      }
      for (auto& entry : persistent_refs) {
         uib.add_outbound_reference(entry.actor);
         uib.add_outbound_reference(entry.cell_or_world);
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