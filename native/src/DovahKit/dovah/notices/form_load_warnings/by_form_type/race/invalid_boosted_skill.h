#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::race {
   class invalid_boosted_skill : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr invalid_boosted_skill(
            form_stub& stub,
            int8_t     skill,
            size_t     which
         )
         :
            base_form_load_warning(stub),
            skill(skill),
            which(which)
         {}

         size_t which; // which boost index
         int8_t skill;
   };
}
#include "../../../_util.undef.h"