#pragma once
#include <cstdint>
#include "../base_form_load_warning.h"
#include "dovah/data/magic_spell_type.h"

#include "../_util.define.h"
namespace dovah::notices::form_load_warnings {
   class illegal_magic_spell_type : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr illegal_magic_spell_type(
            form_stub& subject,
            magic_spell_type v
         ) :
            base_form_load_warning(subject),
            type(v)
         {}

         magic_spell_type type;
   };
}
#include "../_util.undef.h"