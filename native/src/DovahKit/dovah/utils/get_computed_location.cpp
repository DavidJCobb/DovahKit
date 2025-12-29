#include "./get_computed_location.h"
#include "../data/hardcoded_form_ids.h"
#include "../forms/components/extra_data/types/e/encounter_zone.h"
#include "../forms/components/extra_data/types/l/location.h"
#include "../forms/Cell.h"
#include "../forms/EncounterZone.h"
#include "../forms/Worldspace.h"

namespace {
   static bool _is_valid_zone(dovah::form_stub* zone) {
      if (!zone || zone->form_type != dovah::form_type::encounter_zone)
         return false;
      if (zone->formID == dovah::hardcoded_form_ids::NoZoneZone)
         return false;
      return true;
   }
   static dovah::form_stub* _location_of_zone(dovah::form_stub& zone) {
      auto loaded = zone.load().ptr_cast<dovah::loaded_forms::EncounterZone>();
      if (loaded)
         return loaded->location.get_form_stub();
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
   extern form_stub* get_computed_location(form_stub& stub) {
      switch (stub.form_type) {
         case form_type::cell:
            {
               auto loaded = stub.load().ptr_cast<loaded_forms::Cell>();
               if (loaded)
                  return get_computed_location(*loaded);
            }
            break;
         case form_type::worldspace:
            {
               auto loaded = stub.load().ptr_cast<loaded_forms::Worldspace>();
               if (loaded)
                  return get_computed_location(*loaded);
            }
            break;
      }
      return nullptr;
   }
}