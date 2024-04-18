#pragma once
#include <cstdint>
#include <optional>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::landscape {
   //
   // A texture layer was targeted to an invalid quad.
   //
   class invalid_quad_for_land_texture : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         enum class texture_type {
            default_texture, // BTXT
            blended_texture, // ATXT
         };

      public:
         constexpr invalid_quad_for_land_texture(
            form_stub&   subject,
            texture_type type,
            uint8_t      q,
            std::optional<uint16_t> l = {}
         )
         :
            base_form_load_warning(subject),
            type(type),
            quad(q),
            layer(l)
         {}

         texture_type type;
         uint8_t      quad;
         std::optional<uint16_t> layer; // not present for default textures
   };
}
#include "../../../_util.undef.h"