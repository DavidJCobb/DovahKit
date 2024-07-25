#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::race {
   class invalid_face_texture_sex : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr invalid_face_texture_sex(
            form_stub& stub,
            uint32_t   invalid_sex,
            form_stub* texture_set
         )
         :
            base_form_load_warning(stub),
            invalid_sex(invalid_sex),
            texture_set(texture_set)
         {}

         uint32_t   invalid_sex;
         form_stub* texture_set = nullptr;
   };
}
#include "../../../_util.undef.h"