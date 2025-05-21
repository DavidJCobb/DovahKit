#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::sound_category {
   //
   // A sound category specified itself as its parent category (PNAM).
   //
   class is_own_parent : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr is_own_parent(form_stub& subject) : base_form_load_warning(subject) {}
   };
}
#include "../../../_util.undef.h"