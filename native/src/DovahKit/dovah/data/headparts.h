#pragma once
#include <cstdint>

namespace dovah {
   enum class head_part_type : uint32_t {
      misc,
      face,
      eyes,
      hair,
      facial_hair,
      scar,
      eyebrows,
   };

   // If `true`, then HeadParts of this type would be sorted into the "Base Head 
   // Parts" lists for ActorBases and Races: they are types that an actor should 
   // generally only have one of. If `false`, then HeadParts of this type would 
   // be sorted into the "Additional Head Parts" lists.
   constexpr const bool is_base_head_part_type(head_part_type v) {
      switch (v) {
         using enum head_part_type;
         case face:
         case eyes:
         case hair:
         case facial_hair:
         case eyebrows:
            return true;
      }
      return false;
   }
}
