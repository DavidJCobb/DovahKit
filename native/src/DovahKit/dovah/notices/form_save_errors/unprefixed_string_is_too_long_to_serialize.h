#pragma once
#include "../base_form_save_error.h"

namespace dovah {
   class form_stub;
}

#include "../_util.define.h"
namespace dovah::notices::form_save_errors {
   //
   // There is a string somewhere in the form that gets written to a fixed-size 
   // buffer. The length of that string is too long to fit in the buffer.
   //
   class unprefixed_string_is_too_long_to_serialize : public base_form_save_error {
      public:
         MAKE_ERROR_OVERLOADS;
      public:
         constexpr unprefixed_string_is_too_long_to_serialize(form_stub& s, size_t size, size_t max_serializable_size, uint32_t signature)
         :
            base_form_save_error(s),
            size(size),
            max_serializable_size(max_serializable_size),
            subrecord_signature(signature)
         {}

         size_t size;
         size_t max_serializable_size;

         uint32_t subrecord_signature = 0;
   };
}
#include "../_util.undef.h"
