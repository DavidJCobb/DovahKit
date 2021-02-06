#include "Location.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Location::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      this->subrecordFlags = 0;
      form_reference_t formID;
      uint32_t  keywordCount = 0;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
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
            case 'ACUN':
               this->subrecordFlags |= subrecord_flag::unique_is_a;
               // and fall through
            case 'LCUN':
               while (subrecord.is_in_bounds(12)) {
                  auto& unique = this->uniques.emplace_back();
                  subrecord.unchecked_read(unique.actor_base);
                  subrecord.unchecked_read(unique.actor);
                  subrecord.unchecked_read(unique.location);
               }
               break;
            case 'RCUN':
               while (subrecord.is_in_bounds(4))
                  if (subrecord.read(formID))
                     this->uniques_R.push_back(formID);
               break;
            case 'ACSR':
               this->subrecordFlags |= subrecord_flag::static_is_a;
               // and fall through
            case 'LCSR':
               while (subrecord.is_in_bounds(16)) {
                  auto& stat = this->statics.emplace_back();
                  subrecord.unchecked_read(stat.ref_type);
                  subrecord.unchecked_read(stat.reference);
                  subrecord.unchecked_read(stat.cell_or_world);
                  subrecord.unchecked_read(stat.unk0C);
                  subrecord.unchecked_read(stat.unk0E);
               }
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
               this->keywords.load(subrecord, intfc);
               break;
            case 'PNAM':
               subrecord.read(this->parent_location);
               break;
            case 'NAM1':
               subrecord.read(this->music);
               break;
            case 'FNAM':
               subrecord.read(this->unreported_crime_faction);
               break;
            case 'MNAM':
               subrecord.read(this->marker);
               break;
            case 'RNAM':
               subrecord.read(this->radius);
               break;
            case 'NAM0':
               subrecord.read(this->horse_marker);
               break;
            case 'CNAM':
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
      uint32_t  keywordCount = 0;
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'PNAM': // parent location
            case 'NAM1': // music
            case 'FNAM': // unreported crime faction
            case 'MNAM': // marker
            case 'NAM0': // horse marker
               if (!uib.is_final_file())
                  break;
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
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
            case 'ACUN':
            case 'LCUN':
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
            case 'ACSR':
            case 'LSCR':
               //
               // TODO: Is this coalesced across multiple files?
               //
               while (subrecord.is_in_bounds(16)) {
                  subrecord.unchecked_read(formID);
                  uib.add_outbound_reference(formID);
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
   }
}