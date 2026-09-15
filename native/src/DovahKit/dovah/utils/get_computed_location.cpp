#include "./get_computed_location.h"
#include "../data/hardcoded_form_ids.h"
#include "../form_stubs/helpers/get_assigned_encounter_zone.h"
#include "../form_stubs/helpers/get_assigned_location.h"
#include "../forms/components/extra_data/types/e/encounter_zone.h"
#include "../forms/components/extra_data/types/l/location.h"
#include "../forms/Cell.h"
#include "../forms/Worldspace.h"

namespace {
   static bool _is_valid_zone(const dovah::form_stub* zone) {
      if (!zone || zone->form_type != dovah::form_type::encounter_zone)
         return false;
      if (zone->formID == dovah::hardcoded_form_ids::NoZoneZone)
         return false;
      return true;
   }
   static dovah::form_stub* _location_of_zone(const dovah::form_stub& zone) {
      auto* loc = dovah::form_stub_helpers::get_assigned_location(zone);
      if (loc && loc->form_type == dovah::form_type::location)
         return loc;
      return nullptr;
   }
}

namespace dovah::utils {
   extern form_stub* get_computed_location(const loaded_forms::Cell& cell) {
      loaded_form_ptr<loaded_forms::Worldspace> loaded_world;
      //
      // Encounter Zones override any Location set on the cell or worldspace.
      //
      if (auto* extra = cell.extra_data.get<loaded_forms::components::extra_data_types::encounter_zone>()) {
         auto* zone = extra->form.get_form_stub();
         if (_is_valid_zone(zone))
            if (auto* loc = _location_of_zone(*zone))
               return loc;
      }
      auto* world = cell.stub.get_parent_form();
      if (world && world->form_type == form_type::worldspace) {
         loaded_world = world->load().ptr_cast<loaded_forms::Worldspace>();
         if (loaded_world) {
            auto* zone = loaded_world->encounter_zone.get_form_stub();
            if (_is_valid_zone(zone))
               if (auto* loc = _location_of_zone(*zone))
                  return loc;
         }
      }
      //
      // Check the cell's own Location, and then the world's Location.
      //
      if (auto* extra = cell.extra_data.get<loaded_forms::components::extra_data_types::location>()) {
         auto* stub = extra->form.get_form_stub();
         if (stub && stub->form_type == form_type::location)
            return stub;
      }
      if (loaded_world) {
         auto* stub = loaded_world->location.get_form_stub();
         if (stub && stub->form_type == form_type::location)
            return stub;
      }
      return nullptr;
   }
   extern form_stub* get_computed_location(const loaded_forms::Worldspace& worldspace) {
      if (worldspace.location)
         return worldspace.location.get_form_stub();
      auto* zone = worldspace.encounter_zone.get_form_stub();
      if (_is_valid_zone(zone))
         if (auto* loc = _location_of_zone(*zone))
            return loc;
      return nullptr;
   }
   extern form_stub* get_computed_location(const form_stub& stub) {
      switch (stub.form_type) {
         case form_type::cell:
            {
               dovah::form_stub* world = stub.get_parent_form();
               if (world && world->form_type != dovah::form_type::worldspace)
                  world = nullptr;
               //
               // First, the game tries the Encounter Zone of the cell or, if it doesn't 
               // have one set, the Encounter Zone of its containing worldspace.
               //
               auto* zone = form_stub_helpers::get_assigned_encounter_zone(stub);
               if (!zone || zone->form_type != dovah::form_type::encounter_zone) {
                  if (world) {
                     zone = form_stub_helpers::get_assigned_encounter_zone(*world);
                     if (zone && zone->form_type != dovah::form_type::encounter_zone)
                        zone = nullptr;
                  }
               }
               if (zone && zone->formID != dovah::hardcoded_form_ids::NoZoneZone)
                  if (auto* loc = _location_of_zone(*zone))
                     return loc;
               //
               // If that fails, then it checks for an explicitly-set location on the cell.
               //
               //
               auto* loc = form_stub_helpers::get_assigned_location(stub);
               if (loc && loc->form_type == dovah::form_type::location)
                  return loc;
               //
               // Absent an explicitly set location on the cell, the game checks for the 
               // computed location of the cell's containing worldspace. (It may seem a bit 
               // redundant to do the full "computed location" logic on the worldspace, to 
               // check the worldspace's location and encounter zone, given that we fall 
               // through to the worldspace's encounter zone above if the cell has no zone. 
               // However, if the cell uses the NoZoneZone, then we (and the game) will act 
               // as though the cell has no zone, BUT we (and the game) WON'T fall through 
               // to checking the worldspace's zone ahead; it'll only be checked here.)
               //
               if (world) {
                  return get_computed_location(*world);
               }
            }
            break;
         case form_type::worldspace:
            //
            // When determining a worldspace's computed location, an explicitly set location 
            // takes priority over an explicitly set Encounter Zone. This is the inverse of 
            // how these are prioritized on cells.
            //
            if (auto* loc = form_stub_helpers::get_assigned_location(stub)) {
               if (loc->form_type == dovah::form_type::location)
                  return loc;
            }
            if (auto* zone = form_stub_helpers::get_assigned_encounter_zone(stub)) {
               if (_is_valid_zone(zone))
                  if (auto* loc = _location_of_zone(*zone))
                     return loc;
            }
            break;
      }
      return nullptr;
   }
}