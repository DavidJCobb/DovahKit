#pragma once
#include <cstdint>
#include <optional>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::head_part {
   class invalid_morph_type : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         enum class texture_type {
            default_texture, // BTXT
            blended_texture, // ATXT
         };

      public:
         constexpr invalid_morph_type(
            form_stub& subject,
            uint32_t   morph_type
         )
         :
            base_form_load_warning(subject),
            morph_type(morph_type)
         {}

         uint32_t morph_type;
   };
}
#include "../../../_util.undef.h"