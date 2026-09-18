#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::cell {
   //
   // If a cell is assigned to both a Location and an Encounter Zone, but the zone 
   // is also assigned to a Location, then the zone's Location takes priority over 
   // the cell's Location.
   //
   class location_ignored_if_zone_has_one : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr location_ignored_if_zone_has_one(
            form_stub& subject,
            form_stub& subject_location,
            form_stub& zone,
            form_stub& zone_location
         )
         :
            base_form_load_warning(subject),
            subject_location(&subject_location),
            zone(&zone),
            zone_location(&zone_location)
         {}

         form_stub* subject_location = nullptr;
         form_stub* zone             = nullptr;
         form_stub* zone_location    = nullptr;
   };
}
#include "../../../_util.undef.h"