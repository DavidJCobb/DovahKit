#pragma once
#include "lu/integral_character_code.h"
#include "../base_form_load_warning.h"

#include "dovah/notices/_boilerplate/base_warning.define.h"
namespace dovah::notices::form_load_warnings {
   class unrecognized_subrecord final : public base_form_load_warning {
      public:
         DEFINE_NOTICE_BOILERPLATE;
      public:
         constexpr unrecognized_subrecord(
            form_stub& subject,
            lu::four_cc sig
         ) :
            base_form_load_warning(subject),
            signature(sig)
         {}

         lu::four_cc signature = 0;
   };
}
#include "dovah/notices/_boilerplate/base_warning.undef.h"