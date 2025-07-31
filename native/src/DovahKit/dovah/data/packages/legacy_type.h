#pragma once
#include <array>
#include <cstdint>
#include <limits>
#include <optional>

namespace dovah::packages {
   enum class legacy_type : uint8_t {
      invalid = std::numeric_limits<uint8_t>::max(),
      
      find            =  0,
      follow          =  1,
      escort          =  2,
      eat             =  3,
      sleep           =  4,
      wander          =  5,
      travel          =  6,
      accompany       =  7,
      use_item_at     =  8,
      ambush          =  9,
      flee_non_combat = 10, // Actor flees without actually entering a combat state.
      use_magic       = 11,
      sandbox         = 12,
      patrol          = 13,
      guard           = 14,
      dialogue        = 15,
      use_weapon      = 16,
      find_deprecated = 17, // Remapped to 0 (find) on load.
      custom          = 18, // "Package"
      custom_template = 19, // "Package Template"

      //
      // Types beyond 19 exist and seem to be used internally, but they 
      // aren't recognized by the CK's loader.
      //
   };

   struct legacy_type_info {
      public:
         enum class have {
            no       = 0,
            yes      = 1,
            optional = 2,
            unknown  = 3,
         };

      public:
         legacy_type type;
         std::array<have, 2> has_location = { have::no, have::no };
         std::array<have, 2> has_target   = { have::no, have::no };
         std::optional<legacy_type> remap_on_load_to;
         bool is_modern = false;
   };
   inline constexpr const auto all_legacy_type_info = []() {
      using enum legacy_type_info::have;
      return std::array{
         legacy_type_info{
            .type = legacy_type::find,
            .has_location = { unknown, no },
            .has_target   = { yes,     no }
         },
         legacy_type_info{
            .type = legacy_type::follow,
            .has_location = { optional, optional },
            .has_target   = { unknown,  no }
         },
         legacy_type_info{
            .type = legacy_type::escort,
            .has_location = { yes, optional },
            .has_target   = { yes, no }
         },
         legacy_type_info{
            .type = legacy_type::eat,
            .has_location = { yes, optional },
            .has_target   = { yes, no }
         },
         legacy_type_info{
            .type = legacy_type::sleep,
            .has_location = { yes, no },
            .has_target   = { no,  no }
         },
         legacy_type_info{
            .type = legacy_type::wander,
            .has_location = { yes, no },
            .has_target   = { no,  no }
         },
         legacy_type_info{
            .type = legacy_type::travel,
            .has_location = { yes, no },
            .has_target   = { no,  no }
         },
         legacy_type_info{
            .type = legacy_type::accompany,
            .has_location = { no,  no },
            .has_target   = { yes, no }
         },
         legacy_type_info{
            .type = legacy_type::use_item_at,
            .has_location = { yes, yes },
            .has_target   = { yes, no }
         },
         legacy_type_info{
            .type = legacy_type::ambush,
            .has_location = { yes,      yes },
            .has_target   = { optional, no }
         },
         legacy_type_info{
            .type = legacy_type::flee_non_combat,
            .has_location = { unknown, no },
            .has_target   = { unknown, no }
         },
         legacy_type_info{
            .type = legacy_type::use_magic,
            .has_location = { unknown, no },
            .has_target   = { unknown, no }
         },
         legacy_type_info{
            .type = legacy_type::sandbox,
            .has_location = { yes, no },
            .has_target   = { no,  no }
         },
         legacy_type_info{
            .type = legacy_type::patrol,
            .has_location = { yes, no },
            .has_target   = { no,  no }
         },
         legacy_type_info{
            .type = legacy_type::guard,
            .has_location = { optional, no },
            .has_target   = { yes,      no }
         },
         legacy_type_info{
            .type = legacy_type::dialogue,
            .has_location = { optional, optional },
            .has_target   = { yes,      no }
         },
         legacy_type_info{
            .type = legacy_type::use_weapon,
            .has_location = { yes, optional },
            .has_target   = { yes, yes }
         },
         legacy_type_info{
            .type = legacy_type::find_deprecated,
            .has_location     = { unknown, no },
            .has_target       = { yes,     no },
            .remap_on_load_to = legacy_type::find,
         },
         legacy_type_info{
            .type = legacy_type::custom,
            .has_location = { no, no },
            .has_target   = { no, no },
            .is_modern    = true,
         },
         legacy_type_info{
            .type = legacy_type::custom_template,
            .has_location = { no, no },
            .has_target   = { no, no },
            .is_modern    = true,
         },
      };
   }();
   static_assert(
      []() -> bool {
         for (size_t i = 0; i < all_legacy_type_info.size(); ++i)
            if (all_legacy_type_info[i].type != (legacy_type)i)
               return false;
         return true;
      }(),
      "Legacy package type descriptors must be sequential and contiguous."
   );
}