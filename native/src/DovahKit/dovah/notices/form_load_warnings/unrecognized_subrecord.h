#pragma once
#include <cstdint>
#include <optional>
#include "../base_form_load_warning.h"

#include "../_util.define.h"
namespace dovah::notices::form_load_warnings {
   class unrecognized_subrecord final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr unrecognized_subrecord(form_stub& subject, uint32_t sig) : base_form_load_warning(subject), signature(sig) {}

         uint32_t signature = 0;
   };
}
#include "../_util.undef.h"