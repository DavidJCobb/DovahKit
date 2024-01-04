#pragma once
#include <cstdint>

namespace dovah::loaded_forms::components::papyrus {
   //
   // Enumeration applied to bound property data in the VMAD subrecord. Similar to 
   // script status values, but discarded by the game and apparently only useful to 
   // the Creation Kit.
   // 
   // The CK displays the following strings for each status value:
   // 
   //    1: "Property edited locally" 
   //        (or, if the script has non-zero status, "Property inherited and edited locally")
   //    2: "Property inherited from parent"
   //    3: "Property inherited and cleared locally"
   // 
   // All other status values display "<unknown>". (Script status don't have this 
   // fallback.)
   //
   enum class property_status : uint8_t {
      unknown = 0, // default used by the game for VMAD subrecords that predate the creation of this enum (VMAD header < 4)
      //
      defined_locally       = 1,
      defined_only_on_base  = 2,
      inherited_and_removed = 3,
   };
}