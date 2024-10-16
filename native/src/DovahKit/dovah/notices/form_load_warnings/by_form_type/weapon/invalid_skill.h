#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::weapon {
   class invalid_skill : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr invalid_skill(
            form_stub& stub,
            uint32_t   skill
         )
         :
            base_form_load_warning(stub),
            skill(skill)
         {}

         uint32_t skill;
   };
}
#include "../../../_util.undef.h"