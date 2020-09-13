#include "Location.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Location::load(tes_record_reader& record) {
      this->subrecordFlags = 0;
      form_id_t formID;
      uint32_t  keywordCount = 0;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'ACPR':
               this->subrecordFlags |= subrecord_flag::population_is_a;
               // and fall through
            case 'LCPR':
               while (subrecord.is_in_bounds(12)) {
                  auto& pop = this->population.emplace_back();
                  subrecord.unchecked_read(pop.actorRefID);
                  subrecord.unchecked_read(pop.cellOrWorldID);
                  subrecord.unchecked_read(pop.gridY);
                  subrecord.unchecked_read(pop.gridX);
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
                  subrecord.unchecked_read(unique.actorBaseID);
                  subrecord.unchecked_read(unique.actorRefID);
                  subrecord.unchecked_read(unique.locationID);
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
                  subrecord.unchecked_read(stat.locRefTypeID);
                  subrecord.unchecked_read(stat.refID);
                  subrecord.unchecked_read(stat.cellOrWorldID);
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
                  subrecord.unchecked_read(ep.actorID);
                  subrecord.unchecked_read(ep.refID);
                  subrecord.unchecked_read(ep.gridY);
                  subrecord.unchecked_read(ep.gridX);
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
               subrecord.read(keywordCount);
               break;
            case 'KWDA':
               if (!keywordCount)
                  keywordCount = subrecord.size() / 4;
               for (uint32_t i = 0; i < keywordCount; i++)
                  if (subrecord.read(formID))
                     stub->add_outbound_reference(formID);
               break;
            case 'PNAM':
               subrecord.read(this->parentLocationID);
               break;
            case 'NAM1':
               subrecord.read(this->musicID);
               break;
            case 'FNAM':
               subrecord.read(this->unreportedCrimeFactionID);
               break;
            case 'MNAM':
               subrecord.read(this->marker);
               break;
            case 'RNAM':
               subrecord.read(this->radius);
               break;
            case 'NAM0':
               subrecord.read(this->horseMarkerRefID);
               break;
            case 'CNAM':
               subrecord.read(this->color.r);
               subrecord.read(this->color.g);
               subrecord.read(this->color.b);
               subrecord.read(this->color.alpha);
               break;
         }
      }
   }
   /*static*/ void Location::generateUseInfo(tes_record_reader& record, form_stub* stub) {
      uint32_t keywordCount = 0;
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'PNAM': // parent location
            case 'NAM1': // music
            case 'FNAM': // unreported crime faction
            case 'MNAM': // marker
            case 'NAM0': // horse marker
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               break;
            case 'KSIZ':
               subrecord.read(keywordCount);
               break;
            case 'KWDA':
               if (!keywordCount)
                  keywordCount = subrecord.size() / 4;
               for (uint32_t i = 0; i < keywordCount; i++)
                  if (subrecord.read(formID))
                     stub->add_outbound_reference(formID);
               break;
            case 'ACLR':
            case 'LCPR':
               while (subrecord.is_in_bounds(12)) {
                  subrecord.unchecked_read(formID);
                  stub->add_outbound_reference(formID);
                  subrecord.unchecked_read(formID);
                  stub->add_outbound_reference(formID);
                  subrecord.skip_bytes(4);
               }
               break;
            case 'RCPR':
            case 'ACID':
            case 'LCID':
               while (subrecord.is_in_bounds(4))
                  if (subrecord.read(formID))
                     stub->add_outbound_reference(formID);
               break;
            case 'ACUN':
            case 'LCUN':
            case 'ACEP':
            case 'LCEP':
               while (subrecord.is_in_bounds(12)) {
                  subrecord.unchecked_read(formID);
                  stub->add_outbound_reference(formID);
                  subrecord.unchecked_read(formID);
                  stub->add_outbound_reference(formID);
                  subrecord.skip_bytes(4);
               }
               break;
            case 'ACSR':
            case 'LSCR':
               while (subrecord.is_in_bounds(16)) {
                  subrecord.unchecked_read(formID);
                  stub->add_outbound_reference(formID);
                  subrecord.unchecked_read(formID);
                  stub->add_outbound_reference(formID);
                  subrecord.unchecked_read(formID);
                  stub->add_outbound_reference(formID);
                  subrecord.skip_bytes(4);
               }
               break;
            case 'ACEC':
            case 'LCEC':
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
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