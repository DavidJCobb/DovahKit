#pragma once
#include "NiIntegerExtraData.h"

namespace nifDK::block_types {
   class BSXFlags : public NiIntegerExtraData {
      public:
         static constexpr const char* const type_name = "BSXFlags";
      public:
         struct flag {
            enum type : uint32_t {
               animated                = 1 <<  0,
               has_havok               = 1 <<  1,
               ragdoll                 = 1 <<  2,
               complex                 = 1 <<  3,
               addon                   = 1 <<  4,
               is_editor_marker        = 1 <<  5,
               dynamic                 = 1 <<  6,
               articulated             = 1 <<  7,
               needs_transform_updates = 1 <<  8,
               external_emittance      = 1 <<  9,
               magic_shader_particles  = 1 << 10,
               lights                  = 1 << 11,
               breakable               = 1 << 12,
               searched_breakable      = 1 << 13, // run-time only?
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;
   };
}