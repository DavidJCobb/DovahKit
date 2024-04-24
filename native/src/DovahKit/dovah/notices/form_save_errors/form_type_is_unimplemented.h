#pragma once
#include <cstdint>
#include "../base_form_save_error.h"

#include "../_util.define.h"
namespace dovah::notices::form_save_errors {
   //
   // Saving this data is not yet implemented.
   //
   class form_type_is_unimplemented final : public base_form_save_error {
      public:
         MAKE_ERROR_OVERLOADS;
      public:
         constexpr form_type_is_unimplemented(form_stub& subject) : base_form_save_error(subject) {}
   };
}
#include "../_util.undef.h"