#pragma once
#include "../base_form_save_error.h"

namespace dovah {
   class form_stub;
}

#include "../_util.define.h"
namespace dovah::notices::form_save_errors {
   //
   // There is a variable-length list somewhere in the form that relies on a 
   // serialized length. The number of items we want to write in is too large 
   // to fit in the bytecount allotted to that length (e.g. 256 items when the 
   // length is encoded as a single byte).
   //
   class length_prefixed_string_is_too_long_to_serialize : public base_form_save_error {
      public:
         MAKE_ERROR_OVERLOADS;
      public:
         constexpr length_prefixed_string_is_too_long_to_serialize(form_stub& s, size_t size, size_t max_serializable_size, uint32_t signature)
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
